#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/Command.h>
#include <X11/Label.h>
#include <X11/Box.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Viewport.h>
#include <X11/Form.h>
#include <X11/Core.h>

Widget corridor_shell;
Widget corridor_form1;
extern Widget toplevel;
extern  Arg form_args[];
extern Arg label_args[];
extern Arg cmd_args[];
extern XtTranslations cmd_translations;
extern XtTranslations cmd4_translations;
extern XtTranslations cmd5_translations;
extern XtTranslations cell_translations;
extern XtTranslations form_translations;

extern void dispatche();

void quit(widget, closure, calldata)
	Widget widget;
	caddr_t closure;
	caddr_t calldata;
{
	XtDestroyWidget((Widget) closure);
	fprintf(stderr, "Bye Now !\n");
	exit(0);
}

Widget corridor_cmd5;

corridor()

{
   static Widget corridor_form2,
                corridor_label,
                corridor_cmd1,
                corridor_cmd2,
                cost_shell,
                cost_form,
                cost_cmd1,
                cost_cmd2,
                cost_cmd3,
                corridor_form3,
                corridor_cmd3,
                corridor_cmd4,
                corridor_form4,
                corridor_cmd6,
		corridor_form5,
                drain_shell,
                drain_form,
                drain_cmd1,
                drain_cmd2,
                drain_cmd3;

  static XtCallbackRec callback[3];
  static Arg args[4];

  XtArgVal width;

  corridor_shell = XtCreateApplicationShell ("corridor", 
                                             topLevelShellWidgetClass,
                                             toplevel, NULL, 0);

  corridor_form1 = XtCreateManagedWidget("", formWidgetClass, corridor_shell,
                                        form_args, 2);

  corridor_label = XtCreateManagedWidget("CORRIDOR TOOLS",labelWidgetClass,
                                       corridor_form1, label_args, 8);

  XtSetArg(form_args[2], XtNvertDistance,0);
  XtSetArg(form_args[3], XtNfromVert, corridor_label);

  corridor_form2 = XtCreateManagedWidget("", formWidgetClass, corridor_form1,
                                         form_args, 4);
  XtOverrideTranslations(corridor_form2, form_translations);

  XtSetArg(args[0], XtNwidth, &width);

  XtGetValues(corridor_label, args, 1);

  XtSetArg(cmd_args[1], XtNwidth, width-3);

  XtSetArg(cmd_args[3], XtNcallback, NULL);

  corridor_cmd1 = XtCreateManagedWidget("Cost analysis", commandWidgetClass,
                                         corridor_form2, cmd_args,4);
  XtOverrideTranslations(corridor_cmd1, cmd4_translations);
  XtSetArg(cmd_args[1], XtNwidth, 3);
 
  XtSetArg(cmd_args[4], XtNtranslations, NULL);
  XtSetArg(cmd_args[5],XtNfromHoriz, corridor_cmd1);
  XtSetArg(cmd_args[6],XtNhorizDistance, 0);

  corridor_cmd2 = XtCreateManagedWidget("", commandWidgetClass,
                                        corridor_form2, cmd_args, 7);

  XtOverrideTranslations(corridor_cmd2, cmd5_translations);

  XtSetArg(args[0], XtNtranslations, cell_translations);
  cost_shell = XtCreatePopupShell("cost", overrideShellWidgetClass,
                                  corridor_cmd2, args, 1);

  cost_form = XtCreateManagedWidget("", formWidgetClass, cost_shell,
                                         form_args, 2);
  XtSetArg(cmd_args[1], XtNwidth, width+6);
  XtSetArg(cmd_args[4], XtNtranslations, cmd_translations);

  cost_cmd1 =  XtCreateManagedWidget("Cost map", commandWidgetClass,
                                         cost_form, cmd_args,5);
  XtSetArg(cmd_args[5], XtNvertDistance,0);
  XtSetArg(cmd_args[6], XtNfromVert, cost_cmd1);

  cost_cmd2 = XtCreateManagedWidget("Start locations", commandWidgetClass,
                                     cost_form, cmd_args, 7);
  
  XtSetArg(cmd_args[6], XtNfromVert, cost_cmd2);

  cost_cmd3 = XtCreateManagedWidget("Run", commandWidgetClass,
                                     cost_form, cmd_args, 7);

  XtSetArg(form_args[3], XtNfromVert, corridor_form2);

  corridor_form3 = XtCreateManagedWidget("", formWidgetClass, corridor_form1,
                                         form_args, 4);
  XtOverrideTranslations( corridor_form3, form_translations);

  XtSetArg(cmd_args[1], XtNwidth, width-3);

  XtSetArg(cmd_args[3], XtNcallback, NULL);

  corridor_cmd3 = XtCreateManagedWidget("Drain analysis", commandWidgetClass,
                                         corridor_form3, cmd_args,4);
  XtOverrideTranslations(corridor_cmd3, cmd4_translations);
  XtSetArg(cmd_args[1], XtNwidth, 3);

  XtSetArg(cmd_args[4], XtNtranslations, NULL);
  XtSetArg(cmd_args[5],XtNfromHoriz, corridor_cmd3);
  XtSetArg(cmd_args[6],XtNhorizDistance, 0);

  corridor_cmd4 = XtCreateManagedWidget("", commandWidgetClass,
                                        corridor_form3, cmd_args, 7);

  XtOverrideTranslations(corridor_cmd4, cmd5_translations);

  XtSetArg(args[0], XtNtranslations, cell_translations);
  drain_shell = XtCreatePopupShell("drain", overrideShellWidgetClass,
                                  corridor_cmd4, args, 1);

  drain_form = XtCreateManagedWidget("", formWidgetClass, drain_shell,
                                         form_args, 2);
  XtSetArg(cmd_args[1], XtNwidth, width+6);
  XtSetArg(cmd_args[4], XtNtranslations, cmd_translations);

  drain_cmd1 =  XtCreateManagedWidget("Drain map", commandWidgetClass,
                                         drain_form, cmd_args,5);
  XtSetArg(cmd_args[5], XtNvertDistance,0);
  XtSetArg(cmd_args[6], XtNfromVert, drain_cmd1);

  drain_cmd2 = XtCreateManagedWidget("End locations", commandWidgetClass,
                                     drain_form, cmd_args, 7);

  XtSetArg(cmd_args[6], XtNfromVert, drain_cmd2);

  drain_cmd3 = XtCreateManagedWidget("Run", commandWidgetClass,
                                     drain_form, cmd_args, 7);

  XtSetArg(form_args[3], XtNfromVert, corridor_form3);

  corridor_form4 = XtCreateManagedWidget("", formWidgetClass, corridor_form1,
                                         form_args, 4);

  XtSetArg(cmd_args[1], XtNwidth, width);
	callbac                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               