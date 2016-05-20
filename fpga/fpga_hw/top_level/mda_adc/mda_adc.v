// Motion in perpendicular plane will cause variations on the specific axis, note that this represent motion AROUND a specific axis
// z-axis = side to side (short)
// y-axis = side to side (long)
// x-axis = vertical

/*

 ---------------------------------|----------------------------------
 ---------------------------------|- Y - Axis -----------------------
 ---------------------------------|----------------------------------
 ---------------------------------|----------------------------------
 ---------------------=========================----------------------
 ---------------------||                     ||----------------------
 ---------------------||                     ||______________________
 ---------------------||        DE0-Nano     ||--- X - Axis ---------
 ---------------------||                     ||----------------------
 ---------------------||                     ||----------------------
 ---------------------=========================----------------------
 ---------------------------------|----------------------------------
 ---------------------------------|----------------------------------
 ---------------------------------|----------------------------------
 ---------------------------------|----------------------------------
 YEAH... BUT NO, this is actually just the depth sensor... jinks. So
 just disregard the above.
 ahsueh1996
 */

// used to be called imu. now called just the adc module because it does
// not have anything to do with the imu.
module mda_adc (
    slave_clk,
	 slave_reset_n,
	 slave_chipselect_n,
	 slave_addr,
	 slave_read_n,
	 slave_write_n,
	 slave_readdata,
	 slave_writedata,
	 adc_clk,
	 ADC_CONVST,
	 ADC_SCK,
	 ADC_SDI,
	 ADC_SDO
    );

	// avalon slave port
input									slave_clk;
input									slave_reset_n;
input									slave_chipselect_n;
input				  					slave_addr;
input									slave_read_n;
input									slave_write_n;
output	reg	[15:0]			slave_readdata;
input				[15:0]			slave_writedata;



input								adc_clk;

output		          		ADC_CONVST;
output		          		ADC_SCK;
output	            		ADC_SDI;
input 		          		ADC_SDO;

reg [11:0] 	adc_data = 12'b0,
				slave_shift_reg = 12'b0,
				slave_data = 12'b0;

////////////////////////////////////
// avalon slave port
`define WRITE_REG_START_CH				0
`define WRITE_REG_MEASURE_NUM			1

// write for control
reg 				measure_fifo_start;
reg  [11:0] 	measure_fifo_num;
reg	[2:0]		measure_fifo_ch;
always @ (posedge slave_clk or negedge slave_reset_n)	
begin
	if (~slave_reset_n)
		measure_fifo_start <= 1'b0;
	else if (~slave_chipselect_n && ~slave_write_n && slave_addr == `WRITE_REG_START_CH)   
		{measure_fifo_ch, measure_fifo_start} <= slave_writedata[3:0]; 
	else if (~slave_chipselect_n && ~slave_write_n && slave_addr == `WRITE_REG_MEASURE_NUM)   
		measure_fifo_num <= slave_writedata;
end

///////////////////////
// read 
`define READ_REG_MEASURE_DONE	0
`define READ_REG_ADC_VALUE		1
wire slave_read_status;
wire slave_read_data;


assign slave_read_status = (~slave_chipselect_n && ~slave_read_n && slave_addr == `READ_REG_MEASURE_DONE) ?1'b1:1'b0;
assign slave_read_data = (~slave_chipselect_n && ~slave_read_n && slave_addr == `READ_REG_ADC_VALUE) ?1'b1:1'b0;

reg measure_fifo_done;
always @ (posedge slave_clk)	
begin
	if (slave_read_status)   
		slave_readdata <= {11'b0, measure_fifo_done};
	else if (slave_read_data)   
		slave_readdata <= slave_data;
end

reg pre_slave_read_data;
always @ (posedge slave_clk or negedge slave_reset_n)	
begin
	if (~slave_reset_n)
		pre_slave_read_data <= 1'b0;
	else
		pre_slave_read_data <= slave_read_data;
end

// read ack for adc data. (note. Slave_read_data is read lency=2, so slave_read_data is assert two clock)
assign fifo_rdreq = (pre_slave_read_data & slave_read_data)?1'b1:1'b0;

////////////////////////////////////
// create triggle message: adc_reset_n

reg pre_measure_fifo_start;
always @ (posedge adc_clk)	
begin
	pre_measure_fifo_start <= measure_fifo_start;
end

wire adc_reset_n;
assign adc_reset_n = (~pre_measure_fifo_start & measure_fifo_start)?1'b0:1'b1;

////////////////////////////////////
// control measure_start 
reg [11:0] measure_count;

reg config_first;
reg wait_measure_done;
reg measure_start;
wire measure_done;
wire [11:0] measure_dataread;

always @ (posedge adc_clk or negedge adc_reset_n)	
begin
	if (~adc_reset_n)
	begin
		measure_start <= 1'b0;
		config_first <= 1'b1;
		measure_count <= 0;
		measure_fifo_done <= 1'b0;
		wait_measure_done <= 1'b0;
	end 
	else if (~measure_start & ~wait_measure_done)
	begin
		measure_start <= 1'b1;
		wait_measure_done <= 1'b1;
	end
	else if (wait_measure_done) // && measure_start)
	begin
		measure_start <= 1'b0;
		if (measure_done)
		begin
			if (config_first)
			begin
				config_first <= 1'b0;
				wait_measure_done <= 1'b0; //carl's edit
			end
			else
			begin	// read data and save into fifo
				wait_measure_done <= 1'b0;
				measure_fifo_done <= 1'b1;
			end
		end
	end
end


// write data into fifo

reg pre_measure_done;

always @ (posedge adc_clk or negedge adc_reset_n)	
begin
	if (~adc_reset_n)
		pre_measure_done <= 1'b0;
	else
		pre_measure_done <= measure_done;
end


wire adc_newdata;
assign adc_newdata = (~pre_measure_done & measure_done & ~config_first)?1'b1:1'b0;



always @ (posedge adc_clk or negedge adc_reset_n)
begin
	if (~adc_reset_n)
		adc_data <= 12'b0;
	else if (adc_newdata)
	begin
		adc_data <= measure_dataread;
	end
end

always @ (posedge slave_clk)
begin
	slave_shift_reg <= adc_data;
	slave_data <= slave_shift_reg;
end
	

///////////////////////////////////////
// SPI

mda_adc_controller mda_adc_ctrl_inst(
	.clk(adc_clk), // max 40mhz

	// start measure
	.measure_start(measure_start), // posedge triggle
	.measure_done(measure_done),
	.measure_ch(measure_fifo_ch),
	.measure_dataread(measure_dataread),	
	

	// adc interface
	.ADC_CONVST(ADC_CONVST),
	.ADC_SCK(ADC_SCK),
	.ADC_SDI(ADC_SDI),
	.ADC_SDO(ADC_SDO) 
);
	
endmodule
