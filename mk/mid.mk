#########################################################################
# these define the various directories which contain GRASS programs
# or files used by GRASS programs
BIN             = $(GISBASE)/bin
ETC             = $(GISBASE)/etc
BIN_INTER       = $(ETC)/bin/inter
BIN_CMD         = $(ETC)/bin/cmd
TXT             = $(GISBASE)/txt
MAN1            = $(GISBASE)/man/1
MAN2            = $(GISBASE)/man/2
MAN3            = $(GISBASE)/man/3
MAN4            = $(GISBASE)/man/4
MAN5            = $(GISBASE)/man/5
MAN6            = $(GISBASE)/man/6
HELP            = $(GISBASE)/man/help
HTML            = $(GISBASE)/documents
SCRIPTS         = $(GISBASE)/scripts

# other
CFLAGS      = -I$(INCLUDE_DIR) -I$(CONFIG_DIR) $(COMPILE_FLAGS) $(EXTRA_CFLAGS) $(USE_TERMIO)
AR          = $(LIBRULE)
MANROFF     = # 

# various source directories and libraries
SRC_X       = $(SRC)/xgrass
LIBDIR      = $(DSTDIR)/src/libes
X_LIBDIR    = $(DSTDIR)/src/xgrass/libes
INCLUDE_DIR = $(SRC)/include
CONFIG_DIR  = $(DSTDIR)/src/include

VASKLIB     = $(LIBDIR)/libvask.a

GISLIB      = $(LIBDIR)/libgis.a
G3DLIB      = $(LIBDIR)/libg3d.a
ICONLIB     = $(LIBDIR)/libicon.a
LOCKLIB     = $(LIBDIR)/liblock.a
IMAGERYLIB  = $(LIBDIR)/libI.a
ROWIOLIB    = $(LIBDIR)/librowio.a
COORCNVLIB  = $(LIBDIR)/libcoorcnv.a
SEGMENTLIB  = $(LIBDIR)/libsegment.a
GPROJLIB    = $(LIBDIR)/libproj.a
BTREELIB    = $(LIBDIR)/libbtree.a
IBTREELIB   = $(LIBDIR)/libibtree.a
GMATHLIB    = $(LIBDIR)/libgmath.a
DLGLIB      = $(LIBDIR)/libdlg.a
RASTERLIB   = $(LIBDIR)/libraster.a
DISPLAYLIB  = $(LIBDIR)/libdisplay.a
D_LIB       = $(LIBDIR)/libD.a
DATETIMELIB = $(LIBDIR)/libdatetime.a
DRIVERLIB   = $(LIBDIR)/driverlib.a
LINKMLIB    = $(LIBDIR)/liblinkm.a
BITMAPLIB   = $(LIBDIR)/libbitmap.a
XGILIB	    = $(X_LIBDIR)/libXgi.a
XGDLIB	    = $(X_LIBDIR)/libXgd.a
XPMLIB	    = $(X_LIBDIR)/libXpm.a


DIGLIB      = $(LIBDIR)/libdig.a
DIG2LIB     = $(LIBDIR)/libdig2.a
VECTLIB_REAL= $(LIBDIR)/libvect.a
VECTLIB     = $(VECTLIB_REAL) $(DIG2LIB)
DIG_ATTLIB  = $(LIBDIR)/libdig_atts.a

# triangulation libraries
SOSLIB      = $(LIBDIR)/libsos.a
LIALIB      = $(LIBDIR)/liblia.a
OPTRILIB    = $(LIBDIR)/liboptri.a
BASICLIB    = $(LIBDIR)/libbasic.a
GEOMLIB     = $(OPTRILIB) $(SOSLIB) $(LIALIB) $(BASICLIB)		

XDISPLAYLIB = $(LIBDIR)/libXdisplay.a
#########################################################################
