#include "ntdef.h"
#include <stdio.h>
static struct NT_window *window_list=NULL;

static char buffer[500];

void
freeMemory(void *p) {
	if (p!=NULL) free(p);
}
void *
allocateMemory(int s) {
	void *p=NULL;
	if (s)
	{
		p=(void *)malloc(s);
		if (p) memset(p,0,s);
	}
	return p;
}


/*---------------------------------------------------*\
| Function: NT_new_window                             |
| Purpose:  Add a new window id to the Window table   |
| Return:   Pointer to the new Window structure.      |
\*---------------------------------------------------*/
struct NT_window *
NT_new_window()
{
	struct NT_window *new;
	xtrace("NT_new_window\n");
	new = (struct NT_window *) allocateMemory (sizeof(struct NT_window));
	new->next = window_list;
	new->child=NULL;
	new->min = 1;
	new->minx =0;
	new->miny =0;
	new->w = INVALID_HANDLE;
	new->hBitmap = INVALID_HANDLE;
	new->hDC = INVALID_HANDLE;
	window_list = new;
	cjh_printf("NEW window %x\n",window_list);
	return(window_list);
}
	
/*---------------------------------------------------*\
| Function: NT_delete_window                          |
| Purpose:  Remove a window from the window list      |
| Input:    w - pointer to window data                |
| Return:   TRUE if deleted                           |
\*---------------------------------------------------*/
int
NT_delete_window(struct NT_window *w)
{
	NT_window *f;
	xtrace("NT_delete_window\n");
	
	if (w->w != INVALID_HANDLE)
	{
		/* ShowWindow(w->w,SW_HIDE);*/
		DestroyWindow(w->w);
		w->w=INVALID_HANDLE;
	}
	if (w->hBitmap != INVALID_HANDLE)
	{
		DeleteObject(w->hBitmap);
		w->hBitmap = INVALID_HANDLE;
	}
	if (w->hDC != INVALID_HANDLE)
	{
		DeleteDC(w->hDC);
		w->hDC=INVALID_HANDLE;
	}
	
	if (window_list == w)
		window_list=w->next;
	else
	{
		for (f=window_list; f!=NULL && f->next!=w; f=f->next);
		if (f!=NULL)
			f->next = w->next;
	}
	freeMemory(w);
	return TRUE;
}

/*------------------------------------------------*\
| Function: NT_find_window_from_id                 |
| Purpose:  Find the window in the window list     |
|           from the HWND id of the window.        |
| Input:    w - Window id (Windows HWND)           |
| Return:   pointer to NT_window structure for w.  |
\*------------------------------------------------*/
struct NT_window *
NT_find_window_from_id(HWND w)
{
	struct NT_window *current = window_list;
	/* xtrace("NT_find_window_from_id\n"); */
	
	while ( current != NULL &&
			current->w != w )
		current = current->next;
	if(current)
		return(current);
	current=window_list;
	return NULL;
}

/*****************************************************************\

	Function: NT_add_child
	Inputs:   parent and child window IDs.
	Returned: 1

	Comments: When a child window is created (eg. client canvas) we
		  update our internal list of windows and their children.
		  New children are added to the front of the list.

\*****************************************************************/

int
NT_add_child(parent,child)
NT_window *parent,*child;
{
	struct NT_child *new;

	new=(struct NT_child *) allocateMemory (sizeof(struct NT_child));
	new->w=child;
	new->next = parent->child;
	parent->child=new;
	return(1);
}

struct NT_window *
NT_find_child(NT_window *w,unsigned long mask,
								unsigned long val)
{
	struct NT_window *ret = NULL;
	struct NT_child *child = NULL;
	if (w)
	{
		if ((w->mask&mask)==val) ret=w;
		child = w->child;
		while(!ret && child) {
			ret = NT_find_child(child->w, mask, val);
			child=child->next;
		}
	}
	return ret;
}



/*****************************************************************\

	Function: NT_del_child
	Inputs:   parent and child window IDs.
	Returned: TRUE if window is removed, FALSE otherwise.

	Comments: Finds child window if it exits, and it so removes it from
		  the window list.

\*****************************************************************/

int
NT_del_child(parent,child)
struct NT_window *parent;
struct NT_window *child;
{
	struct NT_child *current,*last;
	int status=FALSE;

