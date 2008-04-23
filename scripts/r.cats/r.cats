#!/bin/sh

# MODULE:       r.cats wrapper
# AUTHOR(S):    Glynn Clements
# COPYRIGHT:    (C) 2008 by the GRASS Development Team
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.

g.message -w "This module has been replaced by r.category" 
exec r.category "$@"
