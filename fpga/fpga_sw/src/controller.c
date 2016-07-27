/*
 * controller.c
 *
 * The submarine's motor controller. The motor controller is implemented as several PID controllers.
 * Yaw, pitch, roll and depth have separate controllers.
 *
 * The controllers work by setting a target attitude (each controller is separate), and the PID controller
 * will aim to maintain the target attitude. The PID controllers then consolidate together to set the motor PWM values.
 *
 * calculate_pid() is expected to be called at a constant interval, driven by a timer. update_depth_reading() is expected
 * to be called more often to perform depth-averaging.
 *
 * Author: Ritchie
 */

#include <math.h>

#include "alt_types.h"
#include "io.h"
#include "system.h"
#include "sys/alt_stdio.h"

#include "controller.h"
#include "pid.h"
#include "pwm_force.h"
#include "rs232.h"
#include "utils.h"

#define MAX_YAW 360

#define PRIORITIZE_PITCH_OVER_DEPTH
int superDuperCounter = 0;

// Structures used by the PD controller for stabilization
struct orientation target_orientation = {};
struct orientation current_orientation = {};

// outline which motors are hooked up to which terminals, allow for maximum of eight motors
struct motor_terminal_connections hooks = 
{
  /* terminal[NUM_MOTORS] */ {M_FRONT_LEFT_TERMINAL, M_FRONT_RIGHT_TERMINAL, M_BACK_LEFT_TERMINAL, M_BACK_RIGHT_TERMINAL, 
			MP_FRONT_LEFT_TERMINAL, MP_FRONT_RIGHT_TERMINAL, MP_BACK_LEFT_TERMINAL, MP_BACK_RIGHT_TERMINAL},

  /* isEnabled[NUM_MOTORS] */ {M_FRONT_LEFT_ENABLE, M_FRONT_RIGHT_ENABLE, M_BACK_LEFT_ENABLE, M_BACK_RIGHT_ENABLE, 
			MP_FRONT_LEFT_ENABLE, MP_FRONT_RIGHT_ENABLE, MP_BACK_LEFT_ENABLE, MP_BACK_RIGHT_ENABLE},
};

// Data to average depth readings
int depth_values[NUM_DEPTH_VALUES] = {};
int depth_sum = 0;

// Motor duty cycles, indices from 0 to NUM_MOTORS-1 represent motor board terminals from 1 to NUM_MOTORS
static int motor_duty_cycle[NUM_MOTORS];


// Set target values for the controller

void set_target_speed(int speed)
{
   target_orientation.speed = speed;
}

void set_target_heading(int heading)
{
   target_orientation.yaw = heading;
}

void set_target_depth(int depth)
{
   target_orientation.depth = depth;
}

// Update depth reading by tracking the sum
// of the last NUM_DEPTH_VALUES readings
void update_depth_reading()
{
   static unsigned idx = 0;

   depth_sum -= depth_values[idx];
   int depth_reading = get_depth();
   depth_values[idx] = depth_reading;
   depth_sum += depth_reading;

   // Rotate idx
   idx++;
   idx %= NUM_DEPTH_VALUES;
}

// Get an average depth of the last NUM_DEPTH_VALUES readings
int get_average_depth()
{
   return depth_sum / NUM_DEPTH_VALUES;
}

// Get orientation from acceleration
void get_orientation(struct t_accel_data *accel_data, struct orientation *orientation)
{
  // Calculation orientation
  int z_squared = accel_data->z*accel_data->z, y_squared = accel_data->y*accel_data->y;

  // pitch and roll are zero when sub is "flat"
  orientation->pitch = (z_squared + y_squared == 0) ? 90 : atan2(accel_data->x,sqrt(z_squared + y_squared)) * RAD_TO_DEG;
  orientation->roll = (z_squared == 0) ? 90 : atan2(accel_data->y,sqrt(2*z_squared)) * RAD_TO_DEG;
}

// The PID controller!

static Controller_PID PID_Roll; // 4 controllers for controlling each DOF we care about
static Controller_PID PID_Pitch;
static Controller_PID PID_Yaw;
static Controller_PID PID_Depth;

// Set constants, given PID controller
void set_pid_constants(double P, double I, double D, double Alpha, Controller_PID *pid)
{
    PID_Reset(pid);
    pid->Const_P = P;
    pid->Const_I = I;
    pid->Const_D = D;
    pid->Alpha = Alpha;
}

void pid_init () // call this anytime before calling calculate_pid
{
    // set up PID values
    set_pid_constants_pitch(PITCH_CONST_P, PITCH_CONST_I, PITCH_CONST_D, PITCH_ALPHA);
    set_pid_constants_roll(ROLL_CONST_P, ROLL_CONST_I, ROLL_CONST_D, ROLL_ALPHA);
    set_pid_constants_yaw(YAW_CONST_P, YAW_CONST_I, YAW_CONST_D, YAW_ALPHA);
    set_pid_constants_depth(DEPTH_CONST_P, DEPTH_CONST_I, DEPTH_CONST_D, DEPTH_ALPHA);

    // Initialize target orientation
    set_target_depth(0);
    set_target_speed(0);
    set_target_heading(0);

    // Initialize motor linearization lookup table
    init_lookup();
}

