/////////////////////////////////////////////////////////////////////////
// Module: mda_motor_control_internal.v
// i---------------------------------------------------------------------
// Purpose: Module defines the motor controller logics.
// ---------------------------------------------------------------------
// Version History:
//
// 2016/5/22 - 1.0: Copied legacy verilog and made modifications for Robosub
//					2016 competition. Using new documentation convention.
// Shai Bonen
//////////////////////////////////////////////////////////////////////////

`include "mda_motor_control_defines.v"

// sets the actual output to the H-bridges (the out output from the declaration
// of motor_controller).
module mda_motor_control_internal (input clk, input dir, input on, output reg [3:0] out);

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
	  // Motor Drifts. Turn off all MOSFETs
      2'b00: out_reg <= 4'b0000;
	  // Brake Motor. Turn on both bottom MOSFETs. This line used to be 1010, however we think this (albeit it disables the motor overall) 
	  // wastes power (we have a resistor to bleed charge), therefore we will force both motor leads to ground.
      2'b10: out_reg <= 4'b0101;	
	  // Forward Direction. Turn on Top MOSFET for p direction and bottom MOSFET for n direction.
	  2'b11: out_reg <= 4'b1001;
	  // Backward Direction. Turn on Top MOSFET for n direction and bottom MOSFET for p direction.
      2'b01: out_reg <= 4'b0110;
    endcase

	// Record the current motor state as a record to compare if the state has changed.
    prev_in <= {dir, on};
	// when DEAD_TIME not reached, the motor will be forced off. This ensures the H-bridge transitors have a chance to respond to state transitions.
    out <= (dead_time_counter != `DEAD_TIME) ? 4'd0 : out_reg;	
  end

endmodule
