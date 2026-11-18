set output [file normalize [file join [file dirname [info script]] .. build qspi]]
open_checkpoint [file join $output placed.dcp]
route_design
report_timing_summary -file [file join $output timing.rpt]
report_drc -file [file join $output drc.rpt]
write_bitstream -force [file join $output pico_qspi.bit]
