/* This file contains the source code for the
 * Orchid Designer VGA video card device driver.
 *
 * Author: Paul W. Carlson    Nov. 1988
 */

static char *version = "1.06";

/*======== system include files ===============================*/
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/immu.h>
 
/*======== character map include file ==========================*/
#include "charmap.h"

/*======== stuff specific to the Orchid card ===================*/
#define HERC_COMP	0x03BF	/* Hercules compatibility reg. */
#define ATC_IND_SEL	0x03C0	/* ATC index select register */
#define ATC_IND_REG	0x03C0	/* ATC indexed register */
#define WR_MISC_OUT	0x03C2	/* Misc. output register (write) */
#define TS_IND_SEL	0x03C4	/* TS index select register */
#define TS_IND_REG	0x03C5	/* TS indexed register */
#define PAL_MASK	0x03C6	/* Palette mask register */
#define PAL_READ	0x03C7	/* Palette read register */
#define PAL_WRITE	0x03C8	/* Palette write register */
#define PAL_DATA	0x03C9	/* Palette data register */
#define RD_MISC_OUT	0x03CC	/* Misc. output register (read) */
#define GDC_SEG_SEL	0x03CD	/* GDC segment select register */
#define GDC_IND_SEL	0x03CE	/* GDC index select register */
#define GDC_IND_REG	0x03CF	/* GDC indexed register */
#define CRTC_IND_SEL	0x03D4	/* CRTC index select register */
#define CRTC_IND_REG	0x03D5	/* CRTC indexed register */
#define MODE_CONTROL	0x03D8	/* Display mode control register */
#define INP_STAT_1	0x03DA	/* Input status register one */

#define H_RES	800		/* horizontal resolution */
#define V_RES	600		/* vertical resolution */

static unsigned char G_TSvals[] = {
	0x03, 0x01, 0x0f, 0x00, 0x06, 0x00, 0x00, 0x88 };

