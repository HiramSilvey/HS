min_thickness=1.5;
depth=min_thickness;
m3_dia=2.5;
m4_dia=3.5;
m4_head_height=2;
m4_head_width=8;

edge_to_hole=m4_head_width/2;
width=36.3+(edge_to_hole*2);
length=width;
inner_offset=edge_to_hole*2;
inner_width=width-inner_offset*2;
inner_depth=min_thickness+m4_head_height;

cutout_depth=inner_depth-min_thickness;
cutout_depth_offset=depth-cutout_depth;

board_width=14;
board_height=38.5;
// board_depth=1.65;
board_width_offset=(width-board_width)/2;
board_height_edge_to_sensor_center=4.35;
board_height_offset=(width/2)-board_height_edge_to_sensor_center;
board_height_edge_to_hole_center=16.1;
board_hole_offset=board_height_offset+board_height_edge_to_hole_center;
rail_length=6;
notch_width=(board_width-rail_length)/2;


module hole(x, y, d, dia) {
     translate([x, y, d]) cylinder(h=4.75, d=dia, center=false, $fn=100);
}

module core() {
     // Outer platform
     difference () {
          cube([width, length, depth]);
          hole(edge_to_hole, edge_to_hole, 0, m4_dia);
          hole(width-edge_to_hole, edge_to_hole, 0, m4_dia);
          hole(edge_to_hole, width-edge_to_hole, 0, m4_dia);
          hole(width-edge_to_hole, width-edge_to_hole, 0, m4_dia);
     }
     // Solid inner platform
     translate([inner_offset, 0, depth-inner_depth])
          cube([inner_width, width, inner_depth]);
     translate([0, inner_offset, depth-inner_depth])
          cube([width, inner_width, inner_depth]);
}

difference () {
    core();
    // Header cutout
    translate([width/2+rail_length/2, board_hole_offset+rail_length/2,
              depth-inner_depth]) cube([width/2-rail_length/2-edge_to_hole*2,
                                       width-board_hole_offset+rail_length/2,
                                       inner_depth]);
    // Board cutout
    translate([board_width_offset, board_height_offset, cutout_depth_offset])
         cube([board_width, board_height, inner_depth]);
    // Board notches cutout
    translate([board_width_offset, board_height_offset-min_thickness, cutout_depth_offset])
         cube([notch_width, min_thickness, inner_depth]);
    translate([board_width_offset+board_width-notch_width, board_height_offset-min_thickness, cutout_depth_offset])
         cube([notch_width, min_thickness, inner_depth]);
    // Board hole
    hole(board_width_offset+board_width/2, board_hole_offset, depth-inner_depth, m3_dia);
}

// Platform extension
translate([width/2-board_width/2-min_thickness, width, depth-inner_depth])
     cube([board_width/2+rail_length/2+min_thickness, board_height_offset+board_height-width+min_thickness, min_thickness]);
translate([width/2-rail_length/2, board_height_offset+board_height,
           cutout_depth_offset]) cube([rail_length, min_thickness, cutout_depth]);
translate([width/2-board_width/2-min_thickness, width, cutout_depth_offset]) cube([min_thickness, board_height_offset+board_height-width+min_thickness, cutout_depth]);