#!/usr/bin/env python

# Script that parses BBS kernel log for timing information
#
# For the moment this script only works on individual MS files and does collect
# the information through gds and ssh
#
#
# File:           BBStiming.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2011-03-08
# Last change:    2011-08-02


import os
import sys
import getopt
import datetime                     # needed for timestamps
from optparse import OptionParser   # command line option parsing
import numpy
import re                           # we want to use regular expressions


import matplotlib
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure
import matplotlib.cm as cm          # color maps?

from PyQt4.QtCore import *
from PyQt4.QtGui import *


# Define timer log format
#IDENTIFIER, UNIT_FIELD, STEP_FIELD, SUBSTEP_FIELD, TOTAL_KEY, TOTAL_FIELD, COUNT_KEY, COUNT_FIELD, AVG_KEY, AVG_FIELD = range(10)
#TOTAL_ALL_FIELD, TOTAL_COUNT_FIELD, TOTAL_AVG_FIELD = range(3)
#STEPPOS = 3    # global variabe for step position in timer line




# Open kernel log file
class BBSTiming:

   def __init__(self):
      self.filename=""           # name of file we are parsing (kernel_<>_log or pipeline.log)
      self.logtype=""            # type of log we are working on: kernel or pipeline
      self.timedSteps=[]         # Steps that were timed
      # BBS steps keywords we know of and look for
      self.timedStepsCount=[]    # do we need this?
      self.subbands=[]           # list of subbands present in pipeline.log
      
      self.keywords=["total", "count", "avg"]
      
      self.lines=[]              # list containing timer lines read from kernellog file
      self.modifiedLines=[]      # TIMER lines where individual steps are marked with counters
      self.havePlotting=False    # internal variable to tell us if we have plotting (default=False)
   
   # Module import function which catches the exception if loading fails
   # and returns true or false depending on successfull import
   #
   # module        - name of module to import
   #
   def importModule(self, module):
      try:                       # try to import module
            __import__(module)
            return True
      except ImportError:        # Catches every error
            print "No module ", module, " found"
            return False

   # Identify if we are working on 
   #
   def identifyLog(self):
      print "BBStimming.py: identifyLog()"   # DEBUG
      
      # If we find "Pipeline starting" we have a pipeline.log!
      i=0           # only look for i 10 lines
      for line in self.lines:
         if i>10:
            break

         if line.find("Pipeline starting") != -1:
            self.logtype="pipeline"
            break
         else:
            self.logtype="kernel"
            i=i+1

      if self.logtype=="pipeline":
         # Define timer log format for pipeline.log
         # has additional SUBBAND_FIELD=2 (e.g. node.locus020.bbs.L25960_SAP000_SB231_uv.MS.dppp:)
         SUBBAND_FIELD, IDENTIFIER, UNIT_FIELD, STEP_FIELD, SUBSTEP_FIELD, TOTAL_KEY, TOTAL_FIELD, COUNT_KEY, COUNT_FIELD, AVG_KEY, AVG_FIELD = range(2, 13)
         TOTAL_ALL_FIELD, TOTAL_COUNT_FIELD, TOTAL_AVG_FIELD = range(3, 6)
      else:
         # Define timer log format for kernel_<pid>.log
         IDENTIFIER, UNIT_FIELD, STEP_FIELD, SUBSTEP_FIELD, TOTAL_KEY, TOTAL_FIELD, COUNT_KEY, COUNT_FIELD, AVG_KEY, AVG_FIELD = range(10)
         TOTAL_ALL_FIELD, TOTAL_COUNT_FIELD, TOTAL_AVG_FIELD = range(3)


   # Read BBS Kernellog from location and parse it into timing components
   #
   #
   def readLogfile(self, filename):
      print "readLogfile()"          # DEBUG

      try:
         log_fh=open(filename, "r")
      except IOError, err:
         print str(err)
         raise

      # Look for timing information
      # throw everything away that does not start with TIMER
      lines=log_fh.readlines()   # can we make the more efficient? Only need end of log....
      for line in lines:
         if line.find("TIMER s") != -1:
            self.lines.append(line)

      self.identifyLogfile(filename)   # identify if we have a kernel_<pid>.log or pipeline.log


   # Identify steps in lines
   #
   # We return a list of dictionaries, where each dictionary represents a line with the
   # keywords found in the log as keywords and the corresponding values as entries
   # "Steps" are grouped together by the fact that they have a common STEP name, but different
   # substeps
   #
   # We need to consider that steps of the same name are run multiple times and got to name
   # them accordingly and then replace them in the line to still be able to search for them :-(
   #
   def identifySteps(self, lines=None):
      #print "identifySteps()"                # DEBUG
   
      count=1                                # count indicator for multiple steps
      if lines==None:                        # If no lines were provided to search in
         lines=self.lines                    # use the objects timer lines
   
      identifiedSteps=[]                     # we need a dictionary, because every step has substeps
      stepDict={}                            # dictionary for a particular substep


      if self.
      for line in lines:
         line.upper()                        # convert all to upper for string matching
         if line.find("ALL") != -1:          # we use "ALL" as indicator for a step, it is common to all steps and appears only once
         
            fields=line.split()              # split line at white spaces (default)
            unit=fields[UNIT_FIELD]          # keep unit for reference? don't really need it now
            step=fields[STEP_FIELD]          # "TIMER s <STEP> <SUBSTEP>"
                        
            step=step.upper() 
            if step in self.timedSteps:       # if we already encountered this step
               # then find out how often...
               count=1
               stepcount=step + "_" + str(count) 
               while stepcount in self.timedStepsCount:
                  count+=1
                  stepcount=step + "_" + str(count)

               #if self.timedStepsCount not in self.timedStepsCount:
               self.timedStepsCount.append(stepcount)
            else:
               self.timedSteps.append(step)                 # keep original found timedSteps
               count=1
               stepcount=step + "_" + str(count)
               self.timedStepsCount.append(stepcount)
               
            self.timedSteps.sort()
            self.timedStepsCount.sort()

      #print "self.timedSteps = ", self.timedSteps              # DEBUG
      #print "self.timedStepsCount = ", self.timedStepsCount    # DEBUG
      return self.timedSteps, self.timedStepsCount   

   
   # Modify lines to have unique step identifiers
   #
   # Note:
   # Due to the fact that the ALL statement of a step is not guaraneteed
   # to be at the end of step, but also can be at the beginning of a step
   # we can not distinguish a seemingly two-folded step of the same name
   # that has an ALL statement in-between as two distinctive steps. Since
   # the substeps belong to a single named step, though, it could just
   # equally be considered as one step anyway - which is done in this
   # implementation
   #
   def makeUniqueSteps(self, lines=None):
      #print "makeUniqueSteps()"     # DEBUG
      
      if lines==None:
         lines=self.lines
      
      #print "self.timedSteps = ", self.timedSteps     # DEBUG
      lineNumber=0                  # count lines
      counts={}                     # dictionary to hold count for each timer
      prevstep=""                   # compare with previous step
      prevsubstep=""
      prevcount=0
      for line in lines:      
         line=line.upper()
         fields=line.split()
         lineNumber+=1              # need line number for first line and as reference?
         
         step=fields[STEP_FIELD]
         if step != prevstep:
            prevstep=step
            if step in counts:
               counts[step]+=1
            else:
               counts[step]=1

         prevsubstep=fields[SUBSTEP_FIELD]
         stepCount=step+"_"+str(counts[step])
         lines[lineNumber-1]=line.replace(step, stepCount, 1)
      
      # DEBUG
      #print "modified line: "
      #for line in lines:
      #   print line
      self.modifiedLines=lines
      return lines
      

   # Select substeps ( optional only a particular step from the Timing dictionary)
   # optional keyword: total, count or avg (default=total)
   #
   def getSubsteps(self, stepName=None, keyword="total"):
      subSteps={}          # list of all substeps belonging to this step
      searchSteps=[]             # local list of steps, set depending if looking for numbered or grouped steps
      
      # we have to distinguish if we are looking for substeps for all grouped steps
      # or for individual (numbered) steps
      if stepName.find("_") !=-1:
         searchSteps=self.timedStepsCount
      else:
         searchSteps=self.timedSteps

      #print "stepName = ", stepName
      #print "searchSteps = ", searchSteps
        
      if stepName not in searchSteps:
         print "getSubsteps() ", stepName, "not found in dictionary ", searchSteps, "."
      else:
         #print "getSubsteps() stepName = ", stepName     # DEBUG
      
         for line in self.lines:
            fields=line.split()
            step=fields[STEP_FIELD]
            subStepname=fields[SUBSTEP_FIELD]
            
            #print "getSubSteps() subStepname = ", subStepname # DEBUG
            if searchSteps==self.timedSteps:
               pos=step.find("_")
               step=step[0:pos]
            
            value=self.getValue(line, keyword)  # we also want to store the corresponding values in the dictionary                     
            if stepName == None:                # if no particular stepname was supplied
               subSteps[subStepname]=value      # collect all substeps
            else:
               if stepName == step:             # If we are looking for a particular step
                  subSteps[subStepname]=value   

      return subSteps


   # Extract processed subbands from pipeline.log
   #
   def getSubbands(self):
      print "getSubbands()"   # DEBUG

      if self.logtype!="pipeline":
         print "BBStiming.py: ", self.filename, " is not a pipeline.log"
         self.subbands=[]


   # Determine if we have numbered steps or just steps,
   # depending on the step provided
   #
   def determineNumberedSteps(self, step):
      # we have to distinguish if we are looking for substeps for all grouped steps
      # or for individual (numbered) steps
      if step.find("_") !=-1:
         searchSteps=self.timedStepsCount
      else:
         searchSteps=self.timedSteps

      return searchSteps


   # Get final times for a particular step
   # TOTAL, COUNT or AVG (default="TOTAL") 
   #
   def getStepFinal(self, stepname, keyword="TOTAL"):
      #print "getStep()"                # DEBUG

      stepname=stepname.upper()
      keyword=keyword.upper()
      value=0
      
      searchSteps=self.determineNumberedSteps(stepname)
      
      #print "searchSteps = ", searchSteps   # DEBUG
      
      # Check if stepname is in steps
      if stepname not in searchSteps:
         print "getStep() " + stepname + " is not in self.timedSteps"
         return False
      else:
         for line in self.modifiedLines:          # find stepname in lines with matching keyword
            line=line.upper()                     # convert to lower case

            if line.find(stepname)!=-1 and line.find("ALL")!=-1:     # if we found the stepname on this line
               if line.find(keyword):                                # and the keyword, too
                  value=self.getValue(line, keyword)
                  return value
               else:
                  continue
         #print "getFinalStep() value = ", value      # DEBUG
      return value
         
      
   # Get the corresponding value for a keyword in a timer line
   # e.g. AVG, TOTAL, COUNT (default=TOTAL)
   # 
   def getValue(self, line, keyword="TOTAL"):
      line=line.upper()
      keyword=keyword.upper()

      #print "getValue() line = ", line          # DEBUG
      #print "getValue() keyword = ", keyword    # DEBUG

      pos=line.find(keyword)
      if pos == -1:
         print "getValue() " + keyword + " not found."
         return False
      else:
         fields=line.split()
         for i in range(0, len(fields)):
            if fields[i]==keyword:
               value=float(fields[i+1])
            else:
               i+=1
      return value 


   # Get the value for a keyword for all steps and substeps of the
   # same name
   #
   # step      - step name to search for
   # substep   - substep name to search for (default="", i.e. any)
   #
   def getSubStepValue(self, step, substep="", keyword="TOTAL"):
      #print "getSubStepValue()"     # DEBUG
      #print "step = ", step         # DEBUG
      #print "substep = ", substep   # DEBUG

      values=[]                     # list of found values
      step=step.upper()
      substep=substep.upper()
      if step not in self.timedSteps:
         print "getSubStepValue() ", step , "not in", self.timedSteps
         return -1
      if len(self.modifiedLines)==0:
         print "getSubStepValue() no UPPERCASE lines unified"
         return -1

      for line in self.modifiedLines:
         line=line.upper()
         # we want to find both the step and substep pattern
         if line.find(step)!=-1 and line.find(substep)!=-1 and line.find("ALL")==-1:  # ONLY substeps!
            value=self.getValue(line, keyword)
            values.append(value)

      return values


   #***************************************
   #
   # Higher level analysis functions
   #
   #***************************************

   # Get the sum of all expression timer occurences for this step
   # and keyword (default="TOTAL")
   #
   def getSummedValueOfStep(self, step, keyword="TOTAL"):
      print "getSummedValueOfStep()"      # DEBUG

      sum=0                  # depending on the keyword this can be the summed avg, counts or total
      for line in self.lines:       
         if line.find(step) != -1 and line.find("ALL") == -1:     # we don't want to include the totals
            if line.find(keyword) != -1:
               value=getValue(keyword)
               sum+=value

      return sum
   
   
   # Find an expression in a list, i.e. in this case
   # this can be a step or substep list
   #
   def findPattern(self, pattern, list):
      print "findExpression()"     # DEBUG
   
      results=[]
      for l in list:
         if l.find(pattern) != -1:
            results.append(l)
      return results
            
   
   # Export raw lines to a file
   # if no filename is given, a default filename with timestamp is generated
   #
   def exportLines(self, filename=None, modified=True):
      if filename==None:                                        # if no filename is provided
         filename="BBS_Timer" + str(datetime.datetime.now())    # ...generate timestamp filename
   
      try:
         outfile_fh=open(filename, "wa")
      except IOError, err:
         print str(err)

      if modified==False:
         lines=self.lines
      else:
         lines=self.modifiedLines
         
      for line in lines:
         outfile_fw.write(line)
      
      outfile_fh.close()

      