static unsigned char G_CRTvals[] = {
	0x7a, 0x63, 0x64, 0x1d, 0x72, 0x9b, 0x73, 0xf0,
	0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x5b, 0xae, 0x57, 0x32, 0x00, 0x57, 0x73, 0xc3,
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static unsigned char G_ATCvals[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x41, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x10 };

static unsigned char G_GDCvals[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0f, 0xff };

static unsigned char A_TSvals[] = {
	0x02, 0x02, 0x03, 0x00, 0x02, 0x00, 0x00, 0x88 };

static unsigned char A_CRTvals[] = {
	0x5f, 0x4f, 0x50, 0x02, 0x55, 0x81, 0xbf, 0x1f,
	0x00, 0x4f, 0x0d, 0x0e, 0x00, 0x00, 0x01, 0x90,
	0x9c, 0x8e, 0x8f, 0x28, 0x1f, 0x96, 0xb9, 0xa3,
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static unsigned char A_ATCvals[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x0c, 0x00, 0x0f, 0x08, 0x00, 0x00, 0x00 };

static unsigned char A_GDCvals[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00, 0xff };

static unsigned char red_int[] = {
	 0,  0,  0,  0, 42, 42, 42, 42, 21, 21, 21, 21, 63, 63, 63, 63 };

static unsigned char grn_int[] = {
	 0,  0, 42, 42,  0,  0, 21, 42, 21, 21, 63, 63, 21, 21, 63, 63 };

static unsigned char blu_int[] = {
	 0, 42,  0, 42,  0, 42,  0, 42, 21, 63, 21, 63, 21, 63, 21, 63 };

/*============= ioctl functions and structure =============*/
void	vio_draw(),	vio_move(),	vio_putrast(),	vio_setcolor(),
	vio_setpix(),	vio_getpix(),	vio_horline(),	vio_erase(),
	vio_putpal(),	vio_getpal(),	vio_xhair(),	vio_rubbox(),
	vio_rubline(),	vio_setxhair(),	vio_setrbox(),	vio_setrline(),
	vio_textmode(),	vio_grafmode(),	vio_vertline(),	vio_circle(),
	vio_fillcirc(),	vio_bittext(),	vio_hilite(),	vio_unhilite(),
	vio_getrast(),	vio_getcurs(),	vio_putcurs(),	vio_version();

void (*vio_funct[28])() = {
	vio_draw,	vio_move,	vio_putrast,	vio_setcolor,
	vio_setpix,	vio_getpix,	vio_horline,	vio_erase,
	vio_putpal,	vio_getpal,	vio_xhair,	vio_rubbox,
	vio_rubline,	vio_setxhair,	vio_setrbox,	vio_setrline,
	vio_textmode,	vio_grafmode,	vio_vertline,	vio_circle,
	vio_fillcirc,	vio_bittext,	vio_hilite,	vio_unhilite,
	vio_getrast,	vio_getcurs,	vio_putcurs,	vio_version
};

static struct vio_struct {
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int arg5;
    unsigned char *ptr;
} vio_args;

static int *vio_argp;

/*============ variables global to the driver ========================*/
static union {
    unsigned int total_offset;	/* offset from start of video memory */
    struct {
    unsigned short offset;	/* offset from start of video segment */
    unsigned short number;	/* video segment number */
    } segment;
} video;

static struct {
    int x;			/* current x coordinate */
    int y;			/* current y coordinate */
    int color;			/* current plotting color */
} current;

static struct {
    int old_x;			/* old mouse cursor x coordinate */
    int old_y;			/* old mouse cursor y coordinate */
    int fixed_x;		/* fixed x coord. of box or line */
    int fixed_y;		/* fixed y coord. of box or line */
    int float_x;		/* floating x coordinate */
    int float_y;		/* floating y coordinate */
} mouse;

static int vio_busy;
static unsigned char *graphics_base, *text_base, *sptalloc();
static unsigned char curs_low, curs_high;
static unsigned char vio_screen[4000];

static unsigned char bit_val[] = { 128, 64, 32, 16, 8, 4, 2, 1 }; 

/*=========== mouse misc. global variables =========================*/
static int vio_x1, vio_y1, vio_x2, vio_y2;
static unsigned char hor_pix[26], ver_pix[26];
static unsigned int vio_voff;
static unsigned char box_top[H_RES], box_bot[H_RES], 
		     box_lt[V_RES],  box_rt[V_RES];
static int vio_old_dx, vio_old_dy, vio_xinc, vio_yinc;
static vio_err, vio_errinc, vio_errdec;
static unsigned char *vio_pix_ptr, vio_pixels[H_RES];

/**************** Function: vio_open *************************************
 *
 * This function opens the driver and allocates video memory to the driver.
 */
vio_open(dev, flag, otyp)
dev_t dev;
int flag, otyp;
{
    int off;

    /* Check if already open */
    if (vio_busy)
    {	u.u_error = EBUSY;
	return;
    }
    else vio_busy = 1;
    	
    /* Allocate graphics video memory to the driver */
    graphics_base = NULL;
    graphics_base = sptalloc(btoc(0x10000), PG_P | PG_LOCK, btoc(0xA0000), 0);
    if (graphics_base == NULL) 
    {	u.u_error = EIO;
	return;
    }

    /* Allocate text video memory to the driver */
    text_base = NULL;
    text_base = sptalloc(btoc(0x1000), PG_P | PG_LOCK, btoc(0xB8000), 0);
    if (text_base == NULL)
    {	u.u_error = EIO;
	return;
    }

    /* Clear the saved screen array */
    off = 0;
    while (off < 4000)
    {	*(vio_screen + (off++)) = 32;
	*(vio_screen + (off++)) = 7;
    }
}



/************* Function: vio_ioctl ********************************
 *
 * This function calls all the graphics functions built into the driver.
 */
vio_ioctl(dev, cmd, argp, mode)
dev_t dev;
int cmd, *argp, mode;
{
    vio_argp = argp;
    copyin(vio_argp, &vio_args, sizeof(struct vio_struct));
    (*vio_funct[cmd & 63])();
}


/************* Function: vio_close ********************************
 *
 * This function releases the video memory.
 */
vio_close(dev, flag, otyp)
dev_t dev;
int flag, otyp;
{
    /* Free graphics video memory */
    if (graphics_base != NULL) sptfree(graphics_base, btoc(0x10000), 0);

    /* Free text video memory */
    if (text_base != NULL) sptfree(text_base, btoc(0x1000), 0);

    /* Set flag to not busy */
    vio_busy = 0;
}


/*********** Function: vio_move *************************************
 *
 * This function moves the graphics cursor to vio_args.arg1,vio_args.arg2
 * and computes the video segment number and offset.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= x coordinate
 *	arg2	= y coordinate
 *  On exit -
 *	unchanged
 */
void vio_move()
{
    /* set the current coordinates */
    current.x = vio_args.arg1;
    current.y = vio_args.arg2;
    
    /* compute the video segment number and offset */
    video.total_offset = H_RES * current.y + current.x;
}


/************ Function: vio_draw *************************************
 *
 * This function draws a line from current.x,current.y to 
 * vio_args.arg1, vio_args.arg2;
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= x coordinate
 *	arg2	= y coordinate
 *  On exit -
 *	unchanged
 */
void vio_draw()
{
    register int dx, dy;
    int xinc, yinc, err, errinc, errdec;
    
    /* compute delta x, delta y, and x and y increments */
    dx = vio_args.arg1 - current.x;
    if (dx > 0) xinc = 1;
    else
    {	xinc = -1;
    	dx = -dx;
    }
    dy = vio_args.arg2 - current.y;
    if (dy > 0) yinc = H_RES;
    else
    {	yinc = -H_RES;
    	dy = -dy;
    }
    
    /* if delta x is greater than delta y */
    if (dx > dy)
    {	errinc = dy<<1;
    	errdec = errinc - (dx<<1);
	err = errinc - dx;
	for (dx++; ; dx--)
	{   outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	    *(graphics_base + video.segment.offset) = current.color;
	    if (dx == 1) break;
	    video.total_offset += xinc;
	    if (err < 0) err += errinc;
	    else
	    {	video.total_offset += yinc;
	    	err += errdec;
	    }
	}
    }
    
    /* if delta x is less or equal to delta y */
    else
    {	errinc = dx<<1;
    	errdec = errinc - (dy<<1);
	err = errinc - dy;
	for (dy++; ; dy--)
	{   outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	    *(graphics_base + video.segment.offset) = current.color;
	    if (dy == 1) break;
	    video.total_offset += yinc;
	    if (err < 0) err += errinc;
	    else
	    {	video.total_offset += xinc;
	    	err += errdec;
	    }
	}
    }
    
    /* update current coordinates */
    current.x = vio_args.arg1;
    current.y = vio_args.arg2;
}


/*********** Function: vio_putrast *****************************
 *
 * This function fills a rectangular area.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= left x coordinate
 *	arg2	= top y coordinate
 *	arg3	= right x coordinate
 *	arg4	= bottom y coordinate
 *	ptr	= address of user color array at least (arg3 - arg1 + 1) long
 *  On exit -
 *	unchanged
 */
void vio_putrast()
{

    register int y, npixels, offset, pixtoend;
    unsigned char seg;
    
    npixels = vio_args.arg3 - vio_args.arg1 + 1;
    if (npixels > 0)
    {  
        video.total_offset = H_RES * vio_args.arg2 + vio_args.arg1;
        offset = video.segment.offset;
        seg = video.segment.number & 7;
        outb(GDC_SEG_SEL, seg | 64);
        for (y = vio_args.arg2; y <= vio_args.arg4; y++)
        {   pixtoend = 0x10000 - offset;
	    if (npixels <= pixtoend)
	    {   copyin(vio_args.ptr, graphics_base + offset, npixels);
	        offset += H_RES;
	    }
	    else if (pixtoend >= 0)
	    {   copyin(vio_args.ptr, graphics_base + offset, pixtoend);
	        seg = (seg + 1) & 7;
    	        outb(GDC_SEG_SEL, seg | 64);
	        copyin(vio_args.ptr + pixtoend, graphics_base, 
			npixels - pixtoend);
	        offset = vio_args.arg1 + (seg << 6);
	    }
	    else
	    {   video.total_offset = H_RES * y + vio_args.arg1;
        	offset = video.segment.offset;
        	seg = video.segment.number & 7;
    	        outb(GDC_SEG_SEL, seg | 64);
	        copyin(vio_args.ptr, graphics_base + offset, npixels);
	        offset += H_RES;
	    }
        }
    }
}



/*********** Function: vio_setcolor **************************
 *
 * This function stores vio_args.arg1 in current.color.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= color
 *  On exit -
 *	unchanged
 */
void vio_setcolor()
{
    /* set the current color */
    current.color = vio_args.arg1;
}


/************* Function: vio_setpix **************************
 *
 * This function sets a pixel at vio_args.arg1,vio_args.arg2 in 
 * the current.color.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= x coordinate
 *	arg2	= y coordinate
 *  On exit -
 *	unchanged
 */
void vio_setpix()
{
    /* compute the video segment number and offset */
    video.total_offset = H_RES * vio_args.arg2 + vio_args.arg1;

    /* select the 64K video segment */
    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    
    /* set the pixel */
    *(graphics_base + video.segment.offset) = current.color;
}


/*********** Function: vio_getpix ***************************
 *
 * This function reads the color of the pixel at vio_args.arg1,vio_args.arg2.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= x coordinate
 *	arg2	= y coordinate
 *  On exit -
 *	arg3	= pixel color
 */
void vio_getpix()
{
    /* select the 64K video segment */
    video.total_offset = H_RES * vio_args.arg2 + vio_args.arg1;
    outb(GDC_SEG_SEL, ((video.segment.number << 3) | 64) & 127);
    
    /* return the pixel color */
    vio_args.arg3 = *(graphics_base + video.segment.offset);
    copyout(&vio_args, vio_argp, sizeof(struct vio_struct));
}
    

/************ Function: vio_horline ******************************
 *
 * This function draws a horizontal line (much fastrer than move()
 * and draw().
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= starting x coordinate
 *	arg2	= ending x coordinate
 *	arg3	= y coordinate
 *	ptr    	= pointer to user color array (800 bytes)
 *  On exit -
 *	unchanged
 */
void vio_horline()
{
    register int npixels, pixtoend;
    unsigned char seg;

    video.total_offset = H_RES * vio_args.arg3 + vio_args.arg1;
    seg = (unsigned char)(video.segment.number & 7);
    outb(GDC_SEG_SEL, (seg | 64) & 127);
    npixels = vio_args.arg2 - vio_args.arg1 + 1;
    if (npixels > 0)
    {	pixtoend = 0x10000 - video.segment.offset;
        if (npixels <= pixtoend)
            copyin(vio_args.ptr, graphics_base + video.segment.offset, 
		npixels);
    	else 
        {  copyin(vio_args.ptr, graphics_base + video.segment.offset, 
		pixtoend);
	   seg++;
           outb(GDC_SEG_SEL, (seg | 64) & 127);
           copyin(vio_args.ptr + pixtoend, graphics_base, npixels - pixtoend);
        }
    }
}


/************ Function: vio_erase ******************************
 *
 * This function erases the screen.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	ptr    	= pointer to user color array (1024 bytes)
 *  On exit -
 *	unchanged
 */
void vio_erase()
{
    register int offset, segment;

    for (segment = 0x40; segment < 0x47; segment++)
    {	outb(GDC_SEG_SEL, (unsigned char)segment);
	for (offset = 0x00; offset < 0x10000; offset += 0x400)
	    copyin(vio_args.ptr, graphics_base + offset, 0x400);
    }
    outb(GDC_SEG_SEL, 0x47);
    for (offset = 0x00; offset < 0x5300; offset += 0x400)
	    copyin(vio_args.ptr, graphics_base + offset, 0x400);
    copyin(vio_args.ptr, graphics_base + 0x5000, 0x300);
}


/*********** Function: vio_putpal ***************************
 *
 * This function sets the palette position's red, green, and blue
 * intensities.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= palette position
 *	arg2	= red intensity
 *	arg3	= green intensity
 *	arg4	= blue intensity
 *  On exit -
 *	unchanged
 */
void vio_putpal()
{
    /* Wait for start of vertical retrace */
    while (!(inb(INP_STAT_1) & 8));
    while (inb(INP_STAT_1) & 8);

    /* Now send the intensities */
    outb(PAL_WRITE, (unsigned char)vio_args.arg1);
    outb(PAL_DATA,  (unsigned char)vio_args.arg2);
    outb(PAL_DATA,  (unsigned char)vio_args.arg3);
    outb(PAL_DATA,  (unsigned char)vio_args.arg4);
}


/*********** Function: vio_getpal ***************************
 *
 * This function reads the red, green, and blue intensities for the position.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= palette position
 *  On exit -
 *	arg2	= red intensity
 *	arg3	= green intensity
 *	arg4	= blue intensity
 */
void vio_getpal()
{
    outb(PAL_READ, (unsigned char)vio_args.arg1);
    vio_args.arg2 = inb(PAL_DATA);
    vio_args.arg3 = inb(PAL_DATA);
    vio_args.arg4 = inb(PAL_DATA);
    copyout(&vio_args, vio_argp, sizeof(struct vio_struct));
}


/************ Function: vio_setxhair ******************************
 *
 * This function sets the initial position of the cross-hairs and
 * saves the pixels under them.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg3	= x coordinate
 *	arg4	= y coordinate
 *  On exit -
 *	unchanged
 */
void vio_setxhair()
{
    register int i, seg_num, index;
    
    /* set mouse coordinates and compute endpoints */
    mouse.float_x = mouse.old_x = vio_args.arg3;
    mouse.float_y = mouse.old_y = vio_args.arg4;
    vio_x2 = mouse.float_x + 12;
    vio_x1 = vio_x2 - 25;
    if (vio_x1 < 0) vio_x1 = 0;
    if (vio_x2 > H_RES - 1) vio_x2 = H_RES - 1;
    vio_y2 = mouse.float_y + 12;
    vio_y1 = vio_y2 - 25;
    if (vio_y1 < 0) vio_y1 = 0;
    if (vio_y2 > V_RES - 1) vio_y2 = V_RES - 1;
    
    /* do horizontal cross-hair */
    video.total_offset = H_RES * mouse.float_y + vio_x1;
    index = 0;
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++, index++)
    {	if (i == mouse.float_x) continue;
    	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	hor_pix[index] = *(graphics_base + video.segment.offset);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
    
    /* do vertical cross-hair */
    video.total_offset = H_RES * vio_y1 + mouse.float_x;
    index = 0;
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES, index++)
    {	if (i == mouse.float_y) continue;
    	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	ver_pix[index] = *(graphics_base + video.segment.offset);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
}



/***************** Function: vio_xhair *****************************
 *
 * This function erases the cross-hair at the previous location and, if
 * vio_args.arg5 > 0, moves the cross_hair to vio_args.arg3,vio_args.arg4.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg3	= floating x coordinate
 *	arg4	= floating y coordinate
 *	arg5	= flag (0 = just erase, > 0 = erase and replot)
 *  On exit -
 *	unchanged
 */
void vio_xhair()
{
    register int i, seg_num, index;
    
    /* erase horizontal cross-hair */
    video.total_offset = H_RES * mouse.old_y + vio_x1;
    index = 0;
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++, index++)
    {	if (i == mouse.old_x) continue;
    	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = hor_pix[index];
    }
    
    /* erase vertical cross-hair */
    video.total_offset = H_RES * vio_y1 + mouse.old_x;
    index = 0;
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES, index++)
    {	if (i == mouse.old_y) continue;
    	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = ver_pix[index];
    }

  if (vio_args.arg5)
  {
    
    /* save the coordinates and compute new endpoints */
    mouse.old_x = mouse.float_x = vio_args.arg3;
    mouse.old_y = mouse.float_y = vio_args.arg4;
    vio_x2 = mouse.float_x + 12;
    vio_x1 = vio_x2 - 25;
    if (vio_x1 < 0) vio_x1 = 0;
    if (vio_x2 > H_RES - 1) vio_x2 = H_RES - 1;
    vio_y2 = mouse.float_y + 12;
    vio_y1 = vio_y2 - 25;
    if (vio_y1 < 0) vio_y1 = 0;
    if (vio_y2 > V_RES - 1) vio_y2 = V_RES - 1;
    
    /* do the new horizontal cross-hair */
    video.total_offset = H_RES * mouse.float_y + vio_x1;
    index = 0;
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++, index++)
    {	if (i == mouse.float_x) continue;
    	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	hor_pix[index] = *(graphics_base + video.segment.offset);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
    
    /* do the new vertical cross-hair */
    video.total_offset = H_RES * vio_y1 + mouse.float_x;
    index = 0;
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES, index++)
    {	if (i == mouse.float_y) continue;
    	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	ver_pix[index] = *(graphics_base + video.segment.offset);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
  }
}


