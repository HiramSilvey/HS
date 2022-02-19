HEIGHT=0.2392;
HALF_MM=0.0393700787/2;
BOT_RAD=0.7679312825/2-HALF_MM;
TOP_RAD=0.8496779203/2-HALF_MM;
ANGLE=16.84;
NOTCH_DEPTH=0.029;
NOTCH_HALF_WIDTH=3;

module notch(angle) {
  polyhedron(
    points=[
      [BOT_RAD*cos(angle-NOTCH_HALF_WIDTH),BOT_RAD*sin(angle-NOTCH_HALF_WIDTH),0],
      [(BOT_RAD+NOTCH_DEPTH)*cos(angle),(BOT_RAD+NOTCH_DEPTH)*sin(angle),0],
      [BOT_RAD*cos(angle+NOTCH_HALF_WIDTH),BOT_RAD*sin(angle+NOTCH_HALF_WIDTH),0],
      [TOP_RAD*cos(angle-NOTCH_HALF_WIDTH),TOP_RAD*sin(angle-NOTCH_HALF_WIDTH),HEIGHT],
      [(TOP_RAD+NOTCH_DEPTH)*cos(angle),(TOP_RAD+NOTCH_DEPTH)*sin(angle),HEIGHT],
      [TOP_RAD*cos(angle+NOTCH_HALF_WIDTH),TOP_RAD*sin(angle+NOTCH_HALF_WIDTH),HEIGHT],
    ],
    faces=[
      [0,1,2],
      [0,3,4,1],
      [0,2,5,3],
      [3,5,4],
      [1,4,5,2]
    ]
  );
}

module gate_cutout() {
  notch(ANGLE);
  notch(90-ANGLE);
  notch(90+ANGLE);
  notch(180-ANGLE);
  notch(180+ANGLE);
  notch(270-ANGLE);
  notch(270+ANGLE);
  notch(360-ANGLE);
}

module ring() {
  difference() {
    cylinder(h=HEIGHT, r1=BOT_RAD+HALF_MM, r2=TOP_RAD+HALF_MM, $fn=360);
    cylinder(h=HEIGHT, r1=BOT_RAD, r2=TOP_RAD, $fn=360);
  }
}

difference() {
  union() {
    difference() {
      rotate(-22.5) import("third_party/jlf_oct_gate.stl");
      translate([0,0,0.0109]) cylinder(h=HEIGHT, r1=BOT_RAD, r2=TOP_RAD, $fn=360);
    }
    translate([0,0,0.0109]) ring();
  }
  translate([0,0,0.0109]) gate_cutout();
}