# Class that pops up a plot window instance with its own plot
#
class PlotWindow(QFrame):

   # Init the class: create plot window and canvas layout (and corresponding buttons)
   # Steps contains a dictionary of dictionaries with the identified steps
   #
   def __init__(self, parent):
      #QFrame.__init__(self)
      QMainWindow.__init__(self, None)
      self.setWindowTitle('BBS timing statistics')
      
      self.parent=parent                 # Parent: BBSTiming class
      self.currentPlotStyle="bar"        # current style of plot: bar, colorbar, lines 
      self.plotStyles=["bar", "colorbar", "line"]     # supported plot styles                 #
      self.summation=False               # show "individual" execution times or "summation"
      
      self.fig=None                      # figure
      self.axes=None                     # plot axes
      
      self.create_main_frame()
      self.createWidgets()
      self.createLayouts()
      self.createConnections()

      self.setMinimumWidth(700)
      self.setMinimumHeight(400)
      

   # Create a main frame
   #
   def create_main_frame(self):
      self.main_frame = QWidget()
      self.dpi = 75
      self.setMinimumWidth(300)
      self.setMinimumHeight(300)

      # We want matplotlib export functionality, so include toolbar
      # MPL Canvas and Toolbar
      # Create canvas for plotting
      self.fig = Figure((5, 4), dpi=75)
      self.canvas = FigureCanvas(self.fig)
      self.canvas.setParent(self)
      self.fig.subplots_adjust(left=0.1, right=0.96, top=0.94, bottom=0.06)  # set a small rim

      self.mpl_toolbar = NavigationToolbar(self.canvas, self)
      self.mpl_toolbar.show()   # first hide the toolbar
   
   
   # Create the layouts for the widgets
   #
   def createLayouts(self):
      # Layouts
      self.buttonLayout=QVBoxLayout()
      self.canvasLayout=QVBoxLayout()
      self.mainLayout=QHBoxLayout()
      
      # Add Widgets to the layout
      self.buttonLayout.addWidget(self.loadButton)
      self.buttonLayout.addWidget(self.quitButton)
      self.buttonLayout.insertStretch(-1)


      self.subStepsLayout=QHBoxLayout()
      self.subStepsLayout.addWidget(self.showSubstepsCheckBox)
      self.subStepsLayout.addWidget(self.showSubstepsLabel)
      self.buttonLayout.addLayout(self.subStepsLayout)
      self.showIndividualStepsLayout=QHBoxLayout()
      self.showIndividualStepsLayout.addWidget(self.showIndividualCheckBox)
      self.showIndividualStepsLayout.addWidget(self.showIndividualLabel)
      self.buttonLayout.addLayout(self.showIndividualStepsLayout)

      self.buttonLayout.addWidget(self.stepComboBox)
      #self.buttonLayout.addWidget(self.stepListView)      
      self.buttonLayout.addWidget(self.substepComboBox)
      self.buttonLayout.addWidget(self.keywordComboBox)
      self.buttonLayout.addWidget(self.plotStyleComboBox)
      self.buttonLayout.addWidget(self.stepComboBox)
      self.buttonLayout.addWidget(self.plotButton)
      self.buttonLayout.insertStretch(-1)

      
      self.canvasLayout.addWidget(self.canvas)
      self.canvasLayout.addWidget(self.mpl_toolbar)

      self.mainLayout.addLayout(self.buttonLayout)
      self.mainLayout.addLayout(self.canvasLayout)
      self.setLayout(self.mainLayout)


   # Create GUI widgets
   #
   def createWidgets(self):
      #print "createWidgets()"             # DEBUG

      self.loadButton=QPushButton("Load logfile")
      self.loadButton.setToolTip('load a BBS kernellog file')
      self.loadButton.setMaximumWidth(200)
      self.quitButton=QPushButton("Quit")
      self.quitButton.setMaximumWidth(200)
      self.quitButton.setToolTip('Quit the application')
      self.quitButton.setMaximumWidth(200)
      self.plotButton=QPushButton("Plot")
      self.plotButton.setToolTip('force a replot')
      
      self.showSubstepsCheckBox=QCheckBox()
      self.showSubstepsLabel=QLabel("Show substeps")
      self.showSubstepsCheckBox.setToolTip("Show also substeps in timing")
      self.showSubstepsCheckBox.setCheckState(Qt.Unchecked)       # Default: False
      self.showSubstepsCheckBox.show()
      self.showSubstepsLabel.show()

      self.showIndividualCheckBox=QCheckBox()
      self.showIndividualLabel=QLabel("Numbered steps")
      self.showIndividualCheckBox.setToolTip("Show individually numbered steps")
      self.showIndividualCheckBox.setCheckState(Qt.Unchecked)       # Default: False
      self.showIndividualCheckBox.show()
      self.showIndividualLabel.show()

      # GUI Widgets depending on log file
      self.createStepComboBox()           # Selector for Step    
      self.createKeywordComboBox()        # Selector for: total, count, avg

      # Selector for plot type: bar, line, multibar
      self.createPlotstyleComboBox()
      self.createStepComboBox()
      #self.createStepListView()
