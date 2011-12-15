#!/usr/bin/env python

# PlotWindow class (with cursor)
#
# File:           plotwindow.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2011-12-13
# Last change;    2011-12-13  
#
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

#import pyrap.quanta as pq   # used to convert date from double to normal format
import numpy as np
import lofar.bbs.plothistogram as ph          # TODO: need to move this into LOFAR python module
#import unicodedata
#import time        # for timing functions
#import datetime    # needed for timestamps
#import math
import matplotlib
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure


class Cursor:
    def __init__(self, ax, parent):
        self.parent=parent
        self.ax = ax
        self.lx = ax.axhline(color='r')  # the horiz line
        self.ly = ax.axvline(color='r')  # the vert line

        # text location in axes coords
        self.txt = ax.text( 0.7, 0.9, '', transform=ax.transAxes)

    def mouse_move(self, event):
        if not event.inaxes: return

        x, y = event.xdata, event.ydata
        # update the line positions
        self.lx.set_ydata(y )
        self.ly.set_xdata(x )

        self.txt.set_text( 'x=%1.2f, y=%1.2f'%(x,y) )
        self.parent.fig.canvas.draw()


class SnaptoCursor:
    """
    Like Cursor but the crosshair snaps to the nearest x,y point
    For simplicity, I'm assuming x is sorted
    """
    def __init__(self, ax, x, y, parent):
        self.parent=parent
        self.ax = ax
        self.lx = ax.axhline(color='r')  # the horiz line
        self.ly = ax.axvline(color='r')  # the vert line
        self.x = x
        self.y = y
        # text location in axes coords
        self.txt = ax.text( 0.7, 0.9, '', transform=ax.transAxes)

    def mouse_move(self, event):

        if not event.inaxes: return

        x, y = event.xdata, event.ydata

        indx = np.searchsorted(self.x, [x])[0]
        x = self.x[indx]
        y = self.y[indx]
        # update the line positions
        self.lx.set_ydata(y )
        self.ly.set_xdata(x )

        self.txt.set_text( 'x=%1.2f, y=%1.2f'%(x,y) )
        print 'x=%1.2f, y=%1.2f'%(x,y)
        self.parent.fig.canvas.draw()
        
# Example usage      
"""
t = arange(0.0, 1.0, 0.01)
s = sin(2*2*pi*t)
ax = subplot(111)

cursor = Cursor(ax)
#cursor = SnaptoCursor(ax, t, s)
connect('motion_notify_event', cursor.mouse_move)

ax.plot(t, s, 'o')
axis([0,1,-1,1])
show()
"""


# Class that pops up a plot window instance with its own plot
#
class PlotWindow(QFrame):

   # Init the class: create plot window and canvas layout (and corresponding buttons)
   #
   def __init__(self, parent):
      QDialog.__init__(self)

      self.rim=0.05                 # rim around plot

      # The plot class holds its data now, so that it can be exported after a different
      # PlotWindow has been created
      self.parent=parent            # parent object/class
      self.fig=None
      self.showCursor=False
      self.cursorId=None
      self.markerId=None
      self.marker1=None              # position marker in the plot
      self.marker2=None

      self.x = parent.x             # plotted x axis data
      self.y1 = parent.y1           # plotted y axis data
      self.y2 = parent.y2           # plotted y2 axis data
      self.messages = parent.messages   # solver messages

      # Create canvas for plotting
      self.fig = Figure((5, 4), dpi=100)
      self.canvas = FigureCanvas(self.fig)
      self.canvas.setParent(self)
      self.fig.subplots_adjust(left=self.rim+0.05, right=1.0-self.rim, top=1.0-self.rim, bottom=self.rim)  # set a small rim

      self.mpl_toolbar = NavigationToolbar(self.canvas, self)
      self.mpl_toolbar.show()   # first hide the toolbar

      self.setMinimumWidth(900)
      self.setMinimumHeight(600)

      self.setWindowTitle = self.parent.tableName  + ": " + str(self.parent.solverQuery.getRank()) + " Parameters"

      # Create Buttons for data export
      #
      self.exportButton=QPushButton("&Export Data")
      self.exportButton.setToolTip("Export the currently plotted data")
      self.exportButton.setMaximumWidth(100)

      self.exportComboBox=QComboBox()
      self.exportComboBox.addItem("ASCII")
      # export in Matlab format is only possible if scipy.io module has been imported
      if parent.haveModule('scipy') == True or parent.haveModule('scipy.io') == True:
         self.exportComboBox.addItem("Matlab")
      self.exportComboBox.setToolTip('File format for exporting data')
      self.exportComboBox.setMaximumWidth(100)
      self.exportComboBox.setMinimumHeight(25)

      self.showCursorCheckBox=QCheckBox("Show cursor")
      self.showCursorCheckBox.setToolTip("Show a marker line in plot")
      self.showCursorCheckBox.setCheckState(Qt.Unchecked)    # default off

      self.histogramButton=QPushButton("&Histogram")    # button to create a histogram
      self.histogramButton.setToolTip("Create a histogram of the current parameter")
      #self.histogramButton.hide()
      self.histogramButton.setMaximumWidth(120)
      self.histogramButton.setToolTip('Create a histogram of current data')

