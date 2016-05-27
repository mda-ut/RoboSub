/////////////////////////////////////////////////////////////////////////
// Module: mda_motor_control_pwm_gen.v
// i---------------------------------------------------------------------
// Purpose: Module generates pwm signals based on the specified duty cycle
//			and motor state.
// ---------------------------------------------------------------------
// Version History:
//
// 2016/3/26 - 1.0: Modified the old motor_controller. This module determines
//					pwm signals controlling the 4 transistors of 1 H-Bridge.
// Albert Hsueh
//
// 2016/5/22 - 1.1: Copied legacy verilog and made modifications for Robosub
//					2016 competition. Using new documentation convention.
//					Updated variable names and comments to be more clear.
// Shai Bonen
//////////////////////////////////////////////////////////////////////////

`include "mda_motor_control_defines.v"

module mda_motor_control_pwm_gen (input clk, input dir, input on, input [15:0] period, input [15:0] duty_cycle, output reg dir_reg);

  reg [`PERIOD_LENGTH-1:0] duty_counter = 0;

  always @(posedge clk)
  begin
    if (duty_counter == period)
      duty_counter <= 0;
    else
      duty_counter <= duty_counter + 1;
    dir_reg <= (on && duty_counter < duty_cycle) ? ~dir : dir;
  end

endmodule