	if (parent->child==NULL)
	{
	}
	else if (parent->child->w==child)
	{
		current = parent->child;
		parent->child=parent->child->next;
		freeMemory(current);
		status=TRUE;
	}
	else
	{
		last=parent->child;
		current=parent->child->next;
		while (current->w!=child && current!=NULL)
		{
			last=current;
			current=current->next;
		}
		if (current!=NULL)
		{
			last->next=current->next;
			freeMemory(current);
			status=TRUE;
		}
	}
	return(status);
}

/*****************************************************************\

	Function: WinMain
	Inputs:   instance, previous instance, command line arguments,
		  default start up.

	Comments: Called instead of main() as the execution entry point.

\*****************************************************************/
#ifdef NOTCYGWIN
#define MAX_COMMAND_ARGS 20
static HANDLE hInstance,hPrevInstance;
int APIENTRY
WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{
        static char *command_args[MAX_COMMAND_ARGS];
        static int num_command_args;
        static char proEng[] = "proe";
        char *wordPtr,*tempPtr;
        int i,quote;
	hInstance=hInst;
	hPrevInstance=hPrevInst;

        for (i=0;i<MAX_COMMAND_ARGS;i++)
          command_args[i] = NULL;

        wordPtr = lpCmdLine;
        quote = 0;
        num_command_args = 1;
        command_args[0] = proEng;
        while  (*wordPtr && (*wordPtr == ' ' || *wordPtr == '\t'))
           wordPtr++;
        if (*wordPtr == '\"')
        {
          quote = 1;
          wordPtr++;
        }
        if (!*wordPtr)
          main(0,NULL);
        else
        {
          while (*wordPtr && num_command_args < MAX_COMMAND_ARGS)
          {
            tempPtr = wordPtr;
            if (quote)
            {
              while (*tempPtr && *tempPtr != '\"')
                tempPtr++;
              quote = 0;
            }
            else
              while (*tempPtr && *tempPtr != ' ')
                tempPtr++;
            if (*tempPtr)
              *(tempPtr++) = '\0';
            command_args[num_command_args++] = wordPtr;
            wordPtr = tempPtr;
            while (*wordPtr && (*wordPtr == ' ' || *wordPtr == '\t'))
              wordPtr++;
            if (*wordPtr == '\"')
            {
              quote = 1;
              wordPtr++;
            }
          }
          main(num_command_args,command_args);
        }

}
#endif

static ATOM atom=0;
void
NT_SetAtom(ATOM class)
{
	atom = class;
}

HWND
NT_create_window(char *title,DWORD style,int x,int y,int w, int h,HWND parent)
{
	HMODULE hInst = NULL; /* GetModuleHandleA(NULL); */
	return CreateWindow((LPCTSTR)MAKELONG(atom,0),title,style,x,y,w,h,
								 parent,NULL,hInst,NULL);
}

HBRUSH NT_GetWindowBrush ( NT_window *window )
{
    HBRUSH hbrush = INVALID_HANDLE;
    if ( INVALID_HANDLE != window->bgp ) 
    {
        if ( INVALID_HANDLE != window->hBitmap ) 
        {
            hbrush = CreatePatternBrush ( window->hBitmap );
	    if ( NULL == hbrush ) {
	      NT_log ( "NT_GetWindowBrush: CreatePatternBrush failed - using bgp member.\n" );
	      hbrush = window->bgp;
	    }
        } 
        else 
        {
            hbrush = window->bgp;
        }
    } 
    else 
    {
        hbrush = window->bg;
    }
    return hbrush;
}
 
static int cknl(char *sz, ...)
{
   int i = 0;
   if (sz == NULL) return 0;
   while (sz[i] != '\0') i++;
   i--;
   if (sz[i] == '\n') return 1;
   return 0;
};

#ifdef LOGGING
void
NT_log ( char *fmt, ... )
{
    FILE *logger = fopen ( "logger.txt", "a" );
    if ( NULL != logger ) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(logger, fmt, ap);
        /*if (!cknl(fmt)) fputc('\n', logger); */
        fflush(stderr);
        fclose ( logger );
    }
    return;
}
#endif

