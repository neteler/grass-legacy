/**********************************************************************
   creatshell.c - create a shell
 *********************************************************************/
/*******************************************************************************
Xgen was developed by Kurt Buehler, while at the Center for Advanced Decision
Support for Water and Environmental Systems (CADSWES), University of Colorado
at Boulder and at the Indiana Water Resources Research Center (IWRRC),
Purdue University for the U.S. Army Construction Engineering Research
Laboratory in support of the Geographical Resources Analysis Support
System (GRASS) software. The example scripts were developed by Ms. Christine
Poulsen of USA-CERL, much thanks goes to her for her work.

Permission to use, copy, modify and distribute without charge this software,
documentation, etc. is granted, provided that this comment is retained,
and that the names of Kurt Buehler, Christine Poulsen, CADSWES, IWRRC,
the University of Colorado at Boulder, Purdue University, or USA-CERL are not
used in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

The author disclaims all warranties with regard to this software, including
all implied warranties of merchantability and fitness, in no event shall
the author be liable for any special, indirect or consequential damages or
any damages whatsoever resulting from loss of use, data or profits,
whether in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of this
software.
*******************************************************************************/
#include "xgen.h"
#include <X11/StringDefs.h>

/***************************************************************
 * This routine takes a shell description and creates it.
 **************************************************************/
void
Create_Shell(s)
    Shell                          *s;
{
    Resource                       *resource;

    /***************************************************************
     * switch on the shell type and call the appropriate routine
     **************************************************************/
    switch (s->type) {
    case MENU:
        if (NULL != (resource = IndexResource((char *) s, SHELL, "updatefrom")))
            CreateMenu(s, True);
        else
            CreateMenu(s, False);
        break;
    case COMMANDBOARD:
        if (NULL != (resource = IndexResource((char *) s, SHELL, "pane"))) {
            if (resource->val.bval)
                CreateMessageBoard(s);
            else
                CreateCommandBoard(s);
        } else
            CreateCommandBoard(s);
        break;
    }
    /***************************************************************
     * add a destroy callback for the shell.
     * add a popdown callback for the shell.
     * if a shell is killed or popped down thru window manager
     * interaction or something, we need to know.
     **************************************************************/
    XtAddCallback(s->widget, XtNdestroyCallback, ShellDestroyedCB, (caddr_t) s);
    XtAddCallback(s->widget, XtNpopdownCallback, ShellPopdownCB, (caddr_t) s);
}
