#!/usr/bin/env python
#
# Solver statistics dialog
#
# File:           plotcorrmatrix.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2011-12-06
# Last change;    2011-12-18
#
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

#import pyrap.quanta as pq   # used to convert date from double to normal format
import math
import numpy as np
import matplotlib.cm as cm
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure


class plotCorrmatrix(QDialog):
   def __init__(self, parent, corrmatrix):
     QFrame.__init__(self)
    
     self.setWindowTitle("Correlation Matrix")
     
     self.corrmatrix=[[]]
     self.rank=None
     self.rim=0.05
  
     self.parent=parent            # parent object/class
     self.fig=None
  
     # Create canvas for plotting
     self.fig = Figure((7, 7), dpi=100)
     self.canvas = FigureCanvas(self.fig)
     self.canvas.setParent(self)
     self.fig.subplots_adjust(left=self.rim, right=1.0-self.rim, top=1.0-self.rim, bottom=self.rim)  # set a small rim
     self.ax=self.fig.add_subplot(111)

     self.mpl_toolbar = NavigationToolbar(self.canvas, self)
     self.mpl_toolbar.show()
  
     self.setMinimumWidth(700)
     self.setMinimumHeight(700)
     self.corrmatrix=corrmatrix

     self.xLabel="Parameter index"
     self.yLabel="Parameter index"

     self.createWidgets()
     self.createLayouts()
     self.connectSignals()
     self.plot(self.corrmatrix)


   def createWidgets(self):
     # Get time indices (and frequency range) from solverdialog class according to plotwindow index
     self.index = np.searchsorted(self.parent.x, [self.parent.xdata])[0]
     
     print "self.index = ", self.index    # DEBUG
     
     self.start_time=self.parent.parent.solverQuery.timeSlots[self.index]['STARTTIME']
     self.end_time=self.parent.parent.solverQuery.timeSlots[self.index]['ENDTIME']      
     self.start_freq=self.parent.parent.solverQuery.frequencies[self.parent.parent.frequencyStartSlider.value()]['STARTFREQ']
     self.end_freq=self.parent.parent.solverQuery.frequencies[self.parent.parent.frequencyEndSlider.value()]['ENDFREQ']

      
     self.timeLabel=QLabel("Time cell")
     self.startTimeLabel=QLabel("S: " + str(self.parent.parent.convertDate(self.start_time)))
     self.endTimeLabel=QLabel("E: " + str(self.parent.parent.convertDate(self.end_time)))
     self.startTimeLabel.show()
     self.endTimeLabel.show()
     self.freqLabel=QLabel("Freq cell")
     self.startFreqLabel=QLabel("S: " + str(self.start_freq) + " Hz")
     self.endFreqLabel=QLabel("E: " + str(self.end_freq) + " Hz")
     self.startFreqLabel.show()
     self.endFreqLabel.show()

     self.closeButton=QPushButton()
     self.closeButton=QPushButton()
     self.closeButton.setText('Close')
     self.closeButton.setToolTip('close this plotcorrmatrix window')
     self.closeButton.setMaximumWidth(120)
     self.closeButton.show()

     self.prevButton=QPushButton("Prev")
     self.nextButton=QPushButton("Next")
      

   def createLayouts(self):
     self.buttonLayout=QVBoxLayout()                   # layout containing widgets
     self.matrixLayout=QVBoxLayout()                   # layout containing canvas and toolbar
     self.mainLayout=QHBoxLayout()
     self.buttonLayout.addWidget(self.timeLabel)
     self.buttonLayout.addWidget(self.startTimeLabel)
     self.buttonLayout.addWidget(self.endTimeLabel)
     self.buttonLayout.addWidget(self.freqLabel)
     self.buttonLayout.addWidget(self.startFreqLabel)
     self.buttonLayout.addWidget(self.endFreqLabel)
     
     self.prevNextLayout=QHBoxLayout()
     if self.parent.parent.xAxisType!="Iteration":         # we don't support previous and next in iteration mode
       self.prevNextLayout.addWidget(self.prevButton)
       self.prevNextLayout.addWidget(self.nextButton)
       self.buttonLayout.addLayout(self.prevNextLayout)
     self.buttonLayout.insertStretch(-1)
     self.buttonLayout.addWidget(self.closeButton)

     
     self.matrixLayout.addWidget(self.canvas)
     self.matrixLayout.addWidget(self.mpl_toolbar)
     self.mainLayout.addLayout(self.buttonLayout)
     self.mainLayout.addLayout(self.matrixLayout)

     self.setLayout(self.mainLayout)


   def connectSignals(self):
      self.connect(self.closeButton, SIGNAL('clicked()'), SLOT('close()'))
      self.connect(self.prevButton, SIGNAL('clicked()'), self.on_prevButton)
      self.connect(self.nextButton, SIGNAL('clicked()'), self.on_nextButton)
      self.connect(self.prevButton, SIGNAL('clicked()'), self.retrieveCorrMatrix)
      self.connect(self.nextButton, SIGNAL('clicked()'), self.retrieveCorrMatrix)
      
  
   #*********************************************
   #
   # Handler functions for events
   #
   #*********************************************

   def on_prevButton(self):
     self.index = self.index - 1
     if self.index == 0:
       self.prevButton.setDisabled(True)
  
   def on_nextButton(self):
     self.index = self.index + 1
     if self.index > 0 and self.index < len(self.parent.x)-1:
       self.prevButton.setEnabled(True)
     if self.index == len(self.parent.x)-1:
       self.nextButton.setDisabled(True)
     
   def retrieveCorrMatrix(self):
     if self.parent.parent.xAxisType=="Time":
       self.start_time=self.parent.parent.solverQuery.timeSlots[self.index]['STARTTIME']
       self.end_time=self.parent.parent.solverQuery.timeSlots[self.index]['ENDTIME']      
       self.start_freq=self.parent.parent.solverQuery.frequencies[self.parent.parent.frequencyStartSlider.value()]['STARTFREQ']
       self.end_freq=self.parent.parent.solverQuery.frequencies[self.parent.parent.frequencyEndSlider.value()]['ENDFREQ']
     elif self.parent.parent.xAxisType=="Freq":
       self.start_freq=self.parent.parent.solverQuery.frequencySlots[self.index]['STARTFREQ']
       self.end_freq=self.parent.parent.solverQuery.frequencylots[self.index]['ENDFREQ']      
       self.start_time=self.parent.parent.solverQuery.frequencies[self.parent.parent.timeStartSlider.value()]['STARTTIME']
       self.end_time=self.parent.solverQuery.frequencies[self.parent.parent.timeEndSlider.value()]['ENDTIME']
     else:     # Iteration
       # Do nothing? Because there is only one Corrmatrix per solution but not per iteration!?
       print "plotcorrmatrix::retrieveCorrMatrix() can't step forward or backward in per iteration mode"
       return
       self.start_time=self.parent.parent.solverQuery.timeSlots[self.parent.parent.timeStartSlider.value()]['STARTTIME']
       self.end_time=self.parent.parent.solverQuery.timeSlots[self.parent.parent.timeEndSlider.value()]['ENDTIME']      
       self.start_freq=self.parent.parent.solverQuery.frequencies[self.parent.parent.frequencyStartSlider.value()]['STARTFREQ']
       self.end_freq=self.parent.parent.solverQuery.frequencies[self.parent.parent.frequencyEndSlider.value()]['ENDFREQ']

     print "plotcorrmatrix::retrieveCorrMatrix()"                           # DEBUG
     print "plotwindow::plotcorrmatri() start_time = ", self.start_time     # DEBUG
     print "plotwindow::plotcorrmatri() end_time = ", self.end_time         # DEBUG
     print "plotwindow::plotcorrmatri() start_freq = ", self.start_freq     # DEBUG
     print "plotwindow::plotcorrmatri() end_freq = ", self.end_freq         # DEBUG

     self.corrmatrix=self.parent.parent.solverQuery.getCorrMatrix(self.start_time, self.end_time, self.start_freq, self.end_freq)
     self.updateLabels()
     self.plot(self.corrmatrix)

   def updateLabels(self):
     self.startTimeLabel.setText("S: " + str(self.parent.parent.convertDate(self.start_time)))
     self.endTimeLabel.setText("E: " + str(self.parent.parent.convertDate(self.end_time)))
     self.startFreqLabel.setText("S: " + str(self.start_freq) + " Hz")
     self.endFreqLabel.setText("E: " + str(self.end_freq) + " Hz")
     

   #*********************************************
   #
   # Plot a correlation matrix in a plotWindow
   #
   #*********************************************     
   #
   # corrmatrix      - numpy.ndarray holding (linearized) correlation Matrix
   #
   def plot(self, corrmatrix):
     print "plotCorrmatrix::plotCorrMatrix()"   # DEBUG
  
     # We plot into the existing canvas object of the PlotWindow class
     rank=self.parent.parent.solverQuery.getRank()
     
     if rank != math.sqrt(len(corrmatrix)):
       raise ValueError
    
     shape=corrmatrix.shape      # get shape of array (might be 1-D)
     corrmatrix=np.reshape(corrmatrix, (rank, rank))
     if len(shape)==1:                  # if we got only one dimension...
       if shape[0] != rank*rank:        # if the length of the array is not rank^2
         raise ValueError
       else:
         corrmatrix=np.reshape(corrmatrix, (rank, rank))
     elif len(shape)==2:              # we already have a two-dimensional array
       print "shape[0] = ", shape[0], "   shape[1] = ", shape[1]       # DEBUG
       if shape[0] != rank or shape[1] != rank:
         raise ValueError

     # Clear all axes, and clear figure (needed to remove colorbar)
     self.ax.cla()
     self.fig.clf()
     self.ax=self.fig.add_subplot(111)      # create axes again in figure
     # plot CorrMatrix as a figure image
     self.img=self.ax.imshow(corrmatrix, cmap=cm.jet , aspect='equal', interpolation=None)
     self.colorbar = self.fig.colorbar(self.img)
     self.canvas.draw()
