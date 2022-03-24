min_thickness=1.5;
depth=min_thickness;
m4_dia=3.3;
m4_head_height=2;

module hole(x, y, d) {
     translate([x, y, d]) cylinder(h=4.75, d=m4_dia, center=false, $fn=100);
}

edge_to_hole=min_thickness+(m4_dia/2);
width=36+(edge_to_hole*2);
length=width;
inner_offset=edge_to_hole*2;
inner_width=width-inner_offset*2;
inner_depth=min_thickness+m4_head_height;

module core() {
     // Outer platform
     difference () {
          cube([width, length, depth]);
          hole(edge_to_hole, edge_to_hole, 0);
          hole(width-edge_to_hole, edge_to_hole, 0);
          hole(edge_to_hole, width-edge_to_hole, 0);
          hole(width-edge_to_hole, width-edge_to_hole, 0);
     }
     // Solid inner platform
     translate([inner_offset, 0, depth-inner_depth])
          cube([inner_width, width, inner_depth]);
     translate([0, inner_offset, depth-inner_depth])
          cube([width, inner_width, inner_depth]);
}

cutout_offset=inner_offset+min_thickness;
cutout_width1=inner_width-min_thickness*2;
cutout_width2=width-min_thickness*2;
cutout_depth=inner_depth-min_thickness;
cutout_depth_offset=depth-cutout_depth;

board_width=14;
board_height=38.5;
board_depth=1.65;
board_width_offset=(width-board_width)/2;
board_height_edge_to_sensor_center=4.35;
board_height_offset=(width/2)-board_height_edge_to_sensor_center;
board_height_edge_to_hole_center=16.1;
board_hole_offset=board_height_offset+board_height_edge_to_hole_center;
rail_length=5;

difference () {
     core();
     // Inner cutout
     translate([cutout_offset, min_thickness, cutout_depth_offset])
          cube([cutout_width1, cutout_width2, cutout_depth]);
     translate([min_thickness, cutout_offset, cutout_depth_offset])
          cube([cutout_width2, cutout_width1, cutout_depth]);
     // Header cutout
     translate([width/2+rail_length/2, board_hole_offset+rail_length/2,
                depth-inner_depth]) cube([width/2-rail_length/2-edge_to_hole*2,
                                          width-board_hole_offset+rail_length/2,
                                          inner_depth]);
     // Board cutout
     translate([board_width_offset, board_height_offset, cutout_depth_offset])
          cube([board_width, board_height, inner_depth]);
     // Extra wall cutout
     translate([cutout_offset, width-min_thickness, cutout_depth_offset])
          cube([width/2-board_width/2-cutout_offset, min_thickness,
                cutout_depth]);
     // Board hole
     hole(board_width_offset+board_width/2, board_hole_offset, depth-inner_depth);
}

spacing=0;

// Platform extension
translate([width/2-rail_length/2, width, depth-inner_depth])
     cube([rail_length, board_height_offset+board_height-width+min_thickness+spacing, min_thickness]);

// Board rails
translate([width/2-rail_length/2, board_height_offset-min_thickness-spacing,
           cutout_depth_offset]) cube([rail_length, min_thickness, board_depth]);
translate([width/2-rail_length/2, board_height_offset+board_height+spacing,
           cutout_depth_offset]) cube([rail_length, min_thickness, board_depth]);
translate([board_width_offset-min_thickness-spacing, board_hole_offset-rail_length/2,
           cutout_depth_offset]) cube([min_thickness, rail_length, board_depth]);
translate([board_width_offset+board_width+spacing, board_hole_offset-rail_length/2,
           cutout_depth_offset]) cube([min_thickness, rail_length, board_depth]);
