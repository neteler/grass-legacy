"""!
@package nviz.py

@brief Nviz (3D view) module

This module implements 3D visualization mode for map display.

Map Display supports standard 2D view mode ('mapdisp' module) and
2.5/3D mode ('nviz_mapdisp' module).

(C) 2008, 2010 by the GRASS Development Team

This program is free software under the GNU General Public
License (>=v2). Read the file COPYING that comes with GRASS
for details.

@author Martin Landa <landa.martin gmail.com> (Google SoC 2008/2010)
"""

errorMsg = ''

import os
import sys

import wx
import globalvar
try:
    from wx import glcanvas
    import nviz_mapdisp
    sys.path.append(os.path.join(globalvar.ETCWXDIR, "nviz"))
    import grass6_wxnviz as wxnviz
    haveNviz = True
except ImportError, err:
    haveNviz = False
    errorMsg = err

if haveNviz:
    GLWindow = nviz_mapdisp.GLWindow
else:
    GLWindow = None