#ifdef DEBUGGING
void
NT_debug ( char *fmt, ... )
{
    FILE *logger = fopen ( "debugger.txt", "a" );
    if ( NULL != logger ) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(logger, fmt, ap);
        /*        if (!cknl(fmt)) fputc('\n', logger); */
        fflush(stderr);
        fclose ( logger );
    }
    return;
}

void NT_decodeMsg(
    UINT message               /* type of message		     */
    )
{
    switch (message) {
    case WM_KILLFOCUS:
        NT_debug ( "NT_decodeMsg        WM_KILLFOCUS\n" );
        break;
    case WM_SETFOCUS:
        NT_debug ( "NT_decodeMsg        WM_SETFOCUS\n" );
        break;
    case WM_QUIT:
        NT_debug ( "NT_decodeMsg        WM_QUIT\n" );
        break;
    case WM_CLOSE:
        NT_debug ( "NT_decodeMsg        WM_CLOSE\n" );
        break;
    case WM_DESTROY:
        NT_debug ( "NT_decodeMsg        WM_DESTROY\n" );
        break;
    case WM_SYSCHAR:
        NT_debug ( "NT_decodeMsg        WM_SYSCHAR\n" );
        break;
    case WM_CHAR:
        NT_debug ( "NT_decodeMsg        WM_CHAR\n" );
        break;
    case WM_LBUTTONDBLCLK:
        NT_debug ( "NT_decodeMsg        WM_LBUTTONDBLCLK\n" );
        break;
    case WM_MBUTTONDBLCLK:
        NT_debug ( "NT_decodeMsg        WM_MBUTTONDBLCLK\n" );
        break;
    case WM_RBUTTONDBLCLK:
        NT_debug ( "NT_decodeMsg        WM_RBUTTONDBLCLK\n" );
        break;
    case USR_MapNotify:
        NT_debug ( "NT_decodeMsg        USR_MapNotify\n" );
        break;
    case USR_EnterNotify:
        NT_debug ( "NT_decodeMsg        USR_EnterNotify\n" );
        break;
    case WM_MOVE:
        NT_debug ( "NT_decodeMsg        WM_MOVE\n" );
        break;
#if defined(WIN9X)
    case WM_SIZING:
        NT_debug ( "NT_decodeMsg        WM_SIZING\n" );
        break;
#endif			
    case WM_SIZE:
        NT_debug ( "NT_decodeMsg        WM_SIZE\n" );
        break;
    case WM_DESTROYCLIPBOARD:
        NT_debug ( "NT_decodeMsg        WM_DESTROYCLIPBOARD\n" );
        break;
    case WM_PAINT:
        NT_debug ( "NT_decodeMsg        WM_PAINT\n" );
        break;
    case WM_LBUTTONDOWN:
        NT_debug ( "NT_decodeMsg        WM_LBUTTONDOWN\n" );
        break;
    case WM_MBUTTONDOWN:
        NT_debug ( "NT_decodeMsg        WM_MBUTTONDOWN\n" );
        break;
    case WM_RBUTTONDOWN:
        NT_debug ( "NT_decodeMsg        WM_RBUTTONDOWN\n" );
        break;
    case WM_MBUTTONUP:
        NT_debug ( "NT_decodeMsg        WM_MBUTTONUP\n" );
        break;
    case WM_LBUTTONUP:
        NT_debug ( "NT_decodeMsg        WM_LBUTTONUP\n" );
        break;
    case WM_RBUTTONUP:
        NT_debug ( "NT_decodeMsg        WM_RBUTTONUP\n" );
        break;
    case WM_MOUSEMOVE:
        NT_debug ( "NT_decodeMsg        WM_MOUSEMOVE\n" );
        break;
    case WM_MOUSEWHEEL:
        NT_debug ( "NT_decodeMsg        WM_MOUSEWHEEL\n" );
        break;
    case WM_ERASEBKGND:
        NT_debug ( "NT_decodeMsg        WM_ERASEBKGND\n" );
        break;
    case WM_KEYDOWN:
        NT_debug ( "NT_decodeMsg        WM_KEYDOWN\n" );
        break;
    default:
        /*NT_debug ( "NT_decodeMsg       WM_? %x\n", message );*/
        break;
    }
    return;
}


#endif
