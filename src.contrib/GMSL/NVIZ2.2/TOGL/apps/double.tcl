# double.tcl

# An Tk/OpenGL widget demo with two windows, one single buffered and the
# other double buffered.
# Brian Paul  (brianp@ssec.wisc.edu)



proc setup {} {
    wm title . "Single vs Double Buffering"

    frame .f1
    togl .f1.o1 -width 200 -height 200  -rgba true  -double false -depth true -ident Box1
    togl .f1.o2 -width 200 -height 200  -rgba true  -double true -depth true -ident Box2

    scale  .sx   -label {X Axis} -from 0 -to 360 -command {setAngle x} -orient horizontal
    scale  .sy   -label {Y Axis} -from 0 -to 360 -command {setAngle y} -orient horizontal
    button .btn  -text Quit -command exit

    bind .f1.o1 <B1-Motion> {
	motion_event [lindex [%W config -width] 4] \
		     [lindex [%W config -height] 4] \
		     %x %y
    }

    bind .f1.o2 <B1-Motion> {
	motion_event [lindex [%W config -width] 4] \
		     [lindex [%W config -height] 4] \
		     %x %y
    }

    pack .f1.o1 .f1.o2  -side left -padx 3 -pady 3 -fill both -expand t
    pack .f1  -fill both -expand t
    pack .sx  -fill x
    pack .sy  -fill x
    pack .btn -fill x
}



# This is called when mouse button 1 is pressed and moved in either of
# the OpenGL windows.
proc motion_event { width height x y } {
    .f1.o1 setXrot [expr 360.0 * $y / $height]
    .f1.o2 setXrot [expr 360.0 * $y / $height]
    .f1.o1 setYrot [expr 360.0 * ($width - $x) / $width]
    .f1.o2 setYrot [expr 360.0 * ($width - $x) / $width]

#    .sx set [expr 360.0 * $y / $height]
#    .sy set [expr 360.0 * ($width - $x) / $width]

    .sx set [getXrot]
    .sy set [getYrot]
}

# This is called when a slider is changed.
proc setAngle {axis value} {
    global xAngle yAngle zAngle

    switch -exact $axis {
	x {.f1.o1 setXrot $value
	   .f1.o2 setXrot $value}
	y {.f1.o1 setYrot $value
	   .f1.o2 setYrot $value}
    }
}

# Execution starts here!
setup


