gui_start
open_mw_lib /users/course/2021F/pda13000000/u107062107/HW1/run/AES
::iccGUI::open_mw_cel  initial
open_mw_cel initial
create_floorplan -core_utilization 0.93 -flip_first_row -left_io2core 30 -bottom_io2core 30 -right_io2core 30 -top_io2core 30
create_placement -congestion -timing_driven
legalize_placement -effort medium -incremental
derive_pg_connection -power_net {VDD} -ground_net {VSS} -power_pin {VDD} -ground_pin {VSS} -create_ports top
set_fp_rail_constraints -add_layer  -layer METAL7 -direction horizontal -max_strap 10 -min_strap 5 -max_width 5 -min_width 2 -spacing interleaving
set_fp_rail_constraints -add_layer  -layer METAL6 -direction vertical -max_strap 10 -min_strap 5 -max_width 5 -min_width 2 -spacing interleaving
set_fp_rail_constraints  -set_ring -nets  {VDD VSS}  -horizontal_ring_layer { METAL7 } -vertical_ring_layer { METAL6 } -ring_width 2 -extend_strap core_ring
create_fp_virtual_pad -net VDD -layer METAL7 -point {-2.640 212.135}
create_fp_virtual_pad -net VDD -layer METAL7 -point {379.315 213.455}
create_fp_virtual_pad -net VSS -layer METAL7 -point {-2.640 200.240}
create_fp_virtual_pad -net VSS -layer METAL7 -point {376.670 202.880}
create_fp_virtual_pad -net VDD -layer METAL6 -point {0.000 196.275}
create_fp_virtual_pad -net VDD -layer METAL6 -point {376.670 192.310}
create_fp_virtual_pad -net VSS -layer METAL6 -point {-1.320 179.090}
create_fp_virtual_pad -net VSS -layer METAL6 -point {377.995 196.275}
synthesize_fp_rail  -nets {VDD VSS} -synthesize_power_plan -power_budget 100
commit_fp_rail
preroute_standard_cells -extend_for_multiple_connections  -extension_gap 10 -connect horizontal  -remove_floating_pieces  -do_not_route_over_macros  -fill_empty_rows  -port_filter_mode off -cell_master_filter_mode off -cell_instance_filter_mode off -voltage_area_filter_mode off -route_type {P/G Std. Cell Pin Conn}
place_opt  -power -cts
set_fix_hold [all_clocks]
set_max_area 0
set physopt_area_critical_range 0.1
clock_opt -fix_hold_all_clocks -no_clock_route
report_timing
route_zrt_group -all_clock_nets
route_zrt_auto
report_timing
verify_zrt_route
insert_stdcell_filler -cell_without_metal            "FILL64 FILL32 FILL16 FILL8 FILL4 FILL2 FILL1"            -connect_to_power {VDD} -connect_to_ground {VSS}
report_timing > timing.report
verify_zrt_route > route.report
report_area -physical > area.report