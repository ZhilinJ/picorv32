# ZedBoard QSPI 固件和 JTAG 终端输出

同一根 **PROG (J17) USB-JTAG 数据线**用于烧录板载 QSPI、配置 PL、读取应用输出。
该版本使用 Zynq PS 的 QSPI 控制器和 AXI GP0。PC 上的 XSDB 脚本从 QSPI
读出应用、校验 CRC、复制到双端口 BRAM，然后释放 PicoRV32 复位。
PicoRV32 从 `0x00010000` 启动，执行完 `main()` 后通过共享 BRAM 报告输出和退出状态。

## 电脑端准备

需要支持 RV32I/ILP32 的 RISC-V GCC 和 binutils、Python 3，以及带 Zynq-7000
器件支持的 Vivado 和 Vitis。实板使用 2025.2 版本。工具路径可通过
`CROSS_COMPILE`（含末尾连字符的工具链前缀）和 `AMD_ROOT`（AMD 安装根目录）覆盖。
`sim` 还需要 Icarus Verilog。

在 PicoRV32 仓库根目录运行：

```bash
make -C vivado_zedboard/qspi bit
make -C vivado_zedboard/qspi sim
make -C vivado_zedboard/qspi app
```

文件输出到 `vivado_zedboard/build/qspi/`。
如果 Vivado 批处理在布线或生成 bitstream 时退出，可以打开已经生成的
`vivado_zedboard/build/qspi/project/pico_qspi.xpr`，在 Vivado 图形界面运行
**Generate Bitstream**。实板验证使用的是该工程生成的 `pico_ps_wrapper.bit`。

`app.bin` 是原始 RV32I 应用，`pico_flash.bin` 是添加长度和 CRC 头、用于烧录的镜像。
默认编译 `demo.c`。要编译自己的 C 文件，使用
`make -C vivado_zedboard/qspi app APP=/绝对路径/main.c`。
在应用里包含 `console.h` 并调用 `console_puts("hello\n")`。让 `main()` 返回，
脚本便会读取最多 1008 字节的输出并检查返回值。实板运行设有 5 秒的超时。

## JTAG 烧录并运行

1. 将 ZedBoard 设置为 **JTAG boot mode** 后上电，连接 **PROG**。
2. 在单独的终端运行 `"$HOME/Xilinx/2025.2/Vivado/bin/hw_server"`，保持它在运行。默认地址为 `tcp:127.0.0.1:3121`。
3. 按下文从工程输出的 `pico_qspi.xsa` 生成 Cortex-A9 **Zynq FSBL ELF**。
   FSBL 是烧录工具的 ARM 初始化程序。
4. 确认 QSPI 偏移 `0x00f00000` 所在的擦除扇区可用于本项目。
   Program Flash 会擦除包含目标镜像的扇区。如果板上已有固件或数据，先确认布局。
5. 在仓库根目录运行：

```bash
make -C vivado_zedboard/qspi flash FSBL=/绝对路径/zynq_fsbl.elf
make -C vivado_zedboard/qspi run
```

`flash` 默认编译示例并烧录。自定义应用可加 `APP=/绝对路径/main.c`。
`run` 优先使用 `build/qspi/pico_qspi.bit`，也可自动使用 Vivado 工程
生成的 `build/qspi/project/pico_qspi.runs/impl_1/pico_ps_wrapper.bit`。
可用 `BITSTREAM=/绝对路径/文件.bit` 指定文件。
`run` 使用 JTAG 配置现有 bitstream，从板载 Flash 读回程序，执行后显示：

```text
QSPI CRC OK: <字节数> bytes. Starting PicoRV32.
PicoRV32 QSPI demo: BRAM PASS (0x00005A2A)
PicoRV32 exit status: 0
```

实板也验证了复用上游 `firmware/hello.c` 的示例。使用同一个 bitstream：

```bash
make -C vivado_zedboard/qspi flash APP=hello_demo.c \
  FSBL="$PWD/vivado_zedboard/build/qspi/zynq_fsbl.elf" &&
make -C vivado_zedboard/qspi run
```

预期在 CRC 检查通过后看到 `hello world` 和 `PicoRV32 exit status: 0`。
更新 C 应用时重复 `flash` 和 `run`，无需重新综合。Flash 在断电后保留固件。
这套流程在重新上电后仍由电脑运行 `make run`，完成 FPGA 配置和固件加载。

## 已通过实板验证的应用

以下应用均已在同一份 ZedBoard RV32I 硬件设计上完成 RTL 仿真和实板运行。
实板日志通过 XSDB 读取共享 BRAM，应用正常结束时退出状态为 `0`。
周期数和指令数对应本工程的工具链及编译选项。

