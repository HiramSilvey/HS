min_thickness=1;
frame_thickness=2;

shaft_height=2;
shaft_dia=6.89;
magnet_height=1.59;
magnet_dia=25.8;
outer_dia=magnet_dia+(2*min_thickness);
cutout_width=(outer_dia-frame_thickness)/2;

$fn=360;

union() {
  difference() {
    body();
    magnet();
    shaft();
    translate([frame_thickness/2, frame_thickness/2, 0]) cutout();
    translate([-(cutout_width+frame_thickness/2), frame_thickness/2, 0]) cutout();
    translate([frame_thickness/2, -(cutout_width+frame_thickness/2), 0]) cutout();
    translate([-(cutout_width+frame_thickness/2), -(cutout_width+frame_thickness/2), 0]) cutout();
  }
  difference() {
    translate([0, 0, magnet_height]) cylinder(h=shaft_height, d1=shaft_dia+frame_thickness*2, d2=shaft_dia+frame_thickness*2);
    shaft();
  }
}

module body() {
  cylinder(h=magnet_height+shaft_height, d1=outer_dia, d2=outer_dia);
}

module magnet() {
  translate([0,0,0]) {
    cylinder(h=magnet_height, d1=magnet_dia, d2=magnet_dia);
  }
}

module shaft() {
  translate([0,0,magnet_height]) {
    cylinder(h=shaft_height, d1=shaft_dia, d2=shaft_dia);
  }
}

module cutout() {
  cube([cutout_width, cutout_width, magnet_height+shaft_height]);
}