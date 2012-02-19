"""!
@package location_wizard.base

@brief Location wizard - base classes

Classes:
 - base::BaseClass

(C) 2007-2011 by the GRASS Development Team

This program is free software under the GNU General Public License
(>=v2). Read the file COPYING that comes with GRASS for details.

@author Michael Barton
@author Jachym Cepicky
@author Martin Landa <landa.martin gmail.com>   
"""

import wx

class BaseClass(wx.Object):
    """!Base class providing basic methods"""
    def __init__(self):
        pass

    def MakeLabel(self, text = "", style = wx.ALIGN_LEFT, parent = None):
        """!Make aligned label"""
        if not parent:
            parent = self
        return wx.StaticText(parent = parent, id = wx.ID_ANY, label = text,
                             style = style)

    def MakeTextCtrl(self, text = '', size = (100,-1), style = 0, parent = None):
        """!Generic text control"""
        if not parent:
            parent = self
        return wx.TextCtrl(parent = parent, id = wx.ID_ANY, value = text,
                           size = size, style = style)

    def MakeButton(self, text, id = wx.ID_ANY, size = (-1,-1), parent = None):
        """!Generic button"""
        if not parent:
            parent = self
        return wx.Button(parent = parent, id = id, label = text,
                         size = size)
