// This file defines the parameters for the motor controller

// # of dead time cycles to ensure H-bridge does not short
// 75 cycles = 1.5 microseconds
`define DEAD_TIME 75

//	for the pwm, defines the register size for period and duty_cycles
// eg. period[PERIOD_LENGTH-1:0]; << from motor_controller.v
`define PERIOD_LENGTH 16
