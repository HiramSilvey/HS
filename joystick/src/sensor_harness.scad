min_thickness=1.5;
depth=min_thickness;
m3_dia=2.5;

module hole(x, y) {
     translate([x, y, 0]) cylinder(h=depth, d=m3_dia, center=false, $fn=100);
}

width=53;
length=width;
edge_to_hole=3.75;
inner_offset=edge_to_hole*2;
inner_width=width-inner_offset*2;
inner_depth=4.75;

module core() {
     // Outer platform
     difference () {
          cube([width, length, depth]);
          hole(edge_to_hole, edge_to_hole);
          hole(width-edge_to_hole, edge_to_hole);
          hole(edge_to_hole, width-edge_to_hole);
          hole(width-edge_to_hole, width-edge_to_hole);
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

spacing=0.1;

board_width=14;
board_height=38.5;
board_depth=1.65+spacing;
board_width_offset=19.5;
board_height_offset=22.35;
board_hole_offset=38.6;
clip_length=5;

difference () {
     core();
     // Inner cutout
     translate([cutout_offset, min_thickness, cutout_depth_offset])
          cube([cutout_width1, cutout_width2, cutout_depth]);
     translate([min_thickness, cutout_offset, cutout_depth_offset])
          cube([cutout_width2, cutout_width1, cutout_depth]);
     // Header cutout
     translate([width/2+clip_length/2, board_hole_offset+clip_length/2,
                depth-inner_depth]) cube([width/2-clip_length/2-edge_to_hole*2,
                                          width-board_hole_offset+clip_length/2,
                                          inner_depth]);
     // Board cutout
     translate([board_width_offset, board_height_offset, cutout_depth_offset])
          cube([board_width, board_height, inner_depth]);
     // Extra wall cutout
     translate([cutout_offset, width-min_thickness, cutout_depth_offset])
          cube([width/2-board_width/2-cutout_offset, min_thickness,
                cutout_depth]);
}

clip_lip=0.1;
clip_lip_height=1.5;

module clip_top() {
     clip_top_points = [
          // Base
          [0, 0, 0],
          [clip_length, 0, 0],
          [0, min_thickness+clip_lip, 0],
          [clip_length, min_thickness+clip_lip, 0],
          // Back
          [0, 0, clip_lip_height],
          [clip_length, 0, clip_lip_height],
          ];

     clip_top_faces = [
          // Base
          [0, 1, 3, 2],
          // Back
          [0, 4, 5, 1],
          // Front
          [3, 5, 4, 2],
          // Left
          [1, 5, 3],
          // Right
          [0, 2, 4],
          ];

     polyhedron(clip_top_points, clip_top_faces);
}

// Clip bases
translate([width/2-clip_length/2, board_height_offset-min_thickness-spacing,
           cutout_depth_offset]) cube([clip_length, min_thickness, board_depth]);
translate([width/2-clip_length/2, board_height_offset+board_height+spacing,
           cutout_depth_offset]) cube([clip_length, min_thickness, board_depth]);
translate([board_width_offset-min_thickness-spacing, board_hole_offset-clip_length/2,
           cutout_depth_offset]) cube([min_thickness, clip_length, board_depth]);
translate([board_width_offset+board_width+spacing, board_hole_offset-clip_length/2,
           cutout_depth_offset]) cube([min_thickness, clip_length, board_depth]);

// Clip tops
translate([width/2-clip_length/2, board_height_offset-min_thickness-spacing,
           cutout_depth_offset+board_depth]) clip_top();
translate([width/2-clip_length/2+clip_length,
           board_height_offset+board_height+min_thickness+spacing,
           cutout_depth_offset+board_depth]) rotate([0, 0, 180]) clip_top();
translate([board_width_offset-min_thickness-spacing, board_hole_offset-clip_length/2+clip_length,
           cutout_depth_offset+board_depth]) rotate([0, 0, 270]) clip_top();
translate([board_width_offset+board_width+min_thickness+spacing, board_hole_offset-clip_length/2,
           cutout_depth_offset+board_depth]) rotate([0, 0, 90]) clip_top();

// Clip platform extension
translate([width/2-clip_length/2, width, depth-inner_depth])
cube([clip_length, board_height_offset+board_height-width+min_thickness+spacing, min_thickness]);
