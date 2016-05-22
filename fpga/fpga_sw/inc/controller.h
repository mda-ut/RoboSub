// Function prototypes implemented in controller.c

#ifndef _MDA_CONTROLLER_H
#define _MDA_CONTROLLER_H

#include <stdbool.h>
#include "alt_types.h"

#define ABS(x) (((x) > 0) ? (x) : (-(x)))

/* acceleration data in x,y,z */
struct t_accel_data {
  alt_16 x, y, z;
};

struct orientation {
  // angles
  int yaw;
  int pitch;
  int roll;

  // scalars
  int speed;
  int depth;
};

// Motor 0 = front left horizontal
// Motor 1 = front right horizontal
// Motor 2 = back left horizontal
// Motor 3 = back right horizontal
// Motor 4 = front left vertical
// Motor 5 = front right vertical
// Motor 6 = back left vertical
// Motor 7 = back right vertical
/*#define M_FRONT_LEFT motor_duty_cycle[0]
#define M_FRONT_RIGHT motor_duty_cycle[1]
#define M_LEFT motor_duty_cycle[2]
#define M_RIGHT motor_duty_cycle[3]
#define M_REAR motor_duty_cycle[4]*/

#define M_FRONT_LEFT motor_duty_cycle[0]
#define M_FRONT_RIGHT motor_duty_cycle[1]
#define M_BACK_LEFT motor_duty_cycle[2]
#define M_BACK_RIGHT motor_duty_cycle[3]
#define MP_FRONT_LEFT motor_duty_cycle[4]
#define MP_FRONT_RIGHT motor_duty_cycle[5]
#define MP_BACK_LEFT motor_duty_cycle[6]
#define MP_BACK_RIGHT motor_duty_cycle[7]

void set_target_speed(int speed);
void set_target_heading(int heading);
void set_target_depth(int depth);

// the following function calculates the pwm needed for 4 stabilizing motors. 
// It ensures motors are balanced and no motor exceeds 0.8*FULL_PWM by reducing 
// the force if any motor pwm does exceed the limit.
// It also ensures no more than 4 motors run simultaneously
#define MAX_FORCE_BALANCING_LOOPS 10
bool stabilizing_motors_force_to_pwm (
        double f_0, double f_1, double f_2, double f_3,
        double *m_0, double *m_1, double *m_2, double *m_3
				      );

void get_orientation(struct t_accel_data *accel_data, struct orientation *orientation);

void update_depth_reading();
int get_average_depth();
void calculate_pid();
void controller_output(int pitch_setting, int roll_setting, int heading, int speed, int depth_setting);

void print_debug_controller();

void pid_init();
void set_pid_constants_pitch(double P, double I, double D, double Alpha);
void set_pid_constants_roll(double P, double I, double D, double Alpha);
void set_pid_constants_yaw(double P, double I, double D, double Alpha);
void set_pid_constants_depth(double P, double I, double D, double Alpha);

#endif
