// This is the Avalon slave for the -I-M-U- (scratch that) adc
//
// Registers 0-7 store ADC outputs

// modified Mar. 26 2016 ahsueh1996
module mda_adc_wrapper (
	  input  wire        slave_reset_n,  //     reset_sink.reset_n
	  output wire        ADC_CONVST,   //    conduit_end.convst_out
	  output wire        ADC_SCK,      //               .sck_out
	  output wire        ADC_SDI,      //               .sdi_out
	  input  wire        ADC_SDO,      //               .sdo_in
	  input  wire        chipselect_n, //      adc_slave.chipselect_n
	  input  wire 		   addr,         //               .address
	  input  wire        read_n,       //               .read_n
	  output wire [31:0] readdata,     //               .readdata
	  input  wire        write_n,      //               .write_n
	  input  wire [31:0] writedata,    //               .writedata
	  input  wire        adc_clk,      // clock_sink_adc.clk
	  input  wire        slave_clk     //     clock_sink.clk
 );

   mda_adc mda_adc(
		.slave_clk(slave_clk),
		.slave_reset_n(slave_reset_n),
		.slave_chipselect_n(chipselect_n),
		.slave_addr(addr),
		.slave_read_n(read_n),
		.slave_write_n(write_n),
		.slave_readdata(readdata),
		.slave_writedata(writedata),
		.adc_clk(adc_clk),
		.ADC_CONVST(ADC_CONVST),
		.ADC_SCK(ADC_SCK),
		.ADC_SDI(ADC_SDI),
		.ADC_SDO(ADC_SDO)
   );
endmodule

