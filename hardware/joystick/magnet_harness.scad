// All units are in mm.

spacing=0.1;  // Calibrated for the Prusa i3 FDM printer.

min_thickness=1;
shaft_height=2;
shaft_dia=6.85+spacing;
magnet_height=3.175;  // 0.125"
magnet_dia=19.05+spacing;  // 0.750"
outer_dia=magnet_dia+(2*min_thickness);

$fn=360;

difference() {
  body();
  magnet();
  shaft();
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
