#include "text.h"
#include "graphics.h"
#include "local_proto.h"

static int _top, _bottom, _left, _right;
static int _draw;
static int background = -1;
static int border = -1;
static int color = -1;
static int hcolor = -1;
static int hwidth = 0;
static float size = 3;
static double scale = 1.0;
static int rotation = 0;
static int width = 1;
static int xref = CENTER;
static int yref = CENTER;
static int newline = 0;
static int process_text (char *,int,int);
static int xshift (int);
static int yshift (int);


int draw_text (char *string, int x, int y)
{
    int i;
    BOX box;

    text_bounds (string, x, y, &box);

#ifdef DEBUG
 fprintf (stdout,"text(%s)\n",string);
 fprintf (stdout," ref(");
 switch (yref) {
  case CENTER: fprintf (stdout,"center"); break;
  case LOWER: fprintf (stdout,"lower"); break;
  case UPPER: fprintf (stdout,"upper"); break;
 }
 fprintf (stdout,",");
 switch (xref) {
  case CENTER: fprintf (stdout,"center"); break;
  case LEFT: fprintf (stdout,"left"); break;
  case RIGHT: fprintf (stdout,"right"); break;
 }
 fprintf (stdout,")\n");
 fprintf (stdout," at(%d,%d)->(%d,%d)\n",x,y,x+xshift(x),y+yshift(y));
 fprintf (stdout," _left(%d) _right(%d) _top(%d) _bottom(%d)\n",
	_left, _right, _top, _bottom);
#endif

    _draw = 1;

    set_width (1);
    if (background >= 0)
    {
        set_color (background);
        for (i = box.top; i<=box.bottom; i++)
            draw_line (box.left,i,box.right,i);
    }

    if (border >= 0)
    {
        set_color (border);
        draw_line (box.left,  box.top,    box.right, box.top);
        draw_line (box.left,  box.bottom, box.right, box.bottom);
        draw_line (box.left,  box.top,    box.left,  box.bottom);
        draw_line (box.right, box.top,    box.right, box.bottom);
    }

/* now add the text */
    if (color >= 0)
    {
        set_width (width);
	newline = graphics.width1 + graphics.width2;
    }
    if (hcolor >= 0 && hwidth > 0)
    {
        set_color (hcolor);
        set_width (width + hwidth);
	newline = graphics.width1 + graphics.width2;
        process_text (string, x+xshift(x), y+yshift(y));
        set_width (1);
    }
    if (color >= 0)
    {
        set_color (color);
        set_width (width);
        process_text (string, x+xshift(x), y+yshift(y));
        set_width (1);
    }

    return 0;
}

/* determines the bounds of the text to be printed */
/* box will shifted to the referenced location     */
/* _top,_bottom,_left,_right will be unshifted     */

int 
text_bounds (char *string, int x, int y, BOX *box)
{
    _top = _bottom = y;
    _left = _right = x;

    set_width (width + hwidth);
    newline = graphics.width1 + graphics.width2;
    _draw = 0;
    process_text (string, x, y);
    _draw = 1;

    _top    -= graphics.width1;
    _left   -= graphics.width1;
    _bottom += graphics.width2;
    _right  += graphics.width2;

    set_width (1);

    if (border >= 0 || background >= 0)
    {
        _top    -= 3;
        _left   -= 3;
        _right  += 3;
        _bottom += 3;
    }

    box->top    = _top    + yshift(y) ;
    box->bottom = _bottom + yshift(y) ;
    box->left   = _left   + xshift(x) ;
    box->right  = _right  + xshift(x) ;

    return 0;
}

static int xshift (int x)
{
    switch (xref)
    {
    case RIGHT:  return  x - _right;
    case CENTER: return  x - (_left + _right)/2;
    default:     return  x - _left;
    }

    return 0;
}

static int yshift (int y)
{
    switch (yref)
    {
    case UPPER:  return  y - _top;
    case CENTER: return  y - (_bottom + _top)/2;
    default:     return  y - _bottom;
    }

    return 0;
}

static int process_text (char *string, int x, int y)
{
    float rot;
    int X, Y;

    X = x;
    Y = y;
    rot = rotation ? 90.0 : 0.0 ;

/* look for \n and simulate a line feed */
    while (*string)
        if (string[0] == '\\' && (string[1] == 'n' || string[1] == 'N'))
        {
            if (rotation) x += XSIZE*size-1 + newline;
            else          y += YSIZE*size-1 + newline;
            X = x;
            Y = y;
            string += 2;
        }
        else        /* graph_char() will call text_line() */
            graph_char (&X, &Y, size, rot, *string++);

    return 0;
}

/* text_line() will be called by graph_char() */
int 
text_line (int x1, int y1, int x2, int y2)
{
    if (!_draw)
    {
        if (y1 < _top)    _top = y1;
        if (y1 > _bottom) _bottom = y1;
        if (y2 < _top)    _top = y2;
        if (y2 > _bottom) _bottom = y2;

        if (x1 < _left)   _left = x1;
        if (x1 > _right)  _right = x1;
        if (x2 < _left)   _left = x2;
        if (x2 > _right)  _right = x2;
    }
    else
    {
        draw_line (x1,y1,x2,y2);
    }

    return 0;
}
/****************** text configuration routines *******************/

int set_text_background (int b)    { background = b; return 0;}
int set_text_border (int b)        { border = b; return 0;}
int set_text_color (int c)         { color = c; hcolor = -1; return 0;}
int set_text_rotation (int r)      { rotation = r; return 0;}
int set_text_size (double s) { size = s * scale; return 0;}
int set_text_width (int w)         { width = w; hwidth = 0; return 0;}
int set_text_xref (int x)          { xref = x; return 0;}
int set_text_yref (int y)          { yref = y; return 0;}
int set_text_scale (double s) { scale = s; return 0;}
int set_text_hwidth (int w)        { hwidth = (w>0?(2*w):0); return 0;}
int set_text_hcolor (int c)        { hcolor = c; return 0;}

/* this next routine is designed to give a reasonable size to
 * text with the GRASS fonts.
 * It was computed by trial and error
 */
int set_reasonable_text_size (void)   { set_text_size(0.4); return 0;}
