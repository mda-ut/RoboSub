// This is the motor_controller for each H-bridge circuit
// Direction and on/off can be specified, as well as duty cycle

`include "mda_motor_control_defines.v"


module mda_motor_control (input clk, input on, input [15:0] period, input [15:0] duty_cycle, output [3:0] out);
	
	wire motor_on, motor_dir;
	
	mda_motor_control_pwm_gen pwm_generator (clk, on, period, duty_cycle, motor_dir, motor_on);
	mda_motor_control_internal mi(clk, motor_dir, motor_on, out);

endmodule

/*
// The following is the old motor_controller decommissioned Mar. 26 2016.
// 	This old code seems to be inefficient, our motors will not reach full potential
//		if we keep flipping between directions.
// clk: clock signal for sequential processes
// dir: 
// on:
// period: defines how long 1 pwm period is. Max is 2^16-1
// duty_cycle:	defines the pwm. This looks like an abs value instead of the normal %duty_cycle. % = duty_cycle.period*100
// out:
module OLD_motor_controller (input clk, input dir, input on, input [15:0] period, input [15:0] duty_cycle, output [3:0] out);

  reg [`PERIOD_LENGTH-1:0] duty_counter = 0;
  reg dir_reg;

  // flips the dir of the H-bridge for some duration of each period, defined
  // by the duty_cycle. Duty_counter is used as a counter for each period.
  // Updated on posedge so each duty_counter increment correspond to 1 cycle
  // of the clock which is (for example) 1/16MHz of a second.
  always @(posedge clk)
  begin
    if (duty_counter == period)
      duty_counter <= 0;
    else
      duty_counter <= duty_counter + 1;
    dir_reg <= (on && duty_counter < duty_cycle) ? ~dir : dir;		// direction is flipping
  end

  // Instantiate module. See module below
  motor_internal mi(clk, dir_reg, on, out);

endmodule
*/
