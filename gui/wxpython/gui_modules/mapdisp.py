"""
MODULE:    mapdisp.py

CLASSES:
    * Command
    * BufferedWindow
    * MapFrame
    * DecDialog
    * TextDialog
    * SavedRegion
    * MapApp

PURPOSE:   GIS map display canvas, with toolbar for various display
           management functions, and second toolbar for vector
           digitizing. Can be used either from GIS Manager or as p.mon backend

           Usage:
            python mapdisp.py monitor-identifier /path/to/command/file

AUTHORS:   The GRASS Development Team
           Michael Barton
           Jachym Cepicky
           Martin Landa

COPYRIGHT: (C) 2006-2007 by the GRASS Development Team
           This program is free software under the GNU General Public
           License (>=v2). Read the file COPYING that comes with GRASS
           for details.
"""

import os
import sys
import time
import glob
import math
import tempfile

import wx
import wx.aui
from threading import Thread

try:
    import subprocess
except:
    CompatPath = os.path.join( os.getenv("GISBASE"),"etc","wx")
    sys.path.append(CompatPath)
    from compat import subprocess

for gmpath in [os.path.join( os.getenv("GISBASE"),"etc","wx","gui_modules" ),
             os.path.join( os.getenv("GISBASE"),"etc","wx","icons" )]:
    sys.path.append(gmpath)

import render
import toolbars
import grassenv
import track
import menuform
import select
import disp_print
import gcmd
import dbm
import defaultfont
import histogram
import profile
from digit import Digit               as Digit
from digit import DigitCategoryDialog as DigitCategoryDialog
from digit import DigitZBulkDialog    as DigitZBulkDialog
from debug import Debug               as Debug
from icon  import Icons               as Icons

import images
imagepath = images.__path__[0]
sys.path.append(imagepath)

# for cmdlinef
cmdfilename = None

class Command(Thread):
    """
    Creates  thread, which will observe the command file and see, if there
    is new command to be executed
    """
    def __init__ (self,parent,Map):
        Thread.__init__(self)

        global cmdfilename

        self.parent = parent
        self.map = Map
        self.cmdfile = open(cmdfilename,"r")

    def run(self):
        """
        Run this in thread
        """
        dispcmd = []
        while 1:
            self.parent.redraw = False
            line = self.cmdfile.readline().strip()
            if line == "quit":
                break

            if line:
                try:
                    Debug.msg (3, "Command.run(): cmd=%s" % (line))

                    self.map.AddLayer(item=None, type="raster",
                                      name='',
                                      command=line,
                                      l_opacity=1)

                    self.parent.redraw =True

                except Exception, e:
                    print "Command Thread: ",e

            time.sleep(0.1)

        sys.exit()

