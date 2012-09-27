#!/usr/bin/env python

# Solver statistics dialog
#
# File:           solverdialog.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2010-08-05
# Last change;    2012-09-27  
#
#

# Import
import sys, os, random
#import lofar.bbs.solverquery as sq
import lofar.bbs.plotwindow
import solverquery as sq  # DEBUG!!!
#import plotwindow         # DEBUG!!!
import lofar.parmdb as parmdb


from PyQt4.QtCore import *
from PyQt4.QtGui import *

import pyrap.quanta as pq   # used to convert date from double to normal format
import numpy as np
import unicodedata
import time        # for timing functions
import datetime    # needed for timestamps
import math
import matplotlib
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure
import matplotlib.cm as cm     # color maps?


# Class to hold all the GUI elements of a Solver application form
# on_draw() creates instances of plotWindow class for each plot
#
class SolverAppForm(QMainWindow):

    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)
        self.setWindowTitle('Solver statistics')

        self.useScipy=False

        self.solverQuery = sq.SolverQuery()       # attribute to hold SolverQuery object
        self.table=False                          # attribute to check if we have an open table
        self.tableName=""                         # name of opened table
        self.tableType=""                         # table type
        self.SolutionSubplot=None                 # attribute to define if solutions are shown or not
        self.ParameterSubplot=None

        self.solutions_plot=False                 # Plot solutions, too
        self.scatter=False                        # Plot scatter plots?
        self.clf=True                             # clear figure before replot
        self.newfigure=False                      # create a new figure on replot?
        self.physicalValues=False                 # physical interpretation of parameters

        self.xAxisType="Time"                     # x-axis type: Time, Frequency or Iteration

        self.x=None                               # array holding x-values
        self.relativeX=[]                         # array holding x-values relative to start
        self.y1=None                              # array holding top subplot y-values
        self.y2=None                              # array holding bottom subplot y-values
        self.messages = None                      # array to hold solver messages

        # Labels                                  # axis labels
        self.xLabel="Time"                        # x-axis label for all subplots
        self.y1Label=""                           # y-axis label for solutions subplot
        self.y2Label=""                           # y-axis label for solver parameter subplot

        self.parmMap={}                           # dictionary mapping parmDB names to solution indices

        self.currentPlot=None                     # originally only hold one figure
        self.plots=[]                             # list to hold all the dialog's figures
 
        self.perIteration=False                   # per default set per iteration to False
        self.parmDB=None                          # parmDB database of parameters contained the MS
        self.parms=[]                             # Parameters that were solved for in this solver run
        self.currentSolution=None                 # current solution to plot solver parameters for, consisting of a tuple of
                                                  # start_time, end_time, start_freq and end_freq

        self.importModule('scipy.io')             # try to import scipy.io module

        self.create_main_frame()
        self.create_status_bar()

        self.__styles = ["%s%s" % (x, y) for y in ["-", ":"] for x in ["b", "g", "r", "c",
    "m", "y", "k"]]


    # Save the plot to a graphics file (PNG, PDF, EPS, etc.)
    #
    def save_plot(self):
        file_choices = "PNG (*.png)|*.png"

        path = unicode(QFileDialog.getSaveFileName(self, 'Save file', '', file_choices))

        if path:
            self.canvas.print_figure(path, dpi=self.dpi)
            self.statusBar().showMessage('Saved to %s' % path, 2000)


    # Load the Measurementset solver table
    #
    def load_table(self):
        #if self.table == True:
        #    print "load_table() we have already a table"   # DEBUG
        #   self.close_table()

        # Customization: check if ~/Cluster/SolutionTests exists
        if os.path.exists('/data/scratch'):
            setDir=QString('/data/scratch')
        else:
            setDir=QString('')

        path = unicode(QFileDialog.getExistingDirectory(self, 'Load table', setDir, QFileDialog.ShowDirsOnly))
        path=str(path)  # Convert to string so that it can be used by load table

        if path:
            self.statusBar().showMessage("Loading solver statistics table")
            self.solverQuery=self.solverQuery.open(path)
            self.statusBar().clearMessage()

            # Fill in information from the table into the Dialog object for faster access?
            # strictly not necessary, since we can access them in the solverQuery object

            self.open_table(path)
        else:
            print "load_table: invalid path"


    # Open a table (on startup, if a command line argument is given)
    #
    def open_table(self, tableName):
        # If a table is already loaded, close it:
        if self.table == True:
            print "open_table() we already have a table!"   # DEBUG
            self.close_table()    # close currently opened table

        # Check if path exists and it is a director (i.e. a table)
        if os.path.exists(tableName) == False:
            raise IOError
        else:
            if os.path.isdir(tableName) == False:
                raise IOError

        # Open it and set object attributes
        self.solverQuery=self.solverQuery.open(tableName)
        self.determineTableType()

        self.create_table_widgets()
        self.table=True
        self.tableName=tableName

        # Enable checkboxes
        if self.tableType == "PERITERATION" or self.tableType == "PERITERATION_CORRMATRIX":
            self.showIterationsCheckBox.setEnabled(True)
            self.perIteration=False   # initially set that to False
        else:
            self.showIterationsCheckBox.setEnabled(False)
            self.perIteration=False

        # By default we do not want to show plots per iteration
        self.showIterationsCheckBox.setCheckState(Qt.Unchecked)

        self.singleCellCheckBox.setEnabled(True)
        #self.solutionsButton.setEnabled(True)    # dynamic solutionsplot
        self.plotButton.setEnabled(True)
        #self.histogramButton.setEnabled(True)

        # Update layouts to get correct GUI
        self.buttonsLayout.update()
        #self.mainLayout.update()      # we don't have a mainLayout anymore


    # Close the table that is currently loaded
    #
    def close_table(self):
        print "close_table() removing now widgets"  # DEBUG

        # Remove table specific widgets
        self.xAxisComboBox.deleteLater()
        self.timeStartSlider.deleteLater()
        self.timeEndSlider.deleteLater()
        self.frequencyStartSlider.deleteLater()
        self.frequencyStartSliderLabel.deleteLater()
        self.frequencyEndSlider.deleteLater()
        self.timeEndSliderLabel.deleteLater()
        self.frequencyStartSlider.deleteLater()
        self.frequencyEndSlider.deleteLater()
        self.frequencyStartSliderLabel.deleteLater()
        self.frequencyEndSliderLabel.deleteLater()
        # NOW in self.close_parmDB()
        self.parametersComboBox.deleteLater()
        self.parmValueComboBox.deleteLater()

        # Delete the Widgets
        del self.xAxisComboBox
        del self.timeStartSlider
        del self.timeEndSlider
        del self.timeStartSliderLabel
        del self.frequencyStartSlider
        del self.frequencyEndSlider
        del self.frequencyStartSliderLabel
        del self.frequencyEndSliderLabel
        # NOW in self.close_parmDB()       
        #del self.parametersComboBox
        #del self.parmValueComboBox

        #print "self.parametersComboBox = ", self.parametersComboBox
        
        self.buttonsLayout.update()
        #self.mainLayout.update()     # There is no mainLayout anymore
        self.close_parmDB()    # we also must close the parmDB (and remove its widgets)
        self.solverQuery.close()
        self.table=False       # we don't have an open table anymore

        


    # Open parmDB
    #
    def open_parmDB(self, parmDBname):
        self.parmDB=parmdb.parmdb(parmDBname)


    # Close parmDB
    # TODO: What is the proper way to close the parmDB?
    #
    def close_parmDB(self):
        print "close_parmDB()"   # DEBUG
        self.parametersComboBox.deleteLater()
        self.parmValueComboBox.deleteLater()
        del self.parametersComboBox
        del self.parmValueComboBox

        if self.parmDB!=None:    # Only try to close it, if it is currently open
           self.parmDB.close()   # TODO: What is the right method to close parmDB?
           self.parmDB=None


    # Convert date from a double MJD time (s) to a date format string
    # YYYY/MM/DD/HH:MM:SS
    #
    def convertDate(self, date=None):
      #print "convertDate()"                     # DEBUG

      dateString=""
      if date==None:
        raise ValueError
      elif isinstance(date, np.ndarray):
        print "array"
        for i in range(0, len(date)):
          q=pq.quantity(date[i], 's')
          print q
          dateString[i].append(q.formatted('YMD'))
      elif isinstance(date, float):
        dateString=""
        q=pq.quantity(date, 's')
        dateString=q.formatted('YMD')
      
      return dateString
      

    # Function to handle table index slider has changed
    #
    def on_timeStartSlider(self, index):
        # Read time at index
        starttime=self.solverQuery.timeSlots[index]['STARTTIME']
        
        if self.showDatesCheckBox.isChecked():
          self.timeStartSliderLabel.setText("S:" +  str(self.convertDate(starttime)))
        else:
          self.timeStartSliderLabel.setText("S:" +  str(starttime) + " s")

        # Handle behaviour of timeEndSlider in combination with timeStartSlider
        # If timeEndSlider is smaller than timeStartSlider adjust the latter
        if self.timeEndSlider.sliderPosition() < self.timeStartSlider.sliderPosition():
            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())

        self.repaint()


    # Function to handle table index slider has changed
    #
    def on_timeEndSlider(self, index):
        # Read time at index
        endtime=self.solverQuery.timeSlots[index]['ENDTIME']

        if self.showDatesCheckBox.isChecked():
          self.timeEndSliderLabel.setText("E:" +  str(self.convertDate(endtime)))
        else:
          self.timeEndSliderLabel.setText("E:" +  str(endtime) + " s")

        # Handle behaviour of timeEndSlider in combination with timeStartSlider
        # If timeEndSlider is smaller than timeStartSlider adjust the latter
        if self.timeEndSlider.sliderPosition() < self.timeStartSlider.sliderPosition():
            self.timeStartSlider.setValue(self.timeEndSlider.sliderPosition())

        self.repaint()


    # If only a single cell is to be shown, adjust timeEndSlider
    # to the position of timeStartSlider, and lock them
    def on_singleCell(self):
        if self.singleCellCheckBox.isChecked():
            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())
            self.syncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))

            if self.tableType=="PERITERATION" or self.tableType=="PERITERATION_CORRMATRIX":
               self.showIterationsCheckBox.setEnabled(True)
        else:
            self.unsyncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))
            self.showIterationsCheckBox.setCheckState(Qt.Unchecked)     # deactivate show iterations if we show interval
            self.showIterationsCheckBox.setEnabled(False)


    # Put sliders into sync, so that moving one also moves the other
    #
    def syncSliders(self):
        # Time sliders
        self.connect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.timeEndSlider, SLOT('setValue(int)'))
        self.connect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.timeStartSlider, SLOT('setValue(int)'))
        # Frequency sliders
        self.connect(self.frequencyStartSlider, SIGNAL('valueChanged(int)'), self.frequencyEndSlider, SLOT('setValue(int)'))
        self.connect(self.frequencyEndSlider, SIGNAL('valueChanged(int)'), self.frequencyStartSlider, SLOT('setValue(int)'))     


    # Unsync sliders, so that they can move independently again
    #
    def unsyncSliders(self):
       # Time sliders
       self.disconnect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.timeEndSlider, SLOT('setValue(int)'))
       self.disconnect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.timeStartSlider, SLOT('setValue(int)'))
       # Frequeny sliders
       self.disconnect(self.frequencyStartSlider, SIGNAL('valueChanged(int)'), self.frequencyEndSlider, SLOT('setValue(int)'))
       self.disconnect(self.frequencyEndSlider, SIGNAL('valueChanged(int)'), self.frequencyStartSlider, SLOT('setValue(int)'))


    # Function to handle frequency index slider has changed
    #
    def on_frequencyStartSlider(self, index):
        print "on_frequencyStartSlider(self, index):"
        # Read frequency at index
        self.solverQuery.getTimeSlots()
        startfreq=self.solverQuery.timeSlots[index]['STARTTIME']
        endfreq=self.solverQuery.timeSlots[index]['ENDTIME']
        self.frequencyStartSliderLabel.setText("Freq: \n" + str(startfreq))


    # Function to handle frequency index slider has changed
    #
    def on_frequencyEndSlider(self, index):
        print "on_frequencyEndSlider(self, index):"
        # Read frequency at index
        self.solverQuery.getTimeSlots()
        startfreq=self.solverQuery.timeSlots[index]['STARTTIME']
        endfreq=self.solverQuery.timeSlots[index]['ENDTIME']
        self.frequencyEndSliderLabel.setText("Freq: \n" + str(endfreq))


    # Handler for showSolutionsPlot button
    # changes the GUI interface accordingly and calls a redraw of the
    # figure through the on_plot() function
    #
    def on_solutions(self):
        #self.fig.clf()
        #self.delAllAxes()         # clear all the axes
        #self.canvas.draw()

        # If solutions are currently shown in the plot
        if self.solutions_plot==True:
            # Change GUI elements to include parms selection etc.
            self.addSolutionsplotButton.setText("Add Solutions")
            self.parmsComboBox.hide()
            self.parmValueComboBox.hide()

            #self.ParameterSubplot=self.fig.add_subplot(211)
            self.solutions_plot=False   # Indicate in attribute that we now do not show solutions anymore

            #self.ParameterSubplot=self.fig.add_subplot(111)  # dynamic display of solutionplot

            # TODO
            #titleString="Solution: " +  str(self.parmsComboBox.currentText()) + "& solver parameter " +  + str(self.parametersComboBox.currentText())

        else:
            # Remove solutions parms GUI elements, i.e. do not show them anymore
            self.addSolutionsplotButton.setText("Remove Solutions")
            self.parmsComboBox.show()
            self.parmValueComboBox.show()

            self.SolutionsSubplot=self.fig.add_subplot(211)
            self.ParameterSubplot=self.fig.add_subplot(212)

            self.solutions_plot=True   # indicate in attribute that we now do show solutions

        # Call self.on_plot() function to replot current parameter (+solution if toggled)
        self.on_plot()


    # If the selected solution changed, then also change
    # items in self.parmValueComboBox
    #
    def on_solution_changed(self):

        self.parmValueComboBox.clear()
        # add items again
        parameter=str(self.parmsComboBox.currentText())  # need to convert QString to Python string
        if parameter.find("Gain") is not -1:
            self.parmValueComboBox.addItem("Amplitude")
            self.parmValueComboBox.addItem("Phase")


    # Handler to export the currently plotted data to disk
    #
    # fileformat      - format to export to default=ASCII
    #
    def on_export(self, fileformat='ASCII'):
        print "on_export()"        # DEBUG
        
        self.parmsComboBox.currentText()
        parm=self.parmsComboBox.currentText()             # Parameter from Solution, e.g. Gain:1:1:LBA001
        parmvalue=self.parmValueComboBox.currentText()    # physical parameter value, e.g. Amplitude
        parameter=self.parametersComboBox.currentText()   # solver parameter, e.g. Chisqr
        #fileformat=self.exportComboBox.currentText()

        # Generate filenames with time stamps, so that we don't overwrite previous data
        #
        filename_physparm = parm + "_" + parmvalue + "_" + str(datetime.datetime.now())
        filename_parameter = parm + "_" + parameter + "_" + str(datetime.datetime.now())
        # We have to replace the ":" by "-" and " " by "_"
        filename_physparm=filename_physparm.replace(":", "-")
        filename_parameter=filename_parameter.replace(":", "-")
        filename_physparm=filename_physparm.replace(" ", "_")
        filename_parameter=filename_parameter.replace(" ", "_")
        # Depending on fileformat, append ".dat" or ".m" (maybe more formats in the future)
        if fileformat == "ASCII":
            filename_physparm = filename_physparm + ".dat"
            filename_parameter = filename_parameter + ".dat"
        elif fileformat == "Matlab":
            filename_physparm = filename_physparm + ".mat"
            filename_parameter = filename_parameter + ".mat"


        #print "on_export() filename_physparm", filename_physparm       # DEBUG
        #print "on_export() filename_parameter", filename_parameter     # DEBUG

        # use exportData() function
        print "on_export() fileformat = ", fileformat   # DEBUG
        if fileformat=="ASCII":
            self.exportDataASCII(filename_physparm, parmvalue)     # export the physical parameter
            self.exportDataASCII(filename_parameter, parameter)    # export the solver parameter
        elif fileformat=="Matlab":
            self.exportDataMatlab(filename_physparm, parmvalue)     # export the physical parameter
            self.exportDataMatlab(filename_parameter, parameter)    # export the solver parameter




    #**************************************************
    #
    # Mainwindow creation, create Widgets etc.
    #
    #**************************************************

    # Create the main frame of the dialog
    #
    def create_main_frame(self):
        self.main_frame = QWidget()
        self.dpi = 100

        # Minimum size in pixels to fit all Widgets (in this instance QT autosize by layouts works)
        #self.setMinimumWidth(200)
        #self.setMinimumHeight(600)

        #**********************************************************
        #
        # Widgets
        #
        #**********************************************************
        self.createWidgets()    # create all the widgets

    def createWidgets(self):
        print "createWidgets()"   # DEBUG

        # Create buttons to access solverStatistics
        self.loadButton=QPushButton("&Load solver table")      # Load MS/solver button
        self.loadButton.setToolTip("Load a solver statistics table")
        self.saveButton=QPushButton("&Save plot")              # Save plot button
        self.saveButton.setToolTip("Save the plot to an image")
        self.plotButton=QPushButton("&Plot")                   # plot button
        self.plotButton.setToolTip("Redraw the plot")
        #self.addSolutionsplotButton=QPushButton("&Solutions plot")      # Add a plot of the solutions to the window
        #self.addSolutionsplotButton.setToolTip("Show solutions plot")
        #self.solutionsButton=QPushButton("Show Solutions")     # Show a plot of the corresponding solutions
        #self.solutionsButton.setToolTip("Show solutions plot along-side")
        self.quitButton=QPushButton("&Quit")                   # Quit the application
        self.quitButton.setToolTip("Quit application")

        self.plottingOptions=QLabel('Plotting options')
        self.showIterationsCheckBox=QCheckBox()                # Checkbox to show individual iterations parameters
        self.showIterationsCheckBox.setCheckState(Qt.Unchecked)       # Default: False
        self.showIterationsCheckBox.setText('Show iterations')
        self.showIterationsCheckBox.setToolTip('Show all iterations for this solution')
        self.showIterationsCheckBox.setCheckState(Qt.Unchecked)
        self.showDatesCheckBox=QCheckBox()
        self.showDatesCheckBox.setCheckState(Qt.Checked)      # by default now convert to ISO date
        self.showDatesCheckBox.setText("Convert to ISO date")
        self.showDatesCheckBox.setToolTip("Convert casa times to ISO format YY/MM/DD:HH:MM:SS")

        self.singleCellCheckBox=QCheckBox()                    # Checkbox to show individual iterations parameters
        self.singleCellCheckBox.setCheckState(Qt.Unchecked)           # Default: False
        self.singleCellCheckBox.setText('Show single cell')
        self.singleCellCheckBox.setToolTip('Show only a single cell solution')
        self.singleCellCheckBox.setCheckState(Qt.Unchecked)             # we seem to need these to have "normal" CheckBoxes

        self.scatterCheckBox=QCheckBox()                       # Checkbox to set plot to scatter plot
        self.scatterCheckBox.setCheckState(Qt.Unchecked)
        self.scatterCheckBox.setText('Scatter plot')
        self.scatterCheckBox.setCheckState(Qt.Unchecked)

        self.colorizeCheckBox=QCheckBox()                      # Checkbox to create alternating colours
        self.colorizeCheckBox.setCheckState(Qt.Unchecked)
        self.colorizeCheckBox.setText('Colourize')
        self.colorizeCheckBox.setToolTip('Colourize plot points')

        self.physicalValuesCheckBox=QCheckBox()
        self.physicalValuesCheckBox.setCheckState(Qt.Checked)
        self.physicalValuesCheckBox.setText('Physical values')
        self.physicalValuesCheckBox.setToolTip('Show physical values')

        # "Clear figure" and "New figugre"
        self.clfCheckBox=QCheckBox()
        self.clfCheckBox.setCheckState(Qt.Checked)
        self.clfCheckBox.setText('Clear figure')
        self.clfCheckBox.setToolTip('Clear the current figure on replot')
        self.clfCheckBox.setCheckState(Qt.Unchecked)

        self.newCheckBox=QCheckBox()
        self.newCheckBox.setCheckState(Qt.Unchecked)
        self.newCheckBox.setText('New figure')
        self.newCheckBox.setToolTip('Plot in new figure window')
        self.newCheckBox.setCheckState(Qt.Unchecked)

        self.createxAxisCombo()   # create xAxis Combobox

        # Check if a table has been loaded
        if self.table is False:
            self.showIterationsCheckBox.setEnabled(False)
            self.singleCellCheckBox.setEnabled(False)
            #self.solutionsButton.setEnabled(False)              # dynamic display of solutions
            self.plotButton.setEnabled(False)
            #self.histogramButton.setEnabled(False)                   # by default disable it

        #**********************************************************
        #
        # Layouts
        #
        #**********************************************************

        #self.mainLayout=QHBoxLayout()
        self.buttonsLayout=QVBoxLayout()
        #plotLayout=QVBoxLayout()     # plots are now done externally in figures
        timeStartLayout=QHBoxLayout()    # small internal layout for 
        timeEndLayout=QHBoxLayout()

        # Add the button widgets to the buttonsLayout  (self.colorizeCheckBox) left out for the 
        for widget in [self.loadButton, self.saveButton, self.plotButton, self.quitButton, self.plottingOptions, self.showIterationsCheckBox, self.singleCellCheckBox, self.scatterCheckBox, self.showDatesCheckBox, self.xAxisComboBox]:
            self.buttonsLayout.addWidget(widget)
            widget.setMaximumWidth(170)   # restrain all widgets to that maximum width
            widget.show()                 # DEBUG does this fix the display update issue?

        #self.buttonsLayout.insertStretch(-1)      # add a stretcher at the end
        #self.buttonsLayout.insertSpacing(20, 10)

        # Add widgets to internal horizontal layouts
        #histogramLayout.addWidget(self.histogramButton)
        #histogramLayout.addWidget(self.histogramBinSpin)
        self.buttonsLayout.insertSpacing(20, 13)
        #exportLayout.addWidget(self.exportButton)
        #exportLayout.addWidget(self.exportComboBox)

        #self.buttonsLayout.addLayout(exportLayout)
        #self.buttonsLayout.addLayout(histogramLayout)

        self.main_frame.setLayout(self.buttonsLayout)
        self.setCentralWidget(self.main_frame)



        #**********************************************************
        #
        # Connections
        #
        #**********************************************************

        # Set connections:
        self.connect(self.loadButton, SIGNAL('clicked()'), self.load_table)
        self.connect(self.saveButton, SIGNAL('clicked()'), self.save_plot)
        self.connect(self.plotButton, SIGNAL('clicked()'), self.on_plot)
        #self.connect(self.addSolutionsplotButton, SIGNAL('clicked()'), self.on_solutions)  # dynamic display of solutions
        self.connect(self.showIterationsCheckBox, SIGNAL('stateChanged(int)'), self.on_showIterations)
        self.connect(self.singleCellCheckBox, SIGNAL('stateChanged(int)'), self.on_singleCell)
        self.connect(self.clfCheckBox, SIGNAL('stateChanged(int)'), self.on_clf)
        self.connect(self.newCheckBox, SIGNAL('stateChanged(int)'), self.on_newFigure)
        self.connect(self.physicalValuesCheckBox, SIGNAL('stateChanged(int)'), self.on_physicalValues)
        self.connect(self.showDatesCheckBox, SIGNAL('stateChanged(int)'), self.on_convertDate)
        #self.connect(self.showMessageCheckBox, SIGNAL('stateChanged(int)'), self.on_showMessage)
        #lself.connect(self.exportButton, SIGNAL('clicked()'), self.on_export)
        #self.connect(self.histogramButton, SIGNAL('clicked()'), self.on_histogram)
        self.connect(self.xAxisComboBox, SIGNAL('currentIndexChanged(int)'), self.on_xAxis)


    # Create widgets that depend on information from the
    # solver table, and can only be created after table has
    # been opened
    #
    def create_table_widgets(self):
        # Create QComboBox with entries from casa table
        # get all available solver parameters from casa table
        # Before we opened the table this is kept empty
        # If we have a table opened
        self.createSliders()

        self.create_parms_dropdown()
        self.create_parms_value_dropdown()
        self.create_solver_dropdown(self)

        # Update layouts
        self.buttonsLayout.update()

        #self.fig.canvas.draw()

    # Remove widgets - executed on loading of a new table
    #
    def removeWidgets(self):
       print "removeWidgets()"    # DEBUG

       # Remove plotting widgets
       self.buttonsLayout.removeWidget(self.plottingOptions)
       self.buttonsLayout.removeWidget(self.showIterationsCheckBox)

       # Remove table specific widgets
       self.timeStartSlider.hide()
       self.timeEndSlider.hide()
       self.timeStartSliderLabel.hide()
       self.timeEndSliderLabel.hide()
       self.frequencyStartSlider.hide()
       self.frequencyEndSlider.hide()
       self.frequencyStartSliderLabel.hide()
       self.frequencyEndSliderLabel.hide()
       self.parametersComboBox.hide()

       self.buttonsLayout.removeWidget(self.xAxisComboBox)
       self.buttonsLayout.removeWidget(self.timeStartSlider)
       self.buttonsLayout.removeWidget(self.timeEndSlider)
       self.buttonsLayout.removeWidget(self.timeStartSliderLabel)
       self.buttonsLayout.removeWidget(self.timeEndSliderLabel)
       self.buttonsLayout.removeWidget(self.frequencyStartSlider)
       self.buttonsLayout.removeWidget(self.frequencyEndSlider)
       self.buttonsLayout.removeWidget(self.frequencyStartSliderLabel)
       self.buttonsLayout.removeWidget(self.frequencyEndSliderLabel)
       self.buttonsLayout.removeWidget(self.parametersComboBox)
       self.buttonsLayout.update()


    # Create sliders from table information
    #
    def createSliders(self):
            # Slider to access time axis range of solver statistics
            self.solverQuery.getTimeSlots()
            self.solverQuery.getFreqs()

            # tracking is disabled, the slider emits the valueChanged() signal only when the user releases the slider.
            self.timeStartSlider=QSlider(Qt.Horizontal)
            self.timeStartSliderLabel = QLabel("S:")
            self.timeStartSlider.setTracking(False)
            starttime=self.solverQuery.timeSlots[0]['STARTTIME']              # read first STARTTIME

            if self.showDatesCheckBox.isChecked():
              self.timeStartSliderLabel.setText("S:" +  str(self.convertDate(starttime)))  # initialize StartTimeLabel with it
            else:
              self.timeStartSliderLabel.setText("S:" +  str(starttime) + " s")  # initialize StartTimeLabel with it

            self.timeStartSlider.setSingleStep(1)                        # step behaviour for single steps
            self.timeStartSlider.setPageStep(10)
            self.timeStartSlider.setMaximumWidth(170)
            self.timeStartSlider.setFocus()

            self.timeEndSlider=QSlider(Qt.Horizontal)
            self.timeEndSliderLabel = QLabel("E:")
            self.timeEndSlider.setTracking(False)
            #endtime=self.solverQuery.timeSlots[0]['ENDTIME']              # read first ENDTIME
            # set endtime to last entry in TimeSlots ENDTIME
            endtime=self.solverQuery.timeSlots[len(self.solverQuery.timeSlots)-1]['ENDTIME']
            if self.showDatesCheckBox.isChecked():
              self.timeEndSliderLabel.setText("E:" +  str(self.convertDate(endtime)))  # initialize EndTimeLabel with it
            else:
              self.timeEndSliderLabel.setText("E:" +  str(endtime) + " s")  # initialize EndTimeLabel with it

            self.timeEndSlider.setSingleStep(1)                        # step behaviour for single steps
            self.timeEndSlider.setPageStep(10)
            self.timeEndSlider.setMaximumWidth(170)

            self.frequencyStartSlider=QSlider(Qt.Horizontal)
            startfreq=self.solverQuery.frequencies[0]['STARTFREQ']
            self.frequencyStartSlider.setTracking(False)
            pos=str(startfreq/1e6).find('.')+5               # give frequency in MHz with 3 decimal places
            startFreqString=str(startfreq/1e6)[0:pos]
            self.frequencyStartSliderLabel = QLabel("Freq: " +startFreqString + " MHz")
            self.frequencyStartSlider.setSingleStep(1)                        # step behaviour for single steps
            self.frequencyStartSlider.setPageStep(10)
            self.frequencyStartSlider.setMaximumWidth(170)

            self.frequencyEndSlider=QSlider(Qt.Horizontal)
            endfreq=self.solverQuery.frequencies[0]['ENDFREQ']
            self.frequencyEndSlider.setTracking(False)
            pos=str(endfreq/1e6).find('.')+5                  # give frequency in MHz with 3 decimal places
            endFreqString=str(endfreq/1e6)[0:pos]
            self.frequencyEndSliderLabel = QLabel("Freq: " + endFreqString + " MHz")
            self.frequencyEndSlider.setSingleStep(1)                        # step behaviour for single steps
            self.frequencyEndSlider.setPageStep(10)
            self.frequencyEndSlider.setMaximumWidth(170)


            self.timeStartSlider.setRange(0, self.solverQuery.getNumTimeSlots()-1)
            self.connect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.on_timeStartSlider)
            self.timeEndSlider.setRange(0, self.solverQuery.getNumTimeSlots()-1)
            self.timeEndSlider.setValue(endtime)
            self.connect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.on_timeEndSlider)

            self.frequencyStartSlider.setRange(0, self.solverQuery.getNumFreqs()-1)
            self.connect(self.frequencyStartSlider, SIGNAL('valueChanged(int)'), self.on_frequencyStartSlider)
            self.frequencyEndSlider.setRange(0, self.solverQuery.getNumFreqs()-1)
            self.connect(self.frequencyEndSlider, SIGNAL('valueChanged(int)'), self.on_frequencyEndSlider)

            # Get solver parameter names from table
            self.parameters=self.solverQuery.getParameterNames()

            # Add widgets to buttonLayout (these are now class attributes to make them accessible)
            # Only add sliders and their labels to the buttonLayout if it is not there yet:
            self.buttonsLayout.addWidget(self.timeStartSliderLabel)
            self.buttonsLayout.addWidget(self.timeEndSliderLabel)
            self.buttonsLayout.addWidget(self.timeStartSlider)
            self.buttonsLayout.addWidget(self.timeEndSlider)
            self.buttonsLayout.addWidget(self.frequencyStartSliderLabel)
            self.buttonsLayout.addWidget(self.frequencyEndSliderLabel)
            self.buttonsLayout.addWidget(self.frequencyStartSlider)
            self.buttonsLayout.addWidget(self.frequencyEndSlider)

            #self.buttonsLayout.setSizeConstraint(1)
            self.buttonsLayout.setSizeConstraint(QLayout.SetDefaultConstraint)


    # Create drop down menu with solver parameters to choose
    # from
    #
    def create_solver_dropdown(self, solver):
        # Get parameter names from solver table
        parameterNames=self.solverQuery.getParameterNames()
        self.parametersComboBox=QComboBox()
        self.parametersComboBox.setMaximumWidth(170)

        # loop over parameterNames from SolverQuery and add them to ComboBox
        # Only add CORRMATRIX if it is actually available in the table
        #
        for p in parameterNames:
            if p=="CORRMATRIX" and (self.tableType=="PERITERATION" or self.tableType=="PERSOLUTION"):
                print
            else:
                self.parametersComboBox.addItem(p)


        # Finally add the ComboBox to the buttonsLayout
        self.buttonsLayout.addWidget(self.parametersComboBox)


    # Create an xAxis drop down menu based on a list of
    # plotable axes (this is currently only hard coded, and
    # maybe should be moved to the class attribute?)
    #
    def createxAxisCombo(self):
       print "solverDialog::createAxisCombo()"  # DEBUG
       
       plotableAxes=[]   # define plotable axes

       self.xAxisComboBox=QComboBox()
       self.xAxisComboBox.setMaximumWidth(125)
       self.xAxisComboBox.setMaximumHeight(25)
       self.xAxisComboBox.addItem("Time")    # TODO: read the entries as columns from SolverQuery
       self.xAxisComboBox.addItem("Freq")    # do not keep it hardcoded


    # Create a drop down menu to choose which physical value
    # of a parameter is being plotted (e.g. Amplitude / Phase)
    #
    # TODO: This is at the moment hard coded to Amplitude & Phase
    #
    def create_parms_value_dropdown(self):
        print "create_parms_value_dropdown()"   # DEBUG

        self.parmValueComboBox=QComboBox()
        self.parmValueComboBox.setMaximumWidth(170)

        # Make this a bit more intelligently, decide depending on parameter
        # names what physical value is contained in them
        parameter=str(self.parmsComboBox.currentText())   # need to convert QString to python string

        if parameter.find("Gain") is not -1:
            self.parmValueComboBox.addItem("Amplitude")
            self.parmValueComboBox.addItem("Phase")

        self.buttonsLayout.addWidget(self.parmValueComboBox)
        #self.parmValueComboBox.hide()


    # Create a drop down menu that contains the different parameters that BBS
    # was used to solve for; these are read from the Tablekeywords in the SolverTable
    #
    #
    def create_parms_dropdown(self):
        self.parmsComboBox=QComboBox()
        self.parmsComboBox.setMaximumWidth(170)

        parmMap=self.createParmMap()
        parmnames=self.populate(parmMap)

        for name in parmnames:
            self.parmsComboBox.addItem(name)

        self.connect(self.parmsComboBox, SIGNAL('currentIndexChanged(int)'), self.on_solution_changed)
        self.buttonsLayout.addWidget(self.parmsComboBox)


    # Create a status bar at the bottom of the MainWindow
    #
    def create_status_bar(self):
        print "create_status_bar()"
        self.status_text = QLabel("Solver statistics")
        self.statusBar().addWidget(self.status_text, 1)


    #******************************************************
    #
    # Checkbox handler functions
    #
    #******************************************************


    # Redraw the plot(s) with the current parameters
    # retrieves data through getSolutions() and getParameter() functions
    #
    # THIS function is not used anymore, now using PlotWindow::plot()
    #
    def on_plot(self):
        parm=self.parmsComboBox.currentText()   # Solution parameter, e.g. Gain:1:1:LBA001
        parameter=str(self.parametersComboBox.currentText())    # Get solver parameter from drop down

        # update class attribute plot arrays
        if self.solutions_plot == True:    # TODO
            self.y1=self.getSolutions(perIteration=self.perIteration)
        else:
            print  # do we need an else here?
            self.y1=self.getSolutions(perIteration=self.perIteration)

        self.x, self.y2=self.getParameter(parameter)   # get parameter to plot
        self.getMessages()                             # get dictionary with solver messages

      	# TODO: get current PlotWindow
        self.plots.append(lofar.bbs.plotwindow.PlotWindow(self))  # call PlotWindow class with this class as parent
        #self.plots.append(plotwindow.PlotWindow(self))  # DEBUG
        print "on_plot() finished drawing"


    # Get the last plot Window
    #
    def getLastPlot(self):
       print "solverDialog::getCurrentPlot()"
       length=len(self.plotWindows)
       return self.plotWindows[length-1]


    # Get the current number of plot Windows
    #
    def getNumberOfPlots(self):
       print "solverDialog::getNumberPlots()"
       numPlots=len(self.plotWindows)

       return numPlots


    # Get the current plot Window
    #
    def getCurrentPlot(self):
       print "getCurrentPlot()"

       # Loop over all plotWindows (parent attribute)
       for plot in parent.plotWindows:
          if self.isActiveWindow() == True:   # if that plotWindow has focus...
             return plot                      # ... return it
          else:
             continue


    # Get index of plot with a specific number
    #
    # num       - number of PlotWindow to look for
    # 
    def getPlotNumber(self, num):
       print "getPlotNumber()"

       return self.plotWindows(num)


    # Delete the PlotWindow with Number
    # TODO: be called on closing of a plotWindow
    #
    def deletePlotWindow(self, num):
       print "deletePlotWindow()"
       parent.plotWindows.remove(num)   # remove the plotWindow with that number from the list



    #****************************************************
    #
    # Widget action functions
    #
    #****************************************************

    # Set clear figure attribute self.clf depending on state
    # of clfCeckBox
    #
    def on_clf(self):
        self.clf=self.clfCheckBox.isChecked()


    # Set new figure attribute to generate another plot in a
    # new figure window
    #
    def on_newFigure(self):
        self.newfigure=self.newCheckBox.isChecked()


    # Trigger handling of physical interpretation of parameters
    #
    def on_physicalValues(self):
        print "on_physicalValue()"      # DEBUG
        self.physicalValues=self.physicalValuesCheckBox.isChecked()

    def on_convertDate(self):
        #print "on_convertDate()"       # DEBUG        

        # Update x-axis labels
        self.setXLabel()
        self.setYLabel()

        # and S: and E: labels for sliders

        indexStart=self.timeStartSlider.sliderPosition()
        indexEnd=self.timeEndSlider.sliderPosition()
        starttime=self.solverQuery.timeSlots[indexStart]['STARTTIME']
        endtime=self.solverQuery.timeSlots[indexEnd]['ENDTIME']

        if self.showDatesCheckBox.isChecked():
          self.timeStartSliderLabel.setText("S:" +  str(self.convertDate(starttime)))
        else:
          self.timeStartSliderLabel.setText("S:" +  str(starttime) + " s")
        if self.showDatesCheckBox.isChecked():
          self.timeEndSliderLabel.setText("E:" +  str(self.convertDate(endtime)))
        else:
          self.timeEndSliderLabel.setText("E:" +  str(endtime) + " s")
        
        self.timeStartSlider.setValue(self.timeStartSlider.sliderPosition())
        self.timeEndSlider.setValue(self.timeEndSlider.sliderPosition())
        

    # Set class attribute when showIterationsCheckBox is clicked
    # and changes its state
    #
    def on_showIterations(self):
        self.perIteration=self.showIterationsCheckBox.isChecked()

        # If periteration has been enabled, set everything to singleCell mode
        if self.perIteration:
            self.xLabel="Iteration"   # TODO: this should be handled by the self.xAxisType attribute
            self.xAxisType="Iteration"

            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())
            self.syncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))
            self.showIterationsCheckBox.setEnabled(True)
            self.singleCellCheckBox.setCheckState(Qt.Checked)
            #self.singleCellCheckBox.setCheckState(Qt.Unchecked)     # we seem to need this Tristate to have "normal" CheckBoxes
        else:
            if self.showDatesCheckBox.isChecked():
              self.xLabel="Time (UTC) in s after " + str(self.convertDate(self.x[0]))
            else:
              self.xLabel="Time (UTC) in s after " + str(self.x[0])    # TODO this must distinguish between Time and Frequency!

    # If xAxis has been changed in ComboBox
    #
    def on_xAxis(self):
       print "solverDialog::on_xAxis()"   # DEBUG

       self.xAxis=self.xAxisComboBox.currentText()
       self.xLabel=self.xAxis    

    # Determine the table type PERSOLUTION, PERITERATION or
    # PERSOLUTION_CORRMATRIX or PERITERATION_CORRMATRIX
    #
    def determineTableType(self):
        # Decide on type which plotting to do (PERSOLUTION,PERITERATION, with or without CORRMATRIX)
        self.tableType=self.solverQuery.getType()

        if self.tableType == "PERSOLUTION" or self.tableType == "PERSOLUTION_CORRMATRIX":
            self.perIteration=False
        elif self.tableType == "PERITERATION":
            self.perIteration=True


        print "determineTableType() self.tableType = ", self.tableType   # DEBUG


    # Set the X label property accordingly (time or frequency)
    #
    # TODO: react to all possible cases
    #
    def setXLabel(self):
        print "setXLabel()"                 # DEBUG

        if self.xAxisType == "Time":
            # first check we have a valid self.x
            if self.x==None:      # get time
              self.x=self.solverQuery.getTimeSlots()
            if self.showDatesCheckBox.isChecked():
              self.xLabel="Time (UTC) in s after " + str(self.convertDate(self.x[0]))
            else:
              self.xLabel="Time (UTC) in s after " + str(self.x[0])    # TODO this must distinguish between Time and Frequency!
        elif self.xAxisType == "Frequency":
            # first check we have a valid self.x
            if self.x==None:
              self.x=self.sq.getFreqs()[0]
            self.xLabel="Frequency in Hz"
        else:
            self.xLabel="Iteration No."


    # Set the Y label property according to the parameter
    #
    def setYLabel(self):
        print "setYLabel()"

        parameter=self.parametersComboBox.currentText()
        solverParm=self.parmValueComboBox.currentText()
        
        if len(parameter) > 10:
          parameter=parameter[0:math.floor(len(parameter))] + "\n" + parameter[math.ceil(len(parameter)):]
        
        print "parameter = ", parameter  # DEBUG
        
        self.y1Label=parameter
        self.y2Label=solverParm


    #******************************************************************
    #
    # Parameter and solutions functions
    #
    #******************************************************************

    # Function that reads the selected parameter from the solver table
    #
    # Parameters are passed on as dictionaries with entries "result" = false, last, all, iteration
    # and the corresponding entry "last", "all", or "iteration" containing the actual result as
    # a numpy.ndarray
    #
    def getParameter(self, parameter):
        # Get time and frequency intervals from the QWidgets
        start_time=self.solverQuery.timeSlots[self.timeStartSlider.value()]['STARTTIME']
        end_time=self.solverQuery.timeSlots[self.timeEndSlider.value()]['ENDTIME']
        start_freq=self.solverQuery.frequencies[self.frequencyStartSlider.value()]['STARTFREQ']
        end_freq=self.solverQuery.frequencies[self.frequencyEndSlider.value()]['ENDFREQ']


        #print "getParameter() timeStartSlider.value() = ", self.timeStartSlider.value(), " timeEndSlider.value() = ", self.timeEndSlider.value()   # DEBUG
        #print "getParameter() self.solverQuery.timeSlots[", self.timeEndSlider.value(), "] = ", self.solverQuery.timeSlots[self.timeEndSlider.value()]   # DEBUG


        if self.singleCellCheckBox.isChecked() == True or self.timeStartSlider.value() == self.timeEndSlider.value():
            singleCell=True
        elif self.singleCellCheckBox.isChecked() == False and self.timeStartSlider.value() != self.timeEndSlider.value():
            singleCell=False

        #periteration=self.perIteration        # get a local copy from the class attribute (TODO: change this to use self.perIteration)

        # We have to distinguish between a single solver value (from just one solution cell)
        # and the case we were given a series of values within an interval

        # There are two generic plotting cases:
        # (1) Only a single cell is displayed, then the plot
        #     displays the solver parameters per iteration (if present in the table)
        # (2) If the time sliders specify a time interval
        #     then the trend of the selected solver parameter over time is plotted
        #
        # ----------------------------------------
        # (1) Plotting a single cell (if there, with iterations)
        if singleCell==True:

            # If we only plot per solution
            if self.perIteration == False:
                if parameter == "CORRMATRIX":
                    print "getParameter(): CORRMATRIX"             # DEBUG
                    corrmatrix, x, ranks=self.solverQuery.getCorrMatrix(start_time, end_time, start_freq, end_freq, getStartTimes=True, getRank=True)
                    rank=self.solverQuery.getRank()

                    #print "getCorrMatrix() corrmatrix = ", corrmatrix
                    #print "getCorrMatrix() rank = ", rank
                    #print "getCorrMatrix() x = ", x

                    return x, corrmatrix   # return abcissa and corrmatrix/corrmatrices

                # "Normal parameter"
                else:
                    print "readParameter() start_time = ", start_time, " end_time = ", end_time      # DEBUG

                    # This solverQuery functions fetches the parameter along with the corresponding time stamps
                    y, x=self.solverQuery.readParameter(parameter, start_time, end_time, start_freq, end_freq)
                    return x, y["last"]

            # If we plot a single solution per iteration
            elif self.perIteration == True:
                self.xAxisType="Iteration"
                y, x=self.solverQuery.readParameter(parameter, start_time, end_time, start_freq, end_freq, iteration='all')
                # Set x to go from iteration 1 to the last one found in the dictionary for y
                x = range(1, len(y))
                y = self.rearrangeIteration(y)
                return x, y


        # (2) Getting a range of values over a time interval
        elif singleCell==False:
            print "getParameter(): plotting a time interval from time_start till time_end"  # DEBUG

            #x=self.solverQuery.getMidTimes(start_time, end_time)

            # Get data from table per iterations
            # Check if special parameter is asked for, e.g. getSolution
            if parameter == "SOLUTION":
                print "getSolutions() start_time = ", start_time, " end_time = ", end_time      # DEBUG

                y, x=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq)
                return y, x

            elif parameter == "CORRMATRIX":
                corrmatrix, x, ranks=self.solverQuery.getCorrMatrix(start_time, end_time, start_freq, end_freq, getStartTimes=True, getRank=True)
                rank=self.solverQuery.getRank()

                return corrmatrix, rank

            # "Normal parameter"
            else:
                print "getParameter(): Normal parameter"   # DEBUG

                y, x = self.solverQuery.readParameter(parameter, start_time, end_time, start_freq, end_freq)
                #y=self.solverQuery.readParameter(parameter, start_time, end_time, start_freq, end_freq)
                #x=self.solverQuery.getMidTimes(start_time, end_time)

                print "x=",x  # DEBUG
                print "y=",y  # DEBUG

                return x, y['last']

        else:
            print "getParameter(): can't plot with these options"



    # Get solutions from the solver table
    #
    # perIteration       - improvement of a solution per iteration (default=False)
    # xAxis              - return corresponding abcissa as second return value (default=False)
    #
    def getSolutions(self, perIteration=False, xAxis=False):
        # Get time and frequency intervals from the QWidgets
        start_time=self.solverQuery.timeSlots[self.timeStartSlider.value()]['STARTTIME']
        end_time=self.solverQuery.timeSlots[self.timeEndSlider.value()]['ENDTIME']
        start_freq=self.solverQuery.frequencies[self.frequencyStartSlider.value()]['STARTFREQ']
        end_freq=self.solverQuery.frequencies[self.frequencyEndSlider.value()]['ENDFREQ']

        solutions_array=[]

        if perIteration == True:
            solutions=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq, iteration='all')
            x=range(1, len(solutions)+1)

            for iter in range(1, len(solutions)):
                solutions_array.append(solutions[iter])
        else:
            solutions=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq, iteration='last')

            for iter in range(0, len(solutions['last'])):
                solutions_array.append(solutions['last'][iter])


        # Picking the solution that has been choosen in the solved parameters comboBox
        solutionIndex=self.parmsComboBox.currentIndex()
        parameter=self.parmsComboBox.currentText()
        physValue=self.parmValueComboBox.currentText()

        #print "getSolutions() parameter = ", parameter  # DEBUG
        #print "getSolutions() physValue = ", physValue  # DEBUG

        if perIteration == True:
            x=range(1, len(solutions))   # why do we need the +1?
        else:
            x=self.solverQuery.getMidTimes(start_time, end_time)

        if physValue == "Amplitude":
            solution=self.computeAmplitude(parameter, solutions_array)
        elif physValue == "Phase":
            solution=self.computePhase(parameter, solutions_array)

        if xAxis==True:
            return solution, x
        else:
            return solution


    # Get the solver messages for this time and freq range (or per iteration)
    #
    def getMessages(self):
        print "getMessages()"   # DEBUG

        # Get time and frequency intervals from the QWidgets
        start_time=self.solverQuery.timeSlots[self.timeStartSlider.value()]['STARTTIME']
        end_time=self.solverQuery.timeSlots[self.timeEndSlider.value()]['ENDTIME']
        start_freq=self.solverQuery.frequencies[self.frequencyStartSlider.value()]['STARTFREQ']
        end_freq=self.solverQuery.frequencies[self.frequencyEndSlider.value()]['ENDFREQ']
        
        if self.perIteration==True:
          self.messages=self.solverQuery.getMessages(start_time, end_time, start_freq, end_freq, iteration="all")
        else:
          self.messages=self.solverQuery.getMessages(start_time, end_time, start_freq, end_freq, iteration="last")

        #print "solverDialog.py::getMessages() self.messages = ", self.messages   # DEBUG

    """
    # Plot a corrmatrix on the lower subplot
    # corrmatrixDict  - (linear) array with correlation matrix to plot
    # ranks           - list of corresponding ranks of corrmatrices (to check for consistency, default=None)
    # fig             - figure to plot on (default=None)
    # sub             - if a subplot is given plot in subplot (default=None)
    #
    def plotCorrMatrix(self, corrmatrices, x=None, ranks=None, fig=None, sub=None):
        print "plotCorrMatrix() type(corrmatrices): ", type(corrmatrices)    # DEBUG

        # Do we need this? Better to compare to sqrt(len(corrmatrix)
        #ranks=self.solverQuery.getRanks(start_time, end_time, start_freq, end_freq)
        if ranks!=None:
            if len(ranks) != len(corrmatrices):
                raise ValueError
        else:
            ranks=np.zeros(len(corrmatrices), dtype=int)
            for i in range(0, len(corrmatrices)):
                ranks[i]=math.sqrt(len(corrmatrices[i]))

        # First determine, if we got a single corrMatrix or a list of them
        if len(corrmatrices >= 1):
            for i in range(0, len(corrmatrices)):
                shape=corrmatrices[i].shape      # get shape of array (might be 1-D)

                #print "plotCorrMatrix() shape = ", shape

                if len(shape)==1:                # if we got only one dimension...
                    if shape[0] != ranks[i]*ranks[i]:        # if the length of the array is not rank^2
                        raise ValueError
                    else:
                        #print "plotCorrMatrix() ranks[i] = ", ranks[i]                            # DEBUG
                        #print "plotCorrMatrix() shape[0] = ", shape[0]                            # DEBUG
                        #print "plotCorrMatrix() corrmatrices[i] = ", corrmatrices[i]              # DEBUG
                        #print "plotCorrMatrix() type(corrmatrices[i]) = ", type(corrmatrices[i])  # DEBUG

                        corrmatrix=np.reshape(corrmatrices[i], (ranks[i], ranks[i]))
                elif len(shape)==2:              # we already have a two-dimensional array
                    if shape[0] != ranks[i] or shape[1] != ranks[i]:
                        raise ValueError

                # plot CorrMatrix as a figure image
                #corrImage=pl.matshow(corrmatrix)
                #pl.colorbar(corrImage)                   # Add a color bar to the figure

                #pl.plot(corrImage, cmap=cm.jet)
                ax.figimage(corrmatrix, cmap=cm.jet, origin='lower')
                self.canvas.draw()
        else:
            print "plotCorrMatrix() corrmatrices have length 0"
    """

    """
    # (Modified Joris'plot function)
    #
    # Plotting function that reads the setting for  scatter, newfig etc. from the GUI
    # and plots all arrays that are defined in the class attributes
    #
    # x          - abcissa to plot
    # y          - value to plot
    # subplot    - subplot to plot into (default=self.ParameterSubplot)
    # clf        - clear figure (default=False)
    #
    def plot(self, y, x=None, subplot=None, clf=False):

        tplotStart=time.time()   # take start time to measure plot() execution time

        # If no x axis is given, use indexing into y as axis
        if x is None:
            x = [range(len(yi)) for yi in y]

        if subplot==None:
            subplot=self.ParameterSubplot    # by default plot to self.ParameterSubplot

        # Get plotting parameters from GUI
        clf=self.clfCheckBox.isChecked()
        scatter=self.scatterCheckBox.isChecked()
        colorize=self.colorizeCheckBox.isChecked()

        if clf:              # If a clear figure was sent, too
            self.fig.clf()

        if scatter:
         self.ParameterSubplot.scatter(x, y, edgecolors="None", c="red", marker="o")
        else:
         self.ParameterSubplot.plot(x, y, marker="o")

        # Plot ParameterSubplot
        #
        #
        # If only a single cell was provided
        #
        if isinstance(self.y, np.int32) or isinstance(y, int) or isinstance(y, float) or (isinstance(y, np.ndarray) and len(y)==1) or (isinstance(y, list) and len(y)==1):

            scatter=True   # it seems we always have to use scatter plots for single values
            if scatter:
                self.ParameterSubplot.scatter(x, y, edgecolors="None", c="red", marker="o")
                #axes.scatter(x, y, edgecolors="None",
                           #  c=self.__styles[0][0], marker="o")
            else:
                self.ParameterSubplot.plot(x, y, marker="o")
        elif len(y) > 1:   # if a numpy array was provided
                if scatter:
                    self.ParameterSubplot.scatter(x, y, edgecolors="None",
                        c=self.__styles[len(self.__styles)-1][0], marker="o")
                else:
                    self.ParameterSubplot.plot(x, y , marker="o")


        # Do stuff after plotting (update canvas and take time)
        #self.fig.canvas.draw()  # "redraw" figure

        tplotEnd=time.time()                  # take final time after redrawing of the canvas
        tplotTime=tplotEnd-tplotStart
        print "plot(): plotting took %6.2f ms" % (tplotTime*1000)
    """


    #******************************************************
    #
    # Plotting helper functions
    #
    #******************************************************

    # Cut two arrays x,y to the length of the smaller one
    #
    def cutMinLength(self, x, y):
        length=min(len(x), len(y))
        x=x[0:length]   # drop remainder of x
        y=y[0:length]   # drop remainder of y

        return x,y
        

    #*****************************************************
    #
    # Helper functions
    #
    #*****************************************************

    # Function that tries to import scipy and catches an exception
    # TODO: how to do that elegantly without Function call
    #
    def importScipy(self):
        try:                                  # try to import scipy.io - used to export to Matlab format
            import scipy.io
            self.useScipy=True
            return True
        except ImportError:        # Catches every error
            print "No module scipy found, you can not export data to Matlab format"
            self.useScipy=False
        return False


    # Module import function which catches the exception if loading fails
    # and returns true or false depending on successfull import
    #
    # module        - name of module to import
    #
    def importModule(self, module):
        try:                       # try to import module
            print "importModule(", module, ")"   # DEBUG
            __import__(module)
            return True
        except ImportError:        # Catches every error
            print "No module ", module, " found"
            return False


    # Check if a particular module has been imported
    #
    def haveModule(self, module):
        print "haveModule() module = ", module
        #print "haveModule() sys.modules = ", sys.modules
        print "module in sys = ", module in sys.modules

        if module in sys.modules:
            return True
        else:
            return False

    # Return relativeTimes, starting at STARTTIME with 0
    #
    def computeRelativeX(self):
       self.relativeX=np.zeros(len(self.x), dtype=float)
       for i in range(0, len(self.x)):
          self.relativeX[i]=self.x[i] - self.x[0]

       return self.relativeX


    # Rearrange a dictionary of iterations returned by SolverQuery
    # into an array that then can be plotted
    # Each iteration no. is a dictionary keyword
    #
    def rearrangeIteration(self, parameterDict):
        #print "rearrangeIteration()"    # DEBUG

        if isinstance(parameterDict, dict) == False:
            print "plotIterations(): parameterDict is not a dictionary"
            return False     # return an error

        y=[]    # list to hold individual iteration results
        length=len(parameterDict)   # the first entry is "result" giving the type of dictionary

        for i in range(1, length):
            y.append(parameterDict[i])   # read from dictionary and rearrange (starting with index 0)

        return y


    # Get all the ranks for the current selection
    #
    def getRanks(self, start_time, end_time, start_freq, end_freq):
        print "solverDialog::getRanks()"    # DEBUG
        ranks=self.solverQuery.getRank()

        return ranks

    # Delete all axes in a figure, figure defaults to None
    # then self.fig is used
    #
    def delAllAxes(self):
        for ax in self.fig.axes:
            print "delAllAxes(): deleting ", ax    # DEBUG
            ax.delaxes()


    # DEBUG function
    def printAllAxes(self):
        i=0
        for ax in self.fig.axes:
            i=i+1
            print "printAllAxes(): ax(%2d) = %s" % (i, ax)


    # Set the title for this figure
    #
    def setTitle(self, title=""):
        print "setTitle(): "   # DEBUG
        self.fig.set_title(title)


    # Convert the absolute times from the table to time stamps
    # which are given relative to the start time of the observation
    #
    #
    def computeRelativeTimes(self, times):
       print "computeRelativeTimes()"

       relativeTimes=np.ndarray(len(times))     # array to hold relative times which has equal lenght as midtimes

       for i in range(0, len(times)):
          relativeTimes[i] = times[i] - times[0]

       return relativeTimes


    #*****************************************************
    #
    # Export functions
    #
    #*****************************************************

    # Export the whole arraus to an ASCII text file
    #
    # filename       - basename for ASCII file to write to
    # parameter      - parameter to export
    #
    def exportDataASCII(self, filename, parameter):
        print "export_data()"   # DEBUG
        fh=open(filename, 'w')

        print "exportDataASCII() parameter = ", parameter   # DEBUG

        # if parameter is a physical parameter
        if self.parmValueComboBox.findText(parameter) != -1:

            print "exportDataASCII() physical parameter"   # DEBUG

            if isinstance(self.y1, float) or isinstance(self.y2, float):
                line=str(self.x)  + "\t"  + str(self.y1) + "\n"
                fh.write(line)
            else:
                for i in range(0, len(self.x)):
                    line=str(self.x[i]) + "\t"  + str(self.y1[i]) + "\n" #+ self.y2[i] + "\n"
                    fh.write(line)
        # if parameter is part of the Solver parameters
        elif self.parametersComboBox.findText(parameter) != -1:

            print "exportDataASCII() solver parameter"   # DEBUG

            if isinstance(self.y1, float) or isinstance(self.y2, float):
                line=str(self.x)  + "\t"  + str(self.y2) + "\n"
                fh.writeline(line)
            else:
                for i in range(0, len(self.x)):
                    line=str(self.x[i]) + "\t" + str(self.y2[i]) + "\n"
                    fh.write(line)

        # If the parameter is CORRMATRIX then use the dedicated function
        elif parameter=="CORRMATRIX":
            exportCorrMatrix(filename, fileformat="ASCII")

        fh.close()
        return True


    # Export data in Matlab format
    #
    # filename    - name of file to write to
    # parameter   - parameter to save
    # compress    - use Matlab compression for Matrices (default=False)
    #
    def exportDataMatlab(self, filename, parameter, compress=False):
        print "exportDataMatlab()"

        # Check if we have scipy.io imported
        imported_modules=sys.modules()
        if scipy.io not in imported_modules:
            print "exportDataMatlab() scipy.io needed to export to Matlab format"
            return False
        else:
            print "generating Matlab file"
            dctionary={}
            dictionary[parameter]=parameter

            print "exportDataMatlab() dictionary = ", dictionary    # DEBUG

            scipy.io.savemat(filename, appendmat=True, do_compression=compress)


    # Export correlation matrix to a file
    #
    # filename       - basename for ASCII file to write to
    # fileformat     - file format to write to ("ASCII"=default, "Matlab")
    #
    def exportCorrMatrix(self, filename, fileformat="ASCII"):
        print "exportCorrMatrix()"       # DEBUG

        # self.y2 stores correlation matrix if parameter was selected
        print "self.y2 = ", self.y2      # DEBUG

        # Save in ASCII format
        if fileFormat=="ASCII":
            fh=open(filename, 'w')

            rank=math.sqrt(len(self.y2))    # rank = sqrt(corrMatrix-length)
            for i in range(0 , rank):
                for j in range(0, rank):
                    if j < rank-1:
                        line = str(self.y2) + "\t"
                    else:
                        line = str(self.y2) + "\n"

            fh.close()                      # close ASCII file
        # Save in Matlab format, using scipy.io
        elif fileFormat=="Matlab":
            # we need to write the matrix as a dictionary
            mdict['CorrMatrix']=self.y2

            print "exportData() Matlab file format"     # DEBUG
            # This apparantly only works with scipy (which is installed on the cluster)
            scipy.io.savemat(filename, mdict)

        return True     # on successful write



    # Export the currently displayed plot to an ASCII text file
    #
    # TODO: how do we get the limits of the current axes as indices into the data?
    #
    # subplot - subplot to export to ASCII
    # filename - name of file to write to
    #
    def exportPlot(self, subplot, filename):
        print "export_plot()"          # DEBUG

        fh=open(filename, 'w')   # open "filename" for writing

        if fh == 0:   # If we did not get a file handle
            print "export_plot() could not open file ", filename, " for writing."
            return False
        else:
            # Get currently displayed data points from Matplotlib

            # Loop over data points

            # write them to the file
            data=str(x) + "\t" + str(y) + "\n"
            fh.write(data)


