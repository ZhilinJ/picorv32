CROSS_COMPILE ?= /opt/riscv32i/bin/riscv32-unknown-elf-
AMD_ROOT ?= $(HOME)/Xilinx/2025.2
VIVADO ?= $(AMD_ROOT)/Vivado/bin/vivado
XSDB ?= $(AMD_ROOT)/Vivado/bin/xsdb
VITIS_LOADER ?= $(AMD_ROOT)/Vitis/bin/loader
OUT := ../build/qspi
APP ?= demo.c
FSBL ?=
HW_URL ?= tcp:127.0.0.1:3121
BITSTREAM ?=

.PHONY: all app bit flash run sim FORCE
all: app
app: $(OUT)/pico_flash.bin

$(OUT):
	mkdir -p $@

$(OUT)/app.elf: $(APP) console.c console.h ../uart/app.ld ../firmware/start.S Makefile FORCE | $(OUT)
	$(CROSS_COMPILE)gcc -march=rv32i -mabi=ilp32 -Os -ffreestanding -fno-pic \
		-msmall-data-limit=0 -mno-relax -nostdlib -I. \
		-Wl,--defsym=__reserved_top=0x400,-T,../uart/app.ld,--wrap=main,--build-id=none \
		-o $@ ../firmware/start.S "$(APP)" console.c -lgcc

$(OUT)/app.bin: $(OUT)/app.elf
	$(CROSS_COMPILE)objcopy -O binary $< $@

$(OUT)/pico_flash.bin: $(OUT)/app.bin pack.py
	python3 pack.py $< $@

bit: | $(OUT)
	cd $(OUT) && "$(VIVADO)" -mode batch -source ../../qspi/build.tcl
	cd $(OUT) && "$(VIVADO)" -mode batch -source ../../qspi/route.tcl

flash: $(OUT)/pico_flash.bin
	@test -n "$(FSBL)" && test -f "$(FSBL)" || { echo "Set FSBL to a ZedBoard Cortex-A9 FSBL ELF" >&2; exit 1; }
	"$(VITIS_LOADER)" -exec rdi_zynq_flash -f "$(abspath $<)" -offset 0x00f00000 \
		-flash_type qspi-x4-single -fsbl "$(abspath $(FSBL))" -verify -url "$(HW_URL)"

run:
	"$(XSDB)" run.tcl "$(abspath $(OUT))" "$(HW_URL)" $(if $(BITSTREAM),"$(abspath $(BITSTREAM))")

sim: $(OUT)/app.bin
	iverilog -g2012 -s tb_pico_flash_cpu -o $(OUT)/qspi_cpu_sim ../../picorv32.v pico_flash_cpu.v tb_pico_flash_cpu.v
	cd $(OUT) && vvp qspi_cpu_sim
