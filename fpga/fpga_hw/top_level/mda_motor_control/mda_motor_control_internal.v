/////////////////////////////////////////////////////////////////////////
// Module: mda_motor_control_internal.v
// i---------------------------------------------------------------------
// Purpose: Module defines the motor controller logics.
// ---------------------------------------------------------------------
// Version History:
//
// 2016/5/22 - 1.0: Copied legacy verilog and made modifications for Robosub
//					2016 competition. Using new documentation convention.
// Shai Bonen1
// 2016/6/19 - 2.0: Revised for usage with L298 dual motor board.
// 					out_reg will now have pwm,forward,reverse,ena. Added
//					the input pwm. This param is whether pwm is in on time
//					or off time.
// Albert Hsueh
//
// 2016/6/21 - 2.1: Because of dh2 breakage, reduce out_reg to just use pwm, forward and reverse.
//					But really nothing needs to change here. just that out[3]
//					is not useful anymore. only change is, on disable, we put
//					low to both foward and reverse.
// Albert Hsueh

// 2016/6/21 - 2.2: Revised to work with bts7960 driver. WE ONLY CARE about the out[2:1], nothing else.
//					When forward, out[2] should have the pwm.
//					When reverse, out[1] should have the pwm.
//					With v2.0's code, out[2:1] already has pwm as observed from oscillascope.
//					Lastly, added a break case functionality
// Albert Hsueh
//////////////////////////////////////////////////////////////////////////

`include "mda_motor_control_defines.v"

// sets the actual output to the H-bridges (the out output from the declaration
// of motor_controller).
module mda_motor_control_internal (input clk, input dir, input on, input pwm, output reg [3:0] out);
	// Set initial state
	reg [3:0] out_reg;
	reg [9:0] dead_time_counter = `DEAD_TIME;
	reg [1:0] prev_in = 2'b00;

	always @(posedge clk)
	begin
  	// If Motor Status has changed (i.e. direction or on/off), need to wait for DEAD_TIME so that transitors have time to switch between states. 
	// Motors change status if it's different from prev_in.
		if ({dir, on} != prev_in)
		// Reset dead_time_counter
			dead_time_counter <= 10'd0;

		// Increment counter if we haven't yet reached the max value  
		if (dead_time_counter != `DEAD_TIME)
		// Increment dead_time_counter
			dead_time_counter <= dead_time_counter + 10'd1;

		// Set out_reg (i.e. H-Bridge MOSFET state) appropriately based on direction and on/off
		casex ({dir, on})
		// Motor Drifts. disable motor
			2'b00: out_reg <= (pwm<<3|4'b0111) & 4'b1000;
		// Brake Motor. Attempt to do this by putting pwm to both out 2 and 1, and also enable motor so it can resist motion
			2'b10: out_reg <= (pwm<<3|4'b0111) & 4'b1111;	
		// Forward Direction. enable, put to forward, and pwm
			2'b11: out_reg <= (pwm<<3|4'b0111) & 4'b1101;
		// Backward Direction. enable, put to reverse, and pwm
			2'b01: out_reg <= (pwm<<3|4'b0111) & 4'b1011;
		endcase

		// Record the current motor state as a record to compare if the state has changed.
		prev_in <= {dir, on};
		// when DEAD_TIME not reached, the motor will be forced off. This ensures the H-bridge transitors have a chance to respond to state transitions.
		out <= (dead_time_counter != `DEAD_TIME) ? 4'd0 : out_reg;	
	end

endmodule
