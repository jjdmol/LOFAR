#!/opt/local/bin/python

# Solver statistics dialog
#
# File:			solverDialog.py
# Author:		Sven Duscha (duscha@astron.nl)
# Date:			2010-08-05
# Last change;		2010-08-19
#
#

# Import
import sys, os, random
import SolverQuery as sq

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import unicodedata
import matplotlib
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure


class SolverAppForm(QMainWindow):

    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)
        self.setWindowTitle('Solver statistics')

        self.solverQuery = sq.SolverQuery()       # attribute to hold SolverQuery object
        self.create_main_frame()
        self.create_status_bar()

        self.table=False            # attribute to check if we have an open table


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
        if self.table is True:
            self.close_table()

        # Customization: check if ~/Cluster/SolutionTests exists
        if os.path.exists('/Users/duscha/Cluster/SolutionTests'):
            setDir=QString('/Users/duscha/Cluster/SolutionTests')
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
            self.table=True

            self.create_table_widgets()


    # Open a table (on startup, if a command line argument is given)
    #
    def open_table(self, tableName):
        # If a table is already loaded, close it:
        if self.table is True:
            print "closing table..."
            self.close_table()

        # Check if path exists and it is a director (i.e. a table)
        if os.path.exists(tableName) is False:
            raise IOError
        else:
            if os.path.isdir(tableName) is False:
                raise IOError

        # Open it and set object attributes
        self.solverQuery=self.solverQuery.open(tableName)

        self.create_table_widgets()
        self.table=True


    # Close the table that is currently loaded
    #
    def close_table(self):
        # Remove table specific widgets
        self.buttonsLayout.removeWidget(self.timeStartSlider)
        self.buttonsLayout.removeWidget(self.timeEndSlider)
        self.buttonsLayout.removeWidget(self.timeStartSliderLabel)
        self.buttonsLayout.removeWidget(self.timeEndSliderLabel)
        self.buttonsLayout.removeWidget(self.frequencySlider)
        self.buttonsLayout.removeWidget(self.frequencySliderLabel)
        self.buttonsLayout.removeWidget(self.parametersComboBox)

        self.buttonsLayout.update()
        self.mainLayout.update()
        self.solverQuery.close()
        self.table=False       # we don't have an open table anymore


    # Function to handle table index slider has changed
    #
    def on_timeStartSlider(self, index):
        # Read time at index
        starttime=self.solverQuery.timeSlots[index]['STARTTIME']

        self.timeStartSliderLabel.setText("S:" +  str(starttime) + " s")

        # Handle behaviour of timeEndSlider in combination with timeStartSlider
        # If timeEndSlider is smaller than timeStartSlider adjust the latter
        if self.timeEndSlider.sliderPosition() < self.timeStartSlider.sliderPosition():
            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())
            #self.timeEndSlider.setSliderPosition(self.timeStartSlider.sliderPosition())
            #self.timeEndSlider.emit(SIGNAL('valueChanged()'))

        self.repaint()

    # Function to handle table index slider has changed
    #
    def on_timeEndSlider(self, index):
        # Read time at index
        endtime=self.solverQuery.timeSlots[index]['ENDTIME']
        self.timeEndSliderLabel.setText("E:" + str(endtime) + " s")

        # Handle behaviour of timeEndSlider in combination with timeStartSlider
        # If timeEndSlider is smaller than timeStartSlider adjust the latter
        if self.timeEndSlider.sliderPosition() < self.timeStartSlider.sliderPosition():
            self.timeStartSlider.setValue(self.timeEndSlider.sliderPosition())


    # If only a single cell is to be shown, adjust timeEndSlider
    # to the position of timeStartSlider, and lock them
    def on_singleCell(self):
        #print "on_singleCell(self)"   # DEBUG

        if self.singleCellCheckBox.isChecked():
            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())
            self.syncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))
        else:
            self.unsyncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))
    

    # Put sliders into sync, so that moving one also moves the other
    #
    def syncSliders(self):
        #print "syncSliders(self)"   # DEBUG
        self.connect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.timeEndSlider, SLOT('setValue(int)'))         
        self.connect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.timeStartSlider, SLOT('setValue(int)'))


    # Unsync sliders, so that they can move independently again
    #
    def unsyncSliders(self):
        #print "unsyncSliders(self)"  # DEBUG
        self.disconnect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.timeEndSlider, SLOT('setValue(int)'))         
        self.disconnect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.timeEndSlider, SLOT('setValue(int)'))


    # Function to handle frequency index slider has changed
    #
    def on_frequencySlider(self, index):
        print "on_frequencySlider(self, index):"
        # Read frequency at index
        self.solverQuery.getTimeSlots()
        startfreq=self.solverQuery.timeSlots[index]['STARTTIME']
        endfreq=self.solverQuery.timeSlots[index]['ENDTIME']
        self.timeFreqSliderLabel.setText("Freq: \n" + str(startfreq))


    # Create the main frame of the dialog
    #
    def create_main_frame(self):
        self.main_frame = QWidget()

        #**********************************************************
        #
        # Canvas / Matplotlib
        #
        #**********************************************************

        # Create the mpl Figure and FigCanvas objects
        # 5x4 inches, 100 dots-per-inch
        self.dpi = 100
        self.fig = Figure((5.0, 4.0), dpi=self.dpi)
        self.canvas = FigureCanvas(self.fig)
        self.canvas.setParent(self.main_frame)

        self.axes = self.fig.add_subplot(111)

        # Create the navigation toolbar, tied to the canvas
        self.mpl_toolbar = NavigationToolbar(self.canvas, self.main_frame)


        #**********************************************************
        #
        # Widgets
        #
        #**********************************************************


        # Create buttons to access solverStatistics
        self.loadButton=QPushButton("&Load solver table")      # Load MS/solver button
        self.loadButton.setToolTip("Load a solver statistics table")
        self.saveButton=QPushButton("&Save plot")              # Save plot button
        self.saveButton.setToolTip("Save the plot to an image")
        self.drawButton=QPushButton("&Draw")                   # Draw button
        self.drawButton.setToolTip("Redraw the plot")
        self.addPlotButton=QPushButton("&Add plot")            # Add a (new) plot Window
        self.addPlotButton.setToolTip("Add a plot window")
        self.solutionsButton=QPushButton("Show Solutions")     # Show a plot of the corresponding solutions        
        self.solutionsButton.setToolTip("Show solutions plot along-side")
        self.quitButton=QPushButton("&Quit")                   # Quit the application
        self.quitButton.setToolTip("Quit application")

        self.plottingOptions=QLabel('Plotting options')
        self.showIterationsCheckBox=QCheckBox()                # Checkbox to show individual iterations parameters
        self.showIterationsCheckBox.setCheckState(False)       # Default: False
        self.showIterationsCheckBox.setText('Show iterations')

        self.singleCellCheckBox=QCheckBox()                # Checkbox to show individual iterations parameters
        self.singleCellCheckBox.setCheckState(False)       # Default: False
        self.singleCellCheckBox.setText('Show single cell')

        self.messageLabel=QLabel()


        #**********************************************************
        #
        # Layouts
        #
        #**********************************************************

        self.mainLayout=QHBoxLayout()
        self.buttonsLayout=QVBoxLayout()
        plotLayout=QVBoxLayout()

        # Add the button widgets to the buttonsLayout
        for w in [self.loadButton, self.saveButton, self.drawButton, self.addPlotButton, self.quitButton, self.plottingOptions, self.showIterationsCheckBox, self.singleCellCheckBox]:
            self.buttonsLayout.addWidget(w)

        self.buttonsLayout.insertStretch(-1);    # add a stretcher at the end
        self.buttonsLayout.setSizeConstraint(3)  # enum 3 = QLayout::SetFixedSize
        
        # Add the canvas and the mpl toolbar to the plotLayout
        plotLayout.addWidget(self.canvas)
        plotLayout.addWidget(self.mpl_toolbar)

        self.mainLayout.addLayout(self.buttonsLayout)
        self.mainLayout.addLayout(plotLayout)

        self.main_frame.setLayout(self.mainLayout)
        self.setCentralWidget(self.main_frame)



        #**********************************************************
        #
        # Connections
        #
        #**********************************************************

        # Set connections:
        self.connect(self.loadButton, SIGNAL('clicked()'), self.load_table)
        self.connect(self.saveButton, SIGNAL('clicked()'), self.save_plot)
        self.connect(self.drawButton, SIGNAL('clicked()'), self.on_draw)
        self.connect(self.showIterationsCheckBox, SIGNAL('stateChanged(int)'), self.on_draw)
        self.connect(self.singleCellCheckBox, SIGNAL('stateChanged(int)'), self.on_singleCell)
        #self.connect(self.showMessageCheckBox, SIGNAL('stateChanged(int)'), self.on_showMessage)


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
 
        self.create_dropdown_menu(self)


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
            self.timeStartSliderLabel.setText("S:" +  str(starttime) + " s")  # initialize StartTimeLabel with it


            self.timeEndSlider=QSlider(Qt.Horizontal)
            self.timeEndSliderLabel = QLabel("E:")
            self.timeEndSlider.setTracking(False)
            endtime=self.solverQuery.timeSlots[0]['ENDTIME']              # read first ENDTIME
            self.timeEndSliderLabel.setText("E:" +  str(endtime) + " s")  # initialize EndTimeLabel with it

            self.frequencySlider=QSlider(Qt.Horizontal)
            startfreq=self.solverQuery.frequencies[0]['STARTFREQ']
            self.frequencySliderLabel = QLabel("Freq:" + str(startfreq) + " Hz")


            self.timeStartSlider.setRange(0, self.solverQuery.getNumTimeSlots()-1)
            self.connect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.on_timeStartSlider) 
            self.timeEndSlider.setRange(0, self.solverQuery.getNumTimeSlots()-1)
            self.connect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.on_timeEndSlider) 

            self.frequencySlider.setRange(0, self.solverQuery.getNumFreqs()-1)
            self.connect(self.frequencySlider, SIGNAL('valueChanged(int)'), self.on_frequencySlider)

            # Get solver parameter names from table
            self.parameters=self.solverQuery.getParameterNames()

            # Add widgets to buttonLayout (these are now class attributes to make them accessible)
            # Only add sliders and their labels to the buttonLayout if it is not there yet:
            self.buttonsLayout.addWidget(self.timeStartSliderLabel)
            self.buttonsLayout.addWidget(self.timeEndSliderLabel)
            self.buttonsLayout.addWidget(self.timeStartSlider)
            self.buttonsLayout.addWidget(self.timeEndSlider)
            self.buttonsLayout.addWidget(self.frequencySliderLabel)
            self.buttonsLayout.addWidget(self.frequencySlider)

            self.buttonsLayout.setSizeConstraint(3)


    # Create drop down menu with solver parameters to choose
    # from
    #
    def create_dropdown_menu(self, solver):
        print "create_dropdown_menu(self, solver)"

        # Get parameter names from solver table
        parameterNames=self.solverQuery.getParameterNames()
        self.parametersComboBox=QComboBox()

        for p in parameterNames:
            self.parametersComboBox.addItem(p)

        # Finally add the ComboBox to the buttonsLayout
        self.buttonsLayout.addWidget(self.parametersComboBox)


    # Create a status bar at the bottom of the MainWindow
    #
    def create_status_bar(self):
        print "create_status_bar()"
        self.status_text = QLabel("Solver statistics")
        self.statusBar().addWidget(self.status_text, 1)


    # Redraw the plot with the current parameters
    #
    def on_draw(self):
        print "on_draw(self)"             # DEBUG

        # Solver parameter
        parameter=str(self.parametersComboBox.currentText())

        # Get indices of sliders:
        tindex=self.timeStartSlider.sliderPosition()
        findex=self.frequencySlider.sliderPosition()
        # Calculate index
        index=tindex+(self.solverQuery.getNumFreqs()*findex)



        # Plot the currently selected solver parameter
        # at the current grid position (time and freq)
        # with one iteration or all
        
        # If we only want to see the last iteration
        if self.showIterationsCheckBox.isChecked() == False:
            print "on_draw() if-branch"    # DEBUG
            
            # Get current timeSlot
            starttime=self.solverQuery.timeSlots[tindex]['STARTTIME']
            endtime=self.solverQuery.timeSlots[tindex]['ENDTIME']

            # Get current freqSlot
            startfreq=self.solverQuery.frequencies[findex]['STARTFREQ']
            endfreq=self.solverQuery.frequencies[findex]['ENDFREQ']            

            # Read data for selected parameter from solver table
            data=self.solverQuery.readParameterIdx(parameter, index)

            print "on_draw(): data: ", data
            print "on_draw(): type(data).__name__", type(data).__name__

            # Plot it on canvas
            self.plot(data)
            self.repaint()

        else:    # Plot all iterations
            print "on_draw() else-branch"  # DEBUG


    # Plotting function that does the actual plotting on the canvas
    #
    def plot(self, y, x=None, scatter=False, title=None, xlabel=None, ylabel=None, labels=None):
        global __styles

        __styles = ["%s%s" % (u, v) for v in ["-", ":"] for u in ["b", "g", "r", "c",
        "m", "y", "k"]]

        # Clear figure first
        self.fig.clf()

        axes = self.fig.gca()
        if not title is None:
            axes.set_title(title)
        if not xlabel is None:
            axes.set_xlabel(xlabel)
        if not ylabel is None:
            axes.set_ylabel(ylabel)

        # We have to distinguish between a single solver value (from just one solution cell)
        # and the case we were given a series of values within an interval

        print "plot(): type(y).__name__", type(y).__name__   # DEBUG

        # Single cell
        if type(y).__name__ is 'int' or 'double' or 'float':
            print "x: ", x
            print "y: ", y
            print "plot a single cell"

            if scatter:
                axes.scatter(x, y, edgecolors="None", c=__styles[0][0], marker="o")
            else:
                axes.plot(x, y, __styles[0][0], marker="o")

        elif type(y).__name__ is 'str':
            print "Message: ", y

        else:
            print "plot interval"

            # If no abcissa was given, create an indexed one from the y data
            if x is None:   
                x = [range(len(yi)) for yi in y]

            for i in range(0,len(y)):
                if labels is None:
                    if scatter:
                        axes.scatter(x[i], y[i] + offset, edgecolors="None",
                            c=__styles[i % len(__styles)][0], marker="o")
                    else:
                        axes.plot(x[i], y[i] + offset, __styles[i % len(__styles)])
                else:
                    if scatter:
                        axes.scatter(x[i], y[i] + offset, edgecolors="None",
                            c=__styles[i % len(__styles)][0], marker="o",
                            label=labels[i])
                    else:
                        axes.plot(x[i], y[i] + offset, __styles[i % len(__styles)],
                            label=labels[i])

                if stack:
                    if sep_abs:
                        offset += sep
                    else:
                        offset += y[i].mean() + sep * y[i].std()


        # Add labels to the plot
        if not labels is None:
            axes.legend(prop=FontProperties(size="x-small"), markerscale=0.5)



# Main function
#
def main():
    app = QApplication(sys.argv)
    form = SolverAppForm()

    # for some reason this must be here, and can not be in create_main_frame()
    form.connect(form.quitButton, SIGNAL('clicked()'), app, SLOT('quit()'))

    form.show()

    # If there is a table given as command line argument, open that table
    if len(sys.argv) > 1 and sys.argv[1] is not "":
        tableName=sys.argv[1]
        form.open_table(tableName)

    app.exec_()


# Main entry function
#
if __name__ == "__main__":
    main()

