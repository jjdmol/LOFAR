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
from FindFiles import *
from ReorderData import *

class ReshuffleData(QDialog):
    def __init__(self, parent):
        QDialog.__init__(self, parent, "Reorder a dataset across the storage", True)
        self.source = QLineEdit("lifs001:/data/L2008_06100", self)
        self.source.setMinimumWidth(190)
        self.source.move(100, 15)
        l = QLabel("Source: ", self)
        l.move(15, 15)
        l.setMaximumWidth(80)
        self.destination= QLineEdit("/data/L2008_06100", self)
        self.destination.setMinimumWidth(190)
        self.destination.move(100, 55)
        l = QLabel("Destination: ", self)
        l.move(15, 55)
        l.setMaximumWidth(80)
        self.resize(300, 180)
        b = QPushButton("Reshuffle", self)
        b.move(100, 90)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.goFind)
        b = QPushButton("Close", self)
        b.move(100, 125)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.accept)

    def showResults(self, name):
        q = QDialog(self, "Completed data", True)
        q.resize(300, 150)
        l = QLabel("Finished re-ordering data for: " + name, q)
        l.move(15, 15)
        l.setMaximumWidth(280)
        b = QPushButton("Close", q)
        b.move(100, 90)
        b.show()
        q.show()
        self.connect(b, SIGNAL("clicked()"), q.accept)


    def goFind(self):
        self.setCursor(Qt.busyCursor)
        host, path = os.path.dirname(str(self.source.text())).split(':')
        location   = os.path.basename(str(self.source.text()))
        try:
            reload(FindFiles)
        except:
            import FindFiles
        files = FindFiles.main([(host, path)], location)
        try:
            reload(ReorderData)
        except:
            import ReorderData
        ReorderData.main(files, self.destination.text())
        self.unsetCursor()
        self.showResults(self.destination.text())