/*************** Function: vio_setrbox ****************************
 *
 * This function sets the initial position of the rubber box and
 * saves the pixels under it.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= fixed x coordinate
 *	arg2	= fixed y coordinate
 *	arg3	= floating x coordinate
 *	arg4	= floating y coordinate
 *  On exit -
 *	unchanged
 */
void vio_setrbox()
{
    register int i, seg_num;
    unsigned char *ptr;
    
    /* set mouse coordinates and compute endpoints */
    if (vio_args.arg3 >= vio_args.arg1)
    {	vio_x1 = vio_args.arg1;
    	vio_x2 = vio_args.arg3;
    }
    else
    {	vio_x1 = vio_args.arg3;
    	vio_x2 = vio_args.arg1;
    }
    if (vio_args.arg4 >= vio_args.arg2)
    {	vio_y1 = vio_args.arg2;
    	vio_y2 = vio_args.arg4;
    }
    else
    {	vio_y1 = vio_args.arg4;
    	vio_y2 = vio_args.arg2;
    }
    mouse.fixed_x = vio_args.arg1;
    mouse.fixed_y = vio_args.arg2;
    mouse.float_x = vio_args.arg3;
    mouse.float_y = vio_args.arg4;
    
    /* save top of box */
    vio_voff = H_RES * vio_y1 + vio_x1;
    video.total_offset = vio_voff;
    ptr = box_top;
    for (i = vio_x1; ; i++, video.total_offset++)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
	if (i == vio_x2) break;
    }
    
    /* save right side of box */
    ptr = box_rt;
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
    }
    
    /* save left side of box */
    video.total_offset = vio_voff;
    ptr = box_lt;
    for (i = vio_y1; ; i++, video.total_offset += H_RES)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
	if (i == vio_y2) break;
    }
    
    /* save bottom of box */
    ptr = box_bot;
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
    }
    
    /* draw top of box */
    video.total_offset = vio_voff;
    for (i = vio_x1; ; i++, video.total_offset++)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
	if (i == vio_x2) break;
    }
    
    /* draw right side of box */
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
    
    /* draw left side of box */
    video.total_offset = vio_voff;
    for (i = vio_y1; ; i++, video.total_offset += H_RES)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
	if (i == vio_y2) break;
    }
    
    /* draw bottom of box */
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
    
}


