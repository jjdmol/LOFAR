# Plotting class for plotting solver parameters that were written with
# the ParmDBLog class
#
# File:			SolverPlot.py
# Author:		Sven Duscha (duscha@astron.nl)
# Date:			2010-07-20
# Last change:		2010-07-20


class SolverPlot:

   def __init__(self, parent):
        QtGui.QMainWindow.__init__(self, parent)

        self.setGeometry(300, 300, 350, 300)
        self.setWindowTitle('OpenFile')

        self.textEdit = QtGui.QTextEdit()
        self.setCentralWidget(self.textEdit)
        self.statusBar()
        self.setFocus()

    
    # Create a histogram of "data"
    def createHisto( data ):
        n, bins, patches = pl.hist(data, 50, normed=1, histtype='stepfilled')
        pl.setp(patches, 'facecolor', 'g', 'alpha', 0.75) 
        return

    def save_plot(self):
        file_choices = "PNG (*.png)|*.png"
        
        path = unicode(QFileDialog.getSaveFileName(self, 
                        'Save file', '', 
                        file_choices))
        if path:
            self.canvas.print_figure(path, dpi=self.dpi)
            self.statusBar().showMessage('Saved to %s' % path, 2000)

    def createSubplot(plot):

        return

    # Create Plot on canvas
    def plot():
        ax.plot(list, "r+")    
        ax.set_title(colname)
        plt.draw()
        return

    # Plotting function, update on change of plotting parameters
    def replot():
        return

