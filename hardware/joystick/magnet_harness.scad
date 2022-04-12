spacing=0.4;

min_thickness=1;
shaft_height=2;
shaft_dia=7.92+spacing;
magnet_height=1.59;
magnet_dia=25.4+0.4;
outer_dia=magnet_dia+(2*min_thickness);
shaft_notch_height=1.5;
adjusted_notch=shaft_notch_height-spacing;
notch_depth=1.0-spacing;
cutout_width=(outer_dia-shaft_notch_height)/2;

$fn=360;

union() {
  difference() {
    body();
    magnet();
    shaft();
    translate([shaft_notch_height/2, shaft_notch_height/2, 0]) cutout();
    translate([-(cutout_width+shaft_notch_height/2), shaft_notch_height/2, 0]) cutout();
    translate([shaft_notch_height/2, -(cutout_width+shaft_notch_height/2), 0]) cutout();
    translate([-(cutout_width+shaft_notch_height/2), -(cutout_width+shaft_notch_height/2), 0]) cutout();
  }
  difference() {
    translate([0, 0, magnet_height]) cylinder(h=shaft_height, d1=shaft_dia+shaft_notch_height*2, d2=shaft_dia+shaft_notch_height*2);
    shaft();
  }
  bar();
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

module bar() {
  translate([-magnet_dia/2, -adjusted_notch/2, magnet_height])
  cube([magnet_dia, adjusted_notch, notch_depth]);
}

module cutout() {
  cube([cutout_width, cutout_width, magnet_height+shaft_height]);
}