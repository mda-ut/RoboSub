// --------------------------------------------------------------------
// Copyright (c) 2011 by Terasic Technologies Inc. 
// --------------------------------------------------------------------
//
// Permission:
//
//   Terasic grants permission to use and modify this code for use
//   in synthesis for all Terasic Development Boards and Altera Development 
//   Kits made by Terasic.  Other use of this code, including the selling 
//   ,duplication, or modification of any portion is strictly prohibited.
//
// Disclaimer:
//
//   This VHDL/Verilog or C/C++ source code is intended as a design reference
//   which illustrates how these types of functions can be implemented.
//   It is the user's responsibility to verify their design for
//   consistency and functionality through the use of formal
//   verification methods.  Terasic provides no warranty regarding the use 
//   or functionality of this code.
//
// --------------------------------------------------------------------
//           
//                     Terasic Technologies Inc
//                     E. Rd Sec. 1. JhuBei City,
//                     HsinChu County, Taiwan
//                     302
//
//                     web: http://www.terasic.com/   
//                     email: support@terasic.com
//
// --------------------------------------------------------------------



module DE0_Nano(

	//////////// CLOCK //////////
	CLOCK_50,

	//////////// LED //////////
	LED,

	//////////// KEY //////////
	KEY,

	//////////// SW //////////
	SW,

	//////////// ADC //////////
	ADC_CONVST,
	ADC_SDI,
	ADC_SCK,
	ADC_SDO,

	//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
	GPIO_0,

	//////////// GPIO_1, GPIO_1 connect to GPIO Default //////////
	GPIO_1
	);

	//=======================================================
	//  PARAMETER declarations
	//=======================================================


	//=======================================================
	//  PORT declarations
	//=======================================================

	//////////// CLOCK //////////
	input 		          		CLOCK_50;

	//////////// LED //////////
	output		     [7:0]		LED;

	//////////// KEY //////////
	input 		     [1:0]		KEY;

	//////////// SW //////////
	input 		     [3:0]		SW;


	//////////// ADC //////////
	output		          		ADC_CONVST;
	output		          		ADC_SDI;
	output		          		ADC_SCK;
	input 		          		ADC_SDO;

	//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
	inout 		    [35:0]		GPIO_0;

	//////////// GPIO_1, GPIO_1 connect to GPIO Default //////////
	inout 		    [35:0]		GPIO_1;


	//=======================================================
	//  REG/WIRE declarations
	//=======================================================
	wire reset_n;

	wire power;
	wire [2:0] voltage_mux;
	wire imu_in, IMU_IN;
	wire leakdetect;	
	wire [5:0] extra_signals;
	
	//motor pin wires
	wire [3:0] HB1, HB2, HB3, HB4, HB5, HB6, HB7, HB8;
	wire [3:0] hb1, hb2, hb3, hb4, hb5, hb6, hb7, hb8;
	
	//pneumatics wires (currently unused)
	wire [1:0] PN1, PN2, PN3, PN4, PN5, PN6, PN7, PN8;
	//assign PN1 = 2'b0, PN2 = 2'b0, PN3 = 2'b0, PN4 = 2'b0, PN5 = 2'b0, PN6 = 2'b0, PN7 = 2'b0, PN8 = 2'b0;
	
	wire dual_ch0_A, dual_ch0_B, dual_ch1_A, dual_ch1_B; // inputs
	wire dual_ch0_sclk, dual_ch0_cs, dual_ch1_sclk, dual_ch1_cs; //outputs
	
	


	//=======================================================
	//  Structural coding
	//=======================================================

	assign reset_n = 1'b1;
	assign GPIO_0[35] = power; //led1_enable;
	assign GPIO_0[34] = power;
	assign {GPIO_0[26], GPIO_0[28], GPIO_0[30]} = voltage_mux;
	assign LED[0] = power;
	assign LED[1] = IMU_IN;
	assign LED[2] = GPIO_0[14];
	
	assign extra_signals = 6'b0;
	assign {GPIO_1[1], GPIO_1[0], GPIO_1[2], GPIO_1[4], GPIO_1[6], GPIO_1[8]} = extra_signals; 

	wire[3:0] random;
	global_disable #(
	  .NUM_IN(3),
	  .NUM_IOS(49)
	) dis_inst(
	  .clk(CLOCK_50),
	  .shutdown(~{KEY, power}),
	  .gpio_in({hb1, hb2, hb3, hb4, hb5, hb6, hb7, hb8, imu_in, 16'b0}),
	  .gpio_out_default({49'b0}),
	  .gpio_out({HB1, HB2, HB3, HB4, HB5, HB6, HB7, HB8, IMU_IN, PN1, PN2, PN3, PN4, PN5, PN6, PN7, PN8})
	);
	
	//motor wires ordered {forward hhb top, forward hhb bot, reverse hhb top, reverse hhb bot}
	//hhb = half h-bridge
	assign {GPIO_0[11], GPIO_0[13], GPIO_0[15], GPIO_0[17]} = HB1;
	assign {GPIO_0[5], GPIO_0[3], GPIO_0[9], GPIO_0[7]} = HB2;
	assign {GPIO_0[29], GPIO_0[27], GPIO_0[33], GPIO_0[31]} = HB3;
	assign {GPIO_0[19], GPIO_0[21], GPIO_0[23], GPIO_0[25]} = HB4;
	assign {GPIO_1[11], GPIO_1[13], GPIO_1[15], GPIO_1[17]} = HB5;
	assign {GPIO_1[5], GPIO_1[3], GPIO_1[9], GPIO_1[7]} = HB6;
	assign {GPIO_1[29], GPIO_1[27], GPIO_1[33], GPIO_1[31]} = HB7;
	assign {GPIO_1[19], GPIO_1[21], GPIO_1[23], GPIO_1[25]} = HB8;
	
	
	assign {GPIO_0[12], GPIO_0[10]} = PN1;
	assign {GPIO_0[8], GPIO_0[6]} = PN2;
	assign {GPIO_1[24], GPIO_1[22]} = PN3;
	assign {GPIO_1[28], GPIO_1[26]} = PN4;
	assign {GPIO_1[32], GPIO_1[30]} = PN5;
	assign {GPIO_1[35], GPIO_1[34]} = PN6;
	assign {GPIO_0[4], GPIO_0[2]} = PN7;
	assign {GPIO_0[0], GPIO_0[1]} = PN8;

	// assign IMU reset to low
	reg imu_reset = 1'b0;
	assign GPIO_1[14] = imu_reset;
	assign GPIO_0[20] = IMU_IN;
	
	//sonar assignments
	assign dual_ch0_A = GPIO_1[18];
	assign dual_ch0_B = GPIO_1[16];
	assign dual_ch1_A = GPIO_0[18];
	assign dual_ch1_B = GPIO_0[16];
	
	assign dual_ch0_sclk = 1'b0;
	assign dual_ch0_cs = 1'b0;
	assign dual_ch1_sclk = 1'b0;
	assign dual_ch1_cs = 1'b0;
	assign GPIO_0[22] = dual_ch0_sclk;
	assign GPIO_0[24] = dual_ch0_cs;
	assign GPIO_1[10] = dual_ch1_sclk;
	assign GPIO_1[12] = dual_ch1_cs;
	
	assign leakdetect = GPIO_1[20];
	
	reg [6:0] counter = 7'b0;
	reg prev_power = 1'b0;
	always@(posedge CLOCK_50) begin
		prev_power <= power;
		imu_reset <= 1'b0;
		if (power & !prev_power) begin
			if (counter < 7'd100) begin
				counter <= counter + 7'b1;
				prev_power <= 1'b0;
				imu_reset <= 1'b1;
			end
			else begin
				counter <= 7'b0;
			end
		end
	end

	DE0_Nano_SOPC DE0_Nano_SOPC_inst(
						//global signals
						.clk_50(CLOCK_50),                                    	//                        clk_50_clk_in.clk
						.reset_n(KEY[0]),                                   		//                  clk_50_clk_in_reset.reset_n
						
						//power management signals
						.kill_sw_from_the_power_management_slave_0(power), 		// power_management_slave_0_conduit_end.kill_sw
						.mux_from_the_power_management_slave_0(voltage_mux),     //                                     .mux
						.data_to_the_power_management_slave_0(GPIO_0[32]),      	//                                     .data
						
						//Motor pins
						.GPIO_out_from_the_motor_controller_0 ({hb1, hb2, hb3, hb4, hb5, hb6, hb7, hb8}), //       motor_controller_0_conduit_end.export
	
						//IMU signals
						.UART_RXD_to_the_RS232_0(!power || GPIO_0[14]),                   //                  RS232_0_conduit_end.export
						.UART_TXD_from_the_RS232_0(imu_in),                 				//                RS232_0_conduit_end_1.export
						
						//ADC signals primarily used for depth sensor
						.adc_controller_0_conduit_end_sys_clk(CLOCK_50),      	//         adc_controller_0_conduit_end.sys_clk
						.adc_controller_0_conduit_end_ADC_CONVST(ADC_CONVST),   	//                                     .ADC_CONVST
						.adc_controller_0_conduit_end_ADC_SDI(ADC_SDI),      		//                                     .ADC_SDI
						.adc_controller_0_conduit_end_ADC_SCK(ADC_SCK),      		//                                     .ADC_SCK
						.adc_controller_0_conduit_end_ADC_SDO(ADC_SDO),      		//                                     .ADC_SDO
						
						//keys and switches
						.in_port_to_the_key(KEY),                        			//              key_external_connection.export
						.in_port_to_the_sw(SW),                          			//               sw_external_connection.export
						.out_port_from_the_led(),                     				//              led_external_connection.export
						
						//pll signals (unused)
						.pll_0_locked_export(),                       				//                         pll_0_locked.export
						.pll_0_outclk1_clk(),                         				//                        pll_0_outclk1.clk
						.pll_0_outclk3_clk(),                         				//                        pll_0_outclk3.clk
						.pll_adc_clk(),                               				//                              pll_adc.clk
						.pll_io_clk(),                                				//                               pll_io.clk
						.pll_sys_clk()                               				//                              pll_sys.clk
	);

	

							  
	//assign I2C_SCLK = (select_i2c_clk)?i2c_clk:spi_clk;

endmodule