#      self.histogramBinSpin=QSpinBox()            # spinbox for histogram binsize
#      self.histogramBinSpin.setMinimum(5)
#      self.histogramBinSpin.setMaximum(100)
#      self.histogramBinSpin.setSingleStep(5)
#      self.histogramBinSpin.setMaximumWidth(120)
#      self.histogramBinSpin.setMinimumHeight(25)


      self.solverMessageLabel=QLabel("Solver Message:")

      self.solverMessageText=QLineEdit()
      self.solverMessageText.setText('Undefined')
      self.solverMessageText.setToolTip('Message returned by LSQFit after iteration')
      self.solverMessageText.setReadOnly(True)
      self.solverMessageText.setMaximumWidth(125)

      self.closeButton=QPushButton()
      self.closeButton.setText('Close')
      self.closeButton.setToolTip('close this plot window')
      self.closeButton.setMaximumWidth(120)


      # Set connections
      self.connect(self.exportButton, SIGNAL('clicked()'), self.on_export)
      self.connect(self.histogramButton, SIGNAL('clicked()'), self.on_histogram)
      self.connect(self.closeButton, SIGNAL('clicked()'), SLOT('close()'))
      self.connect(self.showCursorCheckBox, SIGNAL('stateChanged(int)'), self.on_cursor)

      # Layouts for canvas and buttons
      #
      #
      buttonLayout = QVBoxLayout()
      buttonLayout.addWidget(self.exportButton)
      buttonLayout.addWidget(self.exportComboBox)
      buttonLayout.addWidget(self.showCursorCheckBox)
      buttonLayout.addWidget(self.histogramButton)
      buttonLayout.addWidget(self.solverMessageLabel)
      buttonLayout.addWidget(self.solverMessageText)
      buttonLayout.insertStretch(-1)
      buttonLayout.addWidget(self.closeButton)
      #buttonLayout.insertStretch(-1)
      #buttonLayout.setMaximumWidth(160)

      # Canvas layout
      canvasLayout = QVBoxLayout()
      canvasLayout.addWidget(self.canvas, 1)
      canvasLayout.addWidget(self.mpl_toolbar)

      mainLayout = QHBoxLayout()
      mainLayout.addLayout(buttonLayout)
      mainLayout.addLayout(canvasLayout)
      self.setLayout(mainLayout)

      self.show()           # show the plotWindow widget
      self.parent.setXLabel()
      self.parent.setYLabel()
      
      # Matplotlib event connections
      #if self.showMarker==True:
      #  cid = self.fig.canvas.mpl_connect('motion_notify_event', self.update_marker)
      #  cid = self.fig.canvas.mpl_connect('button_press_event', onclick)

      cid = self.fig.canvas.mpl_connect('motion_notify_event', self.on_solverMessage)

      self.plot()


   # React on showMarkerCheckBox signal
   #
   """
   def on_marker(self):
      #print "on_marker checkState = ", self.showMarkerCheckBox.checkState()  # DEBUG
      if self.showMarkerCheckBox.isChecked()==True:
        self.showMarker=True
        self.markerId = self.fig.canvas.mpl_connect('motion_notify_event', self.update_marker)
      else:
        print "on_marker() disconnecting marker event"
        self.showMarker=False
        self.fig.canvas.mpl_disconnect(self.markerId)

   # Plot a vertical line marker on the solutions plot showing the solution
   # of the currently plotted solver parameters
   # TODO: EXPERIMENTAL
   #
   def update_marker(self, event):
      #print "plotMarker() pos = ", event.xdata       # DEBUG
      
      #print 'button=%d, x=%d, y=%d, xdata=%f, ydata=%f'%(
      #   event.button, event.x, event.y, event.xdata, event.ydata)     # DEBUG      
      
      # Create markers (vertical lines) in both plots
      self.marker1=self.ax1.axvline(x=event.xdata, linewidth=1.5, color='r')
      self.marker2=self.ax2.axvline(x=event.xdata, linewidth=1.5, color='r')
 
      # We need to remove all unnecessary marker in plot 1 and plot 2
      # TODO: find better method, keeps double marker sometimes
      while len(self.ax1.lines)-1 > 1:
        self.ax1.lines[len(self.ax1.lines)-1].remove()
      while len(self.ax2.lines)-1 > 1:
        self.ax2.lines[len(self.ax1.lines)-1].remove()  
       
      self.marker1=self.ax1.axvline(x=event.xdata, linewidth=1.5, color='r')
      self.marker2=self.ax1.axvline(x=event.xdata, linewidth=1.5, color='r')
#      self.canvas.draw_idle()
      self.canvas.draw()
   """


   # Activate / Deactivate Matplotlib demo cursor 
   #
   def on_cursor(self):
      print "on_cursor()"   # DEBUG
      self.showCursor=self.showCursorCheckBox.isChecked()
   
      print "on_cursor() self.showCursor = ", self.showCursor
   
      if self.showCursor==True:
        #self.cursor = SnaptoCursor(self.ax1, self.x, self.y1, self)
        self.cursor = Cursor(self.ax1, self)
        self.fig.canvas.mpl_connect('motion_notify_event', self.cursor.mouse_move)
        self.cursorId = self.fig.canvas.mpl_connect('button_press_event', self.on_click)
      else:
        #self.fig.delaxes(self.cursor.ax)
        #self.fig.delaxes(self.cursor.ly)
        self.cursor.lx.remove()
        self.cursor.ly.remove()
        self.fig.canvas.mpl_disconnect(self.cursorId)
        
        
   # Handle export button clicked()
   #
   def on_export(self):
      fileformat=self.exportComboBox.currentText()
      self.parent.on_export(fileformat)


   # Functio to execute on a click event (experimental)
   #
   def on_click(self, event):
      print 'button=%d, x=%d, y=%d, xdata=%f, ydata=%f'%(
         event.button, event.x, event.y, event.xdata, event.ydata)     # DEBUG      

      # Pick per iteration details if possible
      if self.parent.tableType == "PERITERATION" or self.tableType == "PERITERATION_CORRMATRIX":
          print "trying to get per iterations for this cell"        # DEBUG
      else:
          print "on_clickMarker() table is not of correct type"


   # Display a histogram of the converged solutions (i.e. LASTITER=TRUE)
   # for the currently selected parameter
   #
   def on_histogram(self):
      print "on_histogram()"    # DEBUG
      self.histoDialog=ph.plothistogram(self)
      self.histoDialog.show()


   def on_solverMessage(self, event):
      if self.messages!=None:                       # only if we indeed inherited messages from parent
        x, y = event.xdata, event.ydata             # get cursor position from figure
        index = np.searchsorted(self.x, [x])[0]     # get Message for the index of cursor position
    
        resultType=self.messages['result']
        self.solverMessageText.setText(self.messages[resultType][index])   


   # Plot data that has been read
   #
   def plot(self):
      print "PlotWindow::plot()"            # DEBUG

      parm=self.parent.parmsComboBox.currentText()   # Solution parameter, e.g. Gain:1:1:LBA001
      parameter=str(self.parent.parametersComboBox.currentText())    # Get solver parameter from drop down

      # Give for Time axis only time relative to start time
      # TODO this does not work
      if self.parent.xAxisType=="Time":
         self.x=self.parent.computeRelativeX()
      
      self.ax1=self.fig.add_subplot(211)   # create solutions subplot
      # Set title and labels
      self.ax1.set_xlabel(self.parent.xLabel)
      self.ax1.set_ylabel(parm + ":" + self.parent.parmValueComboBox.currentText())

      np.set_printoptions(precision=1)