class BufferedWindow(wx.Window):
    """
    A Buffered window class.

    When the drawing needs to change, you app needs to call the
    UpdateMap() method. Since the drawing is stored in a bitmap, you
    can also save the drawing to file by calling the
    SaveToFile(self,file_name,file_type) method.
    """

    def __init__(self, parent, id,
                 pos = wx.DefaultPosition,
                 size = wx.DefaultSize,
                 style=wx.NO_FULL_REPAINT_ON_RESIZE,
                 Map=None, tree=None):

        wx.Window.__init__(self, parent, id, pos, size, style)
        self.parent = parent
        self.Map = Map
        self.tree = tree

        #
        # Flags
        #
        self.resize = False # indicates whether or not a resize event has taken place
        self.dragimg = None # initialize variable for map panning

        #
        # Variable for drawing on DC
        #
        self.pen = None      # pen for drawing zoom boxes, etc.
        self.polypen = None  # pen for drawing polylines (measurements, profiles, etc)
        self.polycoords = [] # List of wx.Point tuples defining a polyline
        self.lineid = None   # ID of rubber band line
        # ID of poly line resulting from cumulative rubber band lines (e.g. measurement)
        self.plineid = None

        #
        # Event bindings
        #
        self.Bind(wx.EVT_PAINT,        self.OnPaint)
        self.Bind(wx.EVT_SIZE,         self.OnSize)
        self.Bind(wx.EVT_IDLE,         self.OnIdle)
        self.Bind(wx.EVT_MOTION,       self.MouseActions)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.MouseActions)

        #
        # Render output objects
        #
        self.mapfile = None   # image file to be rendered
        self.img = ""         # wx.Image object (self.mapfile)
        # used in digitization tool (do not redraw vector map)
        self.imgVectorMap = None
        self.ovldict = {}     # list of images for overlays
        self.ovlcoords = {}   # positioning coordinates for decoration overlay
        self.ovlchk = {}      # showing/hiding decorations
        self.imagedict = {}   # images and their PseudoDC ID's for painting and dragging
        self.crop = {}        # coordinates to crop overlays to their data, indexed by image ID
        self.select = {}      # selecting/unselecting decorations for dragging
        self.textdict = {}    # text, font, and color indexed by id
        self.currtxtid = None # PseudoDC id for currently selected text

        #
        # Zoom objects
        #
        self.zoomhistory = [] # list of past zoom extents
        self.currzoom = 0 # current set of extents in zoom history being used

        #
        # mouse attributes like currently pressed buttons, position on
        # the screen, begin and end of dragging, and type of drawing
        #
        self.mouse = {
            'l': False,
            'r': False,
            'm': False,
            'pos': [None, None],
            'begin': [0, 0],
            'end': [0, 0],
            'use': "pointer",
            'box': "point"
            }

        self.zoomtype = 1   # 1 zoom in, 0 no zoom, -1 zoom out
        self.hitradius = 10 # distance for selecting map decorations

        self.Map.SetRegion() # make sure that extents are updated at init

        # OnSize called to make sure the buffer is initialized.
        # This might result in OnSize getting called twice on some
        # platforms at initialization, but little harm done.
        self.OnSize(None)

        # create PseudoDC used for background map, map decorations like scales and legends
        self.pdc = wx.PseudoDC()
        # used for digitization tool
        self.pdcVector = None
        # pseudoDC for temporal objects (select box, measurement tool, etc.)
        self.pdcTmp = wx.PseudoDC()
        self.redrawAll = True # redraw all pdc layers, Tmp layer is redrawn always (speed issue)

        # will store an off screen empty bitmap for saving to file
        self._buffer = ''

        self.Bind(wx.EVT_ERASE_BACKGROUND, lambda x:None)

        # vars for handling mouse clicks
        self.dragid   = -1
        self.lastpos  = (0, 0)

    def Draw(self, pdc, img=None, drawid=None, pdctype='image', coords=[0,0,0,0]):
        """
        Draws map and overlay decorations
        """
        if drawid == None:
            if pdctype == 'image' :
                drawid = imagedict[img]
            elif pdctype == 'clear':
                drawid == None
            else:
                drawid = wx.NewId()
        else:
            self.ovlcoords[drawid] = coords
            self.ovlchk[drawid] = True
            pdc.SetId(drawid)
            self.select[drawid] = False

        pdc.BeginDrawing()

        if drawid != 99:
            bg = wx.TRANSPARENT_BRUSH
        else:
            bg = wx.Brush(self.GetBackgroundColour())

        pdc.SetBackground(bg)
        #pdc.Clear()
        self.Refresh()

        Debug.msg (5, "BufferedWindow.Draw(): id=%s, pdctype=%s, coord=%s" % \
                       (drawid, pdctype, coords))

        if pdctype == 'clear': # erase the display
            bg = wx.WHITE_BRUSH
            # bg = wx.Brush(self.GetBackgroundColour())
            pdc.SetBackground(bg)
            pdc.Clear()
            self.Refresh()
            pdc.EndDrawing()
            return

        if pdctype == 'image': # draw selected image
            bitmap = wx.BitmapFromImage(img)
            w,h = bitmap.GetSize()
            pdc.DrawBitmap(bitmap, coords[0], coords[1], True) # draw the composite map
            pdc.SetIdBounds(drawid, (coords[0],coords[1],w,h))

        elif pdctype == 'box': # draw a box on top of the map
            if self.pen:
                pdc.SetBrush(wx.Brush(wx.CYAN, wx.TRANSPARENT))
                pdc.SetPen(self.pen)
                x2 = max(coords[0],coords[2])
                x1 = min(coords[0],coords[2])
                y2 = max(coords[1],coords[3])
                y1 = min(coords[1],coords[3])
                rwidth = x2-x1
                rheight = y2-y1
                rect = wx.Rect(x1,y1,rwidth,rheight)
                pdc.DrawRectangleRect(rect)
                pdc.SetIdBounds(drawid,rect)
                self.ovlcoords[drawid] = coords

        elif pdctype == 'line': # draw a line on top of the map
            if self.pen:
                pdc.SetBrush(wx.Brush(wx.CYAN, wx.TRANSPARENT))
                pdc.SetPen(self.pen)
                pdc.DrawLine(coords[0], coords[1], coords[2], coords[3])
                pdc.SetIdBounds(drawid,(coords[0], coords[1], coords[2], coords[3]))
                self.ovlcoords[drawid] = coords

        elif pdctype == 'polyline': # draw a polyline on top of the map
            if self.polypen:
                pdc.SetBrush(wx.Brush(wx.CYAN, wx.TRANSPARENT))
                pdc.SetPen(self.polypen)
                pdc.DrawLines(coords)

                # get bounding rectangle for polyline
                xlist = []
                ylist = []
                if len(coords) > 0:
                    for point in coords:
                        x,y = point
                        xlist.append(x)
                        ylist.append(y)
                    x1=min(xlist)
                    x2=max(xlist)
                    y1=min(ylist)
                    y2=max(ylist)
                    pdc.SetIdBounds(drawid,(x1,y1,x2,y2))
                    self.ovlcoords[drawid] = [x1,y1,x2,y2]

        elif pdctype == 'point': # draw point
            if self.pen:
                pdc.SetPen(self.pen)
                pdc.DrawPoint(coords[0], coords[1])
                coordsBound = (coords[0] - 5,
                               coords[1] - 5,
                               coords[0] + 5,
                               coords[1] + 5)
                pdc.SetIdBounds(drawid, coordsBound)
                self.ovlcoords[drawid] = coords

        elif pdctype == 'text': # draw text on top of map
            text = img[0]
            rotation = float(img[3])
            w,h = self.GetFullTextExtent(img[0])[0:2]
            pdc.SetFont(img[1])
            pdc.SetTextForeground(img[2])
            coords,w,h = self.TextBounds(img,coords)
            if rotation == 0:
                pdc.DrawText(img[0], coords[0], coords[1])
            else:
                pdc.DrawRotatedText(img[0], coords[0], coords[1], rotation)
            pdc.SetIdBounds(drawid, (coords[0], coords[1], w, h))
            self.ovlcoords[drawid] = coords

        pdc.EndDrawing()
        self.Refresh()

        return drawid

    def TextBounds(self, textinfo, coords):
        """
        Return text boundary data
        """
        rotation = float(textinfo[3])

        Debug.msg (4, "BufferedWindow.TextBounds(): text=%s, rotation=%f" % \
                   (textinfo[0], rotation))

        self.Update()
        self.Refresh()
        self.SetFont(textinfo[1])
        w,h = self.GetTextExtent(textinfo[0])
        if rotation == 0:
            coords[2], coords[3] = coords[0] + w, coords[1] + h
            return coords, w, h
        else:
            boxh = math.fabs(math.sin(math.radians(rotation)) * w) + h
            boxw = math.fabs(math.cos(math.radians(rotation)) * w) + h
            coords[2] = coords[0] + boxw
            coords[3] = coords[1] + boxh
            return coords, boxw, boxh

    def OnPaint(self, event):
        """
        Draw PseudoDC to buffered paint DC
        """

        dc = wx.BufferedPaintDC(self, self._buffer)

        # we need to clear the dc BEFORE calling PrepareDC
        bg = wx.Brush(self.GetBackgroundColour())
        dc.SetBackground(bg)
        dc.Clear()

        # use PrepareDC to set position correctly
        self.PrepareDC(dc)

        # create a clipping rect from our position and size
        # and update region
        rgn = self.GetUpdateRegion().GetBox()
        dc.SetClippingRect(rgn)

        if self.redrawAll: # redraw pdc and pdcVector
            # draw to the dc using the calculated clipping rect
            self.pdc.DrawToDCClipped(dc, rgn)

            # draw vector map layer
            if self.pdcVector:
                self.pdcVector.DrawToDCClipped(dc, rgn)

            self._bufferLast = None
        else: # do not redraw pdc and pdcVector
            if self._bufferLast is None:
                # draw to the dc 
                self.pdc.DrawToDC(dc)

                if self.pdcVector:
                    self.pdcVector.DrawToDC(dc)

                # store buffered image
                # self._bufferLast = wx.BitmapFromImage(self._buffer.ConvertToImage())
                self._bufferLast = dc.GetAsBitmap((0, 0, self.Map.width, self.Map.height))

            pdcLast = wx.PseudoDC()
            pdcLast.DrawBitmap(bmp=self._bufferLast, x=0, y=0)
            pdcLast.DrawToDC(dc)

        # draw temporal object on the foreground
        self.pdcTmp.DrawToDCClipped(dc, rgn)
            
    def OnSize(self, event):
        """
        Scale map image so that it is
        the same size as the Window
        """

        Debug.msg(3, "BufferedWindow.OnSize():")

        # set size of the input image
        self.Map.ChangeMapSize(self.GetClientSize())
        self.Map.AlignExtentFromDisplay() # align extent based on center point and display resolution

        # Make new off screen bitmap: this bitmap will always have the
        # current drawing in it, so it can be used to save the image to
        # a file, or whatever.
        self._buffer = wx.EmptyBitmap(self.Map.width, self.Map.height)

        # get the image to be rendered
        self.img = self.GetImage()

        # update map display
        if self.img and self.Map.width + self.Map.height > 0: # scale image during resize
            self.img = self.img.Scale(self.Map.width, self.Map.height)
            self.UpdateMap()

        # re-render image on idle
        self.resize = True

        # reposition checkbox in statusbar
        self.parent.StatusbarReposition()

        # update statusbar
        self.parent.StatusbarUpdate()

    def OnIdle(self, event):
        """
        Only re-render a compsite map image from GRASS during
        idle time instead of multiple times during resizing.
        """

        if self.resize:
            self.UpdateMap(render=True)

        event.Skip()

    def SaveToFile(self, FileName, FileType):
        """
        This draws the psuedo DC to a buffer that
        can be saved to a file.
        """
        dc = wx.BufferedPaintDC(self, self._buffer)
        self.pdc.DrawToDC(dc)
        if self.pdcVector:
            self.pdcVector.DrawToDC(dc)
        self._buffer.SaveFile(FileName, FileType)

    def GetOverlay(self):
        """
        Converts rendered overlay files to wx.Image
        """
        self.ovldict = {}
        for overlay in self.Map.GetListOfLayers(l_type="overlay", l_active=True):
            if os.path.isfile(overlay.mapfile) and os.path.getsize(overlay.mapfile):
                img = wx.Image(overlay.mapfile, wx.BITMAP_TYPE_ANY)
                pdc_id = self.Map.overlays.index(overlay)
                self.ovldict[pdc_id] = img  # image information for each overlay image
                self.imagedict[img] = pdc_id # set image PeudoDC ID

        Debug.msg (3, "BufferedWindow.GetOverlay(): numberof=%d" % len(self.ovldict))

        return self.ovldict


    def GetImage(self):
        """
        Converts redered map files to wx.Image
        """
        if self.Map.mapfile and os.path.isfile(self.Map.mapfile) and \
                os.path.getsize(self.Map.mapfile):
            img = wx.Image(self.Map.mapfile, wx.BITMAP_TYPE_ANY)
        else:
            img = None

        self.imagedict[img] = 99 # set image PseudoDC ID

        return img


    def UpdateMap(self, render=True, renderVector=True):
        """
        Updates the canvas anytime there is a change to the underlaying images
        or to the geometry of the canvas.
        """

        Debug.msg (2, "BufferedWindow.UpdateMap(): render=%s" % \
                   (render))

        #
        # render background image if needed
        #
        if render or self.img == None:
            self.Map.ChangeMapSize(self.GetClientSize())
            self.mapfile = self.Map.Render(force=True)
            self.img = self.GetImage() # id=99
            self.resize = False

        #
        # clear pseudoDcs
        #
        self.pdc.Clear()
        self.pdc.RemoveAll()
        self.pdcTmp.Clear()
        self.pdcTmp.RemoveAll()


        # 
        # draw background map image to PseudoDC
        #
        if not self.img:
            self.Draw(self.pdc, pdctype='clear')
        else:
            try:
                id = self.imagedict[self.img]
            except:
                return False

            self.Draw(self.pdc, self.img, drawid=id) 
         
        #
        # render vector map layer 
        #
        digitToolbar = self.parent.digittoolbar
        if renderVector and digitToolbar and \
                digitToolbar.layerSelectedID != None:
            # set region
            self.parent.digit.driver.UpdateRegion()
            # re-calculate threshold for digitization tool
            self.parent.digit.driver.GetThreshold()
            # draw map
            self.pdcVector.Clear()
            self.pdcVector.RemoveAll()
            self.parent.digit.driver.DrawMap()

        #
        # render overlay
        #
        self.ovldict = self.GetOverlay() # list of decoration overlay images
        if self.ovldict != {}: # draw scale and legend overlays
            for id in self.ovldict:
                img = self.ovldict[id]
                if id not in self.ovlcoords: self.ovlcoords[id] = wx.Rect(0,0,0,0)
                if id not in self.ovlchk: self.ovlchk[id] = False
                if self.ovlchk[id] == True: # draw any active and defined overlays
                    self.Draw(self.pdc, img=img, drawid=id,
                             pdctype='image', coords=self.ovlcoords[id])

        if self.textdict != None: # draw text overlays
            for id in self.textdict:
                self.Draw(self.pdc, img=self.textdict[id], drawid=id,
                          pdctype='text', coords=self.ovlcoords[id])

        self.resize = False

        #
        # render region border (to self.pdcTmp)
        #
        if hasattr(self, "regionCoords"):
            reg = self.Map.GetWindow()
            self.polypen = wx.Pen(colour='red', width=2, style=wx.SOLID)
            c2p = self.Cell2Pixel
            self.regionCoords = []
            self.regionCoords.append(c2p((reg['west'],reg['north'])))
            self.regionCoords.append(c2p((reg['east'],reg['north'])))
            self.regionCoords.append(c2p((reg['east'],reg['south'])))
            self.regionCoords.append(c2p((reg['west'],reg['south'])))
            self.regionCoords.append(c2p((reg['west'],reg['north'])))
            # draw region extent
            self.DrawLines(pdc=self.pdcTmp, polycoords=self.regionCoords)

        #
        # update statusbar
        #
        self.Map.SetRegion()
        self.parent.StatusbarUpdate()

        return True

    def EraseMap(self):
        """
        Erase the map display
        """
        self.Draw(self.pdc, pdctype='clear')

    def DragMap(self, moveto):
        """
        Drag the entire map image for panning.
        """

        dc = wx.BufferedDC(wx.ClientDC(self))
        dc.SetBackground(wx.Brush("White"))
        dc.Clear()
        
        #if not self.img:
        #    return False
        
        # bitmap = wx.BitmapFromImage(self.img)

        # drag not only background image
        # FIXME: when mapdisplay window is hiden by other window,
        # self._buffer contains grey holes
        self.dragimg = wx.DragImage(self._buffer)
        self.dragimg.BeginDrag((0, 0), self)
        self.dragimg.GetImageRect(moveto)
        self.dragimg.Move(moveto)
        
        self.dragimg.DoDrawImage(dc, moveto)
        self.dragimg.EndDrag()

        return True

    def DragItem(self, id, event):
        """
        Drag an overlay decoration item
        """
        Debug.msg (5, "BufferedWindow.DragItem(): id=%d" % \
                       id)
        x,y = self.lastpos
        dx = event.GetX() - x
        dy = event.GetY() - y
        self.pdc.SetBackground(wx.Brush(self.GetBackgroundColour()))
        r = self.pdc.GetIdBounds(id)
        if self.dragid > 100: # text dragging
            rtop = (r[0],r[1]-r[3],r[2],r[3])
            r = r.Union(rtop)
            rleft = (r[0]-r[2],r[1],r[2],r[3])
            r = r.Union(rleft)
        self.pdc.TranslateId(id, dx, dy)
        r2 = self.pdc.GetIdBounds(id)
        r = r.Union(r2)
        r.Inflate(4,4)
        self.Update()
        self.RefreshRect(r, False)
        self.lastpos = (event.GetX(),event.GetY())

    def MouseDraw(self, pdc=None):
        """
        Mouse rectangles and lines
        """
        if not pdc:
            return 

        Debug.msg (5, "BufferedWindow.MouseDraw(): use=%s, box=%s" % \
                       (self.mouse['use'], self.mouse['box']))

        if self.mouse['box'] == "box":
            boxid = wx.ID_NEW
            mousecoords = [self.mouse['begin'][0], self.mouse['begin'][1], \
                           self.mouse['end'][0], self.mouse['end'][1]]
            r = pdc.GetIdBounds(boxid)
            r.Inflate(4,4)
            pdc.ClearId(boxid)
            self.RefreshRect(r, False)
            pdc.SetId(boxid)
            self.Draw(pdc, drawid=boxid, pdctype='box', coords=mousecoords)
        elif self.mouse['box'] == "line":
            self.lineid = wx.ID_NEW
            mousecoords = [self.mouse['begin'][0], self.mouse['begin'][1], \
                           self.mouse['end'][0], self.mouse['end'][1]]
            x1=min(self.mouse['begin'][0],self.mouse['end'][0])
            x2=max(self.mouse['begin'][0],self.mouse['end'][0])
            y1=min(self.mouse['begin'][1],self.mouse['end'][1])
            y2=max(self.mouse['begin'][1],self.mouse['end'][1])
            r = wx.Rect(x1,y1,x2-x1,y2-y1)
            r.Inflate(4,4)
            try:
                pdc.ClearId(self.lineid)
            except:
                pass
            self.RefreshRect(r, False)
            pdc.SetId(self.lineid)

            self.Draw(pdc, drawid=self.lineid, pdctype='line', coords=mousecoords)

    def DrawLines(self, pdc=None, polycoords=None):
        """Draw polylines in PseudoDC"""

        if not pdc:
            return

        if not polycoords:
            polycoords = self.polycoords

        if len(polycoords) > 0:
            self.plineid = wx.ID_NEW + 1
            self.Draw(pdc, drawid=self.plineid, pdctype='polyline', coords=polycoords)

            Debug.msg (4, "BufferedWindow.DrawLines(): coords=%s, id=%s" % \
                           (polycoords, self.plineid))

            return self.plineid

        return -1

    def DrawCross(self, pdc, coords, size, rotation=0):
        """Draw cross in PseudoDC

           TODO: implement rotation
        """
        Debug.msg(4, "BufferedWindow.DrawCross(): pdc=%s, coords=%s, size=%d" % \
                  (pdc, coords, size))
        coordsCross = ((coords[0] - size, coords[1], coords[0] + size, coords[1]),
                       (coords[0], coords[1] - size, coords[0], coords[1] + size))

        self.lineid = wx.NewId()
        for lineCoords in coordsCross:
            self.Draw(pdc, drawid=self.lineid, pdctype='line', coords=lineCoords)

        return self.lineid

    def MouseActions(self, event):
        """
        Mouse motion and button click notifier
        """
        wheel = event.GetWheelRotation() # +- int

        # zoom with mouse wheel
        if wheel != 0:
            Debug.msg (5, "BufferedWindow.MouseAction(): wheel=%d" % wheel)
            # zoom 1/2 of the screen
            begin = [self.Map.width/4, self.Map.height/4]
            end = [self.Map.width - self.Map.width/4,
                   self.Map.height - self.Map.height/4]
            if wheel > 0:
                zoomtype = 1
            else:
                zoomtype = -1
            # zoom
            self.Zoom(begin, end, zoomtype)
            # redraw map
            self.UpdateMap()
            # update statusbar
            self.parent.StatusbarUpdate()
            return

        # left mouse button pressed
        if event.LeftDown():
            self.OnLeftDown(event)

        # left mouse button released
        elif event.LeftUp():
            self.OnLeftUp(event)

        # dragging
        elif event.Dragging():
            Debug.msg (5, "BufferedWindow.MouseAction(): Dragging")
            currpos = event.GetPositionTuple()[:]
            end = (currpos[0] - self.mouse['begin'][0], \
                   currpos[1] - self.mouse['begin'][1])

            # dragging or drawing box with left button
            if self.mouse['use'] == 'pan':
                self.DragMap(end)

            # dragging decoration overlay item
            elif (self.mouse['use'] == 'pointer' and not self.parent.digittoolbar) and \
                    self.dragid != None and \
                    self.dragid != 99:
                self.DragItem(self.dragid, event)

            # dragging anything else - rubber band box or line
            else:
                self.mouse['end'] = event.GetPositionTuple()[:]
                self.MouseDraw(pdc=self.pdcTmp)

        # double click
        elif event.ButtonDClick():
            self.OnButtonDClick(event)

        # middle mouse button pressed
        elif event.MiddleDown():
            self.OnMiddleDown(event)

        # right mouse button pressed
        elif event.RightDown():
            self.OnRightDown(event)

        # right mouse button released
        elif event.RightUp():
            self.OnRightUp(event)

        elif event.Moving():
            self.OnMouseMoving(event)

        # store current mouse position
        self.mouse['pos'] = event.GetPositionTuple()[:]

    def OnLeftDown(self, event):
        """
        Left mouse button pressed
        """
        Debug.msg (5, "BufferedWindow.OnLeftDown(): use=%s" % \
                   self.mouse["use"])

        self.mouse['begin'] = event.GetPositionTuple()[:]

        if self.mouse["use"] in ["measure", "profile"]:
            # measure || profile
            if len(self.polycoords) == 0:

                self.mouse['end'] = self.mouse['begin']
                self.polycoords.append(self.mouse['begin'])
                self.ClearLines(pdc=self.pdcTmp)
            else:
                self.mouse['begin'] = self.mouse['end']
        elif self.mouse['use'] == 'zoom':
            self.redrawAll = False
        elif self.mouse["use"] == "pointer" and self.parent.digittoolbar:
            # digitization
            digitToolbar = self.parent.digittoolbar
            digitClass   = self.parent.digit
            east, north = self.Pixel2Cell(self.mouse['begin'])

            try:
                map = digitToolbar.layers[digitToolbar.layerSelectedID].name
            except:
                map = None
                dlg = wx.MessageDialog(self, _("No vector map layer selected for editing"),
                                       _("Error"), wx.OK | wx.ICON_ERROR)
                dlg.ShowModal()
                dlg.Destroy()
                event.Skip()
                return

            # calculate position of 'update record' dialog
            offset = 5
            posWindow = self.ClientToScreen((self.mouse['begin'][0] + offset,
                                             self.mouse['begin'][1] + offset))

            if digitToolbar.action not in ["moveVertex", "addVertex",
                                           "removeVertex", "editLine"]:
                # set pen
                self.pen = wx.Pen(colour='Red', width=2, style=wx.SHORT_DASH)
                self.polypen = wx.Pen(colour='dark green', width=2, style=wx.SOLID)

            if digitToolbar.action == "addLine":
                if digitToolbar.type in ["point", "centroid"]:
                    # add new point
                    digitClass.AddPoint(map=map,
                                               type=digitToolbar.type,
                                               x=east, y=north)

                    self.UpdateMap(render=False) # redraw map

                    # add new record into atribute table
                    if digitClass.settings["addRecord"]:
                        # select attributes based on layer and category
                        addRecordDlg = dbm.DisplayAttributesDialog(parent=self, map=map,
                                                                   layer=digitClass.settings["layer"],
                                                                   cat=digitClass.settings["category"],
                                                                   pos=posWindow,
                                                                   action="add")
                        if addRecordDlg.mapInfo and \
                               addRecordDlg.ShowModal() == wx.ID_OK:
                            sqlfile = tempfile.NamedTemporaryFile(mode="w")
                            for sql in addRecordDlg.GetSQLString():
                                sqlfile.file.write(sql + ";\n")
                            sqlfile.file.flush()
                            executeCommand = gcmd.Command(cmd=["db.execute",
                                                              "--q",
                                                              "input=%s" % sqlfile.name])

                elif digitToolbar.type in ["line", "boundary"]:
                    # add new point to the line
                    self.polycoords.append(event.GetPositionTuple()[:])
                    self.mouse['begin'] = self.polycoords[-1]
                    self.DrawLines(pdc=self.pdcTmp)

            elif digitToolbar.action == "editLine" and hasattr(self, "moveIds"):
                coords=self.polycoords[-2:]
                idLine = wx.NewId()
                self.pdcVector.SetId(idLine)
                self.pdcVector.DrawLine(coords[0][0], coords[0][1],
                                        coords[1][0], coords[1][1])
                idNode = wx.NewId()
                self.pdcVector.SetIdBounds(idNode, (coords[1][0], coords[1][1],
                                                    coords[1][0], coords[1][1]))
                self.moveIds.append(idNode)

            elif digitToolbar.action == "deleteLine":
                pass

            elif digitToolbar.action in ["moveLine", "moveVertex", "editLine"] and \
                    not hasattr(self, "moveBegin"):
                self.moveBegin = [0, 0]
                self.moveCoords = self.mouse['begin']
                self.moveIds   = []
                if digitToolbar.action in ["moveVertex", "editLine"]:
                    # set pen
                    self.polypen = wx.Pen(colour=digitClass.settings["symbolHighlight"][1],
                                          width=2, style=wx.SHORT_DASH)
                    self.pdcVector.SetPen(self.polypen)

            elif digitToolbar.action == "splitLine":
                pass

            elif digitToolbar.action in ["displayAttrs", "displayCats"]:
                qdist = 10.0 * ((self.Map.region['e'] - self.Map.region['w']) / self.Map.width)
                redraw = False
                if digitToolbar.action == "displayAttrs":
                    # select attributes based on coordinates (all layers)
                    updateRecordDlg = dbm.DisplayAttributesDialog(parent=self, map=map,
                                                                  queryCoords=(east, north),
                                                                  qdist=qdist,
                                                                  pos=posWindow,
                                                                  action="update")
                    if updateRecordDlg.mapInfo and \
                            updateRecordDlg.GetLine():
                        # highlight feature & re-draw map
                        digitClass.driver.SetSelected([updateRecordDlg.GetLine()])
                        self.UpdateMap(render=False)
                        redraw = True
                        if updateRecordDlg.ShowModal() == wx.ID_OK:
                            sqlCommands = updateRecordDlg.GetSQLString()
                            if len(sqlCommands) > 0:
                                sqlfile = tempfile.NamedTemporaryFile(mode="w")
                                for sql in sqlCommands:
                                    sqlfile.file.write(sql + ";\n")
                                sqlfile.file.flush()
                                executeCommand = gcmd.Command(cmd=["db.execute",
                                                                  "--q",
                                                                  "input=%s" % sqlfile.name])
                else: # displayCats
                    categoryDlg = DigitCategoryDialog(parent=self,
                                                      map=map,
                                                      queryCoords=(east, north),
                                                      qdist=qdist,
                                                      pos=posWindow,
                                                      title=_("Update categories"))

                    if categoryDlg.GetLine():
                        # highlight feature & re-draw map
                        digitClass.driver.SetSelected([categoryDlg.GetLine()])
                        self.UpdateMap(render=False)
                        redraw = True

                        if categoryDlg.ShowModal() == wx.ID_OK:
                            pass
                # unselect & re-draw
                if redraw:
                    digitClass.driver.SetSelected([])
                    self.UpdateMap(render=False)

            elif digitToolbar.action == "copyCats":
                if not hasattr(self, "copyCatsList"):
                    self.copyCatsList = []
                else:
                    self.copyCatsIds = []
                    self.mouse['box'] = 'box'

            elif digitToolbar.action == "copyLine":
                self.copyIds = None
                self.layerTmp = None

            elif digitToolbar.action == "zbulkLine":
                if len(self.polycoords) > 1: # start new line
                    self.polycoords = [] 
                    self.ClearLines(pdc=self.pdcTmp)
                self.polycoords.append(event.GetPositionTuple()[:])
                if len(self.polycoords) == 1:
                    self.mouse['begin'] = self.polycoords[-1]
                else:
                    self.mouse['end'] = self.polycoords[-1]
                    self.DrawLines(pdc=self.pdcTmp)
            elif digitToolbar.action == "connectLine":
                if len(digitClass.driver.GetSelected()) > 1:
                    # if two line selected -> reset
                    digitClass.driver.SetSelected([])
                digitClass.driver.SelectLineByPoint(self.Pixel2Cell(self.mouse['begin']))
                    
        else:
            # get decoration id
            self.lastpos = self.mouse['begin']
            idlist = self.pdc.FindObjects(x=self.lastpos[0], y=self.lastpos[1],
                                          radius=self.hitradius)
            if idlist != []:
                self.dragid = idlist[0]

        event.Skip()

    def OnLeftUp(self, event):
        """
        Left mouse button released
        """
        Debug.msg (5, "BufferedWindow.OnLeftUp(): use=%s" % \
                   self.mouse["use"])

        self.mouse['end'] = event.GetPositionTuple()[:]

        if self.mouse['use'] in ["zoom", "pan"]:
            if self.mouse['use'] == 'zoom':
                self.redrawAll = True
            # set region in zoom or pan
            self.Zoom(self.mouse['begin'], self.mouse['end'], self.zoomtype)

            # redraw map
            self.UpdateMap(render=True)

            # update statusbar
            self.parent.StatusbarUpdate()

        elif self.mouse["use"] == "query":
            # querying
            self.parent.QueryMap(self.mouse['begin'][0],self.mouse['begin'][1])

        elif self.mouse["use"] in ["measure", "profile"]:
            # measure or profile
            self.mouse['end'] = event.GetPositionTuple()[:]
            if self.mouse["use"] == "measure":
                self.parent.MeasureDist(self.mouse['begin'], self.mouse['end'])
            try:
                self.polycoords.append(self.mouse['end'])
                self.pdc.ClearId(self.lineid)
                self.DrawLines(pdc=self.pdcTmp)
            except:
                pass

        elif self.mouse["use"] == "pointer" and self.parent.digittoolbar:
            # digitization tool active
            digitToolbar = self.parent.digittoolbar
            digitClass   = self.parent.digit

            self.mouse['end'] = event.GetPositionTuple()[:]
            pos1 = self.Pixel2Cell(self.mouse['begin'])
            pos2 = self.Pixel2Cell(self.mouse['end'])

            if digitToolbar.action in ["deleteLine", "moveLine", "moveVertex",
                                       "copyCats", "editLine", "flipLine",
                                       "mergeLine", "snapLine",
                                       "queryLine", "breakLine"]:
                nselected = 0
                # -> delete line || move line || move vertex
                if digitToolbar.action in ["moveVertex", "editLine"]:
                    if len(digitClass.driver.GetSelected()) == 0:
                        nselected = digitClass.driver.SelectLineByPoint(pos1, type="line")
                        # return point on the line
                        
                elif digitToolbar.action == "copyCats":
                    if not hasattr(self, "copyCatsIds"):
                        # collect categories
                        nselected = digitClass.driver.SelectLineByPoint(pos1, type="line")
                        if nselected:
                            qdist = 10.0 * ((self.Map.region['e'] - self.Map.region['w']) / \
                                                self.Map.width)
                            vWhat = gcmd.Command(['v.what',
                                                 '--q',
                                                 'map=%s' % digitClass.map, 
                                                 'east_north=%f,%f' % \
                                                     (float(pos1[0]), float(pos1[1])),
                                                 'distance=%f' % qdist])

                            for line in vWhat.ReadStdOutput():
                                if "Category:" in line:
                                    cat = int(line.split(':')[1].strip())
                                    self.copyCatsList.append(cat)
                    else:
                        # collect ids
                        digitClass.driver.SetSelected([])
                        # return number of selected features
                        nselected = digitClass.driver.SelectLinesByBox(pos1, pos2)
                        if nselected > 0:
                            self.copyCatsIds = digitClass.driver.GetSelected()

                elif digitToolbar.action == "queryLine":
                    selected = digitClass.SelectLinesByQuery(pos1, pos2)
                    nselected = len(selected)
                    if nselected > 0:
                        digitClass.driver.SetSelected(selected)

                else:
                    # -> moveLine || deleteLine, etc. (select by box)
                    nselected = digitClass.driver.SelectLinesByBox(pos1, pos2)

                if nselected > 0:
                    if digitToolbar.action in ["moveLine", "moveVertex",
                                               "copyCats", "editLine"]:
                        # get pseudoDC id of objects which should be redrawn
                        if digitToolbar.action == "moveLine":
                            # -> move line
                            self.moveIds = digitClass.driver.GetSelected(grassId=False)

                        elif digitToolbar.action == "moveVertex":
                            # -> move vertex
                            self.moveIds = digitClass.driver.GetSelectedVertex(pos1)
                            if len(self.moveIds) == 0: # no vertex found
                                digitClass.driver.SetSelected([])

                        elif digitToolbar.action == "editLine":
                            # -> edit line
                            # get id of selected vertex (last or first node)
                            selVertex = digitClass.driver.GetSelectedVertex(pos1)[0]
                            ids = digitClass.driver.GetSelected(grassId=False)
                            for id in ids:
                                if id % 2: # vertex
                                    self.moveIds.append(id)
                            if selVertex < ids[-1] / 2:
                                # choose first or last node of line
                                self.moveIds.reverse()

                    # -> move line || move vertex
                    self.UpdateMap(render=False)
                    if digitToolbar.action != "copyCats":
                        redrawAll = False
                else: # no vector object found
                    self.UpdateMap(render=False, renderVector=False)

            elif digitToolbar.action in ["splitLine", "addVertex", "removeVertex"]:
                pointOnLine = digitClass.driver.SelectLineByPoint(pos1,
                                                       type="line")
                if pointOnLine:
                    self.UpdateMap(render=False) # highlight object
                    if digitToolbar.action in ["splitLine", "addVertex"]:
                        self.DrawCross(pdc=self.pdcVector, coords=self.Cell2Pixel(pointOnLine),
                                       size=5)
                    elif digitToolbar.action == "removeVertex":
                        # get only id of vertex
                        id = digitClass.driver.GetSelectedVertex(pos1)[0] 
                        x, y = self.pdcVector.GetIdBounds(id)[0:2]
                        self.pdcVector.RemoveId(id)
                        self.DrawCross(pdc=self.pdcVector, coords=(x, y),
                                       size=5)
            elif digitToolbar.action == "copyLine":
                if digitClass.settings['backgroundMap'] == '':
                    # no background map -> copy from current vector map layer
                    nselected = digitClass.driver.SelectLinesByBox(pos1, pos2)

                    if nselected > 0:
                        # highlight selected features
                        self.UpdateMap(render=False)
                    else:
                        self.UpdateMap(render=False, renderVector=False)
                else:
                    # copy features from background map
                    self.copyIds = digitClass.SelectLinesFromBackgroundMap(pos1, pos2)
                    if len(self.copyIds) > 0:
                        dVectTmp = ['d.vect',
                                    'map=%s' % digitClass.settings['backgroundMap'],
                                    'cats=%s' % ",".join(["%d" % v for v in self.copyIds]),
                                    '-i',
                                    'color=yellow',
                                    'fcolor=yellow',
                                    'type=point,line,boundary,centroid']
                        self.layerTmp = self.Map.AddLayer(type='vector',
                                                          command=dVectTmp)
                        self.UpdateMap(render=True, renderVector=False)
                    else:
                        self.UpdateMap(render=False, renderVector=False)

            elif digitToolbar.action == "zbulkLine" and len(self.polycoords) == 2:
                # select lines to be labeled
                pos1 = self.Pixel2Cell(self.polycoords[0])
                pos2 = self.Pixel2Cell(self.polycoords[1])
                nselected = digitClass.driver.SelectLinesByBox(pos1, pos2)

                if nselected > 0:
                    # highlight selected features
                    self.UpdateMap(render=False)
                    self.DrawLines(pdc=self.pdcTmp) # redraw temp line
                else:
                    self.UpdateMap(render=False, renderVector=False)

            elif digitToolbar.action == "connectLine":
                if len(digitClass.driver.GetSelected()) > 0:
                    self.UpdateMap(render=False)

        elif self.dragid != None:
            # end drag of overlay decoration
            self.ovlcoords[self.dragid] = self.pdc.GetIdBounds(self.dragid)
            self.dragid = None
            self.currtxtid = None
            id = None
            self.Update()

        event.Skip()

    def OnButtonDClick(self, event):
        """
        Mouse button double click
        """
        Debug.msg (5, "BufferedWindow.OnButtonDClick(): use=%s" % \
                   self.mouse["use"])

        if self.mouse["use"] == "measure":
            # measure
            self.ClearLines(pdc=self.pdcTmp)
            self.polycoords = []
            self.mouse['use'] = 'pointer'
            self.mouse['box'] = 'point'
            self.mouse['end'] = [0, 0]
            self.Refresh()
            self.SetCursor(self.parent.cursors["default"])
        elif self.mouse["use"] == "profile":
            # profile
            pass
        #                self.pdc.ClearId(self.lineid)
        #                self.pdc.ClearId(self.plineid)
        #                print 'coordinates: ',self.polycoords
        #                self.polycoords = []
        #                self.mouse['begin'] = self.mouse['end'] = [0, 0]
        #                self.Refresh()
        elif self.mouse['use'] == 'pointer' and self.parent.digittoolbar:
            # digitization tool
            pass
        else:
            # select overlay decoration options dialog
            clickposition = event.GetPositionTuple()[:]
            idlist  = self.pdc.FindObjects(clickposition[0], clickposition[1], self.hitradius)
            if idlist == []:
                return
            self.dragid = idlist[0]

            self.ovlcoords[self.dragid] = self.pdc.GetIdBounds(self.dragid)
            if self.dragid > 100:
                self.currtxtid = self.dragid
                self.parent.AddText(None)
            elif self.dragid == 0:
                self.parent.AddBarscale(None)
            elif self.dragid == 1:
                self.parent.AddLegend(None)

        event.Skip()

    def OnRightDown(self, event):
        """
        Right mouse button pressed
        """
        Debug.msg (5, "BufferedWindow.OnRightDown(): use=%s" % \
                   self.mouse["use"])

        x,y = event.GetPositionTuple()[:]
        l = self.pdc.FindObjects(x=x, y=y, radius=self.hitradius)
        if not l:
            return

        id = l[0]

        if id != 99:
            if self.pdc.GetIdGreyedOut(id) == True:
                self.pdc.SetIdGreyedOut(id, False)
            else:
                self.pdc.SetIdGreyedOut(id, True)

                r = self.pdc.GetIdBounds(id)
                r.Inflate(4,4)
                self.RefreshRect(r, False)

        event.Skip()

    def OnRightUp(self, event):
        """
        Right mouse button released
        """
        Debug.msg (5, "BufferedWindow.OnRightUp(): use=%s" % \
                   self.mouse["use"])

        digitToolbar = self.parent.digittoolbar
        if digitToolbar:
            digitClass = self.parent.digit
            # digitization tool (confirm action)
            if digitToolbar.action == "addLine" and \
                    digitToolbar.type in ["line", "boundary"]:
                # -> add new line / boundary
                try:
                    map = digitToolbar.layers[digitToolbar.layerSelectedID].name
                except:
                    map = None
                    dlg = wx.MessageDialog(self, _("No vector map layer is selected"),
                                           _("Error"), wx.OK | wx.ICON_ERROR)
                    dlg.ShowModal()
                    dlg.Destroy()

                if map:
                    mapcoords = []
                    # xy -> EN
                    for coord in self.polycoords:
                        mapcoords.append(self.Pixel2Cell(coord))

                    digitClass.AddLine(map=map,
                                  type=digitToolbar.type,
                                  coords=mapcoords)

                    self.UpdateMap(render=False)

                    # add new record into atribute table
                    if digitClass.settings["addRecord"]:
                        offset = 5
                        posWindow = self.ClientToScreen((self.polycoords[-1][0] + offset,
                                                         self.polycoords[-1][1] + offset))

                        # select attributes based on layer and category
                        addRecordDlg = dbm.DisplayAttributesDialog(parent=self, map=map,
                                                                   layer=digitClass.settings["layer"],
                                                                   cat=digitClass.settings["category"],
                                                                   pos=posWindow,
                                                                   action="add")
                        if addRecordDlg.mapInfo and \
                                addRecordDlg.ShowModal() == wx.ID_OK:
                            sqlfile = tempfile.NamedTemporaryFile(mode="w")
                            for sql in addRecordDlg.GetSQLString():
                                sqlfile.file.write(sql + ";\n")
                            sqlfile.file.flush()
                            executeCommand = gcmd.Command(cmd=["db.execute",
                                                              "--q",
                                                              "input=%s" % sqlfile.name])
                    # clean up saved positions
                    self.polycoords = []
            elif digitToolbar.action == "deleteLine":
                # -> delete selected vector features
                digitClass.DeleteSelectedLines()
            elif digitToolbar.action in ["moveLine", "moveVertex"] and \
                    hasattr(self, "moveBegin"):
                # move vector feature
                # move = [self.Distance((0,0), (self.moveBegin[0], 0))[0],
                #        self.Distance((0,0), (0, self.moveBegin[1]))[0]] # TODO d.measure
                # ES -> EN
                # if self.moveBegin[0] < 0.0:
                #    move[0] *= -1.0
                # if self.moveBegin[1] > 0.0:
                #    move[1] *= -1.0

                pTo = self.Pixel2Cell(event.GetPositionTuple())
                pFrom = self.Pixel2Cell(self.moveCoords)
                move = (pTo[0]-pFrom[0], pTo[1]-pFrom[1])

                if digitToolbar.action == "moveLine":
                    # move line
                    digitClass.MoveSelectedLines(move)
                elif digitToolbar.action == "moveVertex":
                    # move vertex
                    digitClass.MoveSelectedVertex(pFrom,
                                                  move)

                redrawAll = True

                del self.moveBegin
                del self.moveCoords
                del self.moveIds
            elif digitToolbar.action == "splitLine":
                # split line
                digitClass.SplitLine(self.Pixel2Cell(self.mouse['begin']))
            elif digitToolbar.action == "addVertex":
                # add vertex
                digitClass.AddVertex(self.Pixel2Cell(self.mouse['begin']))
            elif digitToolbar.action == "removeVertex":
                # remove vertex
                digitClass.RemoveVertex(self.Pixel2Cell(self.mouse['begin']))
            elif digitToolbar.action == "copyCats":
                try:
                    digitClass.CopyCats(self.copyCatsList,
                                               self.copyCatsIds)
                    del self.copyCatsList
                    del self.copyCatsIds
                except:
                    pass
            elif digitToolbar.action == "editLine" and hasattr(self, "moveBegin"):
                line = digitClass.driver.GetSelected()
                coords = []
                for id in self.moveIds: # avoid last point
                    x, y = self.pdcVector.GetIdBounds(id)[0:2]
                    coords.append(self.Pixel2Cell((x, y)))
                digitClass.EditLine(line, coords)

                redrawAll = True

                del self.moveBegin
                del self.moveCoords
                del self.moveIds
            elif digitToolbar.action == "flipLine":
                digitClass.FlipLine()
            elif digitToolbar.action == "mergeLine":
                digitClass.MergeLine()
            elif digitToolbar.action == "breakLine":
                digitClass.BreakLine()
            elif digitToolbar.action == "snapLine":
                digitClass.SnapLine()
            elif digitToolbar.action == "connectLine":
                if len(digitClass.driver.GetSelected()) == 2:
                    digitClass.ConnectLine()
            elif digitToolbar.action == "copyLine":
                digitClass.CopyLine(self.copyIds)
                del self.copyIds
                if self.layerTmp:
                    self.Map.DeleteLayer(self.layerTmp)
                    self.UpdateMap(render=True, renderVector=False)
                del self.layerTmp

            elif digitToolbar.action == "zbulkLine" and len(self.polycoords) == 2:
                pos1 = self.Pixel2Cell(self.polycoords[0])
                pos2 = self.Pixel2Cell(self.polycoords[1])

                selected = digitClass.driver.GetSelected()
                dlg = DigitZBulkDialog(parent=self, title=_("Z bulk-labeling dialog"),
                                       nselected=len(selected))
                if dlg.ShowModal() == wx.ID_OK:
                    digitClass.ZBulkLine(pos1, pos2, dlg.value.GetValue(), dlg.step.GetValue())

                self.UpdateMap(render=False, renderVector=True)

            if digitToolbar.action != "addLine":
                # unselect and re-render
                digitClass.driver.SetSelected([])
                self.UpdateMap(render=False)

        event.Skip()

    def OnMiddleDown(self, event):
        """Middle mouse button pressed"""
        digitToolbar = self.parent.digittoolbar
        # digitization tool
        if self.mouse["use"] == "pointer" and digitToolbar:
            digitClass = self.parent.digit
            if (digitToolbar.action == "addLine" and digitToolbar.type in ["line", "boundary"]) or \
                    digitToolbar.action == "editLine":
                # add line or boundary -> remove last point from the line
                try:
                    removed = self.polycoords.pop()
                    Debug.msg(4, "BufferedWindow.OnMiddleDown(): polycoords_poped=%s" % \
                                  [removed,])

                    self.mouse['begin'] = self.polycoords[-1]
                except:
                    pass

                if digitToolbar.action == "editLine" and len(self.moveIds) > 0:
                    # remove last vertex & line
                    self.pdcVector.RemoveId(self.moveIds[-1]) # remove last vertex
                    if self.moveIds[-1] > self.moveIds[-2]:
                        self.pdcVector.RemoveId(self.moveIds[-1] - 1) # remove last segment
                    else:
                        self.pdcVector.RemoveId(self.moveIds[-1] + 1) # remove last segment
                    self.moveIds.pop() 

                self.ClearLines(pdc=self.pdcTmp)
                self.UpdateMap(render=False, renderVector=False)
                self.DrawLines(pdc=self.pdcTmp)
            elif digitToolbar.action in ["deleteLine", "moveLine", "splitLine",
                                  "addVertex", "removeVertex", "moveVertex",
                                  "copyCats", "flipLine", "mergeLine",
                                  "snapLine", "connectLine", "copyLine",
                                  "queryLine", "breakLine"]:
                # varios tools -> unselected selected features
                digitClass.driver.SetSelected([])
                if digitToolbar.action in ["moveLine", "moveVertex", "editLine"] and \
                        hasattr(self, "moveBegin"):
                    
                    redrawAll = True

                    del self.moveBegin
                    del self.moveCoords
                    del self.moveIds
                elif digitToolbar.action == "copyCats":
                    try:
                        del self.copyCatsList
                        del self.copyCatsIds
                    except:
                        pass
                elif digitToolbar.action == "copyLine":
                    del self.copyIds
                    if self.layerTmp:
                        self.Map.DeleteLayer(self.layerTmp)
                        self.UpdateMap(render=True, renderVector=False)
                    del self.layerTmp

                self.UpdateMap(render=False) # render vector

            elif digitToolbar.action == "zbulkLine":
                # reset polyline
                self.polycoords = []
                digitClass.driver.SetSelected([])
                self.UpdateMap(render=False)


    def OnMouseMoving(self, event):
        """Motion event and no mouse buttons were pressed"""
        digitToolbar = self.parent.digittoolbar
        if self.mouse["use"] == "pointer" and digitToolbar:
            digitClass = self.parent.digit
            self.mouse['end'] = event.GetPositionTuple()[:]
            Debug.msg (5, "BufferedWindow.OnMouseMoving(): coords=%f,%f" % \
                           (self.mouse['end'][0], self.mouse['end'][1]))
            if digitToolbar.action == "addLine" and digitToolbar.type in ["line", "boundary"]:
                if len(self.polycoords) > 0:
                    # draw mouse moving
                    self.MouseDraw(self.pdcTmp)
            elif digitToolbar.action in ["moveLine", "moveVertex", "editLine"] \
                    and hasattr(self, "moveBegin"):
                dx = self.mouse['end'][0] - self.mouse['begin'][0]
                dy = self.mouse['end'][1] - self.mouse['begin'][1]
                self.moveBegin[0] += dx
                self.moveBegin[1] += dy
                if len(self.moveIds) > 0:
                    # draw lines on new position
                    if digitToolbar.action == "moveLine":
                        # move line
                        for id in self.moveIds:
                            self.pdcVector.TranslateId(id, dx, dy)
                    elif digitToolbar.action in ["moveVertex", "editLine"]:
                        # move vertex ->
                        # (vertex, left vertex, left line,
                        # right vertex, right line)

                        ## self.pdcVector.RemoveId(self.moveIds[0])
                        # do not draw static lines
                        self.polycoords = []
                        if digitToolbar.action == "moveVertex":
                            self.pdcVector.TranslateId(self.moveIds[0], dx, dy)
                            if self.moveIds[1] > 0: # previous vertex
                                x, y = self.pdcVector.GetIdBounds(self.moveIds[1])[0:2]
                                self.pdcVector.RemoveId(self.moveIds[1]+1)
                                self.polycoords.append((x, y))
                            self.polycoords.append(self.pdcVector.GetIdBounds(self.moveIds[0])[0:2])
                            if self.moveIds[2] > 0: # next vertex
                                x, y = self.pdcVector.GetIdBounds(self.moveIds[2])[0:2]
                                self.pdcVector.RemoveId(self.moveIds[2]-1)
                                self.polycoords.append((x, y))
                        else: # edit line
                            ## self.pdcVector.TranslateId(self.moveIds[-1], dx, dy)
                            ## self.pdcVector.RemoveId(self.moveIds[-1]) # last vertex
                            ## self.pdcVector.RemoveId(self.moveIds[-1] - 1) # line
                            try:
                                if self.moveIds[-1] > 0: # previous vertex
                                    x, y = self.pdcVector.GetIdBounds(self.moveIds[-1])[0:2]
                                    self.polycoords.append((x, y))
                                    self.polycoords.append(self.mouse['end'])
                            except: # no line
                                self.pdcVector.RemoveId(self.moveIds[-1])
                                self.moveIds = []
                        
                        self.ClearLines(pdc=self.pdcTmp)
                        self.DrawLines(pdc=self.pdcTmp)

                self.Refresh() # TODO: use RefreshRect()
                self.mouse['begin'] = self.mouse['end']
                
            elif digitToolbar.action == "zbulkLine":
                if len(self.polycoords) == 1:
                    # draw mouse moving
                    self.MouseDraw(self.pdcTmp)

        event.Skip()

    def ClearLines(self, pdc=None):
        """
        Clears temporary drawn lines from PseudoDC
        """

        if not pdc:
            return

        exit = True
        
        try:
            pdc.ClearId(self.lineid)
            pdc.RemoveId(self.lineid)
        except:
            exit = False
            
        try:
            pdc.ClearId(self.plineid)
            pdc.RemoveId(self.plineid)
        except:
            exit = False

        Debug.msg(4, "BufferedWindow.ClearLines(): lineid=%s, plineid=%s" %
                  (self.lineid, self.plineid))

        self.Refresh()

        return exit


    def Pixel2Cell(self, (x, y)):
        """
        Convert image coordinates to real word coordinates

        Input : int x, int y
        Output: float x, float y
        """

        try:
            x = int(x)
            y = int(y)
        except:
            return None

        if self.Map.region["ewres"] > self.Map.region["nsres"]:
            res = self.Map.region["ewres"]
        else:
            res = self.Map.region["nsres"]

        w = self.Map.region["center_easting"] - (self.Map.width / 2) * res
        n = self.Map.region["center_northing"] + (self.Map.height / 2) * res

        east  = w + x * res
        north = n - y * res

        # extent does not correspond with whole map canvas area...
        # east  = self.Map.region['w'] + x * self.Map.region["ewres"]
        # north = self.Map.region['n'] - y * self.Map.region["nsres"]
        
        return (east, north)

    def Cell2Pixel(self, (east, north)):
        """
        Convert real word coordinates to image coordinates
        """

        try:
            east  = float(east)
            north = float(north)
        except:
            return None

        if self.Map.region["ewres"] > self.Map.region["nsres"]:
            res = self.Map.region["ewres"]
        else:
            res = self.Map.region["nsres"]

        w = self.Map.region["center_easting"] - (self.Map.width / 2) * res
        n = self.Map.region["center_northing"] + (self.Map.height / 2) * res

        # x = int((east  - w) / res)
        # y = int((n - north) / res)

        x = (east  - w) / res
        y = (n - north) / res

        return (x, y)

    def Zoom(self, begin, end, zoomtype):
        """
        Calculates new region while (un)zoom/pan-ing
        """
        x1, y1 = begin
        x2, y2 = end
        newreg = {}

        # threshold - too small squares do not make sense
        # can only zoom to windows of > 10x10 screen pixels
        if x2 > 10 and y2 > 10 and zoomtype != 0:

            if x1 > x2:
                x1, x2 = x2, x1
            if y1 > y2:
                y1, y2 = y2, y1

            # zoom in
            if zoomtype > 0:
                newreg['w'], newreg['n'] = self.Pixel2Cell((x1, y1))
                newreg['e'], newreg['s'] = self.Pixel2Cell((x2, y2))

            # zoom out
            elif zoomtype < 0:
                newreg['w'], newreg['n'] = self.Pixel2Cell((-x1 * 2, -y1 * 2))
                newreg['e'], newreg['s'] = self.Pixel2Cell((self.Map.width  + 2 * (self.Map.width  - x2),
                                                            self.Map.height + 2 * (self.Map.height - y2)))
        # pan
        elif zoomtype == 0:
            dx = x1 - x2
            dy = y1 - y2
            newreg['w'], newreg['n'] = self.Pixel2Cell((dx, dy))
            newreg['e'], newreg['s'] = self.Pixel2Cell((self.Map.width  + dx,
                                                        self.Map.height + dy))

        # if new region has been calculated, set the values
        if newreg :
            # calculate new center point and display resolution
            self.Map.region['center_easting'] = newreg['w'] + \
                (newreg['e'] - newreg['w']) / 2
            self.Map.region['center_northing'] = newreg['s'] + \
                (newreg['n'] - newreg['s']) / 2
            self.Map.region["ewres"] = (newreg['e'] - newreg['w']) / self.Map.width
            self.Map.region["nsres"] = (newreg['n'] - newreg['s']) / self.Map.height
            self.Map.AlignExtentFromDisplay()

            self.ZoomHistory(self.Map.region['n'], self.Map.region['s'],
                             self.Map.region['e'], self.Map.region['w'])

    def ZoomBack(self):
        """
        Zoom to previous extents in zoomhistory list
        """

        zoom = []
        if len(self.zoomhistory) > 1:
            self.zoomhistory.pop()
            zoom = self.zoomhistory[len(self.zoomhistory)-1]
            # (n, s, e, w)
        if zoom:
            # zoom to selected region
            self.Map.region['center_easting'] = zoom[3] + \
                (zoom[2] - zoom[3]) / 2
            self.Map.region['center_northing'] = zoom[1] + \
                (zoom[0] - zoom[1]) / 2
            self.Map.region["ewres"] = (zoom[2] - zoom[3]) / self.Map.width
            self.Map.region["nsres"] = (zoom[0] - zoom[1]) / self.Map.height
            self.Map.AlignExtentFromDisplay()

            # update map
            self.UpdateMap()

            # update statusbar
            self.parent.StatusbarUpdate()

    def ZoomHistory(self, n, s, e, w):
        """
        Manages a list of last 10 zoom extents

        Return removed history item if exists
        """
        removed = None
        self.zoomhistory.append((n,s,e,w))

        if len(self.zoomhistory) > 10:
            removed = self.zoomhistory.pop(0)

        if removed:
            Debug.msg(4, "BufferedWindow.ZoomHistory(): hist=%s, removed=%s" %
                      (self.zoomhistory, removed))
        else:
            Debug.msg(4, "BufferedWindow.ZoomHistory(): hist=%s" %
                      (self.zoomhistory))
        
        return removed

    def ZoomToMap(self, event):
        """
        Set display extents to match selected raster
        or vector map.
        """
        zoomreg = {}

        # find selected map
        if not self.tree.GetSelection():
            return

        item  = self.tree.GetSelection()
        try:
            layer = self.tree.GetPyData(item)[0]['maplayer']
        except:
            layer = None

        if layer is None:
            return
        
        Debug.msg (3, "BufferedWindow.ZoomToMap(): layer=%s, type=%s" % \
                   (layer.name, layer.type))

        # selected layer must be a valid map
        if layer.type in ('raster', 'rgb', 'his', 'shaded', 'arrow'):
            self.Map.region = self.Map.GetRegion(rast="%s" % layer.name)
        elif layer.type in ('vector', 'thememap', 'themechart'):
            self.Map.region = self.Map.GetRegion(vect="%s" % layer.name)
        else:
            return

        self.Map.SetRegion()
        self.Map.AlignExtentFromDisplay()

        self.ZoomHistory(self.Map.region['n'], self.Map.region['s'],
                         self.Map.region['e'], self.Map.region['w'])

        self.UpdateMap()

        self.parent.StatusbarUpdate()

    def ZoomToWind(self, event):
        """
        Set display geometry to match computational
        region extents (set with g.region)
        """

        self.Map.region = self.Map.GetRegion()
        self.Map.SetRegion()

        self.ZoomHistory(self.Map.region['n'], self.Map.region['s'],
                         self.Map.region['e'], self.Map.region['w'])

        self.UpdateMap()

        self.parent.StatusbarUpdate()

    def DisplayToWind(self, event):
        """
        Set computational region (WIND file) to
        match display extents
        """
        tmpreg = os.getenv("GRASS_REGION")
        os.unsetenv("GRASS_REGION")

        # We ONLY want to set extents here. Don't mess with resolution. Leave that
        # for user to set explicitly with g.region
        new = self.Map.AlignResolution()

        cmdRegion = ["g.region", "--o",
                     "n=%f"    % new['n'],
                     "s=%f"    % new['s'],
                     "e=%f"    % new['e'],
                     "w=%f"    % new['w'],
                     "rows=%f" % new['rows'],
                     "cols=%f" % new['cols']]

        p = gcmd.Command(cmdRegion)

        if tmpreg:
            os.environ["GRASS_REGION"] = tmpreg

    def ZoomToSaved(self, event):
        """
        Set display geometry to match extents in
        saved region file
        """

        zoomreg = {}

        dlg = SavedRegion(self, wx.ID_ANY, "Zoom to saved region extents",
                          pos=wx.DefaultPosition, size=wx.DefaultSize,
                          style=wx.DEFAULT_DIALOG_STYLE,
                          loadsave='load')

        if dlg.ShowModal() == wx.ID_CANCEL:
            dlg.Destroy()
            return

        wind = dlg.wind

        p = gcmd.Command (["g.region", "-ugp", "region=%s" % wind])

        if p.returncode == 0:
            output = p.module_stdout.read().split('\n')
            for line in output:
                line = line.strip()
                if '=' in line: key,val = line.split('=')
                zoomreg[key] = float(val)
            self.Map.region['n'] = zoomreg['n']
            self.Map.region['s'] = zoomreg['s']
            self.Map.region['e'] = zoomreg['e']
            self.Map.region['w'] = zoomreg['w']
            self.ZoomHistory(self.Map.region['n'],self.Map.region['s'],self.Map.region['e'],self.Map.region['w'])
            self.UpdateMap()

        dlg.Destroy()

    def SaveDisplayRegion(self, event):
        """
        Save display extents to named region file.
        """

        dlg = SavedRegion(self, wx.ID_ANY, "Save display extents to region file",
                          pos=wx.DefaultPosition, size=wx.DefaultSize,
                          style=wx.DEFAULT_DIALOG_STYLE,
                          loadsave='save')
        if dlg.ShowModal() == wx.ID_CANCEL:
            dlg.Destroy()
            return

        wind = dlg.wind

        # test to see if it already exists and ask permission to overwrite
        windpath = os.path.join(self.Map.env["GISDBASE"], self.Map.env["LOCATION_NAME"],
                                self.Map.env["MAPSET"],"windows",wind)

        if windpath and not os.path.exists(windpath):
            self.SaveRegion(wind)
        elif windpath and os.path.exists(windpath):
            overwrite = wx.MessageBox(_("Region file <%s>already exists.\nDo you want to overwrite it?") % (wind),
                                      _("Warning"), wx.YES_NO)
            if (overwrite == wx.YES):
                self.SaveRegion(wind)
        else:
            pass

        dlg.Destroy()

    def SaveRegion(self, wind):
        """Save region settings"""
        new = self.Map.AlignResolution()

        cmdRegion = ["g.region",
                     "-u",
                     "n=%f" % new['n'],
                     "s=%f" % new['s'],
                     "e=%f" % new['e'],
                     "w=%f" % new['w'],
                     "rows=%d" % new['rows'],
                     "cols=%d" % new['cols'],
                     "save=%s" % wind,
                     "--o"]

        tmpreg = os.getenv("GRASS_REGION")
        os.unsetenv("GRASS_REGION")

        p = gcmd.Command(cmdRegion)

        if tmpreg:
            os.environ["GRASS_REGION"] = tmpreg

    def Distance(self, beginpt, endpt):
        """Calculete distance"""
        x1, y1 = beginpt
        x2, y2 = endpt
        dEast  = (x2-x1) * self.Map.region["ewres"]
        dNorth = (y2-y1) * self.Map.region["nsres"]

        return (math.sqrt(math.pow((dEast),2) + math.pow((dNorth),2)), (dEast, dNorth))