| 应用 | 来源及功能 | 实板结果 |
| --- | --- | --- |
| `hello_demo.c` | 调用上游 `firmware/hello.c` | `hello world` |
| `demo.c` | 本工程的整字和单字节 BRAM 自检 | `BRAM PASS (0x00005A2A)` |
| `sieve_demo.c` | 复用上游 `firmware/sieve.c` 和 `firmware/stats.c` | 31 个质数，末项 127，`checksum: 1772A48F OK`；累计 97380 周期、19130 条指令、CPI 5.09 |
| `benchmark_demo.c` | 改编自 `picosoc/firmware.c` 的 `cmd_benchmark()`，固定种子执行 20 轮数组运算 | `Chksum: 0x5B8EB866`；591416 周期、112684 条指令、CPI 5.24；`BENCHMARK PASS` |
| `memtest_demo.c` | 改编自同文件的 `cmd_memtest()`，使用独立 4 KiB 数组 | 5 轮跨步整字读写和前 128 字节读写通过；`MEMTEST PASS` |
| `dhrystone_demo.c` | 改编自 `dhrystone/dhry_1.c`、`dhry_2.c`，执行 100 轮并核对最终变量 | 510850 周期、94810 条指令；`Arr_2_Glob[8][7] = 110`；`DHRYSTONE PASS` |

`sieve_demo.c` 的计数是 CPU 启动后的累计值，包含启动和 sieve 日志写入。
Benchmark 和 Dhrystone 使用测试前后的计数差，结果打印位于采样结束之后。
Memtest 检查数组偏移 0、1024、2048、3072 的整字读写，以及前 128 字节的读写。
Dhrystone 使用静态记录、精简输出和当前 RV32I 编译设置，用于功能验证。

例如，在仓库根目录构建 Dhrystone：

```bash
make -C vivado_zedboard/qspi app APP=dhrystone_demo.c \
  OUT="$PWD/vivado_zedboard/build/qspi/dhrystone"
```

保持 `hw_server` 运行后，烧录并执行：

```bash
make -C vivado_zedboard/qspi flash APP=dhrystone_demo.c \
  OUT="$PWD/vivado_zedboard/build/qspi/dhrystone" \
  FSBL="$PWD/vivado_zedboard/build/qspi/zynq_fsbl.elf" &&
make -C vivado_zedboard/qspi run
```

`flash` 会重新构建指定应用并替换 Flash 偏移 `0x00f00000` 的应用镜像。
`run` 读取板上当前镜像，构建时为应用指定的 `OUT` 用于保存电脑端产物。
最后一次已验证的实板运行是 Dhrystone，固件大小为 2900 字节。
通过修改 `APP` 和 `OUT` 可以切换上表中的其他应用。

当前 `make sim` 的 testbench 专门检查 `demo.c` 的 BRAM 自检输出。
新增应用使用临时专用 testbench 完成了仿真，这些临时文件未纳入源码。

## 生成 FSBL

在 Ubuntu 24.04 上，安装 `gcc-arm-none-eabi` 和 `libnewlib-arm-none-eabi`。
运行 `make bit` 生成 `pico_qspi.xsa` 后，在仓库根目录执行以下命令。
如果 AMD 工具装在其他目录，替换 `xsdb` 路径。

```bash
export PICO_XSA="$PWD/vivado_zedboard/build/qspi/pico_qspi.xsa"
export PICO_FSBL_WORK="$(mktemp -d)"
cat > "$PICO_FSBL_WORK/build.tcl" <<'TCL'
hsi::open_hw_design $::env(PICO_XSA)
hsi::generate_app -hw [hsi::current_hw_design] -os standalone -proc ps7_cortexa9_0 -app zynq_fsbl -compile -dir [file join $::env(PICO_FSBL_WORK) app]
TCL
"$HOME/Xilinx/2025.2/Vivado/bin/xsdb" "$PICO_FSBL_WORK/build.tcl" &&
cp "$PICO_FSBL_WORK/app/executable.elf" vivado_zedboard/build/qspi/zynq_fsbl.elf
```

硬件参考 [ZedBoard 用户手册](https://digilent.com/reference/_media/zedboard/zedboard_ug.pdf)、
[AMD Vitis Program Flash](https://docs.amd.com/r/en-US/Vitis-Unified-Software-Platform-Documentation-Embedded-Software-Development-UG1400/Programming-Flash)
和 [Zynq QSPI 配置寄存器](https://docs.amd.com/r/en-US/ug585-zynq-7000-SoC-TRM/Configuration-Register-Settings)。