#      self.ax1.get_xaxis().set_visible(False)  # TODO: How to get correct x-axis ticks?
      np.set_printoptions(precision=2)    # does this work?

      if self.parent.perIteration==True:
         x=range(1, len(self.y1)+1)       # we want the first iteration to be called "1"
         if self.parent.scatterCheckBox.isChecked()==True:
            self.ax1.scatter(x, self.y1)
         else:
            self.ax1.plot(x, self.y1)
      else:
         if self.parent.scatterCheckBox.isChecked()==True:
            self.ax1.scatter(self.x, self.y1)
         else:
            if len(self.y1)==1 or isinstance(self.y1, float):
               self.ax1.scatter(self.x, self.y1)
            else:
               self.ax1.plot(self.x, self.y1)

      # Solver log plot
      self.ax2=self.fig.add_subplot(212, sharex=self.ax1)     # sharex for common zoom
      # Set labels
      #self.ax2.set_xticklabels(self.ax1.get_xticklabels(), visible=True)
      self.ax2.set_ylabel(self.parent.parametersComboBox.currentText())
      if self.parent.perIteration==True:
         x=range(1, len(self.y2)+1)
         if self.parent.scatterCheckBox.isChecked()==True:
            self.ax2.scatter(x, self.y2)
         else:
            self.ax2.plot(x, self.y2)   # have to increase lower y limit (for unknown reason)
      else:
         if self.parent.scatterCheckBox.isChecked()==True:
            self.ax2.scatter(self.x, self.y2)
         else:
            if len(self.y1)==1 or isinstance(self.y2, float):
               self.ax2.scatter(self.x, self.y2)
            else:
               self.ax2.plot(self.x, self.y2)

      self.fig.canvas.draw()
