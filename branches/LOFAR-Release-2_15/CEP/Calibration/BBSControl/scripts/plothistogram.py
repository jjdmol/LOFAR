#!/usr/bin/env python
#
# Solver statistics histogram dialog
#
# File:           plothistogram.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2011-12-06
# Last change;    2011-12-11
#
# This class displays a preference dialog for the solver statistics plotter
# to adjust line styles, 2D/3D settings, use of markers and data pickers etc.
# TODO

import sys
#from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import numpy as np             # needed for histogram
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure
#import matplotlib.pyplot as plt


class plothistogram(QFrame):
   def __init__(self, parent):
      print "__init__()"       # DEBUG
      QFrame.__init__(self)
      
      self.parent=parent
      #self.setModal(False)

      self.data1=self.parent.y1   # get plotted y values from solutions plot
      self.data2=self.parent.y2   # get plotted y values from parameter plot
      self.nbins=50               # default=50 bins
      self.parameter=self.parent.parent.parametersComboBox.currentText()
      self.data=self.parent.y2    # set histogram to solver parameter data

      #print "self.data = ", self.data   # DEBUG

      # Create canvas for plotting
      self.rim=0.1
      self.fig = Figure((5, 4), dpi=100)
      self.canvas = FigureCanvas(self.fig)
      self.canvas.setParent(self)
      self.fig.subplots_adjust(left=self.rim, right=1.0-self.rim, top=1.0-self.rim, bottom=self.rim)  # set a small rim

      self.mpl_toolbar = NavigationToolbar(self.canvas, self)
      self.mpl_toolbar.show()   # first hide the toolbar
      
      self.ax=self.fig.add_subplot(111)
      
      self.createWidgets()
      self.createLayout()
      self.connectSignals()
      self.setLabels()
      self.plot()
      
   def createWidgets(self):
      #print "createWidgets()"     # DEBUG
      
      self.closeButton=QPushButton("Close")
      self.closeButton.setToolTip("close this plotwindow")
      self.closeButton.show()

      self.histogramBinSpin=QSpinBox()            # spinbox for histogram binsize
      self.histogramBinSpin.setToolTip("Number of bins to histogram into")
      self.histogramBinSpin.setMinimum(5)
      #self.histogramBinSpin.setMaximum(150)        # set a maximum of 150 (reasonable?)
      self.histogramBinSpin.setSingleStep(10)       # allow only stepping by 10
      self.histogramBinSpin.setMaximumWidth(120)
      self.histogramBinSpin.setMinimumHeight(25)
      self.histogramBinSpin.setValue(self.nbins)
      self.histogramBinSpin.show()
      self.histogramBinLabel=QLabel("Bins")
      
      self.normedCheckBox=QCheckBox()
      self.normedCheckLabel=QLabel("Normalize")
      self.normedCheckBox.setToolTip("Normalize histogram")
      self.normedCheckBox.show()
      #self.dpi = 100
      
      self.dataComboBox=QComboBox()
      self.dataComboBox.addItem(self.parent.parent.parmValueComboBox.currentText())
      self.dataComboBox.addItem(self.parent.parent.parametersComboBox.currentText())
      self.dataComboBox.setMaximumWidth(120)
      self.dataComboBox.setMinimumHeight(25)
      self.dataComboBox.setCurrentIndex(1)

   def createLayout(self):
      #print "createLayout()"      # DEBUG
      
      self.normedLayout=QHBoxLayout()
      self.normedLayout.addWidget(self.normedCheckBox)
      self.normedLayout.addWidget(self.normedCheckLabel)
      self.buttonLayout=QVBoxLayout()
      self.plotLayout=QVBoxLayout()
      self.mainLayout=QHBoxLayout()
      self.buttonLayout.addLayout(self.normedLayout)
      #self.buttonLayout.addWidget(self.normedCheckBox)
      #self.buttonLayout.addWidget(self.normedCheckLabel)
      self.buttonLayout.addWidget(self.histogramBinSpin)
      self.buttonLayout.addWidget(self.dataComboBox)
      self.buttonLayout.insertStretch(-1)
      self.buttonLayout.addWidget(self.closeButton)
      self.plotLayout.addWidget(self.canvas)
      self.plotLayout.addWidget(self.mpl_toolbar)
      self.mainLayout.addLayout(self.buttonLayout)
      self.mainLayout.addLayout(self.plotLayout)
      
      self.setLayout(self.mainLayout)

   def setLabels(self):
      self.ax.xlabel="Bin"
      self.ax.ylabel="N"
      
   def setTitle(self):
      #=self.parent.parametersComboBox.currentText()
      #self.ax.xlabel=self.parmValueComboBox.currentText()
      #self.ax.xlabel="Bin No."
      #self.ax.ylabel="n"
      self.ax.title=self.parent.parent.parametersComboBox.currentText()
      
   def connectSignals(self):
      self.connect(self.closeButton, SIGNAL('clicked()'), SLOT('close()'))
      self.connect(self.histogramBinSpin, SIGNAL('valueChanged(int)'), self.on_changeBinSpin)      
      self.connect(self.normedCheckBox, SIGNAL('stateChanged(int)'), self.on_normedCheckBox)
      self.connect(self.dataComboBox, SIGNAL('currentIndexChanged(int)'), self.on_data)
      
   def on_changeBinSpin(self):
      self.nbins=self.histogramBinSpin.value()      
      # create histogram
      #n, bins = np.histogram(self.data, self.histogramBinSpin.value(), normed=self.normedCheckBox.isChecked())
      #hist, bins=np.histogram(self.data, self.nbins, normed=self.normedCheckBox.isChecked())
      
      #print "len(bins) = ", len(bins), " len(n) = ", len(n)
      
      #self.ax.plot(bins[1:], n, color='red')
      #self.canvas.draw()
      #self.ax.legend()
      #self.fig.draw_idle()        # redraw (preferably when idle)
      self.plot()

   def on_normedCheckBox(self):
      self.plot()

   def on_data(self):
      print "on_data()"       # DEBUG
      if self.dataComboBox.currentText()==self.parent.parent.parametersComboBox.currentText():
          self.data=self.data2
      else:
          self.data=self.data1
      self.plot()

   def plot(self):
      self.fig.delaxes(self.ax)            # delete all axes first
      self.ax=self.fig.add_subplot(111)
      
      n, bins = np.histogram(self.data, self.histogramBinSpin.value(), normed=self.normedCheckBox.isChecked())

      self.xmin=bins[1]
      self.xmax=bins[len(bins)-1]
      self.ax.bar(bins[1:], n, color='blue', width=(self.xmax-self.xmin)/len(bins))
      self.canvas.draw()

      
      
def main():
  print "main()"
  
  app = QApplication(sys.argv)
  form=plothistogram(app)
  
  form.show()
  app.exec_()
      
      
if __name__=="__main__":
    main()