#*****************************************************
#
# parmDB functions
#
#*****************************************************

    # Populate the parameter selection (menu) with available
    # parms in the solver log table
    # If parms are given as a
    # from the parmDB stored in the casa subtable
    #
    #
    def populate(self, parms=None):

        if parms==None:                   # if no parms was given...
            parms=self.parmDB.getNames()  # get parmNames from parmDB
        else:                             # otherwise get them from the ParmMap
            if isinstance(parms, dict):
                parms=parms.keys()

        for parm in parms:
            split = parm.split(":")

            if contains(split, "Real") or contains(split, "Imag"):
                if contains(split, "Real"):
                    idx = split.index("Real")
                    split[idx] = "Imag"
                    elements = [parm, ":".join(split)]
                else:
                    idx = split.index("Imag")
                    split[idx] = "Real"
                    elements = [":".join(split), parm]

                split.pop(idx)
                name = ":".join(split)

                #print "populate() name = ", name # DEBUG
                self.parms.append(name)

            elif contains(split, "Ampl") or contains(split, "Phase"):
                if contains(split, "Ampl"):
                    idx = split.index("Ampl")
                    split[idx] = "Phase"
                    elements = [parm, ":".join(split)]
                else:
                    idx = split.index("Phase")
                    split[idx] = "Ampl"
                    elements = [":".join(split), parm]

                split.pop(idx)
                name = ":".join(split)
                self.parms.append(name)

            else:
                self.parms.append(name)
                #self.parms.append(Parm(self.parmDB, parm))

        self.parms.sort(cmp=lambda x, y: cmp(x, y))

        #domain = common_domain(self.parms)
        #if not domain is None:
        #    self.resolution[0].setText("%.6f" % ((domain[1] - domain[0]) / 100.0))
        #    self.resolution[1].setText("%.6f" % ((domain[3] - domain[2]) / 100.0))

        return self.parms


    # Joris "Fetch value" function
    #
    def __fetch_value(self, name, domain=None, resolution=None):
        if domain is None:
            tmp = self._db.getValuesGrid(name)[name]
        else:
            if resolution is None:
                tmp = self._db.getValues(name, domain[0], domain[1], domain[2], domain[3])[name]
            else:
                tmp = self._db.getValuesStep(name, domain[0], domain[1], resolution[0], domain[2], domain[3], resolution[1])[name]

        if type(tmp) is dict:
            return tmp["values"]

        return tmp


    # Create parmMap that maps parameter name to indices in solution
    # array
    #
    def createParmMap(self):
        #print "createParmMap()"   # DEBUG
        parmMap={}                 # Dictionary containing Parameter names mapped to indices

        parmNames=['gain', 'directionalgain','mim']  # extend these as necessary

        # Read keywords from TableKeywords
        keywords=self.solverQuery.solverTable.keywordnames()
        for key in keywords:                                    # loop over all the keywords found in the TableKeywords
            for parmName in parmNames:                          # loop over the list of all allowed parmNames
                if parmName in key.lower():                     # if an allowed parmName is found in the key
		    index=keywords.index(key)			# better to use index for getkeyword to avoid . conflict
                    indices=self.solverQuery.solverTable.getkeyword(index)  # extract the indices
                    parmMap[key]=indices                                    # and write them into the python map

        return parmMap


    # Unwrap phase
    #
    def unwrap(phase, tol=0.25, delta_tol=0.25):
        """
        Unwrap phase by restricting phase[n] to fall within a range [-tol, tol]
        around phase[n - 1].

        If this is impossible, the closest phase (modulo 2*pi) is used and tol is
        increased by delta_tol (tol is capped at pi).
        """

        assert(tol < math.pi)

        # Allocate result.
        out = numpy.zeros(phase.shape)

        # Effective tolerance.
        eff_tol = tol

        ref = phase[0]
        for i in range(0, len(phase)):
            delta = math.fmod(phase[i] - ref, 2.0 * math.pi)

            if delta < -math.pi:
                delta += 2.0 * math.pi
            elif delta > math.pi:
                delta -= 2.0 * math.pi

            out[i] = ref + delta

            if abs(delta) <= eff_tol:
                # Update reference phase and reset effective tolerance.
                ref = out[i]
                eff_tol = tol
            elif eff_tol < math.pi:
                # Increase effective tolerance.
                eff_tol += delta_tol * tol
                if eff_tol > math.pi:
                    eff_tol = math.pi

        return out


    # Unwrap phase, windowed mode
    #
    def unwrap_windowed(phase, window_size=5):
        """
        Unwrap phase by estimating the trend of the phase signal.
        """

        # Allocate result.
        out = numpy.zeros(phase.shape)

        windowl = numpy.array([math.fmod(phase[0], 2.0 * math.pi)] * window_size)

        delta = math.fmod(phase[1] - windowl[0], 2.0 * math.pi)
        if delta < -math.pi:
            delta += 2.0 * math.pi
        elif delta > math.pi:
            delta -= 2.0 * math.pi
        windowu = numpy.array([windowl[0] + delta] * window_size)

        out[0] = windowl[0]
        out[1] = windowu[0]

        meanl = windowl.mean()
        meanu = windowu.mean()
        slope = (meanu - meanl) / float(window_size)

        for i in range(2, len(phase)):
            ref = meanu + (1.0 + (float(window_size) - 1.0) / 2.0) * slope
            delta = math.fmod(phase[i] - ref, 2.0 * math.pi)

            if delta < -math.pi:
                delta += 2.0 * math.pi
            elif delta > math.pi:
                delta -= 2.0 * math.pi

            out[i] = ref + delta

            windowl[:-1] = windowl[1:]
            windowl[-1] = windowu[0]
            windowu[:-1] = windowu[1:]
            windowu[-1] = out[i]

            meanl = windowl.mean()
            meanu = windowu.mean()
            slope = (meanu - meanl) / float(window_size)

        return out


    # Normalize the phase
    #
    def normalize(phase):
        """
        Normalize phase to the range [-pi, pi].
        """

        # Convert to range [-2*pi, 2*pi].
        out = numpy.fmod(phase, 2.0 * numpy.pi)

        # Convert to range [-pi, pi]
        out[out < -numpy.pi] += 2.0 * numpy.pi
        out[out > numpy.pi] -= 2.0 * numpy.pi

        return out


    # Compute amplitude for parameter
    #
    def computeAmplitude(self, parameter, solutions):
        #print "computeAmplitude(): parameter = ", parameter   # DEBUG

        parameter=str(parameter)
        self.parmMap=self.createParmMap()

        #print "computeAmplitude() parameter = ", parameter   # DEBUG
        #print "computeAmplitude() parmMap = ", self.parmMap  # DEBUG

        # Insert REAL and Imag into parameter
	pos=parameter.find("Gain")	# this works for Gain: and DirectionalGain
        parameterReal=parameter[:(pos+8)] + ":Real" + parameter[(pos+8):]
        parameterImag=parameter[:(pos+8)] + ":Imag" + parameter[(pos+8):]

        #print "computeAmplitude() parameterReal =", parameterReal   # DEBUG
        #print "computeAmplitude() parameterImag = ", parameterImag  # DEBUG

        # The physical interpretation to compute the amplitude from the two
        # coefficients representing the real and imaginary part is hard coded
        #
        amplitude=[]
        real_idx=self.parmMap[parameterReal][0]
        imag_idx=self.parmMap[parameterImag][0]

        #print "real_idx = ", real_idx   # DEBUG
        #print "imag_idx = ", imag_idx   # DEBUG
        #print "type(solutions) = ", type(solutions)  # DEBUG

        # Decide on data type of solutions
        if isinstance(solutions, int):
            amplitude=math.sqrt(solutions[real_idx]^2 + solutions[imag_idx]^2)
        elif isinstance(solutions, np.ndarray) or isinstance(solutions, list):
            length=len(solutions)

            # compute amplitude for each entry and append to vector
            if length == 1:
                amplitude.append(math.sqrt(solutions[0][real_idx]**2 + solutions[0][imag_idx]**2))
            else:
                for iter in range(0, length):   # Loop over solutions
                    amplitude.append(math.sqrt(solutions[iter][real_idx]**2 + solutions[iter][imag_idx]**2))

        return amplitude


    # Compute phase for parameter
    #
    def computePhase(self, parameter, solutions):
        print "computePhase(): parameter = ", parameter   # DEBUG

        phase=[]
        parameter=str(parameter)   # convert QString to string

        self.parmMap=self.createParmMap()

        # Insert REAL and Imag into parameter
	pos=parameter.find("Gain")	# this works for Gain: and DirectionalGain
        parameterReal=parameter[:(pos+8)] + ":Real" + parameter[(pos+8):]
        parameterImag=parameter[:(pos+8)] + ":Imag" + parameter[(pos+8):]

        real_idx=self.parmMap[parameterReal][0]
        imag_idx=self.parmMap[parameterImag][0]

        # Decide on data type of solutions
        if isinstance(solutions, int):
            print "int"
            phase=math.atan(solutions[imag_idx]/solutions[real_idx])
            #phase=math.sqrt(solutions[real_idx]^2 + solutions[imag_idx]^2)

        elif isinstance(solutions, np.ndarray) or isinstance(solutions, list):
            print "np.ndarray"    # DEBUG

            length=len(solutions)

            # compute amplitude for each entry and append to vector
            if length == 1:
                phase.append(math.atan(solutions[0][imag_idx]/solutions[0][real_idx]))
            else:
                for iter in range(0, length):   # Loop over solutions
                    phase.append(math.atan(solutions[iter][imag_idx]/solutions[iter][real_idx]))

        return phase


