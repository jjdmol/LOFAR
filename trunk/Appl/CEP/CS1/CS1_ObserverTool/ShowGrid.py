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

import sys, os, os.path, time
from qt import *
from StorageDetails import *

class ShowGrid(QMainWindow):
    def __init__(self, parent):
        QMainWindow.__init__(self, parent, "GRID functions", True)
        self.statusBar().message("Ready")
        self.Buttons = []
        self.config  = {'nrListNodes':4, 'nrLifsNodes':12, 'nrLioffNodes':30}
        self.resize(650, 480)
        self.dataset = QLineEdit("L2008_06100", self)
        self.dataset.setMinimumWidth(190)
        self.dataset.move(150, 20)
        b = QPushButton("Find MS", self)
        b.move(30, 20)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotFindFile)
        b = QPushButton("Close", self)
        b.move(260, 420)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.close)

    def showResults(self, lines):
        q = QDialog(self, "Found data", True)
        height = 15
        sv = QScrollView(q)
        sv.enableClipper(True)
        sv.move(15, 15)
        sv.resize(470, 200)
        w = QWidget(sv.viewport())
        sv.addChild(w)
        sv.setVScrollBarMode(QScrollView.AlwaysOn)
        for line in lines:
            if line[0] == '0': continue
            l = QLabel(w)
            l.move(20, height)
            l.setAutoResize(True)
            height += 20
            l.setText(line)
        w.resize(450, height+60)
        q.resize(500, 260)
        b = QPushButton("Close", q)
        b.move(200, 220)
        b.show()
        self.connect(b, SIGNAL("clicked()"), q.accept)
        q.show()

    def slotFindFile(self):
        self.statusBar().message("Retrieving data...")
        self.setCursor(Qt.busyCursor)
        self.showResults(['test 1', 'test 2'])

        self.unsetCursor()
        self.statusBar().message("Finished update: " + time.ctime())
        ##r = os.spawnl(os.P_WAIT, 'GetStorageCapacity.py -rlifs004')
