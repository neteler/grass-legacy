
namespace eval DmToolBar {
    variable toolbar
}

proc DmToolBar::create { tb  } {
    global dmpath
    variable toolbar

    set toolbar $tb

    # DISPLAY/QUERY
    set bbox1 [ButtonBox $toolbar.bbox1 -spacing 0 -padx 1 -pady 1]
    # display
    $bbox1 add -image [image create photo -file "$dmpath/display.gif"] \
        -command "Dm::display" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Display layers"

    # display all
    $bbox1 add -image [image create photo -file "$dmpath/display.all.gif"] \
        -command "Dm::displayall" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Display all (default region)"

    # zoom
    $bbox1 add -image [image create photo -file "$dmpath/zoom.gif"] \
        -command "Dm::zoom" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Zoom"

    # query
    $bbox1 add -image [image create photo -file "$dmpath/query.gif"] \
        -command "Dm::query" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Query map"

    pack $bbox1 -side left -anchor w

    set sep1 [Separator $toolbar.sep1 -orient vertical]
    pack $sep1 -side left -fill y -padx 4 -anchor w


    # LAYER MANIPULATION
    set bbox2 [ButtonBox $toolbar.bbox2 -spacing 0 -padx 1 -pady 1]
    # add group
    $bbox2 add -image [image create photo -file "$dmpath/add.group.gif"] \
        -command "Dm::add group" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Create new group"

    $bbox2 add -image [image create photo -file "$dmpath/add.vector.gif"] \
        -command "Dm::add vector" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Create new vector"

    $bbox2 add -image [Bitmap::get cut] -command "Dm::delete" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Cut selection"
    # $bbox2 add -image [Bitmap::get copy] \
    #    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
    #    -helptext "Copy selection"
    # $bbox2 add -image [Bitmap::get paste] \
    #    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
    #    -helptext "Paste selection"

    pack $bbox2 -side left -anchor w

    set sep2 [Separator $toolbar.sep2 -orient vertical]
    pack $sep2 -side left -fill y -padx 4 -anchor w

    # FILE
    set bbox3 [ButtonBox $toolbar.bbox3 -spacing 0 -padx 1 -pady 1]

     $bbox3 add -image [Bitmap::get new] -command "Dm::new" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Create a new file"
    # $bbox3 add -image [Bitmap::get open] \
    #    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
    #    -helptext "Open an existing file"
    $bbox3 add -image [Bitmap::get save]  -command "Dm::save" \
        -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
        -helptext "Save file"

    pack $bbox3 -side left -anchor w
}