# Joris' helper function
#
def contains(container, item):
    try:
        return container.index(item) >= 0
    except ValueError:
        return False





# Joris' high-level class for accessing the parmDB
#
#
class Parm:
    def __init__(self, db, name, elements=None, isPolar=False):
        self._db = db
        self._name = name
        self._elements = elements
        self._isPolar = isPolar
        self._value = None
        self._value_domain = None
        self._value_resolution = None

        self._readDomain()

    def name(self):
        return self._name

    def isPolar(self):
        return self._isPolar

    def empty(self):
        return self._empty

    def domain(self):
        return self._domain

    def value(self, domain=None, resolution=None, asPolar=True, unwrap_phase=False):
        if self.empty():
            assert(False)
            return (numpy.zeros((1,1)), numpy.zeros((1,1)))

        if self._value is None or self._value_domain != domain or self._value_resolution != resolution:
            self._readValue(domain, resolution)

        if asPolar:
            if self.isPolar():
                ampl = self._value[0]
                phase = normalize(self._value[1])
            else:
                ampl = numpy.sqrt(numpy.power(self._value[0], 2) + numpy.power(self._value[1], 2))
                phase = numpy.arctan2(self._value[1], self._value[0])

            if unwrap_phase:
                for i in range(0, phase.shape[1]):
                    phase[:, i] = unwrap(phase[:, i])

            return (1.0/ampl, phase)

        if not self.isPolar():
            re = self._value[0]
            im = self._value[1]
        else:
            re = self._value[0] * numpy.cos(self._value[1])
            im = self._value[0] * numpy.sin(self._value[1])

        return (1.0/re, im)

    def _readDomain(self):
        if self._elements is None:
            self._domain = self._db.getRange(self.name())
        else:
            domain_el0 = self._db.getRange(self._elements[0])
            domain_el1 = self._db.getRange(self._elements[1])
            self._domain = [max(domain_el0[0], domain_el1[0]), min(domain_el0[1], domain_el1[1]), max(domain_el0[2], domain_el1[2]), min(domain_el0[3], domain_el1[3])]

        self._empty = (self._domain[0] >= self._domain[1]) or (self._domain[2] >= self._domain[3])

    def _readValue(self, domain=None, resolution=None):