class MapFrame(wx.Frame):
    """
    Main frame for map display window. Drawing takes place in child double buffered
    drawing window.
    """

    def __init__(self, parent=None, id=wx.ID_ANY, title="GRASS GIS - Map display",
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_FRAME_STYLE, toolbars=["map"],
                 tree=None, notebook=None, gismgr=None, page=None,
                 Map=None, auimgr=None):
        """
            Main map display window with toolbars, statusbar and
            DrawWindow

            Parameters:
                parent  -- parent window, None, wx.Window()
                id      -- window ID, int, wx.ID_ANY
                title   -- window title, string
                pos     -- where to place it, tupple, wx.Position
                size    -- window size, tupple, wx.Size
                style   -- window style
                toolbars-- array of default toolbars, which should appear,
                           currently: ['map', 'digit']
                notebook-- control book ID in GIS Manager
                tree    -- associated layer tree
                gismgr  -- GIS Manager panel
                page    -- notebook page with layer tree
                Map     -- instance of render.Map
        """

        Debug.msg (1, "MapFrame.__init__(): size=%d,%d" % (size[0], size[1]))

        wx.Frame.__init__(self, parent, id, title, pos, size, style)

        self.gismanager = gismgr   # GIS Manager object
        self.Map        = Map      # instance of render.Map
        self.tree       = tree     # GIS Manager layer tree object
        self.page       = page     # Notebook page holding the layer tree
        self.layerbook  = notebook # GIS Manager layer tree notebook

        #
        # available cursors
        #
        self.cursors = {
            # default: cross
            # "default" : wx.StockCursor(wx.CURSOR_DEFAULT),
            "default" : wx.StockCursor(wx.CURSOR_ARROW),
            "cross"   : wx.StockCursor(wx.CURSOR_CROSS),
            "hand"    : wx.StockCursor(wx.CURSOR_HAND),
            "pencil"  : wx.StockCursor(wx.CURSOR_PENCIL),
            "sizenwse": wx.StockCursor(wx.CURSOR_SIZENWSE)
            }

        #
        # set the size & system icon
        #
        self.SetClientSize(size)
        self.iconsize = (16, 16)
        self.SetIcon(wx.Icon(os.path.join(imagepath,'grass.map.gif'), wx.BITMAP_TYPE_ANY))

        #
        # Fancy gui
        #
        # self._mgr = auimgr
        self._mgr = wx.aui.AuiManager(self)

        #
        # Add toolbars
        #
        self.maptoolbar = None
        self.digittoolbar = None
        self.grtoolbar = None
        for toolb in toolbars:
            self.AddToolbar(toolb)

        #
        # Add statusbar
        #
        self.statusbar = self.CreateStatusBar(number=3, style=0)
        self.statusbar.SetStatusWidths([-5, -2, -1])
        self.toggleStatus = wx.Choice(self.statusbar, wx.ID_ANY,
                                      choices = ["Coordinates",
                                                 "Extent",
                                                 "Geometry",
                                                 "Map scale"])
        self.statusText = "Coordinates"
        self.statusbar.Bind(wx.EVT_CHOICE, self.OnToggleStatus, self.toggleStatus)
        # auto-rendering checkbox
        self.autoRender = wx.CheckBox(parent=self.statusbar, id=wx.ID_ANY,
                                      label=_("Render"))
        self.statusbar.Bind(wx.EVT_CHECKBOX, self.OnToggleRender, self.autoRender)
        self.autoRender.SetValue(False)
        self.autoRender.SetToolTip(wx.ToolTip (_("Enable/disable auto-rendering")))
        # show region
        self.showRegion = wx.CheckBox(parent=self.statusbar, id=wx.ID_ANY,
                                      label=_("Show"))
        self.statusbar.Bind(wx.EVT_CHECKBOX, self.OnToggleShowRegion, self.showRegion)
        self.showRegion.SetValue(False)
        self.showRegion.Hide()
        self.showRegion.SetToolTip(wx.ToolTip (_("Show/hide computational "
                                                 "region extent (set with g.region)")))
        # map scale
        self.mapScale = wx.TextCtrl(parent=self.statusbar, id=wx.ID_ANY,
                                    value="", style=wx.TE_PROCESS_ENTER,
                                    size=(150, -1))
        self.mapScale.Hide()
        self.statusbar.Bind(wx.EVT_TEXT_ENTER, self.OnChangeMapScale, self.mapScale)

        self.Map.SetRegion() # set region

        self.StatusbarReposition() # reposition statusbar

        #
        # Init map display
        #
        self.__InitDisplay() # initialize region values

        # initialize buffered DC & set default cursor
        self.MapWindow = BufferedWindow(self, id=wx.ID_ANY, Map=self.Map, tree=self.tree)
        self.MapWindow.Bind(wx.EVT_MOTION, self.OnMotion)
        self.MapWindow.SetCursor(self.cursors["default"]) 

        #
        # Decoration overlays
        #
        self.ovlchk = self.MapWindow.ovlchk
        self.ovlcoords = self.MapWindow.ovlcoords
        # previously set decoration options parameters to insert into options dialog
        self.params = {}
        # ID of properties window open for overlay, indexed by overlay ID
        self.propwin = {}

        #
        # Bind various events
        #
        self.Bind(wx.EVT_ACTIVATE, self.OnFocus)
        self.Bind(wx.EVT_CLOSE,    self.OnCloseWindow)

        #
        # Update fancy gui style
        #
        self._mgr.AddPane(self.MapWindow, wx.aui.AuiPaneInfo().CentrePane().
                   Dockable(False).BestSize((-1,-1)).
                   CloseButton(False).DestroyOnClose(True).
                   Layer(0))
        self._mgr.Update()

        #
        # Init print module and classes
        #
        self.printopt = disp_print.PrintOptions(self, self.MapWindow)

        #
        # Current location information
        #
        self.projinfo = self.Map.ProjInfo()

        #
        # Initialization of digitization tool
        #
        self.digit = Digit(mapwindow=self.MapWindow)

        #
        # Init zoom history
        #
        self.MapWindow.ZoomHistory(self.Map.region['n'],
                                   self.Map.region['s'],
                                   self.Map.region['e'],
                                   self.Map.region['w'])

    def AddToolbar(self, name):
        """
        Add defined toolbar to the window

        Currently known toolbars are:
            * map
            * digit
            * georect
        """
        if name == "map":
            self.maptoolbar = toolbars.MapToolbar(self, self.Map)

            self._mgr.AddPane(self.maptoolbar.toolbar,
                              wx.aui.AuiPaneInfo().
                              Name("maptoolbar").Caption("Map Toolbar").
                              ToolbarPane().Top().
                              LeftDockable(False).RightDockable(False).
                              BottomDockable(False).TopDockable(True).
                              CloseButton(False).Layer(2))

        elif name == "digit":
            self.digittoolbar = toolbars.DigitToolbar(self, self.Map)

            for toolRow in range(0,2):
                self._mgr.AddPane(self.digittoolbar.toolbar[toolRow],
                                  wx.aui.AuiPaneInfo().
                                  Name("digittoolbar" + str(toolRow)).Caption("Digit Toolbar").
                                  ToolbarPane().Top().Row(toolRow + 1).
                                  LeftDockable(False).RightDockable(False).
                                  BottomDockable(False).TopDockable(True).
                                  CloseButton(False).Layer(2))

            # change mouse to draw digitized line
            self.MapWindow.mouse['box'] = "point"
            self.MapWindow.zoomtype = 0
            self.MapWindow.pen     = wx.Pen(colour='red',   width=2, style=wx.SOLID)
            self.MapWindow.polypen = wx.Pen(colour='green', width=2, style=wx.SOLID)

        elif name == "georect":
            self.grtoolbar = toolbars.GRToolbar(self, self.Map)

            self._mgr.AddPane(self.grtoolbar.toolbar,
                              wx.aui.AuiPaneInfo().
                              Name("grtoolbar").Caption("Georectification Toolbar").
                              ToolbarPane().Top().
                              LeftDockable(False).RightDockable(False).
                              BottomDockable(False).TopDockable(True).
                              CloseButton(False).Layer(2))

        self._mgr.Update()

    def RemoveToolbar (self, name):
        """
        Removes toolbar from the window

        TODO: Only hide, activate by calling AddToolbar()
        """

        # cannot hide main toolbar
        if name == "map":
            return
        elif name == "digit":
            # TODO: not destroy only hide
            for toolRow in range(0,2):
                self._mgr.DetachPane (self.digittoolbar.toolbar[toolRow])
                self.digittoolbar.toolbar[toolRow].Destroy()
            self.digittoolbar = None

        self.maptoolbar.combo.SetValue ("");
        self._mgr.Update()

    def __InitDisplay(self):
        """
        Initialize map display, set dimensions and map region
        """
        self.width, self.height = self.GetClientSize()

        Debug.msg(2, "MapFrame.__InitDisplay():")
        self.Map.ChangeMapSize(self.GetClientSize())
        self.Map.GetRegion() # g.region -upg
        self.Map.SetRegion() # adjust region to match display window

    def OnFocus(self, event):
        """
        Change choicebook
        page to match display
        """

        # change bookcontrol page to page associted with display
        if self.page:
            pgnum = self.layerbook.GetPageIndex(self.page)
            if pgnum > -1:
                self.layerbook.SetSelection(pgnum)
        event.Skip()

    def OnMotion(self, event):
        """
        Mouse moved
        Track mouse motion and update status bar
        """
        # update statusbar if required
        e, n = self.MapWindow.Pixel2Cell(event.GetPositionTuple())
        if self.statusText == "Coordinates":
            self.statusbar.SetStatusText("%.2f,%.2f" % (e, n), 0)

        event.Skip()

    def ReDraw(self, event):
        """
        Redraw button clicked
        """
        Debug.msg(3, "BufferedWindow.ReDraw():")
        self.MapWindow.UpdateMap(render=False)

    def ReRender(self, event):
        """
        Rerender button clicked
        """
        Debug.msg(3, "BufferedWindow.ReRender():")
        self.MapWindow.UpdateMap(render=True)

    def OnPointer(self, event):
        """Pointer button clicled"""

        self.MapWindow.mouse['use'] = "pointer"
        self.MapWindow.mouse['box'] = "point"

        # change the cursor
        if self.digittoolbar:
            # digitization tool activated
            self.MapWindow.SetCursor(self.cursors["cross"])

            # reset mouse['box'] if needed
            if self.digittoolbar.action in ['addLine']:
                if self.digittoolbar.type in ['point', 'centroid']:
                    self.MapWindow.mouse['box'] = 'point'
                else: # line, boundary
                    self.MapWindow.mouse['box'] = 'line'
            elif self.digittoolbar.action in ['addVertex', 'removeVertex', 'splitLine',
                                         'editLine', 'displayCats', 'displayAttrs',
                                         'copyCats', 'connectLine']:
                self.MapWindow.mouse['box'] = 'point'
            else: # moveLine, deleteLine
                self.MapWindow.mouse['box'] = 'box'
        else:
            self.MapWindow.SetCursor(self.cursors["default"])

    def OnZoomIn(self, event):
        """
        Zoom in the map.
        Set mouse cursor, zoombox attributes, and zoom direction
        """
        self.MapWindow.mouse['use'] = "zoom"
        self.MapWindow.mouse['box'] = "box"
        self.MapWindow.zoomtype = 1
        self.MapWindow.pen = wx.Pen(colour='Red', width=2, style=wx.SHORT_DASH)

        # change the cursor
        self.MapWindow.SetCursor(self.cursors["cross"])

    def OnZoomOut(self, event):
        """
        Zoom out the map.
        Set mouse cursor, zoombox attributes, and zoom direction
        """
        self.MapWindow.mouse['use'] = "zoom"
        self.MapWindow.mouse['box'] = "box"
        self.MapWindow.zoomtype = -1
        self.MapWindow.pen = wx.Pen(colour='Red', width=2, style=wx.SHORT_DASH)

        # change the cursor
        self.MapWindow.SetCursor(self.cursors["cross"])

    def OnZoomBack(self, event):
        """
        Zoom last (previously stored position)
        """
        self.MapWindow.ZoomBack()

    def OnPan(self, event):
        """
        Panning, set mouse to drag
        """
        self.MapWindow.mouse['use'] = "pan"
        self.MapWindow.mouse['box'] = "pan"
        self.MapWindow.zoomtype = 0