void set_pid_constants_pitch(double P, double I, double D, double Alpha)
{
    set_pid_constants(P, I, D, Alpha, &PID_Pitch);
}

void set_pid_constants_roll(double P, double I, double D, double Alpha)
{
    set_pid_constants(P, I, D, Alpha, &PID_Roll);
}

void set_pid_constants_yaw(double P, double I, double D, double Alpha)
{
    set_pid_constants(P, I, D, Alpha, &PID_Yaw);
}

void set_pid_constants_depth(double P, double I, double D, double Alpha)
{
    set_pid_constants(P, I, D, Alpha, &PID_Depth);
}

double motor_force_to_pwm (double force) {
    int pwm = ZERO_PWM + pwm_of_force(force*FACTOR_CONTROLLER_FORCE_TO_LBS);
    return pwm;
}

// the following function calculates the pwm needed for 2 or 3 stabilizing motors. 
// It ensures motors are balanced and no motor exceeds 0.8*FULL_PWM by reducing 
// the force if any motor pwm does exceed the limit.
#define MAX_FORCE_BALANCING_LOOPS 10
bool stabilizing_motors_force_to_pwm (
        double f_0, double f_1, double f_2, double f_3,
        double *m_0, double *m_1, double *m_2, double *m_3
)
{
    unsigned loops;
    bool out_of_bound = false;

    for (loops = 0; loops < MAX_FORCE_BALANCING_LOOPS; loops++) {
        // calculate the pwms
        *m_0 = motor_force_to_pwm(f_0);
        *m_1 = motor_force_to_pwm(f_1);
        *m_2 = motor_force_to_pwm(f_2);
        *m_3 = motor_force_to_pwm(f_3);
        

        double pwm_limit = MAX_PWM;

        // if any pwm is out of bound
        if (ABS(*m_0) > pwm_limit ||
            ABS(*m_1) > pwm_limit ||
            ABS(*m_2) > pwm_limit ||
            ABS(*m_3) > pwm_limit)
        {
            // reduce force
            f_0 *= FORCE_REDUCTION_FACTOR;
            f_1 *= FORCE_REDUCTION_FACTOR;
            f_2 *= FORCE_REDUCTION_FACTOR;
            f_3 *= FORCE_REDUCTION_FACTOR;
            out_of_bound = true;
        }
        else {
            break;
        }
    }
    return out_of_bound;
}