# This is now done in a function, too
#      self.substepComboBox=QComboBox()    # we create this here once
#      self.substepComboBox.hide()
#      self.substepComboBox.setToolTip('Substeps of step')
#      self.substepComboBox.setMaximumWidth(200)

      self.createSubbandComboBox()
    
    
   # Create the step combobox containing the timed steps
   # This should be a multi-selection box to plot multiple
   # steps
   #
   def createStepComboBox(self):
      #print "createStepComboBox()"     # DEBUG
      
      self.stepComboBox=QComboBox()      
      for step in self.parent.timedSteps:
         self.stepComboBox.addItem(step)  
   
      self.stepComboBox.addItem("all")
      self.stepComboBox.show()
      self.stepComboBox.setMaximumWidth(200)


   def createSubstepComboBox(self):
      print "createSubstepComboBox()"     # DEBUG

      self.substepComboBox=QComboBox()    # we create this here once
      self.substepComboBox.hide()
      self.substepComboBox.setToolTip('Substeps of step')
      self.substepComboBox.setMaximumWidth(200)


   # Fill step Checkbox with step names, either these are
   # grouped by their common name, or shown as individually
   # numbered steps
   #
   def fillSteps(self):
      print "fillSteps()"           # DEBUG
   
      self.stepComboBox.clear()
      # Decide if we want the steps grouped or individually numbered
      if self.showIndividualCheckBox.isChecked()==True:      
         steps=self.parent.timedStepsCount
      else:
         steps=self.parent.timedSteps
         
      #print "fillSteps() steps = ", steps
      for step in steps:
         # Check if that substep is already in there, avoid duplicates
         if self.stepComboBox.findData(step) == -1:
            self.stepComboBox.addItem(step)         
      self.stepComboBox.addItem("all")          # we again have to add our "all" option
      

   # Get the corresponding substeps for a step
   #
   def fillSubsteps(self):
      print "fillSubsteps()"         # DEBUG

      # Get substeps for currently selected step (or all if "all")
      step=str(self.stepComboBox.currentText())
      
      self.substepComboBox.clear()           # first clear substeps comboBox
      if step != "all" and step!="ALL":
         substeps=self.parent.getSubsteps(step)
         
         #print "fillSubsteps() substeps = ", substeps    # DEBUG
         for substep in substeps:
            # Check if that substep is already in there, avoid duplicates
            if self.substepComboBox.findData(substep) == -1:
               self.substepComboBox.addItem(substep)
      else:
         self.substepComboBox.clear()                 # First remove existing substeps
         
         index=0
         while index < self.stepComboBox.count():  # loop over all steps        
            # TODO: Might not be supported to show substeps for ALL steps...
            #substeps=str(self.stepComboBox.itemText(index))
            #print "fillSubsteps() substeps = ", substeps
            
            #print "fillSubsteps() index = ", index, "maxCount = ", self.stepComboBox.count()
            step=str(self.stepComboBox.itemText(index))
            print "fillSubsteps() step = ", step
            if step != "all" and step != "ALL":
               substeps=self.parent.getSubsteps(step)
            
               keys=substeps.keys()
               #print "keys = ", keys
               for j in range(len(substeps)-1):
                  #print "fillSubsteps() substeps = ", substeps
                  substep=keys[j]
                  if substep != "all" and substep != "ALL":
                     self.substepComboBox.addItem(substep)
                  j=j+1
            
            index=index+1
   
         self.substepComboBox.addItem("ALL")       # we need to add an "ALL"
      
      
   """
   # Alternative view, displaying all the steps
   # in a list view that then can be selected for
   # plotting
   #
   def createStepListView(self):
      print "createStepListBox()"      # DEBUG
      
      self.stepListView=QListView()
      for step in self.parent.timedSteps:
         self.stepListView.insertStringList(step)
      self.stepListView.show()
      step.stepListView.setMaximumWidth(200)
   """

      
   # On timing combobox event
   #
   def createKeywordComboBox(self):   
      print "createTimingComboBox()"   # DEBUG
      
      self.keywordComboBox=QComboBox()
      for key in self.parent.keywords:
         self.keywordComboBox.addItem(key)
      
      self.keywordComboBox.show()
      self.keywordComboBox.setMaximumWidth(200)
      

   # Create a comboBox offering different Matplotlib styles
   #
   def createPlotstyleComboBox(self):
      print "createPlotstyleComboBox()"     # DEBUG
      
      self.plotStyleComboBox=QComboBox()
      for style in self.plotStyles:
         self.plotStyleComboBox.addItem(style)
      
      self.plotStyleComboBox.show()
      self.plotStyleComboBox.setMaximumWidth(200)

   
   # Create subbands comboBox which allows selection of an individual subband
   # from a pipeline.log
   def createSubbandComboBox(self):
      print "createSubbandComboBox()"   # DEBUG

      self.subbandComboBox=QComboBox()
      for sub in self.parent.subbands:
         self.subbandComboBox.addItem(sub)

      self.subbandComboBox.show()
      self.subbandComboBox.setMaximumWidth(200)


   #**************************************
   #
   # Create connections
   #
   #**************************************

   def createConnections(self):
      print "createConnections()"      # DEBUG

      self.connect(self.loadButton, SIGNAL('clicked()'), self.on_loadfile)   
      self.connect(self.quitButton, SIGNAL('clicked()'), self, SLOT('close()')) 

      self.connect(self.stepComboBox, SIGNAL('currentIndexChanged(int)'), self.on_step)
      self.connect(self.stepComboBox, SIGNAL('currentIndexChanged(int)'), self.on_showSubsteps)
      #self.connect(self.substepComboBox, SIGNAL('currentIndexChanged(int)'), self.on_step)
      self.connect(self.substepComboBox, SIGNAL('currentIndexChanged(int)'), self.on_plot)      
      self.connect(self.keywordComboBox, SIGNAL('currentIndexChanged(int)'), self.on_keyword)
      self.connect(self.plotStyleComboBox, SIGNAL('currentIndexChanged(int)'), self.on_plotStyle)
      self.connect(self.showSubstepsCheckBox, SIGNAL('stateChanged(int)'), self.on_showSubsteps)
      self.connect(self.showIndividualCheckBox, SIGNAL('stateChanged(int)'), self.on_individualSteps)
      self.connect(self.subbandComboBox, SIGNAL('currentIndexChanged(int)'), self.on_subband)

      self.connect(self.plotButton, SIGNAL('clicked()'), self.on_plot)


   #**************************************
   #
   # Event handlers
   #
   #**************************************

   # Load Kernellog file dialog
   #
   def on_loadfile(self):
      #print "on_loadfile()"    # DEBUG
      
      # Customization: check if ~/Cluster/SolutionTests exists
      if os.path.exists('/Users/duscha/Desktop/'):
         setDir=QString('/Users/duscha/Desktop/')
      else:
         setDir=QString('')

      path = unicode(QFileDialog.getOpenFileName(self, 'Load Kernellog', setDir))
      path=str(path)  # Convert to string so that it can be used by load table

      if path:
         self.parent.readLogfile(path)
      else:
         print "load_table: invalid path"


   # On step combobox event
   #      
   def on_step(self):
      print "on_step()"                # DEBUG
      #step=str(self.stepComboBox.currentText())
      self.on_plot()


   # On substep combobox event
   #
   def on_substep(self):
      print "on_substep()"             # DEBUG    
      #step=str(self.stepComboBox.currentText())
      self.on_plot()


   # On toggle individual treatment of counted steps
   #
   def on_individualSteps(self):
      print "on_individualSteps()"     # DEBUG
      self.fillSteps()                 # update steps combobox
      self.on_showSubsteps()           # also update substeps combobox
      #step=str(self.stepComboBox.currentText())
      self.on_plot()

   # On selection of different subbands
   #
   def on_subband(self):
      print "on_subband()"             # DEBUG
      self.fillSteps()                 # update steps combobox
      self.on_showSubsteps()           # also update substeps combobox
      self.on_plot()

   # On toggling of show substeps
   #
   def on_showSubsteps(self):
      print "on_substeps()"            # DEBUG
      
      if self.showSubstepsCheckBox.isChecked()==True:
         self.showSubSteps=True
         self.substepComboBox.show()
         self.fillSubsteps()
      else:
         self.showSubSteps=False
         self.substepComboBox.hide()
         self.substepComboBox.clear()      
   
   # On keyword combobox event
   #
   def on_keyword(self):
      print "on_keyword()"             # DEBUG
      self.on_plot()
      

   # On change of plot style
   #
   def on_plotStyle(self):
      print "on_plotStyle()"           # DEBUG
      
      self.plotStyle=self.plotStyleComboBox.currentText()      # change class attribute
 
 
   # On plot button action
   #
   def on_plot(self):
      self.fig.clf()       # clear the figure
      
      print "on_plot()"
      
      step=str(self.stepComboBox.currentText())
      self.plot(step)
      """
      step=str(self.stepComboBox.currentText())
      # If we have all steps selected:
      if step=="ALL" or step=="all":
         for i in range(0, self.stepComboBox.count()):         # loop over all steps
            currentStep=self.stepComboBox.itemText(i)          # get the step name from QComboBox
            
            print "on_plot() currentStep = ", currentStep
            
            self.plot(currentStep)                             # plot it
      else:
         #print "foo"
         self.plot(step)                                       # replot with new plotstyle
      """

   # Replot diagram 
   #
   def plot(self, step):
      print "plot()"                    # DEBUG

      width=0.25            # width of bar plots
      
      # Get the data according to GUI settings     
      step=str(step)
      substep=str(self.substepComboBox.currentText())
      keyword=str(self.keywordComboBox.currentText())
      style=str(self.plotStyleComboBox.currentText())
      
      print "plot() step = ", step, "substep = ", substep, "keyword = ", keyword, "style = ", style      # DEBUG

      self.axes=self.fig.add_subplot(111)          # add 1 subplot to the canvas
      result=[]
      
      # TODO: plot all substeps if we get multiple in results
      if step=="all" or step=="ALL":
         if self.showIndividualCheckBox.isChecked():
            steps=self.parent.timedStepsCount
         else:
            steps=self.parent.timedSteps
         
         print "plot() steps = ", steps      # DEBUG        
         for i in range(0, len(steps)):
            #print "plot()", self.parent.getSubStepValue(str(steps[i]), substep, keyword)
            result.append(self.parent.getStepFinal(str(steps[i]), keyword))
            
            #print "len(result) = ", len(result)
            print "plot() result[" + str(i) + "] = " + str(result[i])
            #self.axes.bar(i, result[i], width)

      
      # If we want to see all steps, but not for individual substeps
      elif step=="all" or step=="ALL" and (substep==None or substep==""):      
         if self.showIndividualCheckBox.isChecked():
            steps=self.parent.getSubsteps(step, keyword)
         else:
            steps=self.parent.timedSteps
            print "steps = ", steps
            #steps.append(self.parent.timedSteps)
            newsteps=[]
            if isinstance(steps, list):
               for i in range(0, len(steps)):   # overplot them in one plot
                  newsteps.append(steps[i])
            
            result=newsteps
      elif substep=="all" or substep=="ALL":
          print "plot(): substep=all"
          result=(self.parent.getSubStepValue(step, keyword))
      elif substep==None or substep=="":
         result=(self.parent.getStepFinal(step, keyword))
      else:
         result=(self.parent.getSubStepValue(step, substep, keyword))
      
      
      #if isinstance(result[0], list):
      #   result=self.linearizeList(result)
      
      if isinstance(result, list):
         print "plot() len(result) = ", len(result)        # DEBUG
         print "plot() result = ", result                  # DEBUG
         #print "plot() result[1] = ", result[1]           # DEBUG

      #
      # Plot on canvas
      #
      ind=[]
      if isinstance(result, float) or isinstance(result, int):
         ind=0
      elif isinstance(result, bool):
         print "plot() invalid result returned"
      else:
         maxInd=len(result)
         for i in range(0, maxInd):
            ind.append(width*1.1*i)      
      
      # Decide on plotstyle which plotting to do
      if self.currentPlotStyle=="bar":
         print "ind = ", ind           # DEBUG
         print "result = ", result     # DEBUG
         rects1 = self.axes.bar(ind, result, width, color='r')
      
      elif self.currentPlotStyle=="colorbar":
         print "plot() colorbar"
      else:
         print "plot() lines"
         self.axes.scatter(0, result)

      #
      # Set axes labels according to selected keyword on y-axis and
      # for each plotted step/substep identifier on the x-axis
      #
      ylabel=str(self.keywordComboBox.currentText())
      if self.keywordComboBox.currentText() == "total" or self.keywordComboBox.currentText() == "avg":
         ylabel=ylabel + " s"
      self.axes.set_ylabel(ylabel)
      
      self.canvas.draw()


   #******************************************************
   #
   # Helper functions
   #
   #*******************************************************   

   def linearizeList(self, reorderlist):
      print "linearizeList()"          # DEBUG
   
      newlist=[]
      
      if isinstance(reorderlist, list):
         Nlists=len(reorderlist)
         for i in range(Nlists):
            if isinstance(reorderlist[i], list):
               for j in len(reorderlist[i]):
                  newlist.append(reorderlist[i][j])
            else:
                  newlist.append(reorderlist[i])
      return newlist
      

   #******************************************************
   #
   # Update GUI Widgets on loading a new Kernellog file
   #
   #*******************************************************

   # Update the Kernel log dependent widgets, i.e. deleting
   # and recreating them
   #   
   def updateWidgets(self):
      print "updateWidgets()"       # DEBUG

      self.deleteWidgets()
      self.createWidgets()   

       
   # Delete GUI Widgets that are created dynamically from the log
   #
   def deleteWidgets(self):         # DEBUG
      print "deleteWidgets()"
      
      self.stepComboBox.deleteLater()
      self.keywordComboBox.deleteLater()
      self.plotstyleComboBox.deleteLater()
      self.stepComboBox.deleteLater()
 


