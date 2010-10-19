############################################################################
#    Copyright (C) 2008 by Adriaan Renting   #
#    renting@astron.nl   #
#                                                                          #
#    This program is free software; you can redistribute it and#or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    GNU General Public License for more details.                          #
#                                                                          #
#    You should have received a copy of the GNU General Public License     #
#    along with this program; if not, write to the                         #
#    Free Software Foundation, Inc.,                                       #
#    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
############################################################################
import sys, os, os.path
from qt import *

class GuessSize(QDialog):
    def __init__(self, parent):
        QDialog.__init__(self, parent, "Storage Details", True)
        self.stations = QLineEdit("16", self)
        self.stations.setMinimumWidth(50)
        self.stations.move(150, 15)
        l = QLabel("Number of Stations:", self)
        l.move(15, 15)
        l.setMinimumWidth(120)
        self.bands = QLineEdit("36", self)
        self.bands.setMinimumWidth(50)
        self.bands.move(150, 55)
        l = QLabel("Number of Subbands:", self)
        l.move(15, 55)
        l.setMinimumWidth(120)
        self.inttime = QLineEdit("10", self)
        self.inttime.setMinimumWidth(50)
        self.inttime.move(150, 95)
        l = QLabel("Integration time:", self)
        l.move(15, 95)
        l.setMinimumWidth(120)
        self.duration = QLineEdit("1:23", self)
        self.duration.setMinimumWidth(50)
        self.duration.move(150, 135)
        l = QLabel("Total time [hh:mm]:", self)
        l.move(15, 135)
        l.setMinimumWidth(120)
        self.resize(300, 250)
        b = QPushButton("Guess", self)
        b.move(15, 170)
        b.show()
        self.result = QLabel("1234MB", self)
        self.result.move(130, 170)
        self.result.setMinimumWidth(200)
        self.connect(b, SIGNAL("clicked()"), self.getGuess)
        b = QPushButton("Close", self)
        b.move(100, 205)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.accept)

    def getGuess(self):
        try:
            reload(GuessDataset)
        except:
            import GuessDataset
        result = GuessDataset.main(str(self.stations.text()), str(self.bands.text()), str(self.inttime.text()), str(self.duration.text()))
        self.result.setText(str(int(result)/1073741824) + " GB (" + result + " bytes)")