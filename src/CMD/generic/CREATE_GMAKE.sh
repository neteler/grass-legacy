:

if [ ! -d /usr/local/bin ]; then mkdir /usr/local/bin ; fi

# create gmake5 script to be used for local compiling
echo ":"                                   > $UNIX_BIN/gmake5
echo "SRC=$SRC/src"                       >> $UNIX_BIN/gmake5
echo "CMD=$SRC/src/CMD"                   >> $UNIX_BIN/gmake5
echo "HEADER=head"                        >> $UNIX_BIN/gmake5
echo "HASX=yes"                           >> $UNIX_BIN/gmake5
echo "HASMotif=no"                        >> $UNIX_BIN/gmake5
echo ". $SRC/src/CMD/generic/gmake.sh"    >> $UNIX_BIN/gmake5
chmod ugo+x $UNIX_BIN/gmake5

# create gmakelinks script to be used for linking after
# local compiling
echo ":"                                      > $UNIX_BIN/gmakelinks5
echo "GMAKE=$UNIX_BIN/gmake5"           >> $UNIX_BIN/gmakelinks5
echo ". $SRC/src/CMD/generic/MAKELINKS.sh"   >> $UNIX_BIN/gmakelinks5
chmod ugo+x $UNIX_BIN/gmakelinks5
