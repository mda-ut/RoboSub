# Run 
# quartus_sh --help=makefiles
# to generate this file
###################################################################
# Project Configuration: 
# 
# Specify the name of the design (project), the Quartus II Settings
# File (.qsf), and the list of source files used.
###################################################################

PROJECT=MDA_DE0_Nano_SOC
QSYS_NAME=nios_to_periph_sys
FAMILY = "Cyclone V"
PART = 5CSEMA4U23C6

QSYS_FILE=$(QSYS_NAME).qsys
QSYS_PATH=$(QSYS_NAME)/synthesis/
QSYS_SUBMODULES=$(QSYS_PATH)submodules/
QSYSINFO_FILE=$(QSYS_PATH)$(QSYS_NAME)_generation.rpt
QSYSINFO_FILE_DEPENDS=$(QSYS_FILE)

PROJECT_SUBMODULE_FILES= mda_adc/*.v mda_common_modules/*.v mda_motor_control/*.v mda_power_management/*.v RS232/*.v
SOURCE_FILES = $(PROJECT).v $(PROJECT_SUBMODULE_FILES)
ASSIGNMENT_FILES = $(PROJECT).qpf $(PROJECT).qsf
OUTPUT_FOLDER=output_files


###################################################################
# Main Targets
#
# all: build everything
# clean: remove output files and database
###################################################################

all: $(OUTPUT_FOLDER)/$(PROJECT).done 

clean: #clean_sopc
	rm -rf *.rpt *.chg smart.log *.htm *.eqn *.pin *.sof *.pof db incremental_db *.smsg *.jdi *.map.summary *.fit.summary *.sta.summary
	quartus_sh --clean -c MDA_DE0_Nano_SOC MDA_DE0_Nano_SOC

#clean_sopc:
#	rm -rf led.v DE0_Nano_SOPC_sim i2c_scl.v cpu_oci_test_bench.v timer.v DE0_Nano_SOPC_log.txt cpu_rf_ram_a.mif cpu_rf_ram_b.mif altpll_sys.v DE0_Nano_SOPC.ptf.bak key.v epcs.v DE0_Nano_SOPC.ptf sysid.v adc_spi_read.v epcs_boot_rom_synth.hex DE0_Nano_SOPC.ptf.8.0 DE0_Nano_SOPC_clock_0.v DE0_Nano_SOPC_clock_1.v DE0_Nano_SOPC_generation_script cpu_ic_tag_ram.mif cpu.ocp DE0_Nano_SOPC_inst.v i2c_sda.v altpll_sys.vo select_i2c_clk.v sdram.v  DE0_Nano_SOPC.html cpu_jtag_debug_module_wrapper.v cpu_jtag_debug_module_tck.v clock_crossing_io2.v DE0_Nano_SOPC.bsf  jtag_uart.v  DE0_Nano_SOPC.sopcinfo  .sopc_builder sopc_builder_log.txt cpu_test_bench.v cpu.v sdram_test_component.v cpu_mult_cell.v sw.v cpu_bht_ram.mif clock_crossing_io.v g_sensor_int.v gsensor_spi.v DE0_Nano_SOPC.qip cpu.sdc cpu_dc_tag_ram.mif cpu_jtag_debug_module_sysclk.v DE0_Nano_SOPC.ptf.pre_generation_ptf DE0_Nano_SOPC.v cpu_ociram_default_contents.mif motor_controller_0.v DE0_Nano_SOPC_clock_2.v


.PHONY: clean all

###################################################################
# Executable Configuration
###################################################################

MAP_ARGS = --family=$(FAMILY) --read_settings_files=on --write_settings_files=off
FIT_ARGS = --part=$(PART) --read_settings_files=off --write_settings_files=off
ASM_ARGS = --read_settings_files=off --write_settings_files=off
STA_ARGS =
EDA_ARGS = --read_settings_files=off --write_settings_files=off

###################################################################
# Target implementations
###################################################################


$(QSYSINFO_FILE): $(QSYSINFO_FILE_DEPENDS) $(SOURCE_FILES)
	qsys-generate $(QSYS_FILE) -bsf -od=$(QSYS_NAME) --family=$(FAMILY) --part=$(PART)
	qsys-generate $(QSYS_FILE) --synthesis=VERILOG -od=$(QSYS_NAME) --family=$(FAMILY) --part=$(PART) #|| [$$? -eq 4]


$(OUTPUT_FOLDER)/$(PROJECT).map.rpt: $(SOURCE_FILES) $(QSYSINFO_FILE)
	quartus_map $(MAP_ARGS) $(PROJECT) -c $(PROJECT)
#quartus_map $(MAP_ARGS) -l $(SOPC_PATH) -l $(SOPC_SUBMODULES) $(PROJECT)

$(OUTPUT_FOLDER)/$(PROJECT).fit.rpt: $(OUTPUT_FOLDER)/$(PROJECT).map.rpt
	quartus_fit $(FIT_ARGS) $(PROJECT) -c $(PROJECT)

$(OUTPUT_FOLDER)/$(PROJECT).asm.rpt: $(OUTPUT_FOLDER)/$(PROJECT).fit.rpt
	quartus_asm $(ASM_ARGS) $(PROJECT) -c $(PROJECT)

$(OUTPUT_FOLDER)/$(PROJECT).sta.rpt: $(OUTPUT_FOLDER)/$(PROJECT).fit.rpt
	quartus_sta $(STA_ARGS) $(PROJECT) -c $(PROJECT)

$(OUTPUT_FOLDER)/$(PROJECT).eda.rpt: $(OUTPUT_FOLDER)/$(PROJECT).fit.rpt
	quartus_eda $(EDA_ARGS) $(PROJECT) -c $(PROJECT)

$(OUTPUT_FOLDER)/$(PROJECT).done: $(QSYSINFO_FILE) $(SOURCE_FILES)
	quartus_map $(MAP_ARGS) $(PROJECT) -c $(PROJECT)
	quartus_fit $(FIT_ARGS) $(PROJECT) -c $(PROJECT)
	quartus_asm $(ASM_ARGS) $(PROJECT) -c $(PROJECT)
	quartus_sta $(STA_ARGS) $(PROJECT) -c $(PROJECT)
	quartus_eda $(EDA_ARGS) $(PROJECT) -c $(PROJECT)
