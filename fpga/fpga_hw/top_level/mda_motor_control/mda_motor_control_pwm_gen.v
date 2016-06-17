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

module mda_motor_control_pwm_gen (input clk, input on, input [15:0] period, input [15:0] duty_cycle, output reg dir_reg, on_reg);
//	clk: clk for sequential logic
//	on: whether the motor is on (1'b1) or off (1'b0)
// 	period: defines how long 1 pwm period is. Max is 2^16-1
// 	duty_cycle:	defines the pwm. It is an abs value representing the number of on cycles relative to the 50% cycle,
//				instead of the normal %duty_cycle.
// 						-> Thus:	 %duty_cycle = duty_cycle/period*100
//						-> From c code, 50% is brake, >50% is forward, <50% is reverse, effectly CENTERED AT 50%
//	dir_reg: direction register
//	on_reg: motor on/off
//
// States table						on		duty_cycle/period		behavior	 dir_reg	on_reg
// During pwm on time: 
// (period_counter < corrected_duty_cycle)
// 										0			any					drift			0			0
//											1			50%					brake			1			0
//											1			>50%				 forward			1			1
//											1			<50%				 reverse			0			1
// During pwm off time:
// (period_counter > corrected_duty_cycle)
// 										0			any					drift			0			0
//											1			50%					brake			1			0
//											1			>50%					brake			1			0
//											1			<50%					brake			1			0
//
//	To get most potential difference, we will pwm between the forward or reverse states and the brake state.

   // Counter Variable Declaration
   reg [`PERIOD_LENGTH-1:0] period_counter = 0;

   // Half Period Reference Declaration
   wire [`PERIOD_LENGTH-1:0] half_period = period >> 1; 

   // duty_cycle CENTERED AT ZERO. Eg. if orginal cycle >%50 percent, new cycle = (org_cycle-cycle_at_half)x2.
   // the result is multiplied by 2 to scale up to the period.
   wire [`PERIOD_LENGTH-1:0] corrected_duty_cycle = (duty_cycle>=half_period) ? (duty_cycle-half_period)<<1 : (half_period-duty_cycle)<<1;

   // This variable will be set to dir_reg under pwm on state.
   wire direction = (duty_cycle>=half_period && on)? 1'b1 : 1'b0;
	
	
   // Updated on posedge so each period_counter increment correspond to 1 cycle
	// of the clock which is (for example) 1/16MHz of a second.
	always @(posedge clk)
	begin
		// reset counter every period
		if (period_counter == period)  period_counter <= 16'b0;
		// increment period counter
		else  period_counter <= period_counter + 16'b1;

		// sets the dir_reg: pwm_on_time -> direction, pwm_off_time -> on state (we want to brake in on state and drift in off)
		dir_reg <= (period_counter < corrected_duty_cycle) ? direction : ((on) ? 1'b1 : 1'b0); // Shai's Edit: 
		// sets the on_reg: pwm_on_time -> on input and we are not set to 50% Duty_Cycle, pwm_off_time -> 0 (b/c we are off)
		on_reg <= (period_counter < corrected_duty_cycle) ? on&&(corrected_duty_cycle!=half_period) : 1'b0;
  end

endmodule
