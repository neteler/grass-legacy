# double.tcl

# An Tk/OpenGL widget demo with two windows, one single buffered and the
# other double buffered. 
# Bill Brown UI GMS Lab
# Summer 1996



proc setup {} {
    wm title . "Single vs Double Buffering"

    frame .f1 -width 200 -height 200
    togl .f1.o1 -rgba true  -double false -depth true -ident Box1 
    togl .f1.o2 -rgba true  -double true -depth true -ident Box2 

    scale  .sx   -label {X Axis} -from 0 -to 360 -command {setAngle x} -orient horizontal
    scale  .sy   -label {Y Axis} -from 0 -to 360 -command {setAngle y} -orient horizontal

    set bufmenu [tk_optionMenu .buftyp btype "single" "double" "auto"]
    $bufmenu entryconfigure 0 -command "do_buffer .f1.o1"
    $bufmenu entryconfigure 1 -command "do_buffer .f1.o2"
    $bufmenu entryconfigure 2 -command "do_bufbind"

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

#    pack .f1.o1 .f1.o2  -side left -padx 3 -pady 3 -fill both -expand t
    pack .f1  -fill both -expand t
    place .f1.o2 -relwidth 1.0 -relheight 1.0
    place .f1.o1 -relwidth 1.0 -relheight 1.0
    pack .sx  -fill x
    pack .sy  -fill x
    pack .buftyp  -fill x
    pack .btn -fill x
    raise .f1.o1
}

proc do_buffer { w } {
global btype

# raise correct window
    raise $w

# unbind auto actions
    bind .f1.o1 <ButtonPress-1> { }
    bind .sx <ButtonPress-1> { }
    bind .sy <ButtonPress-1> { }
    bind . <ButtonRelease-1> { }

# These sets shouldn't be necessary, but option button seemed to be confused

    if { 0 == [ string compare $w ".f1.o2" ] } {
	set btype double
    } else {
	set btype single
    }
}

proc do_bufbind { } {
global btype

    bind .f1.o1 <ButtonPress-1> {
	    raise .f1.o2
    }
    bind .sx <ButtonPress-1> {
	    raise .f1.o2
    }
    bind .sy <ButtonPress-1> {
	    raise .f1.o2
    }
    bind . <ButtonRelease-1> {
	    raise .f1.o1
    }
    set btype auto

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


