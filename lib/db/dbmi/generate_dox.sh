#!/bin/sh

# dirty trick to generate module-pages for doxygen:
# Markus Neteler 10/2002

SEARCHSTRING="db"

echo "Extracting all $SEARCHSTRING\*() functions into local context pages..."

#get list of all C files:
LIST=`ls -1 *.c | sed 's/.c$//g'`

#build the doxygen module pages:
for i in $LIST
do

#write header for dox file:
echo "/*!
\defgroup $i DB $i routines
@{"					> $i.dox

# extract function names starting with $SEARCHSTRING_*():
cat $i.c | grep '\\fn '| sed '/^ *\\fn .*\($SEARCHSTRING[a-zA-Z0-9_]*\).*$/s//\1/p' |sort -u | sed 's/$/\
/g' >> $i.dox

#write footer:
echo "@}
*/"					>> $i.dox

done
