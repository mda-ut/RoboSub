/////////////////////////////////////////////////////////////////////////
// Module: mda_motor_control_defines.v
// i---------------------------------------------------------------------
// Purpose: Contains the defines needed for the mda_motor_control module
//			and its submodules
// ---------------------------------------------------------------------
// Version History:
//
// 2016/5/22 - 1.0: Copied legacy verilog and made modifications for Robosub
//					2016 competition. Using new documentation convention.
// Shai Bonen
//////////////////////////////////////////////////////////////////////////

// This file defines the parameters for the motor controller

// # of dead time cycles to ensure H-bridge does not short
// 50 cycles = 1 microseconds
`define DEAD_TIME 0

//	for the pwm, defines the register size for period and duty_cycles
// eg. period[PERIOD_LENGTH-1:0]; 
`define PERIOD_LENGTH 16
