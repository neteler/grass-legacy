# create_fifos.sh.in

DEV=/usr/local/grass-5.0b/dev

test -d ${DEV} || mkdir ${DEV}

rm -f ${DEV}/fifo.tmp
mkfifo ${DEV}/fifo.tmp
if [ -p ${DEV}/fifo.tmp ]; then
    MKFIFO=mkfifo
    MKNOD_ARG=
fi
rm -f ${DEV}/fifo.tmp

if [ -z "${MKFIFO}" ]; then
    mknod ${DEV}/fifo.tmp p
    if [ -p ${DEV}/fifo.tmp ]; then
        MKFIFO=mknod
        MKNOD_ARG=p
    fi
    rm -f ${DEV}/fifo.tmp
fi

i=0
while [ $i -le 20 ]
do
	if [ ! -p ${DEV}/fifo.${i}a ]; then
		rm -f ${DEV}/fifo.${i}a
		${MKFIFO} ${DEV}/fifo.${i}a ${MKNOD_ARG}
	fi
	if [ ! -p ${DEV}/fifo.${i}b ]; then
		rm -f ${DEV}/fifo.${i}b
		${MKFIFO} ${DEV}/fifo.${i}b ${MKNOD_ARG}
	fi
	i=`expr $i + 1`
done

chmod 0666 ${DEV}/fifo.*