#        event.Skip()

        # change the cursor
        self.MapWindow.SetCursor(self.cursors["hand"])

    def OnErase(self, event):
        """
        Erase the canvas
        """
        self.MapWindow.EraseMap()

    def OnZoomRegion(self, event):
        """
        Zoom to region
        """
        self.Map.getRegion()
        self.Map.getResolution()
        self.UpdateMap()
        # event.Skip()

    def OnAlignRegion(self, event):
        """
        Align region
        """
        if not self.Map.alignRegion:
            self.Map.alignRegion = True
        else:
            self.Map.alignRegion = False
        # event.Skip()

    def OnToggleRender(self, event):
        """Enable/disable auto-rendering"""
        if self.autoRender.GetValue():
            self.ReRender(None)

    def OnToggleShowRegion(self, event):
        """Show/Hide extent in map canvas"""
        if self.showRegion.GetValue():
            # show extent
            self.MapWindow.regionCoords = []
        else:
            del self.MapWindow.regionCoords

        # redraw map if auto-rendering is enabled
        if self.autoRender.GetValue(): 
            self.ReRender(None)


    def OnToggleStatus(self, event):
        """Toggle status text"""
        self.statusText = event.GetString()
        self.StatusbarUpdate()

    def OnChangeMapScale(self, event):
        """Map scale changed by user"""
        scale = event.GetString()
        
        try:
            if scale[:2] != '1:':
                raise ValueError
            value = int(scale[2:])
        except ValueError:
            self.mapScale.SetValue('1:' + str(int(self.mapScaleValue)))
            return 

        dEW = value * (self.Map.region['cols'] / self.ppm[0])
        dNS = value * (self.Map.region['rows'] / self.ppm[1])
        self.Map.region['n'] = self.Map.region['center_northing'] + dNS / 2
        self.Map.region['s'] = self.Map.region['center_northing'] - dNS / 2
        self.Map.region['w'] = self.Map.region['center_easting']  - dEW / 2
        self.Map.region['e'] = self.Map.region['center_easting']  + dEW / 2

        # add to zoom history
        self.MapWindow.ZoomHistory(self.Map.region['n'], self.Map.region['s'],
                                   self.Map.region['e'], self.Map.region['w'])

        # redraw a map
        self.MapWindow.UpdateMap()

    def StatusbarUpdate(self):
        """Update statusbar content"""

        self.showRegion.Hide()
        self.mapScale.Hide()
        self.mapScaleValue = self.ppm = None

        if self.statusText == "Coordinates":
            self.statusbar.SetStatusText("", 0)

        elif self.statusText == "Extent":
            self.statusbar.SetStatusText("%.2f-%.2f,%.2f-%.2f" %
                                         (self.Map.region["w"], self.Map.region["e"],
                                          self.Map.region["s"], self.Map.region["n"]), 0)
            self.showRegion.Show()

        elif self.statusText == "Geometry":
            self.statusbar.SetStatusText("rows=%d;cols=%d;nsres=%.2f;ewres=%.2f" %
                                         (self.Map.region["rows"], self.Map.region["cols"],
                                          self.Map.region["nsres"], self.Map.region["ewres"]), 0)
        elif self.statusText == "Map scale":
            # TODO: need to be fixed...
            ### screen X region problem
            ### user should specify ppm
            dc = wx.ScreenDC()
            dpSizePx = wx.DisplaySize()   # display size in pixels
            dpSizeMM = wx.DisplaySizeMM() # display size in mm (system)
            dpSizeIn = (dpSizeMM[0] / 25.4, dpSizeMM[1] / 25.4) # inches
            sysPpi  = dc.GetPPI()
            comPpi = (dpSizePx[0] / dpSizeIn[0],
                      dpSizePx[1] / dpSizeIn[1])

            ppi = comPpi                  # pixel per inch
            self.ppm = ((ppi[0] / 2.54) * 100, # pixel per meter
                        (ppi[1] / 2.54) * 100)
            
            Debug.msg(4, "MapFrame.StatusbarUpdate(mapscale): size: px=%d,%d mm=%f,%f "
                      "in=%f,%f ppi: sys=%d,%d com=%d,%d; ppm=%f,%f" % \
                          (dpSizePx[0], dpSizePx[1], dpSizeMM[0], dpSizeMM[1],
                           dpSizeIn[0], dpSizeIn[1],
                           sysPpi[0], sysPpi[1], comPpi[0], comPpi[1],
                           self.ppm[0], self.ppm[1]))

            region = self.Map.region

            heightCm = region['rows'] / self.ppm[1] * 100
	    widthCm  = region['cols'] / self.ppm[0] * 100

            Debug.msg(4, "MapFrame.StatusbarUpdate(mapscale): width_cm=%f, height_cm=%f" %
                      (widthCm, heightCm))

            xscale = (region['e'] - region['w']) / (region['cols'] / self.ppm[0])
            yscale = (region['n'] - region['s']) / (region['rows'] / self.ppm[1])
            scale = (xscale + yscale) / 2
            Debug.msg(3, "MapFrame.StatusbarUpdate(mapscale): xscale=%f, yscale=%f -> scale=%f" % \
                          (xscale, yscale, scale))

            self.statusbar.SetStatusText("")
            self.mapScale.SetValue("1:%ld" % scale)
            self.mapScaleValue = scale
            self.mapScale.Show()

        else:
            self.statusbar.SetStatusText("", 1)

    def StatusbarReposition(self):
        """Reposition checkbox in statusbar"""
        # reposition checkbox
        widgets = [(0, self.showRegion),
                   (0, self.mapScale),
                   (1, self.toggleStatus),
                   (2, self.autoRender)]
        for idx, win in widgets:
            rect = self.statusbar.GetFieldRect(idx)
            if idx == 0: # show region / mapscale
                wWin, hWin = win.GetBestSize()
                if win == self.showRegion:
                    x, y = rect.x + rect.width - wWin, rect.y-1
                else:
                    x, y = rect.x + 3, rect.y-1
                w, h = wWin, rect.height+2
            else: # choice || auto-rendering
                x, y = rect.x, rect.y-1
                w, h = rect.width, rect.height+2

            win.SetPosition((x, y))
            win.SetSize((w, h))

    def SaveToFile(self, event):
        """
        Save image to file
        """
        filetype =  "PNG file (*.png)|*.png|"\
                    "TIF file (*.tif)|*.tif|"\
                    "GIF file (*.gif)|*.gif"

        dlg = wx.FileDialog(self, "Choose a file name to save the image as a PNG to",
            defaultDir = "",
            defaultFile = "",
            wildcard = filetype,
            style=wx.SAVE|wx.FD_OVERWRITE_PROMPT)
        if dlg.ShowModal() == wx.ID_OK:
            base = os.path.splitext(dlg.GetPath())[0]
            ext = os.path.splitext(dlg.GetPath())[1]
            if dlg.GetFilterIndex() == 0:
                type = wx.BITMAP_TYPE_PNG
                path = dlg.GetPath()
                if ext != '.png': path = base+'.png'
            elif dlg.GetFilterIndex() == 1:
                type = wx.BITMAP_TYPE_TIF
                if ext != '.tif': path = base+'.tif'
            elif dlg.GetFilterIndex() == 2:
                type = wx.BITMAP_TYPE_TIF
                if ext != '.gif': path = base+'.gif'
            self.MapWindow.SaveToFile(path, type)

        dlg.Destroy()

    def PrintMenu(self, event):
        """
        Print map display
        """

        """
        Print options and output menu
        """
        point = wx.GetMousePosition()
        printmenu = wx.Menu()
        # Add items to the menu
        setup = wx.MenuItem(printmenu, wx.ID_ANY,'Page setup')
        printmenu.AppendItem(setup)
        self.Bind(wx.EVT_MENU, self.printopt.OnPageSetup, setup)

        preview = wx.MenuItem(printmenu, wx.ID_ANY,'Print preview')
        printmenu.AppendItem(preview)
        self.Bind(wx.EVT_MENU, self.printopt.OnPrintPreview, preview)

        doprint = wx.MenuItem(printmenu, wx.ID_ANY,'Print display')
        printmenu.AppendItem(doprint)
        self.Bind(wx.EVT_MENU, self.printopt.OnDoPrint, doprint)

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(printmenu)
        printmenu.Destroy()

    def OnCloseWindow(self, event):
        """
        Window closed
        Also close associated layer tree page
        """
        pgnum = None
        self.Map.Clean()
        if self.page:
            pgnum = self.layerbook.GetPageIndex(self.page)
            if pgnum > -1:
                self.layerbook.DeletePage(pgnum)

        #self.Destroy()

    def GetRender(self):
        """
        Returns the current instance of render.Map()
        """
        return self.Map

    def OnQuery(self, event):
        """
        Query currrent or last map
        """
        # switch GIS Manager to output console to show query results
        self.gismanager.notebook.SetSelection(1)

        self.MapWindow.mouse['use'] = "query"
        self.MapWindow.mouse['box'] = "point"
        self.MapWindow.zoomtype = 0
        # event.Skip()

        # change the cursor
        self.MapWindow.SetCursor(self.cursors["cross"])

    def QueryMap(self, x, y):
        """
        Run *.what command in gis manager output window
        """
        #set query snap distance for v.what at mapunit equivalent of 10 pixels
        qdist = 10.0 * ((self.Map.region['e'] - self.Map.region['w']) / self.Map.width)
        east,north = self.MapWindow.Pixel2Cell((x, y))

        if self.tree.GetSelections():
            mapname = None
            raststr = ''
            vectstr = ''
            rcmd = []
            vcmd = []
            for layer in self.tree.GetSelections():
                type =   self.tree.layers[layer].type
                dcmd = self.tree.GetPyData(layer)[0]
                if type in ('raster', 'rgb', 'his'):
                    for item in dcmd:
                        if 'map=' in item:
                            raststr += "%s," % item.split('=')[1]
                        elif 'red=' in item:
                            raststr += "%s," % item.split('=')[1]
                        elif 'h_map=' in item:
                            raststr += "%s," % item.split('=')[1]
                elif type in ('vector', 'thememap', 'themechart'):
                    for item in dcmd:
                        if 'map=' in item:
                            vectstr += "%s," % item.split('=')[1]

            # build query commands for any selected rasters and vectors
            if raststr != '':
                raststr = raststr.rstrip(',')
                rcmd = ['r.what',
                        '-f',
                        'input=%s' % (raststr),
                        'east_north=%f,%f' % (float(east), float(north))]
            if vectstr != '':
                vectstr = vectstr.rstrip(',')
                vcmd = ['v.what',
                        '-a',
                        'map=%s' % vectstr,
                        'east_north=%f,%f' % (float(east), float(north)),
                        'distance=%f' % qdist]
        else:
            dlg = wx.MessageDialog(self, _('You must select a map in the GIS Manager to query'),
                                   _('Nothing to query'), wx.OK | wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()
            return

        # parse query command(s)
        if self.gismanager:
            if rcmd:
                self.gismanager.goutput.RunCmd(' '.join(rcmd))
            if vcmd:
                self.gismanager.goutput.RunCmd(' '.join(vcmd))
        else:
            os.system(' '.join(rcmd))
            os.system(' '.join(vcmd))

    def OnAnalyze(self, event):
        """
        Analysis tools menu
        """
        point = wx.GetMousePosition()
        toolsmenu = wx.Menu()
        # Add items to the menu
        measure = wx.MenuItem(toolsmenu, wx.ID_ANY, Icons["measure"].GetLabel())
        measure.SetBitmap(Icons["measure"].GetBitmap(self.iconsize))
        toolsmenu.AppendItem(measure)
        self.Bind(wx.EVT_MENU, self.OnMeasure, measure)

        profile = wx.MenuItem(toolsmenu, wx.ID_ANY, Icons["profile"].GetLabel())
        profile.SetBitmap(Icons["profile"].GetBitmap(self.iconsize))
        toolsmenu.AppendItem(profile)
        self.Bind(wx.EVT_MENU, self.Profile, profile)

        histogram = wx.MenuItem(toolsmenu, wx.ID_ANY, Icons["histogram"].GetLabel())
        histogram.SetBitmap(Icons["histogram"].GetBitmap(self.iconsize))
        toolsmenu.AppendItem(histogram)
        self.Bind(wx.EVT_MENU, self.Histogram, histogram)

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(toolsmenu)
        toolsmenu.Destroy()

    def OnMeasure(self, event):
        """
        Init measurement routine that calculates
        map distance along transect drawn on
        map display
        """

        self.totaldist = 0.0 # total measured distance

        # switch GIS Manager to output console to show measure results
        self.gismanager.notebook.SetSelection(1)

        # change mouse to draw line for measurement
        self.MapWindow.mouse['use'] = "measure"
        self.MapWindow.mouse['box'] = "line"
        self.MapWindow.zoomtype = 0
        self.MapWindow.pen = wx.Pen(colour='red', width=2, style=wx.SHORT_DASH)
        self.MapWindow.polypen = wx.Pen(colour='green', width=2, style=wx.SHORT_DASH)

        # change the cursor
        self.MapWindow.SetCursor(self.cursors["pencil"])

        # initiating output
        if self.projinfo['proj'] != 'xy':
            units = self.projinfo['units']
            self.gismanager.goutput.cmd_output.write('\nMeasuring distance ('+units+'):\n')
        else:
            self.gismanager.goutput.cmd_output.write('\nMeasuring distance:\n')

    def MeasureDist(self, beginpt, endpt):
        """
        Calculate map distance from screen distance
        and print to output window
        """
        dist, (north, east) = self.MapWindow.Distance(beginpt, endpt)

        dist = round(dist, 3)
        d, dunits = self.FormatDist(dist)

        self.totaldist += dist
        td, tdunits = self.FormatDist(self.totaldist)

        strdist = str(d)
        strtotdist = str(td)

        if self.projinfo['proj'] == 'xy' or 'degree' not in self.projinfo['unit']:
            angle = int(math.degrees(math.atan2(north,east)) + 0.5)
            angle = angle+90
            if angle < 0: angle = 360+angle

            mstring = 'segment = %s %s\ttotal distance = %s %s\tbearing = %d deg\n' \
                % (strdist,dunits,strtotdist,tdunits,angle)
        else:
            mstring = 'segment = %s %s\ttotal distance = %s %s\n' \
                % (strdist,dunits,strtotdist,tdunits)

        self.gismanager.goutput.cmd_output.write(mstring)

        return dist

    def Profile(self, event):
        """
        Init profile canvas and tools
        """
        self.profile = profile.ProfileFrame(self,
                                           id=wx.ID_ANY, pos=wx.DefaultPosition, size=(700,300),
                                           style=wx.DEFAULT_FRAME_STYLE)
        self.profile.Show()

    def FormatDist(self, dist):
        """Format length numbers and units in a nice way,
        as a function of length. From code by Hamish Bowman
        Grass Development Team 2006"""

        mapunits = self.projinfo['units']
        if mapunits == 'metres': mapunits = 'meters'
        outunits = mapunits
        dist = float(dist)
        divisor = 1.0

        # figure out which units to use
        if mapunits == 'meters':
            if dist > 2500.0:
                outunits = 'km'
                divisor = 1000.0
            else: outunits = 'm'
        elif mapunits == 'feet':
            # nano-bug: we match any "feet", but US Survey feet is really
            #  5279.9894 per statute mile, or 10.6' per 1000 miles. As >1000
            #  miles the tick markers are rounded to the nearest 10th of a
            #  mile (528'), the difference in foot flavours is ignored.
            if dist > 5280.0:
                outunits = 'miles'
                divisor = 5280.0
            else:
                outunits = 'ft'
        elif 'degree' in mapunits:
            if dist < 1:
                outunits = 'min'
                divisor = (1/60.0)
            else:
                outunits = 'deg'

        # format numbers in a nice way
        if (dist/divisor) >= 2500.0:
            outdist = round(dist/divisor)
        elif (dist/divisor) >= 1000.0:
            outdist = round(dist/divisor,1)
        elif (dist/divisor) > 0.0:
            outdist = round(dist/divisor,int(math.ceil(3-math.log10(dist/divisor))))
        else:
            outdist = float('%g' % dist/divisor)

        return (outdist, outunits)


    def Histogram(self, event):
        """
        Init histogram display canvas and tools
        """
        self.histogram = histogram.HistFrame(self,
                                             id=wx.ID_ANY, pos=wx.DefaultPosition, size=(400,300),
                                             style=wx.DEFAULT_FRAME_STYLE)

        #show new display
        self.histogram.Show()
        self.histogram.Refresh()
        self.histogram.Update()


    def OnDecoration(self, event):
        """
        Decorations overlay menu
        """
        point = wx.GetMousePosition()
        decmenu = wx.Menu()
        # Add items to the menu
        addscale = wx.MenuItem(decmenu, wx.ID_ANY, Icons["addbarscale"].GetLabel())
        addscale.SetBitmap(Icons["addbarscale"].GetBitmap(self.iconsize))
        decmenu.AppendItem(addscale)
        self.Bind(wx.EVT_MENU, self.AddBarscale, addscale)

        AddLegend = wx.MenuItem(decmenu, wx.ID_ANY, Icons["addlegend"].GetLabel())
        AddLegend.SetBitmap(Icons["addlegend"].GetBitmap(self.iconsize))
        decmenu.AppendItem(AddLegend)
        self.Bind(wx.EVT_MENU, self.AddLegend, AddLegend)

        addtext = wx.MenuItem(decmenu, wx.ID_ANY, Icons["addtext"].GetLabel())
        addtext.SetBitmap(Icons["addtext"].GetBitmap(self.iconsize))
        decmenu.AppendItem(addtext)
        self.Bind(wx.EVT_MENU, self.AddText, addtext)

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(decmenu)
        decmenu.Destroy()

    def AddBarscale(self, event):
        """
        Handler for scale/arrow map decoration menu selection.
        """

        ovltype = id = 0 # index for overlay layer in render

        if ovltype in self.params:
            params = self.params[ovltype]
        else:
            params = ''

        # If location is latlon, only display north arrow (scale won't work)
#        proj = self.projinfo['proj']
#        if proj == 'll':
#            barcmd = 'd.barscale -n'
#        else:
#            barcmd = 'd.barscale'

        # decoration overlay control dialog
        dlg = DecDialog(self, wx.ID_ANY, _('Scale and North arrow'), size=(350, 200),
                        style=wx.DEFAULT_DIALOG_STYLE,
                        ovltype=ovltype,
                        cmd='d.barscale',
                        drawid=id,
                        checktxt = _("Show/hide scale and arrow"),
                        ctrltxt = _("scale object"),
                        params = params)

        dlg.CenterOnScreen()

#        if ovltype not in self.Map.ovlookup:
#            self.Map.AddOverlay(ovltype, type='overlay', command=['d.barscale'], l_active=False, l_render=False)


        # if OK button pressed in decoration control dialog
        if dlg.ShowModal() == wx.ID_OK:
            if self.ovlchk[id] == True:
                # get overlay images (overlay must be active)
                if not self.Map.ovlookup[ovltype].active:
                    self.Map.ovlookup[ovltype].active = True
                    self.Map.Render(force=True)

                ovldict = self.MapWindow.GetOverlay()

                if id not in ovldict:
                    self.MapWindow.UpdateMap()
                    return

                img = ovldict[id]

                if id not in self.ovlcoords:
                    self.ovlcoords[id] = [10,10]

                self.MapWindow.Draw(self.MapWindow.pdc, drawid=id,
                                    img=img, pdctype='image',
                                    coords=self.ovlcoords[id])

        self.MapWindow.UpdateMap()
        dlg.Destroy()

        # close properties dialog if open
        #        try:
        #            self.propwin[ovltype].Close(True)
        #        except:
        #            pass

    def AddLegend(self, event):
        """
        Handler for legend map decoration menu selection.
        """
        ovltype = id = 1 # index for overlay layer in render

        if ovltype in self.params:
            params = self.params[ovltype]
        else:
            params = ''

        # Decoration overlay control dialog
        dlg = DecDialog(self, wx.ID_ANY, 'Legend', size=(350, 200),
                         style=wx.DEFAULT_DIALOG_STYLE,
                         ovltype=ovltype,
                         cmd='d.legend',
                         drawid=id,
                         checktxt = "Show/hide legend",
                         ctrltxt = "legend object",
                         params = params)

        dlg.CenterOnScreen()

        # If OK button pressed in decoration control dialog
        val = dlg.ShowModal()
        if val == wx.ID_OK:

            if self.ovlchk[id] == True:
                # get overlay images (overlay must be active)
                if not self.Map.ovlookup[ovltype].active:
                    self.Map.ovlookup[ovltype].active = True
                    self.Map.Render(force=True)

                ovldict = self.MapWindow.GetOverlay()

                if id not in ovldict:
                    self.MapWindow.UpdateMap()
                    return

                img = ovldict[id]

                if id not in self.ovlcoords:
                    self.ovlcoords[id] = [10,10]

                self.MapWindow.Draw(self.MapWindow.pdc, drawid=id,
                                    img=img, pdctype='image',
                                    coords=self.ovlcoords[id])

        self.MapWindow.UpdateMap()
        dlg.Destroy()
        # close properties dialog if open
#        try:
#            self.propwin[ovltype].Close(True)
#        except:
#            pass

    def AddText(self, event):
        """
        Handler for text decoration menu selection.
        """
        ovltype = 2 # index for overlay layer in render

        # default values
        maptext = ''
        textfont = self.GetFont()
        textcolor = wx.BLACK
        textcoords = [10, 10, 10, 10]
        rotation = 0.0

        if self.MapWindow.currtxtid == None: # text doesn't already exist
            id = wx.NewId() + 100
        else: # text already exists
            id = self.MapWindow.currtxtid
            textcoords = self.ovlcoords[id]

        dlg = TextDialog(self, wx.ID_ANY, 'Text', size=(400, 200),
                         style=wx.DEFAULT_DIALOG_STYLE,
                         ovltype=ovltype,
                         drawid=id)

        dlg.CenterOnScreen()

        # If OK button pressed in decoration control dialog
        val = dlg.ShowModal()
        if val == wx.ID_OK:
            maptext    = dlg.currText
            textfont   = dlg.currFont
            textcolor  = dlg.currClr
            rotation   = dlg.currRot
            coords,w,h = self.MapWindow.TextBounds((maptext, textfont, textcolor, rotation),textcoords)

        # delete object if if it has no text
        if maptext == '':
            try:
                self.MapWindow.pdc.ClearId(id)
                self.MapWindow.pdc.RemoveId(id)
                del self.MapWindow.textdict[id]
                del self.ovlcoords[id]
            except:
                pass
            return

        self.MapWindow.pdc.ClearId(id)
        self.MapWindow.pdc.SetId(id)
        self.MapWindow.textdict[id] = (maptext,textfont,textcolor,rotation)
        self.MapWindow.Draw(self.MapWindow.pdc, img=self.MapWindow.textdict[id],
                            drawid=id, pdctype='text', coords=textcoords)
        self.MapWindow.UpdateMap()


    def GetOptData(self, dcmd, type, params, propwin):
        """
        Callback method for decoration overlay command generated by
        dialog created in menuform.py
        """
        # Reset comand and rendering options in render.Map. Always render decoration.
        # Showing/hiding handled by PseudoDC
        self.Map.ChangeOverlay(ovltype=type, type='overlay', name='', command=dcmd,
                               l_active=True, l_render=False)
        self.params[type] = params
        self.propwin[type] = propwin

    def OnZoomMenu(self, event):
        """
        Zoom menu
        """
        point = wx.GetMousePosition()
        zoommenu = wx.Menu()
        # Add items to the menu
        zoommap = wx.MenuItem(zoommenu, wx.ID_ANY,'Zoom to selected map')
        zoommenu.AppendItem(zoommap)
        self.Bind(wx.EVT_MENU, self.MapWindow.ZoomToMap, zoommap)

        zoomwind = wx.MenuItem(zoommenu, wx.ID_ANY,'Zoom to computational region (set with g.region)')
        zoommenu.AppendItem(zoomwind)
        self.Bind(wx.EVT_MENU, self.MapWindow.ZoomToWind, zoomwind)

        savewind = wx.MenuItem(zoommenu, wx.ID_ANY,'Set computational region from display')
        zoommenu.AppendItem(savewind)
        self.Bind(wx.EVT_MENU, self.MapWindow.DisplayToWind, savewind)

        zoomsaved = wx.MenuItem(zoommenu, wx.ID_ANY,'Zoom to saved region')
        zoommenu.AppendItem(zoomsaved)
        self.Bind(wx.EVT_MENU, self.MapWindow.ZoomToSaved, zoomsaved)

        savezoom = wx.MenuItem(zoommenu, wx.ID_ANY,'Save display geometry to named region')
        zoommenu.AppendItem(savezoom)
        self.Bind(wx.EVT_MENU, self.MapWindow.SaveDisplayRegion, savezoom)

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(zoommenu)
        zoommenu.Destroy()

# end of class MapFrame

class DecDialog(wx.Dialog):
    """
    Controls setting options and displaying/hiding map overlay decorations
    """
    def __init__(self, parent, id, title, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_DIALOG_STYLE, ovltype=0, cmd='d.barscale',
                 drawid=None, checktxt='', ctrltxt='', params=''):
        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        self.ovltype = ovltype
        self.drawid  = drawid
        self.ovlcmd  = cmd
        self.parent  = parent
        self.ovlchk  = self.parent.MapWindow.ovlchk
        self.params  = params #previously set decoration options to pass back to options dialog

        if self.ovltype not in self.parent.Map.ovlookup:
            self.parent.Map.AddOverlay(self.ovltype, type='overlay', command=[self.ovlcmd], l_active=False, l_render=False)

        #self.MakeModal(True)

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.chkbox = wx.CheckBox(self, wx.ID_ANY, checktxt)
        if not drawid in self.ovlchk:
            self.ovlchk[drawid] = True
        self.chkbox.SetValue(self.ovlchk[drawid])
        box.Add(self.chkbox, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        optnbtn = wx.Button(self, wx.ID_ANY, "Set options")
        box.Add(optnbtn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        label = wx.StaticText(self, wx.ID_ANY, ("Drag %s with mouse in pointer mode\nto position. Double-click to change options" % ctrltxt))
        box.Add(label, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        line = wx.StaticLine(self, wx.ID_ANY, size=(20,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(line, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.TOP, 5)

        btnsizer = wx.StdDialogButtonSizer()

        btn = wx.Button(self, wx.ID_OK)
        btn.SetDefault()
        btnsizer.AddButton(btn)

        btn = wx.Button(self, wx.ID_CANCEL)
        btnsizer.AddButton(btn)
        btnsizer.Realize()

        sizer.Add(btnsizer, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        self.SetSizer(sizer)
        sizer.Fit(self)

        self.Bind(wx.EVT_CHECKBOX, self.OnCheck,   self.chkbox)
        self.Bind(wx.EVT_BUTTON,   self.OnOptions, optnbtn)

    def OnCheck(self, event):
        """
        Handler for checkbox for displaying/hiding decoration
        """
        check = event.IsChecked()
        self.ovlchk[self.drawid] = check

    def OnOptions(self, event):
        """
        Sets option for decoration map overlays
        """

        # display properties dialog (modal mode)
        menuform.GUI().ParseCommand(self.ovlcmd, gmpath,
                                    completed=(self.parent.GetOptData, self.ovltype, self.params),
                                    parentframe=self, modal=True)

class TextDialog(wx.Dialog):
    """
    Controls setting options and displaying/hiding map overlay decorations
    """

    def __init__(self, parent, id, title, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_DIALOG_STYLE,
                 ovltype=2,drawid=None):

        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        self.ovltype = ovltype
        self.drawid  = drawid
        self.parent  = parent

        if drawid in self.parent.MapWindow.textdict:
            self.currText, self.currFont, self.currClr, self.currRot = self.parent.MapWindow.textdict[drawid]
        else:
            self.currClr = wx.BLACK
            self.currText = ''
            self.currFont = self.GetFont()
            self.currRot = 0.0

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        label = wx.StaticText(self, wx.ID_ANY, _("Enter text:"))
        box.Add(label, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        self.textentry = wx.TextCtrl(self, wx.ID_ANY, "", size=(200,-1))
        self.textentry.SetFont(self.currFont)
        self.textentry.SetForegroundColour(self.currClr)
        self.textentry.SetValue(self.currText)
        box.Add(self.textentry, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        label = wx.StaticText(self, wx.ID_ANY, "Rotation:")
        box.Add(label, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        self.rotation = wx.SpinCtrl(self, id=wx.ID_ANY, value="", pos=(30, 50),
                        size=(75,-1), style=wx.SP_ARROW_KEYS)
        self.rotation.SetRange(-360, 360)
        self.rotation.SetValue(int(self.currRot))
        box.Add(self.rotation, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        fontbtn = wx.Button(self, wx.ID_ANY, "Set font")
        box.Add(fontbtn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        label = wx.StaticText(self, wx.ID_ANY, ("Drag text with mouse in pointer mode\nto position. Double-click to change options"))
        box.Add(label, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
        sizer.Add(box, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        line = wx.StaticLine(self, wx.ID_ANY, size=(20,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(line, 0, wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.TOP, 5)

        btnsizer = wx.StdDialogButtonSizer()

        btn = wx.Button(self, wx.ID_OK)
        btn.SetDefault()
        btnsizer.AddButton(btn)

        btn = wx.Button(self, wx.ID_CANCEL)
        btnsizer.AddButton(btn)
        btnsizer.Realize()

        sizer.Add(btnsizer, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        self.SetSizer(sizer)
        sizer.Fit(self)

        # bindings
        self.Bind(wx.EVT_BUTTON,     self.OnSelectFont, fontbtn)
        self.Bind(wx.EVT_TEXT,       self.OnText,       self.textentry)
        self.Bind(wx.EVT_SPINCTRL,   self.OnRotation,   self.rotation)

    def OnText(self, event):
        """Change text string"""
        self.currText = event.GetString()

    def OnRotation(self, event):
        """Change rotation"""
        self.currRot = event.GetInt()
        Debug.msg (5, "TextDialog.OnRotation(): rotation=%f" % \
               self.currRot)

        event.Skip()

    def OnSelectFont(self, event):
        """Change font"""
        data = wx.FontData()
        data.EnableEffects(True)
        data.SetColour(self.currClr)         # set colour
        data.SetInitialFont(self.currFont)

        dlg = wx.FontDialog(self, data)

        if dlg.ShowModal() == wx.ID_OK:
            data = dlg.GetFontData()
            self.currFont = data.GetChosenFont()
            self.currClr = data.GetColour()

            self.textentry.SetFont(self.currFont)
            self.textentry.SetForegroundColour(self.currClr)

            self.Layout()

        dlg.Destroy()


class SavedRegion(wx.Dialog):
    def __init__(self, parent, id, title="", pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_DIALOG_STYLE,
                 loadsave='load'):
        """
        Loading and saving of display extents to saved region file
        """
        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        self.loadsave = loadsave
        self.wind = ''

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        if loadsave == 'load':
            label = wx.StaticText(self, wx.ID_ANY, "Load region:")
            box.Add(label, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
            self.selection = select.Select(self, id=wx.ID_ANY, size=(300,-1),
                                              type='windows')
            box.Add(self.selection, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
            self.selection.Bind(wx.EVT_TEXT, self.onSelection)

        elif loadsave == 'save':
            label = wx.StaticText(self, wx.ID_ANY, "Save region:")
            box.Add(label, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
            self.textentry = wx.TextCtrl(self, wx.ID_ANY, "", size=(200,-1))
            box.Add(self.textentry, 0, wx.ALIGN_CENTRE|wx.ALL, 5)
            self.textentry.Bind(wx.EVT_TEXT, self.onText)

        sizer.Add(item=box, proportion=0, flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL,
                  border=5)

        line = wx.StaticLine(self, wx.ID_ANY, size=(20,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(item=line, proportion=0,
                  flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, border=5)

        btnsizer = wx.StdDialogButtonSizer()

        btn = wx.Button(self, wx.ID_OK)
        btn.SetDefault()
        btnsizer.AddButton(btn)

        btn = wx.Button(self, wx.ID_CANCEL)
        btnsizer.AddButton(btn)
        btnsizer.Realize()


        sizer.Add(item=btnsizer, proportion=0, flag=wx.ALIGN_CENTER|wx.ALL, border=5)

        self.SetSizer(sizer)
        sizer.Fit(self)

    def onSelection(self, event):
        self.wind = event.GetString()

    def onText(self, event):
        self.wind = event.GetString()

class MapApp(wx.App):
    """
    MapApp class
    """

    def OnInit(self):
        wx.InitAllImageHandlers()
        if __name__ == "__main__":
            Map = render.Map() # instance of Map class to render GRASS display output to PPM file
        else:
            Map = None

        self.mapFrm = MapFrame(parent=None, id=wx.ID_ANY, Map=Map, size=(640,480))
        #self.SetTopWindow(Map)
        self.mapFrm.Show()

        if __name__ == "__main__":
            # redraw map, if new command appears
            self.redraw = False
            status = Command(self, Map)
            status.start()
            self.timer = wx.PyTimer(self.watcher)
            # check each 0.1s
            self.timer.Start(100)

        return 1

    def OnExit(self):
        if __name__ == "__main__":
            # stop the timer
            self.timer.Stop()
            # terminate thread (a bit ugly)
            os.system("""echo "quit" >> %s""" % (cmdfilename))

    def watcher(self):
        """Redraw, if new layer appears"""
        if self.redraw:
            self.mapFrm.ReDraw(None)
        self.redraw = False
        return
# end of class MapApp

if __name__ == "__main__":

    ###### SET command variable
    if len(sys.argv) != 3:
        print __doc__
        sys.exit()

    title = sys.argv[1]
    cmdfilename = sys.argv[2]

    import gettext
    gettext.install("gm_map") # replace with the appropriate catalog name

    print "Starting monitor <%s>" % (title)

    gm_map = MapApp(0)
    # set title
    gm_map.mapFrm.SetTitle ("GRASS GIS - Map Display: " + title + " - Location: " + \
                                grassenv.GetGRASSVariable("LOCATION_NAME"))
    gm_map.MainLoop()

    if grassenv.env.has_key("MONITOR"):
        os.system("d.mon sel=%s" % grassenv.GetGRASSVariable("MONITOR"))

    os.remove(cmdfilename)
    os.system("""g.gisenv set="GRASS_PYCMDFILE" """)

    print "Stoping monitor <%s>" % (title)

    sys.exit()
