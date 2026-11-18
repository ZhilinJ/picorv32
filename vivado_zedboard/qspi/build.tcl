set src [file dirname [file normalize [info script]]]
set out [file normalize [file join $src .. build qspi]]
create_project -force pico_qspi [file join $out project] -part xc7z020clg484-1
set rtl [list [file normalize [file join $src .. .. picorv32.v]] [file join $src pico_flash_cpu.v]]
add_files $rtl
create_bd_design pico_ps
set ps [create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 ps]
set_property -dict [list CONFIG.PCW_IMPORT_BOARD_PRESET ZedBoard \
    CONFIG.PCW_USE_M_AXI_GP0 1 CONFIG.PCW_EN_CLK0_PORT 1 \
    CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ 100] $ps
make_bd_intf_pins_external [get_bd_intf_pins ps/FIXED_IO]
make_bd_intf_pins_external [get_bd_intf_pins ps/DDR]
set cpu [create_bd_cell -type module -reference pico_flash_cpu cpu]
set ctrl [create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl ctrl]
set_property -dict {CONFIG.SINGLE_PORT_BRAM 1 CONFIG.DATA_WIDTH 32 CONFIG.PROTOCOL AXI4} $ctrl
set ram [create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen ram]
set_property -dict {CONFIG.Memory_Type True_Dual_Port_RAM CONFIG.Use_Byte_Write_Enable true CONFIG.Byte_Size 8 CONFIG.Write_Width_A 32 CONFIG.Read_Width_A 32 CONFIG.Write_Depth_A 4096 CONFIG.Write_Width_B 32 CONFIG.Read_Width_B 32 CONFIG.Enable_B Use_ENB_Pin CONFIG.Register_PortB_Output_of_Memory_Primitives false} $ram
set gpio [create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio gpio]
set_property -dict {CONFIG.C_GPIO_WIDTH 1 CONFIG.C_ALL_OUTPUTS 1 CONFIG.C_DOUT_DEFAULT 0x00000000} $gpio
set axi [create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect axi]
set_property CONFIG.NUM_MI 2 $axi
set rst [create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset rst]
set locked [create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant locked]
connect_bd_net [get_bd_pins locked/dout] [get_bd_pins rst/dcm_locked]
connect_bd_net [get_bd_pins ps/FCLK_RESET0_N] [get_bd_pins rst/ext_reset_in]
connect_bd_net [get_bd_pins ps/FCLK_CLK0] [get_bd_pins {ps/M_AXI_GP0_ACLK axi/aclk rst/slowest_sync_clk ctrl/s_axi_aclk gpio/s_axi_aclk ram/clkb cpu/clk}]
connect_bd_net [get_bd_pins rst/peripheral_aresetn] [get_bd_pins {ctrl/s_axi_aresetn gpio/s_axi_aresetn axi/aresetn}]
connect_bd_intf_net [get_bd_intf_pins ps/M_AXI_GP0] [get_bd_intf_pins axi/S00_AXI]
connect_bd_intf_net [get_bd_intf_pins axi/M00_AXI] [get_bd_intf_pins ctrl/S_AXI]
connect_bd_intf_net [get_bd_intf_pins axi/M01_AXI] [get_bd_intf_pins gpio/S_AXI]
connect_bd_intf_net [get_bd_intf_pins ctrl/BRAM_PORTA] [get_bd_intf_pins ram/BRAM_PORTA]
foreach {a b} {ram_addr addrb ram_din dinb ram_dout doutb ram_en enb ram_we web} {
    connect_bd_net [get_bd_pins cpu/$a] [get_bd_pins ram/$b]
}
connect_bd_net [get_bd_pins gpio/gpio_io_o] [get_bd_pins cpu/run]
set led [create_bd_port -dir O led]
connect_bd_net $led [get_bd_pins cpu/led]
assign_bd_address -offset 0x40000000 -range 16K [get_bd_addr_segs ctrl/S_AXI/Mem0]
assign_bd_address -offset 0x41200000 -range 64K [get_bd_addr_segs gpio/S_AXI/Reg]
validate_bd_design
save_bd_design
set bd [get_files pico_ps.bd]
set_property synth_checkpoint_mode None $bd
generate_target all $bd
set wrapper [make_wrapper -files $bd -top]
add_files $wrapper
set xdc [file join $src zedboard.xdc]
add_files -fileset constrs_1 $xdc
set_property top pico_ps_wrapper [current_fileset]
set bd_path [get_property NAME $bd]
set init [glob -directory [file join $out project pico_qspi.gen sources_1 bd pico_ps ip] */ps7_init.tcl]
file copy -force [lindex $init 0] [file join $out ps7_init.tcl]
write_hw_platform -fixed -force -file [file join $out pico_qspi.xsa]
close_project

# Use the generated BD in a non-project run for synthesis and implementation.
read_verilog [concat $rtl $wrapper]
read_bd $bd_path
read_xdc $xdc
synth_design -top pico_ps_wrapper -part xc7z020clg484-1
opt_design
place_design
write_checkpoint -force [file join $out placed.dcp]