#****************************************
#
# Script usage function
#
#****************************************


# Display usage help info (is not part of class)
#
def usage():
   print "Usage: ", sys.argv[0], "<options> <filename>"
   print "" 
   print "-o --output      output to ASCII text"
   print "-p --parameter   display info for only specific parameter"
   print "-v --verbose     activate verbose output"
   print "-h --help        display this help information"


#****************************************
#
# Main function
#
#****************************************

def main():
   timing=BBSTiming()        # create BBSTiming object

   # First we try to import matplotlib and its qt4 backends
   for module in ["matplotlib", "matplotlib.backends.backend_qt4agg"]:
      if timing.importModule(module) == False:
         timing.havePlotting=False
      else:
         timing.havePlotting=True

   if len(sys.argv) < 2:
      usage()
      sys.exit(2)


   # Parse command line arguments and on exception display usage message
   filename=""
   parameter=""
   output=""
   try:
      opts, args = getopt.getopt(sys.argv[1:], "ho:p:v", ["help", "output", "parameter", "verbose"])
   except getopt.GetoptError, err:
      print str(err)
      usage()
      sys.exit(2)
   output = None
   verbose = False
   for opt, arg in opts:
      if opt == "-v":
         verbose = True
      elif opt in ("-h", "--help"):
         usage()
         sys.exit()
      elif opt in ("-o", "--output"):         # output to ASCII text file
         output = arg
      elif opt in ("-p", "--parameter"):      # display timing only for this parameter
         parameter = arg
      elif opt in ("-v", "--verbose"):        # display timing only for this parameter
         verbose = True      
      else:
         assert False, "unhandled option"   
   
   filename=sys.argv[len(sys.argv)-1]       # filename must be last argument
   
   # Analyse BBS timer log
   timing.readLogfile(filename)
   timing.identifySteps()
   timing.makeUniqueSteps()

   #for step in timing.timedSteps:         # DEBUG
   #   if step!="all" or step!="ALL":
   #      timing.getSubsteps(step, "TOTAL")
   
   # Test get final values

   #for step in timing.timedStepsCount:
   #   print "step = ", step, " total = ", timing.getStepFinal(step, "total")

   # Test get substep values
   #values=timing.getSubStepValue("VISEQUATOR", "", "avg")   
   #print "values = ", values   # DEBUG


   #*******************************
   # QT GUI
   #
   app = QApplication(sys.argv)
   plotWindow=PlotWindow(timing)
   plotWindow.show()
   app.exec_()   
   

#****************************************
#
# Main entry
#
#****************************************
if __name__ == "__main__":
   main()
