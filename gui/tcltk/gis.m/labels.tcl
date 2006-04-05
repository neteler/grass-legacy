##########################################################################
# labels.tcl - vector labels layer options file for GRASS GIS Manager
# March 2006 Michael Barton, Arizona State University
# COPYRIGHT:	(C) 1999 - 2006 by the GRASS Development Team
#
#		This program is free software under the GNU General Public
#		License (>=v2). Read the file COPYING that comes with GRASS
#		for details.
#
##########################################################################

namespace eval GmLabels {
    variable array opt # labels current options
    variable count 1
    variable array tree # mon    
    variable array lfile # labels
    variable array lfilemask # labels
    variable optlist
    variable array dup # layer
}


proc GmLabels::create { tree parent } {
    variable opt
    variable count
    variable lfile
    variable lfilemask
    variable optlist
	variable dup
    global mon
    global gmpath
    global iconpath
    global guioptfont

    set node "labels:$count"

    set frm [ frame .labelsicon$count]
    set check [checkbutton $frm.check -font $guioptfont \
                           -variable GmLabels::opt($count,1,_check) \
                           -height 1 -padx 0 -width 0]

    image create photo labels_ico -file "$iconpath/module-d.labels.gif"
    set ico [label $frm.ico -image labels_ico -bd 1 -relief raised]
    
    pack $check $ico -side left

	#insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

    $tree insert $sellayer $parent $node \
	-text      "labels $count" \
	-window    $frm \
	-drawcross auto 

    set opt($count,1,_check) 1 
    set dup($count) 0

    set opt($count,1,map) "" 
	set opt($count,1,opacity) 1.0
    set opt($count,1,minreg) "" 
    set opt($count,1,maxreg) "" 
    set opt($count,1,ignore_rot) 0 
    set opt($count,1,mod) 1

	set optlist { _check map minreg maxreg}

    foreach key $optlist {
		set opt($count,0,$key) $opt($count,1,$key)
    } 
    
	# create files in tmp diretory for layer output
	set mappid [pid]
	set lfile($count) [eval exec "g.tempfile pid=$mappid"]
	set lfilemask($count) $lfile($count)
	append lfile($count) ".ppm"
	append lfilemask($count) ".pgm"
    
    incr count
    return $node
}

proc GmLabels::set_option { node key value } {
    variable opt
 
    set id [GmTree::node_id $node]
    set opt($id,1,$key) $value
}

proc GmLabels::select_labels { id } {
    set m [GSelect paint/labels]
    if { $m != "" } { 
        set GmLabels::opt($id,1,map) $m
        GmTree::autonamel $m
    }
}

# display labels options
proc GmLabels::options { id frm } {
    variable opt
    global gmpath
    global bgcolor
    global iconpath

    # Panel heading
    set row [ frame $frm.heading ]
    Label $row.a -text "Display labels for vector objects (created with v.label)" \
    	-fg MediumBlue
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

	#opacity
	set row [ frame $frm.opc]
	Label $row.a -text [G_msg "Opaque "]
	scale $row.b -from 1.0 -to 0.0 -showvalue 1  \
		-orient horizontal -length 300 -resolution 0.01 -fg "#656565"\
		-variable GmLabels::opt($id,1,opacity) 
	Label $row.c -text [G_msg " Transparent"]
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes	
	
    # labels name
    set row [ frame $frm.name ]
    Label $row.a -text [G_msg "Labels file:"]
    Button $row.b -image [image create photo -file "$iconpath/module-d.labels.gif"] \
        -highlightthickness 0 -takefocus 0 -relief raised -borderwidth 1  \
        -helptext [G_msg "labels file to display"] \
		-command "GmLabels::select_labels $id"
    Entry $row.c -width 40 -text "$opt($id,1,map)" \
		-textvariable GmLabels::opt($id,1,map) \
		-background white
    Label $row.d -text "   "
    Button $row.e -text [G_msg "Help"] \
		-image [image create photo -file "$iconpath/gui-help.gif"] \
		-command "run g.manual d.labels" \
		-background $bgcolor \
		-helptext [G_msg "Help"]
    pack $row.a $row.b $row.c $row.d $row.e -side left
    pack $row -side top -fill both -expand yes

    # display only in limited region size range
    set row [ frame $frm.region ]
    Label $row.a -text [G_msg "Display constraints:"]
    LabelEntry $row.b -label "min" -textvariable GmLabels::opt($id,1,minreg) \
            -width 8 -entrybg white
    LabelEntry $row.c -label "max" -textvariable GmLabels::opt($id,1,maxreg) \
            -width 8 -entrybg white
    Label $row.d -text [G_msg "region size"]
    pack $row.a $row.b $row.c $row.d -side left
    pack $row -side top -fill both -expand yes

    # ignore rotation
    set row [ frame $frm.ignore_rot ]
    checkbutton $row.a -text [G_msg " ignore rotation setting and draw horizontally"] -variable \
        GmLabels::opt($id,1,ignore_rot) 
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

    # launch v.label
    set row [ frame $frm.vlabel ]
    Label $row.a -text "Launch v.label to create labels file" 
    Button $row.b -text [G_msg "v.label"] \
	    -command "execute v.label"
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes

}

