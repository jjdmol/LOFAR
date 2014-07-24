#!/usr/bin/env python
#
# Solver statistics preferences dialog
#
# File:           plotpexport.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2011-12-20
# Last change;    2012-01-10
#
# This class displays an export dialog for the solver statistics plotter
# to export data in ASCII or Matlab format (using scipy.savemat)
# TODO


from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os
import sys
import numpy as np



class plotexport(QDialog):
    def __init__(self, parent):
      QDialog.__init__(self)
      
      self.parent=parent
      #self.perIteration=self.parent.parent.perIteration
      #self.useScipy=self.parent.parent.useScipy
      
      self.path=""                  # path to export data to
      self.format="ASCII"           # format to export to (ASCII or Matlab, default=ASCII)
      self.parmList=[]              # List of parameters (parmdb and solver) to export
      
      # Interval to export data over
      self.xIntervalMin=None
      self.xIntervalMax=None

      self.createWidgets()          # Create Widgets
      self.createLayouts()          # Create Layouts
      self.createConnections()      # create connections between widgets
      
    def createWidgets(self):
      print "createWidgets()"             # DEBUG
      
      # Group boxes to group widgets into logical units
      self.parmGroup=QGroupBox()
      self.parmGroup.setTitle("Parameters to export")
      self.parmGroup.setToolTip("select parameters to export")
      self.formatGroup=QGroupBox()
      self.formatGroup.setTitle("Format settings")
      self.formatGroup.setToolTip("format settings")
      
      self.parmComboBox=QComboBox()
      self.parmComboBox.setMaxVisibleItems(4)
      self.solverComboBox=QComboBox()
      self.solverComboBox.setMaxVisibleItems(4)
      
      
      self.fileDialog=QFileDialog()
      self.fileDialog.setFileMode(QFileDialog.AnyFile);
      urls=[]
      urls.append(QUrl.fromLocalFile(os.environ['HOME']))
      if os.path.exists('/data/scratch/' + os.environ['USER']):
        urls.append(QUrl.fromLocalFile('/data/scratch/' + os.environ['USER']))
      self.fileDialog.setSidebarUrls(urls);
      self.saveButton=QPushButton("Save as")
      self.saveButton.setMaximumWidth(120)
      self.filenameLineEdit=QLineEdit()
      self.filenameLineEdit.setReadOnly(True)
      self.filenameLineEdit.setText(os.environ['HOME'])
      self.filenameLineEdit.setText(self.path)


      self.formatComboBox=QComboBox()
      self.formatComboBox.addItem("ASCII")
#      if self.parent.parent.haveModule('scipy') == True or self.parent.parent.haveModule('scipy.io') == True:
#         self.exportComboBox.addItem("Matlab")      

      self.rangeComboBox=QComboBox()
      self.rangeComboBox.addItem("plotted x_min to x_max")
      self.rangeComboBox.addItem("complete selection")
      self.rangeComboBox.addItem("complete set")
      
      self.exportButton=QPushButton("Export")
      self.exportButton.setToolTip("export selected data")
      self.cancelButton=QPushButton("Cancel")
      self.cancelButton.setToolTip("Cancel export and close dialog")

      # Window settings
      self.setMinimumWidth(400)
      self.setMinimumHeight(400)
      self.setMaximumWidth(400)
      self.setMaximumHeight(400)
      self.setWindowTitle("Export Data")
  
      self.updateComboBoxes()
  
    def createConnections(self):
      print "createConnections()"      # DEBUG

      self.connect(self.saveButton, SIGNAL('clicked()'), self.saveAs)
      self.connect(self.exportButton, SIGNAL('clicked()'), self.exportData)
      self.connect(self.cancelButton, SIGNAL('clicked()'), SLOT('close()'))
      
    def createLayouts(self):
      print "createLayouts()"               # DEBUG
      
      self.parametersLayout=QVBoxLayout()
      self.parmGroup.setLayout(self.parametersLayout)

      self.fileNameLayout=QHBoxLayout()
      self.fileNameLayout.addWidget(self.saveButton)
      self.fileNameLayout.addWidget(self.filenameLineEdit)

      self.formatLayout=QVBoxLayout()
      self.formatLayout.addWidget(self.rangeComboBox)
      self.formatLayout.addWidget(self.formatComboBox)
      self.formatGroup.setLayout(self.formatLayout)
      
      self.buttonLayout=QHBoxLayout()
      self.buttonLayout.addWidget(self.exportButton)
      self.buttonLayout.addWidget(self.cancelButton)

      # Combine all layouts into mainLayout
      self.mainLayout=QVBoxLayout()
      self.mainLayout.addWidget(self.parmGroup)
      self.mainLayout.addWidget(self.formatGroup)
      self.mainLayout.addLayout(self.fileNameLayout)
      self.mainLayout.addLayout(self.buttonLayout)      
      self.setLayout(self.mainLayout)          
    
    def updateParmComboBox(self):
      print "updateComboBox()"              # DEBUG
      self.deleteEntriesComboBox()          # Delete entries in parmComboBox
      # Read entries from solverQuery table
      #for i in range(0, self.parent.parametersComboBox.count()):
      #  self.parmComboBox.addItem(self.parent.parametersComboBox.itemText(i))               
 
    def updateComboBoxes(self):
      print "updateFormatComboBox()"        # DEBUG
      self.deleteEntriesComboBox()

      # loop over parms and add them to parmComboBox
      self.formatComboBox.addItem("ASCII")
#      if self.parent.parent.useScipy==True:      # Check availability of scipy
#        self.formatComboBox.addItem("Matlab")
      
      self.addEntriesParmComboBox()
      self.addEntriesSolverComboBox()
      
    def deleteEntriesComboBox(self):
      print "deleteEntriesParmComboBox()"   # DEBUG
      
      # Loop over elements in parmComboBox with parmDB parameters
      i=self.parmComboBox.count()
      while i > 0:     # Delete entries in parmComboBox
        print "removing i = ", i, parmComboBox.currentText()          # DEBUG
        self.parmComboBox.removeItem(i-1)
        i=self.parmComboBox.count()

      i=self.formatComboBox.count()
      while i > 0:   # Delete entries in formatComboBox
        print "removing i = ", i, self.formatComboBox.currentText()   # DEBUG
        self.formatComboBox.removeItem(i-1)
        i=self.formatComboBox.count()

      i=self.solverComboBox.count()
      while i > 0:   # Delete entries in solverComboBox
        print "removing i = ", i, self.solverComboBox.currentText()   # DEBUG
        self.solverComboBox.removeItem(i-1)
        i=self.solverComboBox.count()
    
    def addEntriesParmComboBox(self):
      print "addEntriesParmComboBox()"      # DEBUG

    def addEntriesSolverComboBox(self):
      print "addEntriesSolverComboBox()"    # DEBUG
      
    def saveAs(self):
      print "saveAs()"                      # DEBUG
      self.path = unicode(self.fileDialog.getSaveFileName(self, 'Save file'))
      self.filenameLineEdit.setText(self.path)

    
    def exportData(self):
      print "exportData()"                  # DEBUG
      
    
    #****************************************
    #
    # Helper functions
    #
    #**************************************** 
      
      
# Main function used for debugging
#
def main():
  print "main()"        # DEBUG

  app = QApplication(sys.argv)
  form=plotexport(app)
  
  form.show()
  app.exec_()
      
      
if __name__=="__main__":
    main()
    