/**************** Function: vio_rubbox ***************************
 *
 * This function erases the rubber box at the previous location and,
 * if vio_args.arg5 > 0, moves the floating corner to 
 * vio_args.arg3,vio_args.arg4.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg3	= floating x coordinate
 *	arg4	= floating y coordinate
 *	arg5	= flag (0 = just erase, > 0 = erase and replot)
 *  On exit -
 *	unchanged
 */
void vio_rubbox()
{
    register int i, seg_num;
    unsigned char *ptr;
    
    /* erase top of old box */
    video.total_offset = vio_voff;
    ptr = box_top;
    for (i = vio_x1; ; i++, video.total_offset++)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = *ptr++;
	if (i == vio_x2) break;
    }
    
    /* erase right side of old box */
    ptr = box_rt;
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = *ptr++;
    }
    
    /* erase left side of old box */
    video.total_offset = vio_voff;
    ptr = box_lt;
    for (i = vio_y1; ; i++, video.total_offset += H_RES)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = *ptr++;
	if (i == vio_y2) break;
    }
    
    /* erase bottom of old box */
    ptr = box_bot;
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = *ptr++;
    }
    
  if (vio_args.arg5)
  {

    /* save the coordinates and compute new endpoints */
    mouse.float_x = vio_args.arg3;
    mouse.float_y = vio_args.arg4;
    if (mouse.float_x < mouse.fixed_x) 
    {	vio_x1 = mouse.float_x;
    	vio_x2 = mouse.fixed_x;
    }
    else 
    {	vio_x1 = mouse.fixed_x;
    	vio_x2 = mouse.float_x;
    }
    if (mouse.float_y < mouse.fixed_y) 
    {	vio_y1 = mouse.float_y;
    	vio_y2 = mouse.fixed_y;
    }
    else 
    {	vio_y1 = mouse.fixed_y;
    	vio_y2 = mouse.float_y;
    }
    
    /* save top of new box */
    vio_voff = H_RES * vio_y1 + vio_x1;
    video.total_offset = vio_voff;
    ptr = box_top;
    for (i = vio_x1; ; i++, video.total_offset++)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
	if (i == vio_x2) break;
    }
    
    /* save right side of new box */
    ptr = box_rt;
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
    }
    
    /* save left side of new box */
    video.total_offset = vio_voff;
    ptr = box_lt;
    for (i = vio_y1; ; i++, video.total_offset += H_RES)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
	if (i == vio_y2) break;
    }
    
    /* save bottom of new box */
    ptr = box_bot;
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++)
    {	seg_num = video.segment.number;
    	outb(GDC_SEG_SEL, ((seg_num << 3) | 64) & 127);
	*ptr++ = *(graphics_base + video.segment.offset);
    }
    
    /* draw top of new box */
    video.total_offset = vio_voff;
    for (i = vio_x1; ; i++, video.total_offset++)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
	if (i == vio_x2) break;
    }
    
    /* draw right side of new box */
    for (i = vio_y1; i <= vio_y2; i++, video.total_offset += H_RES)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
    
    /* draw left side of new box */
    video.total_offset = vio_voff;
    for (i = vio_y1; ; i++, video.total_offset += H_RES)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
	if (i == vio_y2) break;
    }
    
    /* draw bottom of new box */
    for (i = vio_x1; i <= vio_x2; i++, video.total_offset++)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = (i & 1) ? 0 : 215;
    }
  }
}