void calculate_pid()
{
   // Get orientation data from IMU and depth from depth sensor
   get_imu_orientation(&current_orientation);
   current_orientation.depth = get_average_depth();
   
   /* current_orientation.pitch and .roll are controlled towards zero
    * and .depth is controlled towards target_orientation.depth
    */
   
   // update the controller with the new pitch and roll values
   PID_Update (&PID_Roll, current_orientation.roll);
   PID_Update (&PID_Pitch, current_orientation.pitch);

   // normalize yaw difference between -180 and 180 degrees
   double delta_yaw = (target_orientation.yaw - current_orientation.yaw);
   delta_yaw = fmod(delta_yaw, MAX_YAW);
   if (delta_yaw > MAX_YAW/2) {
     delta_yaw -= MAX_YAW;
   } else if (delta_yaw < -MAX_YAW/2) {
     delta_yaw += MAX_YAW;
   }
   PID_Update (&PID_Yaw, delta_yaw);

   // calculate depth difference
   double delta_depth = (target_orientation.depth - current_orientation.depth);
   PID_Update (&PID_Depth, delta_depth);
   
   // calculate the force required
   double Roll_Force_Needed = FACTOR_PID_ROLL_TO_FORCE * PID_Output(&PID_Roll);
   double Pitch_Force_Needed = FACTOR_PID_PITCH_TO_FORCE * PID_Output(&PID_Pitch);
   double Depth_Force_Needed = FACTOR_PID_DEPTH_TO_FORCE * PID_Output(&PID_Depth);
   double Yaw_Force_Needed = FACTOR_PID_YAW_TO_FORCE * PID_Output(&PID_Yaw);
   double Forward_Force_Needed = FACTOR_SPEED_TO_FORCE * target_orientation.speed;

   // some print statements to diagnose PID outputs
if(superDuperCounter%100 == 0){
   printf("Roll PID: %f, Roll force needed: %f \n", Roll_Force_Needed/FACTOR_PID_ROLL_TO_FORCE, Roll_Force_Needed);
   printf("Pitch PID: %f, Pitch force needed: %f \n", Pitch_Force_Needed/FACTOR_PID_PITCH_TO_FORCE, Pitch_Force_Needed);
   printf("Depth PID: %f, Depth force needed: %f \n", Depth_Force_Needed/FACTOR_PID_DEPTH_TO_FORCE, Depth_Force_Needed);
   printf("Yaw PID: %f, Yaw force needed: %f \n", Yaw_Force_Needed/FACTOR_PID_YAW_TO_FORCE, Yaw_Force_Needed);
}
   /** orientation stability
    *  If the COM is off center we would have some sort of factors here instead of 0.5
    */

   // find the pwm needed for each motor
   double m_pwm[NUM_MOTORS];
   //subgroup A: m_front_left, m_front_right // back left and back right are not used for MDA_TEMPEST
   //subgroup B: mp_front_left, mp_front_right, mp_back_left, mp_back_right //mp = motor perpendicular
   //correspond motors with enumerations mentioned in settings.h
   
   stabilizing_motors_force_to_pwm ( // this calculates the pwms for yaw motors
				    // These are actually switched for SubZero
      0.5*Yaw_Force_Needed + Forward_Force_Needed, // m_left //we might change hard-coded 0.5 ratio later
      -0.5*Yaw_Force_Needed + Forward_Force_Needed, // m_right
      0, 0, // since back left and back right are not used
	&m_pwm[M_FRONT_LEFT], //polarity of motors: + pointed towards front of sub
	&m_pwm[M_FRONT_RIGHT], //positive yaw is right turn
      	NULL, //&m_back_left //positive foward is forward
      	NULL //&m_back_right // pwm greater than 512 is forward
   );
#ifdef PRIORITIZE_PITCH_OVER_DEPTH
   if (ABS(current_orientation.pitch) > 30) {
      Depth_Force_Needed *= 0.8;
   }
   else if (ABS(current_orientation.pitch) > 35) {
      Depth_Force_Needed *= 0.6;
   }
   else if (ABS(current_orientation.pitch) > 40) {
      Depth_Force_Needed *= 0.3;
   }
#endif
   stabilizing_motors_force_to_pwm ( // this calculates the pwms for pitch and roll motors
      //0.5*Roll_Force_Needed + 0.2*Pitch_Force_Needed + 0.2*Depth_Force_Needed, // m_front_left
      //-0.5*Roll_Force_Needed + 0.2*Pitch_Force_Needed + 0.2*Depth_Force_Needed, // m_front_right
      //-0.5*Pitch_Force_Needed + 0.4*Depth_Force_Needed, // m_rear
				    -0.25*Roll_Force_Needed - 0.2*Pitch_Force_Needed - 0.2*Depth_Force_Needed, // mp_front_left
				    +0.25*Roll_Force_Needed - 0.2*Pitch_Force_Needed - 0.2*Depth_Force_Needed, // mp_front_right
				    -0.25*Roll_Force_Needed + 0.2*Pitch_Force_Needed - 0.2*Depth_Force_Needed, // mp_back_left
				    +0.25*Roll_Force_Needed + 0.2*Pitch_Force_Needed - 0.2*Depth_Force_Needed, // mp_back_right
	&m_pwm[MP_FRONT_LEFT], //assumptions: polarity of motors: + pointed towards water surface
      	&m_pwm[MP_FRONT_RIGHT], //positive roll is left side sinking right side rising
      	&m_pwm[MP_BACK_LEFT], //positive depth is deeper
      	&m_pwm[MP_BACK_RIGHT] //positive pitch is surface
   );

  // use the hooks mappings to map motor duty cycles to respective motors
  // the -1 is there since motor_duty_cycle 0~7 map to terminals 1~8 on motor board

   int i;
   for (i = 0; i < NUM_MOTORS; i++){
     if(hooks.isEnabled[i]){
        motor_duty_cycle[ hooks.terminals[i] ] = (int)m_pwm[i];
	if(superDuperCounter%100 == 0)	
	printf("motor number: %d, connected to terminal number: %d, with pwm %d \n", i, hooks.terminals[i], (int)m_pwm[i]);
     } else {
	motor_duty_cycle[ hooks.terminals[i] ] = 0;
	if(superDuperCounter%100 == 0)	
	printf("motor number: %d, connected to terminal number: %d, disabled \n", i, hooks.terminals[i]);
     }
   }
   /** Note that motor_force_to_pwm returns a value between -400 and 400, and the factors are such that the sum of
    *  each factor for every motor adds up (absolutely) to 1.0. Physics son! 
    */
   
   // write the motor settings
   for ( i = 0; i < NUM_MOTORS; i++ )
   {
      set_motor_duty_cycle(i, motor_duty_cycle[i]);  
   }
superDuperCounter++;  
}

void print_debug_controller()
{
   double Depth_Force_Needed = FACTOR_PID_DEPTH_TO_FORCE * PID_Output(&PID_Depth);

   // Print controller debug messages
   printf ("PID_Depth.P = %f\n", PID_Depth.P*PID_Depth.Const_P);
   printf ("PID_Depth.I = %f\n", PID_Depth.I*PID_Depth.Const_I);
   printf ("PID_Depth.D = %f\n", PID_Depth.D*PID_Depth.Const_D);
   printf ("Depth_PID: %f\n", Depth_Force_Needed);
}
