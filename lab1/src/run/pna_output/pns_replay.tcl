# reset
set_fp_rail_constraints -remove_all_layers
remove_fp_virtual_pad -all              
set_fp_rail_strategy -reset             
set_fp_block_ring_constraints -remove_all
set_fp_rail_region_constraints  -remove 
# global constraints
set_fp_rail_constraints -set_global 

# layer constraints
set_fp_rail_constraints -add_layer  -layer METAL7 -direction horizontal -max_strap 10 -min_strap 5 -max_width 5.000000 -min_width 2.000000 -spacing interleaving 
set_fp_rail_constraints -add_layer  -layer METAL6 -direction vertical -max_strap 10 -min_strap 5 -max_width 5.000000 -min_width 2.000000 -spacing interleaving 

# ring and strap constraints
set_fp_rail_constraints  -set_ring -nets { VDD VSS } -horizontal_ring_layer { METAL7 } -vertical_ring_layer { METAL6 } -ring_width 2.000000 -extend_strap core_ring 

# strategies
set_fp_rail_strategy  -use_tluplus true 

# block ring constraints

# regions

# virtual pads
create_fp_virtual_pad -net VDD -point { -2.640000 212.134995 }
create_fp_virtual_pad -net VDD -point { 379.315002 213.455002 }
create_fp_virtual_pad -net VSS -point { -2.640000 200.240005 }
create_fp_virtual_pad -net VSS -point { 376.670013 202.880005 }
create_fp_virtual_pad -net VDD -point { 0.000000 196.274994 }
create_fp_virtual_pad -net VDD -point { 376.670013 192.309998 }
create_fp_virtual_pad -net VSS -point { -1.320000 179.089996 }
create_fp_virtual_pad -net VSS -point { 377.994995 196.274994 }

# synthesize_fp_rail 
synthesize_fp_rail -nets { VDD VSS } -voltage_supply 1.500000 -power_budget 100.000000   