/*********** Function: vio_setrline ****************************
 *
 * This function sets the initial position of the rubber line and
 * saves the pixels under it.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= fixed x coordinate
 *	arg2	= fixed y coordinate
 *	arg3	= floating x coordinate
 *	arg4	= floating y coordinate
 *  On exit -
 *	unchanged
 */
void vio_setrline()
{
    register int seg_num, dx, dy;
    
    /* get endpoints */
    vio_x1 = vio_args.arg1;
    vio_y1 = vio_args.arg2;
    vio_x2 = vio_args.arg3;
    vio_y2 = vio_args.arg4;
    
    /* save pixels under line and then set them to white */
    vio_voff = H_RES * vio_y1 + vio_x1;
    video.total_offset = vio_voff;
    dx = vio_x2 - vio_x1;
    if (dx > 0) vio_xinc = 1;
    else
    {	vio_xinc = -1;
    	dx = -dx;
    }
    dy = vio_y2 - vio_y1;
    if (dy > 0) vio_yinc = H_RES;
    else
    {	vio_yinc = -H_RES;
    	dy = -dy;
    }
    vio_old_dx = dx;
    vio_old_dy = dy;
    vio_pix_ptr = vio_pixels;
    if (dx > dy)
    {	vio_errinc = dy<<1;
    	vio_errdec = vio_errinc - (dx<<1);
	vio_err = vio_errinc - dx;
	for (dx++; ; dx--)
	{   seg_num = video.segment.number;
	    outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	    *vio_pix_ptr++ = *(graphics_base + video.segment.offset);
	    *(graphics_base + video.segment.offset) = (dx & 1) ? 0 : 215;
	    if (dx == 1) break;
	    video.total_offset += vio_xinc;
	    if (vio_err < 0) vio_err += vio_errinc;
	    else
	    {	video.total_offset += vio_yinc;
	    	vio_err += vio_errdec;
	    }
	}
    }
    else
    {	vio_errinc = dx<<1;
    	vio_errdec = vio_errinc - (dy<<1);
	vio_err = vio_errinc - dy;
	for (dy++; ; dy--)
	{   seg_num = video.segment.number;
	    outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	    *vio_pix_ptr++ = *(graphics_base + video.segment.offset);
	    *(graphics_base + video.segment.offset) = (dy & 1) ? 0 : 215;
	    if (dy == 1) break;
	    video.total_offset += vio_yinc;
	    if (vio_err < 0) vio_err += vio_errinc;
	    else
	    {	video.total_offset += vio_xinc;
	    	vio_err += vio_errdec;
	    }
	}
    }
}


/***************** Function: vio_rubline **************************
 *
 * This function erases the rubber line at the previous location and, if
 * vio_args.arg5 > 0, moves the floating end to vio_args.arg3,vio_args.arg4.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg3	= floating x coordinate
 *	arg4	= floating y coordinate
 *	arg5	= flag (0 = just erase, > 0 = erase and replot)
 *  On exit -
 *	unchanged
 */
void vio_rubline()
{
    register int seg_num, dx, dy;    
    
    /* erase the old line */
    video.total_offset = vio_voff;
    vio_pix_ptr = vio_pixels;
    dx = vio_old_dx;
    dy = vio_old_dy;
    if (dx > dy)
    {	for (dx++; ; dx--)
	{   outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	    *(graphics_base + video.segment.offset) = *vio_pix_ptr++;
	    if (dx == 1) break;
	    video.total_offset += vio_xinc;
	    if (vio_err < 0) vio_err += vio_errinc;
	    else
	    {	video.total_offset += vio_yinc;
	    	vio_err += vio_errdec;
	    }
	}
    }
    else
    {	for (dy++; ; dy--)
	{   outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	    *(graphics_base + video.segment.offset) = *vio_pix_ptr++;
	    if (dy == 1) break;
	    video.total_offset += vio_yinc;
	    if (vio_err < 0) vio_err += vio_errinc;
	    else
	    {	video.total_offset += vio_xinc;
	    	vio_err += vio_errdec;
	    }
	}
    }
    
  if (vio_args.arg5)
  {

    /* get new endpoints */
    vio_x2 = vio_args.arg3;
    vio_y2 = vio_args.arg4;
    
    /* save pixels under new line and then set them to white */
    vio_voff = H_RES * vio_y1 + vio_x1;
    video.total_offset = vio_voff;
    dx = vio_x2 - vio_x1;
    if (dx > 0) vio_xinc = 1;
    else
    {	vio_xinc = -1;
    	dx = -dx;
    }
    dy = vio_y2 - vio_y1;
    if (dy > 0) vio_yinc = H_RES;
    else
    {	vio_yinc = -H_RES;
    	dy = -dy;
    }
    vio_old_dx = dx;
    vio_old_dy = dy;
    vio_pix_ptr = vio_pixels;
    if (dx > dy)
    {	vio_errinc = dy<<1;
    	vio_errdec = vio_errinc - (dx<<1);
	vio_err = vio_errinc - dx;
	for (dx++; ; dx--)
	{   seg_num = video.segment.number;
	    outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	    *vio_pix_ptr++ = *(graphics_base + video.segment.offset);
	    *(graphics_base + video.segment.offset) = (dx & 1) ? 0 : 215;
	    if (dx == 1) break;
	    video.total_offset += vio_xinc;
	    if (vio_err < 0) vio_err += vio_errinc;
	    else
	    {	video.total_offset += vio_yinc;
	    	vio_err += vio_errdec;
	    }
	}
    }
    else
    {	vio_errinc = dx<<1;
    	vio_errdec = vio_errinc - (dy<<1);
	vio_err = vio_errinc - dy;
	for (dy++; ; dy--)
	{   seg_num = video.segment.number;
	    outb(GDC_SEG_SEL, (((seg_num << 3) + seg_num) | 64) & 127);
	    *vio_pix_ptr++ = *(graphics_base + video.segment.offset);
	    *(graphics_base + video.segment.offset) = (dy & 1) ? 0 : 215;
	    if (dy == 1) break;
	    video.total_offset += vio_yinc;
	    if (vio_err < 0) vio_err += vio_errinc;
	    else
	    {	video.total_offset += vio_xinc;
	    	vio_err += vio_errdec;
	    }
	}
    }
  }
}



/***************** Function: vio_textmode **************************
 *
 * This function puts the Orchid card in 80 column color text mode.
 */
