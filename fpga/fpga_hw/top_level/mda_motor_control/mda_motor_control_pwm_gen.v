
`include "mda_motor_control_defines.v"

//	motor_controller, modified from OLD_motor_controller Mar. 26 2016 ahsueh1996
//		This module determines pwm signals controlling the 4 transitors of 1 H-bridge (which controls 1 motor)
//
// 	clk: clock signal for sequential processes
// 	on: on (HIGH) or off
// 	period: defines how long 1 pwm period is. Max is 2^16-1
// 	duty_cycle:	defines the pwm. It is an abs value instead of the normal %duty_cycle.
// 		Thus		 %duty_cycle = duty_cycle.period*100
//			From c code, 50% is no go, >50% is forward, <50% is reverse, effectly CENTERED AT 50%
// 	out: output to the motor H-bridges
//
// States table	on			duty_cycle		on/off pwm	|	behavior	 	on		dir_reg
// 					0			any					on				drift			0		0
//						1			50%					on				brake			0		1
//						1			>50%					on				forward		1		1
//						1			<50%					on				reverse		1		0
//
// 					0			any					off			drift			0		0
//						1			50%					off			brake			0		1
//						1			>50%					off			brake			0		1
//						1			<50%					off			brake			0		1
//
//	To get most potential difference, we will pwm between the forward or reverse states and the brake state.
module mda_motor_control_pwm_gen (input clk, input on, input [15:0] period, input [15:0] duty_cycle, output reg dir_reg, on_reg);

  reg 	[`PERIOD_LENGTH-1:0] duty_counter = 0;

  // duty_cycle corresponding to 50% of the period.
  // Note the integer division for duty_cycle_at_half. This implies that period should be even or that when passing duty_cycle
  // you'll have to make sure you also integer divide (eg. floor of 9/2 = 1001>>1 = 4).
  wire [`PERIOD_LENGTH-1:0] duty_cycle_at_half = period >> 1; 
  // duty_cycle CENTERED AT ZERO. Eg. if orginal cycle >%50 percent, new cycle = (org_cycle-cycle_at_half)x2.
  // the result is multiplied by 2 to scale up to the period.
  wire [`PERIOD_LENGTH-1:0] corrected_duty_cycle = (duty_cycle>=duty_cycle_at_half)? (duty_cycle-duty_cycle_at_half)<<1 : (duty_cycle_at_half-duty_cycle)<<1;
  // want to call this _dir... but there is conflict in naming.
  // This variable will be set to dir_reg under pwm on state.
  wire 	_dir = (duty_cycle>=duty_cycle_at_half && on)? 1'b1 : 1'b0;

  
  // Updated on posedge so each duty_counter increment correspond to 1 cycle
  // of the clock which is (for example) 1/16MHz of a second.
  always @(posedge clk)
  begin
	if (duty_counter == period)
      duty_counter <= 16'b0;
	else
      duty_counter <= duty_counter + 16'b1;
		
	// sets the dir_reg... pwm off : pwm on
	dir_reg <= (duty_counter < corrected_duty_cycle) ? on : _dir;
	// sets the on_reg... pwm off : pwm on 
	on_reg <= (duty_counter < corrected_duty_cycle) ? 1'b0 : on&&(corrected_duty_cycle!=duty_cycle_at_half);
  end

  // Instantiate module. See module below
  //motor_internal mi(clk, dir_reg, on_reg, out);

endmodule