proc GmLabels::save { tree depth node } {
    variable opt
    variable optlist
    
    set id [GmTree::node_id $node]


    foreach key $optlist {
        GmTree::rc_write $depth "$key $opt($id,1,$key)"
    } 
}

proc GmLabels::display { node mod } {
    global mon
    global mapfile
    global maskfile
    global complist
    global opclist
    global masklist
    variable optlist
    variable lfile 
    variable lfilemask
    variable opt
    variable tree
    variable dup
    variable count
    
    set tree($mon) $GmTree::tree($mon)
    set id [GmTree::node_id $node]
    
    set opt($id,1,mod) $mod

    if { ! ( $opt($id,1,_check) ) } { return } 
    if { $opt($id,1,map) == "" } { return } 

    set cmd "d.labels labels=$opt($id,1,map)"

    if { $opt($id,1,minreg) != "" } { 
        append cmd " minreg=$opt($id,1,minreg)"
    }
    if { $opt($id,1,maxreg) != "" } { 
        append cmd " maxreg=$opt($id,1,maxreg)"
    }

	# Decide whether to run, run command, and copy files to temp
	GmCommonLayer::display_command [namespace current] $id $cmd
}


proc GmLabels::query { node } {
    puts "Query not supported for Paint labels layer"
}

proc GmLabels::duplicate { tree parent node id } {
    variable optlist
    variable lfile
    variable lfilemask
    variable opt
    variable count
	variable dup
	global guioptfont
	global iconpath

    set node "labels:$count"
	set dup($count) 1

    set frm [ frame .labelsicon$count]
    set check [checkbutton $frm.check -font $guioptfont \
                           -variable GmLabels::opt($count,1,_check) \
                           -height 1 -padx 0 -width 0]

    image create photo labels_ico -file "$iconpath/module-d.labels.gif"
    set ico [label $frm.ico -image labels_ico -bd 1 -relief raised]
    
    pack $check $ico -side left
	
	#insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

	if { $opt($id,1,map) == ""} {
	    $tree insert $sellayer $parent $node \
		-text      "labels $count" \
		-window    $frm \
		-drawcross auto
	} else {
	    $tree insert $sellayer $parent $node \
		-text      "$opt($id,1,map)" \
		-window    $frm \
		-drawcross auto
	}

	set opt($count,1,opacity) $opt($id,1,opacity)

	set optlist { _check map minreg maxreg}

    foreach key $optlist {
    	set opt($count,1,$key) $opt($id,1,$key)
		set opt($count,0,$key) $opt($count,1,$key)
    } 
	
	set id $count
	
	# create files in tmp directory for layer output
	set mappid [pid]
	set lfile($count) [eval exec "g.tempfile pid=$mappid"]
	set lfilemask($count) $lfile($count)
	append lfile($count) ".ppm"
	append lfilemask($count) ".pgm"

    incr count
    return $node
}