void vio_textmode()
{
    register int position, offset, ram_off;
    register int char_no, char_row;
    unsigned char index;
    int i;

    /*==== Set registers, load font, then set registers again ====*/
    for (i = 0; i < 2; i++)
    {

        /* Enable color by setting bit 0 of Misc. Output Register */
        outb(WR_MISC_OUT, inb(RD_MISC_OUT) | 1);

        /* Halt the timing sequencer and select BIOS ROM Address Map 1 */
        unprotect();
        outb(TS_IND_SEL, 0x00);
        outb(TS_IND_REG, 0x01);
        outb(TS_IND_SEL, 0x07);
        outb(TS_IND_REG, 0x08);

        /* Set clock and polarity for mode 0x03 */
        outb(WR_MISC_OUT, 0x63);

        /* Set CRTC register 0x24 (Compatibility Control) */
        outb(HERC_COMP, 0x03);
        outb(MODE_CONTROL, 0xa0);
        outb(CRTC_IND_SEL, 0x24);
        outb(CRTC_IND_REG, 0x00);

        /* Set the TS registers */
        for (index = 0x01; index < 0x05; index++)
        {   outb(TS_IND_SEL, index);
	    outb(TS_IND_REG, A_TSvals[index]);
        }
        outb(MODE_CONTROL, 0x29);
        outb(HERC_COMP, 0x01);
        outb(WR_MISC_OUT, 0x63);
        for (index = 0x06; index < 0x08; index++)
        {   outb(TS_IND_SEL, index);
	    outb(TS_IND_REG, A_TSvals[index]);
        }
        outb(TS_IND_SEL, 0x00);
        outb(TS_IND_REG, 0x02);

        /* Set the CRTC registers */
        unprotect();
        for (index = 0x00; index < 0x19; index++)
        {   outb(CRTC_IND_SEL, index);
	    outb(CRTC_IND_REG, A_CRTvals[index]);
        }
        outb(CRTC_IND_SEL, 0x23);
        outb(CRTC_IND_REG, 0x00);
        outb(CRTC_IND_SEL, 0x25);
        outb(CRTC_IND_REG, 0x00);

        /* Set the ATC registers */
        ATC_index_mode();
        for (index = 0x00; index < 0x17; index++)
    	{   while (!(inb(INP_STAT_1) & 8));
    	    while (inb(INP_STAT_1) & 8);
            outb(ATC_IND_SEL, index);
	    outb(ATC_IND_REG, A_ATCvals[index]);
        }

        /* Set the GDC registers */
        for (index = 0x00; index < 0x09; index++)
        {   outb(GDC_IND_SEL, index);
	    outb(GDC_IND_REG, A_GDCvals[index]);
        }
        outb(GDC_IND_SEL, 0x0d);
        outb(GDC_IND_REG, 0x00);
        outb(GDC_SEG_SEL, 0x40);

        /* If first time thru loop, load the font from ROM into RAM */
	if (!i)
	{

	    /* Disable the ATC */
	    outb(ATC_IND_SEL, 0x00);

	    /* Write enable bit plane 2 */
	    outb(TS_IND_SEL, 0x02);
	    outb(TS_IND_REG, 0x04);

	    /* Enable font selection, sequential access */
	    outb(TS_IND_SEL, 0x04);
	    outb(TS_IND_REG, 0x07);

	    /* Write mode 0 */
	    outb(GDC_IND_SEL, 0x05);
	    outb(GDC_IND_REG, 0x00);

	    /* Memory map 0xA0000 - 0xB0000 */
	    outb(GDC_IND_SEL, 0x06);
	    outb(GDC_IND_REG, 0x04);
	
	    /* Load the 8x16 chatacter map into RAM */
	    ram_off = 0;
	    offset = 0;
	    for (char_no = 0; char_no < 256; char_no++)
	    {   for (char_row = 0; char_row < 16; char_row++)
	    	{   *(graphics_base + (ram_off++)) = *(char_map + (offset)++);
	        }
	        ram_off += 16;
    	    }
	
    	    /* Load the palette with standard colors */
    	    outb(PAL_MASK, 0xff);
    	    for (position = 0; position < 0x10; position++)
    	    {	while (!(inb(INP_STAT_1) & 8));
    		while (inb(INP_STAT_1) & 8);
     		outb(PAL_WRITE, (unsigned char)position);
		outb(PAL_DATA, red_int[position]);
		outb(PAL_DATA, grn_int[position]);
		outb(PAL_DATA, blu_int[position]);
	    }
        }
    }

    /* Restore the text screen */
    for (offset = 0; offset < 4000; offset++)
     	*(text_base + offset) = *(vio_screen + offset);

    /* Enable the ATC */
    outb(ATC_IND_SEL, 0x20);
}



/***************** Function: vio_grafmode **************************
 *
 * This function puts the Orchid card in 800x600x256 graphics mode.
 */
