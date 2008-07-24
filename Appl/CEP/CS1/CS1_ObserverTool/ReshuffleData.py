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

class ReshuffleData(QDialog):
    def __init__(self, parent):
        QDialog.__init__(self, parent, "Reorder a dataset across the storage", True)
        self.dataset = QLineEdit("L2008_06100", self)
        self.dataset.setMinimumWidth(190)
        self.dataset.move(100, 15)
        l = QLabel("Dataset name:", self)
        l.move(15, 15)
        l.setMaximumWidth(80)
        self.resize(300, 150)
        b = QPushButton("Reshuffle", self)
        b.move(100, 55)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.goFind)
        b = QPushButton("Close", self)
        b.move(100, 90)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.accept)

    def showResults(self, name):
        q = QDialog(self, "Completed data", True)
        q.resize(300, 150)
        l = QLabel("Finished re-ordering data for: " + name, q)
        l.move(15, 15)
        l.setMaximumWidth(80)
        b = QPushButton("Close", q)
        b.move(100, 90)
        b.show()
        q.show()
        self.connect(b, SIGNAL("clicked()"), q.accept)


    def goFind(self):
        files = []
        try:
            reload(ReorderData)
        except:
            import ReorderData
        ReorderData.main(files)
        self.showResults(name)