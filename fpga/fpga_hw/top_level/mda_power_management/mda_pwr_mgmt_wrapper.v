// Power management Avalon slave

module mda_pwr_mgmt_wrapper (
  input clk,
  input reset,
  input chipselect,
  input write,
  input read,
  input [31:0] writedata,
  input data,
  output reg [31:0] readdata,
  output kill_sw,
  output [2:0] mux,
  output error
);

  reg start = 1'b0;

  always @(posedge clk)
    if (chipselect)
      if (write)
      begin
        start = writedata[0];
      end
      else if (read)
      begin
        readdata[2:0] = mux;
      end

pwr_mgmt pm_inst (
  .kill_sw(kill_sw),
  .sel(mux),
  .data(data),
  .ack(chipselect && read),
  .start(start),
  .clk(clk),
  .error(error)
);

endmodule
