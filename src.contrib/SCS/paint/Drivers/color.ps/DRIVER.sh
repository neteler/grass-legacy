:


HRES=0.013888889
VRES=0.013888889
NCHARS=1

TEXTSCALE=1
TEXTFUDGE=0
TEXTSPACE=1
BLOCKSIZE=1
BLOCKSPACE=1
NBLOCKS=1

MAPLP=${MAPLP-$HOME/color.ps}
WIDTH=${WIDTH-620}
HEIGHT=${HEIGHT-585}
cp /dev/null $MAPLP
export HRES VRES NCHARS MAPLP WIDTH HEIGHT
export TEXTSCALE TEXTSPACE TEXTFUDGE BLOCKSIZE BLOCKSPACE NBLOCKS

exec ${PAINT_DRIVER?}

#for B size paper
# WIDTH=750
# HEIGHT=950
