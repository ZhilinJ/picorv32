# Run from the board's QSPI via JTAG. The host copies Flash bytes through
# the Zynq PS debug port into the BRAM shared with PicoRV32.
if {$argc < 2 || $argc > 3} {
    error "Usage: xsdb run.tcl <build-directory> <hw-server-url> ?bitstream?"
}
lassign $argv out url bitstream
if {$argc == 2} {
    set bitstream [file join $out pico_qspi.bit]
    if {![file isfile $bitstream]} {
        set bitstream [file join $out project pico_qspi.runs impl_1 pico_ps_wrapper.bit]
    }
}
set ps_init [file join $out ps7_init.tcl]
foreach file [list $bitstream $ps_init] {
    if {![file isfile $file]} {error "Missing $file. Run make bit first."}
}

connect -url $url
targets -set -filter {name =~ "ARM Cortex-A9 MPCore #0"}
rst -system
after 1000
targets -set -filter {name =~ "ARM Cortex-A9 MPCore #0"} -timeout 5
source $ps_init
ps7_init
targets -set -filter {name =~ "xc7z020*"}
fpga -file $bitstream
targets -set -filter {name =~ "ARM Cortex-A9 MPCore #0"}
ps7_post_config

# Zynq PS QSPI: disable, select single-lane linear READ (0x03),
# then enable. Read-only access to the reserved slot below 16 MiB.
mwr -force 0xe000d014 0
mwr -force 0xe000d000 0x800804d9
mwr -force 0xe000d0a0 0x80000003
mwr -force 0xe000d014 1

set flash 0xfcf00000
set header [mrd -force -bin $flash 3]
binary scan $header a4iu2 magic fields
lassign $fields length crc
if {$magic ne "PRV1" || $length < 4 || $length > 0x3a00} {
    error "No valid PicoRV32 image in QSPI at offset 0x00f00000"
}
set words [expr {($length + 3) / 4}]
set payload [mrd -force -bin [expr {$flash + 12}] $words]
if {[zlib crc32 [string range $payload 0 [expr {$length - 1}]]] != $crc} {
    error "PicoRV32 image from QSPI failed CRC"
}

set gpio 0x41200000
set bram 0x40000000
set mailbox 0x40003c00
mwr -force $gpio 0
mwr -force $mailbox {0 0 0 0}
binary scan $payload iu* data
for {set first 0} {$first < $words} {incr first 128} {
    set addr [expr {$bram + 4 * $first}]
    set chunk [lrange $data $first [expr {$first + 127}]]
    mwr -force $addr $chunk
}
if {[mrd -force -bin $bram $words] ne $payload} {
    error "BRAM readback differs from QSPI payload"
}
puts "QSPI CRC OK: $length bytes. Starting PicoRV32."
mwr -force $gpio 1
set deadline [expr {[clock milliseconds] + 5000}]
while {1} {
    lassign [mrd -force -value $mailbox 4] done count status truncated
    if {$done == 0x5049434f} break
    if {[clock milliseconds] >= $deadline} {
        mwr -force $gpio 0
        error "PicoRV32 did not return from main within 5 seconds"
    }
    after 20
}
if {$count > 1008} {error "Invalid PicoRV32 console length"}
if {$count > 0} {
    set output [mrd -force -bin 0x40003c10 [expr {($count + 3) / 4}]]
    set message [string range $output 0 [expr {$count - 1}]]
    puts -nonewline $message
    if {![string match "*\n" $message]} {puts ""}
}
if {$truncated} {puts stderr "PicoRV32 console output truncated"}
set status [expr {($status & 0x7fffffff) - ($status & 0x80000000)}]
puts "PicoRV32 exit status: $status"
if {$status != 0} {error "PicoRV32 application failed"}
