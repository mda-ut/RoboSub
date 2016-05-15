// This is the Avalon slave for the -I-M-U- (scratch that) adc
//
// Registers 0-7 store ADC outputs

// modified Mar. 26 2016 ahsueh1996
module mda_adc_wrapper (
    input wire 	       spi_clk,    //       clock_sink.clk
    input wire 	       spi_reset,  //       reset_sink.reset
    input wire 	       sys_clk,    //      conduit_end.export
    output wire        ADC_CONVST, //                 .export
    output wire        ADC_SCK,    //                 .export
    output wire        ADC_SDI,    //                 .export
    input wire 	       ADC_SDO,    //                 .export
    input wire 	       chipselect, // avalon_slave_0_1.chipselect
    input wire [3:0]   addr,       //                 .address
    input wire 	       read,       //                 .read
    output reg [31:0] readdata    //                 .readdata
    );

   wire [8*32-1:0]   adc_channels;

   mda_adc mda_adc(
      .reset_n(spi_reset),
      .spi_clk(spi_clk),
      .adc_channels(adc_channels),
      .ADC_SDO(ADC_SDO),
      .ADC_CONVST(ADC_CONVST),
      .ADC_SDI(ADC_SDI),
      .ADC_SCK(ADC_SCK)
      );

   always @(posedge sys_clk)
      if (chipselect & read)
	 casex (addr)
	   4'b0000:
              readdata <= adc_channels[1*32-1:0*32];
	   4'b0001:
              readdata <= adc_channels[2*32-1:1*32];
	   4'b0010:
              readdata <= adc_channels[3*32-1:2*32];
	   4'b0011:
              readdata <= adc_channels[4*32-1:3*32];
	   4'b0100:
              readdata <= adc_channels[5*32-1:4*32];
	   4'b0101:
              readdata <= adc_channels[6*32-1:5*32];
	   4'b0110:
              readdata <= adc_channels[7*32-1:6*32];
	   4'b0111:
              readdata <= adc_channels[8*32-1:7*32];
	   default:
              readdata <= 32'd0;
	 endcase
endmodule

