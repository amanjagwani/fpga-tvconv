# tvconv float: recreate the Vivado project from design_1.tcl, build the bitstream and export the XSA.
# usage: vivado -mode batch -source build.tcl -tclargs <xsa_name>
# Run HLS for every IP first (README); no board files needed (PS7 preset is in design_1.tcl).
set part xc7z020clg400-1

set here [file dirname [file normalize [info script]]]
set repo [file normalize $here/../..]
set xsa_name [lindex $argv 0]
set proj_dir $here/vivado_proj
set xsa_path $here/xsa/$xsa_name
if {$xsa_name eq ""} {
    error "usage: -tclargs <xsa_name>"
}
if {[file exists $xsa_path]} {
    error "$xsa_path exists; pick a new name"
}

create_project tvconv_float $proj_dir -part $part -force
set ips {}
foreach ip {i2s_2_fft fft_2_i2s} {
    lappend ips $repo/$ip/$ip/hls/impl/ip
}
foreach ip {partition_buffer spectral_convolve overlap_add fft_mirror_replace} {
    lappend ips $repo/float/hls/$ip/$ip/hls/impl/ip
}
set_property ip_repo_paths $ips [current_project]
update_ip_catalog
add_files -fileset constrs_1 $here/ZYBO_Master_Constraints.xdc

source $here/design_1.tcl

set wrapper [make_wrapper -files [get_files design_1.bd] -top]
add_files -norecurse $wrapper
set_property top design_1_wrapper [current_fileset]

set_property strategy Performance_ExplorePostRoutePhysOpt [get_runs impl_1]
launch_runs impl_1 -to_step write_bitstream -jobs 8
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    error "impl_1 did not complete"
}
open_run impl_1
report_timing_summary -file $proj_dir/timing_summary.rpt
report_utilization -file $proj_dir/utilization.rpt
file mkdir $here/xsa
write_hw_platform -fixed -include_bit $xsa_path
