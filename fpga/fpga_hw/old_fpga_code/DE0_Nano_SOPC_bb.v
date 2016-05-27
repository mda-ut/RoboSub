
module DE0_Nano_SOPC (
	UART_RXD_to_the_RS232_0,
	UART_TXD_from_the_RS232_0,
	adc_controller_0_conduit_end_sys_clk,
	adc_controller_0_conduit_end_ADC_CONVST,
	adc_controller_0_conduit_end_ADC_SDI,
	adc_controller_0_conduit_end_ADC_SCK,
	adc_controller_0_conduit_end_ADC_SDO,
	clk_50,
	reset_n,
	in_port_to_the_key,
	out_port_from_the_led,
	GPIO_out_from_the_motor_controller_0,
	pll_0_locked_export,
	pll_0_outclk1_clk,
	pll_0_outclk3_clk,
	pll_adc_clk,
	pll_io_clk,
	pll_sys_clk,
	kill_sw_from_the_power_management_slave_0,
	mux_from_the_power_management_slave_0,
	data_to_the_power_management_slave_0,
	in_port_to_the_sw);	

	input		UART_RXD_to_the_RS232_0;
	output		UART_TXD_from_the_RS232_0;
	input		adc_controller_0_conduit_end_sys_clk;
	output		adc_controller_0_conduit_end_ADC_CONVST;
	output		adc_controller_0_conduit_end_ADC_SDI;
	output		adc_controller_0_conduit_end_ADC_SCK;
	input		adc_controller_0_conduit_end_ADC_SDO;
	input		clk_50;
	input		reset_n;
	input	[1:0]	in_port_to_the_key;
	output	[7:0]	out_port_from_the_led;
	output	[31:0]	GPIO_out_from_the_motor_controller_0;
	output		pll_0_locked_export;
	output		pll_0_outclk1_clk;
	output		pll_0_outclk3_clk;
	output		pll_adc_clk;
	output		pll_io_clk;
	output		pll_sys_clk;
	output		kill_sw_from_the_power_management_slave_0;
	output	[2:0]	mux_from_the_power_management_slave_0;
	input		data_to_the_power_management_slave_0;
	input	[3:0]	in_port_to_the_sw;
endmodule
