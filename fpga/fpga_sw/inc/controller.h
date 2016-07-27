// Function prototypes implemented in controller.c

#ifndef _MDA_CONTROLLER_H
#define _MDA_CONTROLLER_H

#include <stdbool.h>
#include "alt_types.h"
#include "settings.h"

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

/* struct which keeps data about which motor maps to which terminal on the motor board
 * works together with motor_duty_cycle array to deliver pwms to motor board
 */
struct motor_terminal_connections {
  // terminal on the motor board, possible value from 1 to 8
  // Indexing for this array corresponds to terminalEnum in settings.h
  // Value @ Index corresponds to which H-Bridge on the motor boards (terminal) that motor enumeration corresponds to
  int terminals[NUM_MOTORS];
  // whether the motor is enabled, 0 false 1 true
  // Indexing for this array corresponds to terminalEnum in settings.h
  // Value @ Index corresponds to whether that motor has been enabled in settings.h
  bool isEnabled[NUM_MOTORS];
};

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