void vio_grafmode()
{
    register int red, green, blue, text_offset;
    unsigned char index, position, R, G, B, seg_sel, *offset;

    /* Save the text screen */
    for (text_offset = 0; text_offset < 4000; text_offset++)
     	*(vio_screen + text_offset) = *(text_base + text_offset);

    /* Enable color by setting bit 0 of Misc. Output Register */
    outb(WR_MISC_OUT, inb(RD_MISC_OUT) | 1);

    /* Halt the timing sequencer and select BIOS ROM Address Map 1 */
    unprotect();
    outb(TS_IND_SEL, 0x00);
    outb(TS_IND_REG, 0x01);
    outb(TS_IND_SEL, 0x07);
    outb(TS_IND_REG, 0x08);

    /* Set clock and polarity for mode 0x30 */
    outb(WR_MISC_OUT, 0xef);

    /* Set CRTC register 0x24 (Compatibility Control) */
    outb(HERC_COMP, 0x03);
    outb(MODE_CONTROL, 0xa0);
    outb(CRTC_IND_SEL, 0x24);
    outb(CRTC_IND_REG, 0x00);

    /* Set the TS registers */
    for (index = 0x01; index < 0x05; index++)
    {	outb(TS_IND_SEL, index);
	outb(TS_IND_REG, G_TSvals[index]);
    }
    outb(MODE_CONTROL, 0x29);
    outb(HERC_COMP, 0x01);
    outb(WR_MISC_OUT, 0xef);
    for (index = 0x06; index < 0x08; index++)
    {	outb(TS_IND_SEL, index);
	outb(TS_IND_REG, G_TSvals[index]);
    }
    outb(TS_IND_SEL, 0x00);
    outb(TS_IND_REG, 0x03);

    /* Set the CRTC registers */
    unprotect();
    for (index = 0x00; index < 0x19; index++)
    {	outb(CRTC_IND_SEL, index);
	outb(CRTC_IND_REG, G_CRTvals[index]);
    }
    outb(CRTC_IND_SEL, 0x23);
    outb(CRTC_IND_REG, 0x00);
    outb(CRTC_IND_SEL, 0x25);
    outb(CRTC_IND_REG, 0x00);

    /* Set the ATC registers */
    ATC_index_mode();
    for (index = 0x00; index < 0x17; index++)
    {	outb(ATC_IND_SEL, index);
	outb(ATC_IND_REG, G_ATCvals[index]);
    }

    /* Set the GDC registers */
    for (index = 0x00; index < 0x09; index++)
    {	outb(GDC_IND_SEL, index);
	outb(GDC_IND_REG, G_GDCvals[index]);
    }
    outb(GDC_IND_SEL, 0x0d);
    outb(GDC_IND_REG, 0x00);
    outb(GDC_SEG_SEL, 0x40);

    /* Load the palette with six-level RGB colors */
    outb(PAL_MASK, 0xff);
    position = 0;
    for (red = 0; red < 6; red++)
    {	R = (unsigned char)(12.6 * (double)red);
	for (green = 0; green < 6; green++)
	{   G = (unsigned char)(12.6 * (double)green);
	    for (blue = 0; blue < 6; blue++)
	    {	B = (unsigned char)(12.6 * (double)blue);
	    	outb(PAL_WRITE, position++);
		outb(PAL_DATA, R);
		outb(PAL_DATA, G);
		outb(PAL_DATA, B);
	    }
	}
    }

    /* Put standard colors in palette positions 216 - 231 */
    for (position = 216; position < 232; position++)
    {	outb(PAL_WRITE, position);
	outb(PAL_DATA, red_int[position - 216]);
	outb(PAL_DATA, grn_int[position - 216]);
	outb(PAL_DATA, blu_int[position - 216]);
    }

    /* Clear video memory */
    for (seg_sel = 0x40; seg_sel < 0x48; seg_sel++)
    {	outb(GDC_SEG_SEL, seg_sel);
	for (offset = graphics_base; offset < graphics_base+0x10000; offset++)
		*offset = 0;
    }

    /* Enable the ATC */
    outb(ATC_IND_SEL, 0x20);
}


static ATC_index_mode()
{
    /* Set the ATC index/data flip-flop to index
     * mode by reading the Input Status 1 register.
     */
    inb(INP_STAT_1);
}


static unprotect()
{
    unsigned char CRTC11;

    /* Disable protection on TS register 7 and CRTC
     * registers 0 - 7 by setting bit 7 of CRTC
     * register 0x11 to 0.  Vertical interrupts are
     * disabled by setting bit 5 to 1.
     */
    outb(CRTC_IND_SEL, 0x11);
    CRTC11 = inb(CRTC_IND_REG);
    outb(CRTC_IND_SEL, 0x11);
    outb(CRTC_IND_REG, (CRTC11 & 0x7f) | 0x20);
}



/************ Function: vio_vertline *****************************
 *
 * This function draws a vertical line.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= starting y coordinate
 *	arg2	= ending y coordinate
 *	arg3	= x coordinate
 *  On exit -
 *	unchanged
 */
void vio_vertline()
{
    register int y;
    
    video.total_offset = H_RES * vio_args.arg1 + vio_args.arg3;
    for (y = vio_args.arg1; y <= vio_args.arg2; 
	y++, video.total_offset += H_RES)
    {	outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
	*(graphics_base + video.segment.offset) = current.color;
    }
}



/**************** Function: vio_circle ****************************
 *
 * This function plots a circle.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= center x coordinate
 *	arg2	= center y coordinate
 *	arg3	= radius
 *  On exit -
 *	unchanged
 */
void vio_circle()
{
    int x, y, cx, cy, r;
    long int dx1000, dy1000, prevdx, prevdy, savdx, savdy;
    long int radius, dx, dy, aspect, iaspect;
    double ratio;
    
    cx = vio_args.arg1;
    cy = vio_args.arg2;
    r  = vio_args.arg3;
    ratio = (double)(9 * H_RES * H_RES) / (double)(16 * V_RES * V_RES);
    aspect  = (long)(1000.0 / ratio);
    iaspect = (long)(1000.0 * ratio);
    radius = (long)r;
    dx = prevdx = radius;
    dx1000 = 1000 * dx;
    for ( dy = 0; dy < dx; dy++)
    {   dx1000 -= (dy * iaspect) / dx;
        dx = (dx1000 + 500) / 1000;
	savdx = dx;
	y = (int)dy;
	do
	{   x = (int)dx;
    	    video.total_offset = H_RES * (cy - y) + cx + x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
    	    video.total_offset = H_RES * (cy - y) + cx - x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
    	    video.total_offset = H_RES * (cy + y) + cx - x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
    	    video.total_offset = H_RES * (cy + y) + cx + x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
	} while (++dx < prevdx);
	dx = savdx;
	prevdx = dx;
    }
    prevdy = dy;
    dy1000 = 1000 * dy;
    dx--;
    do
    {	dy = (dy1000 + 500) / 1000;
	savdy = dy;
	x = (int)dx;
	do
	{   y = (int)dy;
    	    video.total_offset = H_RES * (cy - y) + cx + x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
    	    video.total_offset = H_RES * (cy - y) + cx - x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
    	    video.total_offset = H_RES * (cy + y) + cx - x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
    	    video.total_offset = H_RES * (cy + y) + cx + x;
    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    *(graphics_base + video.segment.offset) = current.color;
	} while (--dy > prevdy);
	dy = savdy;
	prevdy = dy;
	dy1000 += ((dx - 1) * aspect) / (long)y;
    } while (dx-- >= 0);		
}



/*************** Function: vio_fillcirc ****************************
 *
 * This function plots a filled circle.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= center x coordinate
 *	arg2	= center y coordinate
 *	arg3	= radius
 *	ptr    	= pointer to user color array (400 bytes)
 *  On exit -
 *	unchanged
 */
