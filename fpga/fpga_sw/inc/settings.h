// Set some #define values to use as settings
#ifndef SETTINGS_H
#define SETTINGS_H
// allow the user to interact when using commands
#define INTERACTIVE

// constants
#define CLOCK_SPEED 50000000
#define TIMER_RATE_IN_HZ 20
#define NUM_DEPTH_VALUES 64 /* Ensure this is a power of 2 for better performance */

#define NUM_MOTORS 8 /* Number of possible motor board terminals, not number of motors actually under use */
#define STR_LEN 127
#define RAD_TO_DEG 57.3  /* 180/PI */
#define MDA_MOTOR_CONTROL_DUTY_CYCLE (MDA_MOTOR_CONTROL_BASE + 32)

#define ZERO_PWM 512
#define FULL_PWM (2*ZERO_PWM)

// Limits
#define PWM_LIMIT_FACTOR 0.8
#define MAX_PWM (PWM_LIMIT_FACTOR * FULL_PWM)
#define MIN_PWM ((1-PWM_LIMIT_FACTOR) * FULL_PWM)

#define FORCE_REDUCTION_FACTOR 0.85

#ifndef INTERACTIVE
#undef puts
#undef printf
#undef putchar
#else
#include <stdio.h>
#endif

/** Ritchie's PID controller settings. 
 * These are multiplicative constants in the PID equation
 * Alpha is the decay rate for Integral term
 */

#define ROLL_CONST_P 18.0
#define ROLL_CONST_I 0.0 /* NOT CALIBRATED */
#define ROLL_CONST_D 0.0 /* NOT CALIBRATED */
#define ROLL_ALPHA 0.0
#define PITCH_CONST_P 24.0
#define PITCH_CONST_I 0.0 /* NOT CALIBRATED */
#define PITCH_CONST_D 0.0 /* NOT CALIBRATED */
#define PITCH_ALPHA 0.0
#define YAW_CONST_P 12.0
#define YAW_CONST_I 0.0 /* NOT CALIBRATED */
#define YAW_CONST_D 0.0 /* NOT CALIBRATED */
#define YAW_ALPHA 0.0
#define DEPTH_CONST_P 5.0
#define DEPTH_CONST_I 0.000
#define DEPTH_CONST_D 0 //1
#define DEPTH_ALPHA 0.0

/** The following constants define the scaling between PID controller outputs
 *  and the motor force. Say the PID reads a string of angles/depth values and spits
 *  out "30". This constant converts this relative control factor to relative motor force.
 *  We should try to keep the force around -100 to 100 or something
 */
#define FACTOR_PID_ROLL_TO_FORCE 1
#define FACTOR_PID_PITCH_TO_FORCE 1
#define FACTOR_PID_YAW_TO_FORCE 1
#define FACTOR_PID_DEPTH_TO_FORCE 1
#define FACTOR_SPEED_TO_FORCE 5

/** This converts the controller force into lbs, so that it can be mapped to a PWM */
#define FACTOR_CONTROLLER_FORCE_TO_LBS 0.005

/** The following defines which motor is mapped to which terminal on the motor board */
// NOTE: Motor #s match the hardware motor numbering, the code still indexes motors by 0-7 indexing
// Also, make sure motor mapping are 1-for-1, even if a motor is not enabled
// motors parallel to sub
#define M_FRONT_LEFT_TERMINAL 4
#define M_FRONT_RIGHT_TERMINAL 5
#define M_BACK_LEFT_TERMINAL 6
#define M_BACK_RIGHT_TERMINAL 7
// motors perpendicular to sub
#define MP_FRONT_LEFT_TERMINAL 0
#define MP_FRONT_RIGHT_TERMINAL 1
#define MP_BACK_LEFT_TERMINAL 3
#define MP_BACK_RIGHT_TERMINAL 2

/** The following defines which motors are enabled, 0=false, 1=true */
// motors parallel to sub
#define M_FRONT_LEFT_ENABLE 1
#define M_FRONT_RIGHT_ENABLE 1
#define M_BACK_LEFT_ENABLE 0 // not used for MDA_TEMPEST
#define M_BACK_RIGHT_ENABLE 0 // not used for MDA_TEMPEST
// motors perpendicular to sub
#define MP_FRONT_LEFT_ENABLE 1
#define MP_FRONT_RIGHT_ENABLE 1
#define MP_BACK_LEFT_ENABLE 1
#define MP_BACK_RIGHT_ENABLE 1

enum terminalEnum { M_FRONT_LEFT, M_FRONT_RIGHT, M_BACK_LEFT, M_BACK_RIGHT, 
MP_FRONT_LEFT, MP_FRONT_RIGHT, MP_BACK_LEFT, MP_BACK_RIGHT }; 
<<<<<<< HEAD
=======

#endif //SETTINGS_H
>>>>>>> 12bf98d1911d29632f0eca907ff8493e62c22297