#        print "fetching:", self.name()

        if self._elements is None:
            value = numpy.array(self.__fetch_value(self.name(), domain, resolution))
            self._value = (value, numpy.zeros(value.shape))
        else:
            el0 = numpy.array(self.__fetch_value(self._elements[0], domain, resolution))
            el1 = numpy.array(self.__fetch_value(self._elements[1], domain, resolution))
            self._value = (el0, el1)

        self._value_domain = domain
        self._value_resolution = resolution

    def __fetch_value(self, name, domain=None, resolution=None):
        if domain is None:
            tmp = self._db.getValuesGrid(name)[name]
        else:
            if resolution is None:
                tmp = self._db.getValues(name, domain[0], domain[1], domain[2], domain[3])[name]
            else:
                tmp = self._db.getValuesStep(name, domain[0], domain[1], resolution[0], domain[2], domain[3], resolution[1])[name]

        if type(tmp) is dict:
            return tmp["values"]

        # Old parmdb interface.
        return tmp


#****************************************************************
#
# Main function
#
#****************************************************************
#
def main():
    app = QApplication(sys.argv)
    form = SolverAppForm()

    #form.haveScipy=form.importScipy()
    #print "self.haveScipy = ", form.haveScipy

    # for some reason this must be here, and can not be in create_main_frame()
    form.connect(form.quitButton, SIGNAL('clicked()'), app, SLOT('quit()'))

    form.show()

    # If there is a table given as command line argument, open that table
    if len(sys.argv) == 2 and sys.argv[1] is not "":
        tableName=sys.argv[1]
        form.open_table(tableName)

    app.exec_()


# Main entry function
#
if __name__ == "__main__":
    main()
