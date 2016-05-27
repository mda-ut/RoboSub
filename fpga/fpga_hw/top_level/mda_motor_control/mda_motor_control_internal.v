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

  reg [3:0] out_reg;
  reg [9:0] dead_time_counter = `DEAD_TIME;
  reg [1:0] prev_in = 2'b00;

  always @(posedge clk)
  begin
    if ({dir, on} != prev_in)
      dead_time_counter <= 10'd0;
    if (dead_time_counter != `DEAD_TIME)
      dead_time_counter <= dead_time_counter + 10'd1;
    casex ({dir, on})
      2'b00: out_reg <= 4'b0000;
      2'b10: out_reg <= 4'b1010;
      2'b11: out_reg <= 4'b1001;
      2'b01: out_reg <= 4'b0110;
    endcase
    prev_in <= {dir, on};
    out <= (dead_time_counter != `DEAD_TIME) ? 4'd0 : out_reg;
  end

endmodule