void vio_fillcirc()
{
    int x, y, cx, cy, r;
    long int dx1000, dy1000, prevdx, prevdy, savdx, savdy;
    long int radius, dx, dy, aspect, iaspect;
    double ratio;
    
    cx = vio_args.arg1;
    cy = vio_args.arg2;
    r  = vio_args.arg3;
    ratio = (double)(9 * H_RES * H_RES) / (double)(16 * V_RES * V_RES);
    aspect  = (long)(1000.0 / ratio);
    iaspect = (long)(1000.0 * ratio);
    radius = (long)r;
    dx = prevdx = radius;
    dx1000 = 1000 * dx;
    for ( dy = 0; dy < dx; dy++)
    {   dx1000 -= (dy * iaspect) / dx;
        dx = (dx1000 + 500) / 1000;
	savdx = dx;
	y = (int)dy;
	do
	{   x = (int)dx;
	    vio_args.arg1 = cx - x;
	    vio_args.arg2 = cx + x;
	    vio_args.arg3 = cy + y;
	    vio_horline();
	    vio_args.arg3 = cy - y;
	    vio_horline();
	} while (++dx < prevdx);
	dx = savdx;
	prevdx = dx;
    }
    prevdy = dy;
    dy1000 = 1000 * dy;
    dx--;
    do
    {	dy = (dy1000 + 500) / 1000;
	savdy = dy;
	x = (int)dx;
	do
	{   y = (int)dy;
	    vio_args.arg1 = cx - x;
	    vio_args.arg2 = cx + x;
	    vio_args.arg3 = cy + y;
	    vio_horline();
	    vio_args.arg3 = cy - y;
	    vio_horline();
	} while (--dy > prevdy);
	dy = savdy;
	prevdy = dy;
	dy1000 += ((dx - 1) * aspect) / (long)y;
    } while (dx-- >= 0);		
}


/*************** Function: vio_bittext ****************************
 *
 * This function plots bitmapped text starting at cordinates
 * vio_args.arg1,vio_args.arg2.  The characters are 8x14 pixels.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= starting x coordinate
 *	arg2	= starting y coordinate
 *	arg3	= length of text
 *	ptr	= pointer to text
 *  On exit -
 *	unchanged
 */
void vio_bittext() 
{ 
    int pat_off, char_num, col, row, x, y, leng;
    unsigned char text[128], *ptr; 

    x = vio_args.arg1;
    y = vio_args.arg2;
    leng = vio_args.arg3;
    copyin(vio_args.ptr, text, leng);
 
    y -= 14;
    for (char_num = 0; char_num < leng; char_num++) 
    {   pat_off = text[char_num] * 16; 
        ptr = char_map + pat_off; 
        for (col = 0; col < 8; col++) 
        {   for (row = 0; row < 16; row++) 
                if ((*(ptr+row)) & (bit_val[col])) 
    	    	{   video.total_offset = H_RES * (y + row) + (x + col);
    	    	    outb(GDC_SEG_SEL, (video.segment.number | 64) & 127);
    	    	    *(graphics_base + video.segment.offset) = current.color;
		}
        } 
        x += 8; 
    } 
} 


/*************** Function: vio_hilite ***********************
 *
 * This funcltion highlights a palette position.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= palette position
 *  On exit -
 *	unchanged
 */
void vio_hilite()
{
    unsigned char red, green, blue;

    /* Get the RGB intensites */
    outb(PAL_READ, vio_args.arg1);
    red   = inb(PAL_DATA);
    green = inb(PAL_DATA);
    blue  = inb(PAL_DATA);

    /* Subtract 21 from each intensity */
    red   = (red   + 21) & 63;
    green = (green + 21) & 63;
    blue  = (blue  + 21) & 63;

    /* Wait for start of vertical retrace */
    while (!(inb(INP_STAT_1) & 8));
    while (inb(INP_STAT_1) & 8);

    /* Now send the intensities */
    outb(PAL_WRITE, vio_args.arg1);
    outb(PAL_DATA, red);
    outb(PAL_DATA, green);
    outb(PAL_DATA, blue);
}


/*************** Function: vio_unhilite ***********************
 *
 * This funcltion un-highlights a palette position.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= palette position
 *  On exit -
 *	unchanged
 */
void vio_unhilite()
{
    unsigned char red, green, blue;
    
    /* Get the RGB intensites */
    outb(PAL_READ, vio_args.arg1);
    red   = inb(PAL_DATA);
    green = inb(PAL_DATA);
    blue  = inb(PAL_DATA);

    /* Add 21 to each intensity */
    red   = (red   - 21) & 63;
    green = (green - 21) & 63;
    blue  = (blue  - 21) & 63;

    /* Wait for start of vertical retrace */
    while (!(inb(INP_STAT_1) & 8));
    while (inb(INP_STAT_1) & 8);

    /* Now send the intensities */
    outb(PAL_WRITE, vio_args.arg1);
    outb(PAL_DATA, red);
    outb(PAL_DATA, green);
    outb(PAL_DATA, blue);
}


/************ Function: vio_getrast ******************************
 *
 * This function gets a horizontal line of pixels.
 *
 * Elements of structure vio_args:
 *   On entry - 
 *	arg1	= starting x coordinate
 *	arg2	= ending x coordinate
 *	arg3	= y coordinate
 *  On exit -
 *	ptr    	= pointer to user array (800 bytes)
 */
void vio_getrast()
{
    register int npixels, pixtoend;
    unsigned char seg;

    video.total_offset = H_RES * vio_args.arg3 + vio_args.arg1;
    outb(GDC_SEG_SEL, ((video.segment.number << 3) | 64) & 127);
    npixels = vio_args.arg2 - vio_args.arg1 + 1;
    if (npixels > 0)
    {	pixtoend = 0x10000 - video.segment.offset;
        if (npixels <= pixtoend)
           copyout(graphics_base + video.segment.offset, 
		vio_args.ptr, npixels);
    	else 
        { copyout(graphics_base + video.segment.offset, 
		vio_args.ptr, pixtoend);
          outb(GDC_SEG_SEL, (((++video.segment.number) << 3) | 64) & 127);
          copyout(graphics_base, vio_args.ptr + pixtoend, 
		npixels - pixtoend);
        }
    }
}



/************ Function: vio_getcurs ******************************
 *
 * This function saves the CRTC cursor address registers.
 */
void vio_getcurs()
{
	
    outb(CRTC_IND_SEL, 0x0e);
    curs_high = inb(CRTC_IND_REG);
    outb(CRTC_IND_SEL, 0x0f);
    curs_low = inb(CRTC_IND_REG);
}



/************ Function: vio_putcurs ******************************
 *
 * This function restores the CRTC cursor address registers.
 */
void vio_putcurs()
{
	
    outb(CRTC_IND_SEL, 0x0e);
    outb(CRTC_IND_REG, curs_high);
    outb(CRTC_IND_SEL, 0x0f);
    outb(CRTC_IND_REG, curs_low);
}


/************ Function: vio_version ******************************
 *
 * This function gets the current version number.
 */
void vio_version()
{
    int n;

    for (n = 0; version[n]; n++);
    copyout((unsigned char *)version, vio_args.ptr, n);
}
