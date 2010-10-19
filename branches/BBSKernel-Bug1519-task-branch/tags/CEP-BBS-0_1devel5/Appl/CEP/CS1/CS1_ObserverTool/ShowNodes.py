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

class ShowNodes(QMainWindow):
    def __init__(self, parent):
        QMainWindow.__init__(self, parent, "Status processing nodes", True)
        self.statusBar().message("Ready")
        self.Buttons = []
        self.config  = {'nrListNodes':4, 'nrLifsNodes':12, 'nrLioffNodes':30}
        self.resize(650, 480)
        b = QPushButton("Update Info", self)
        b.move(30, 20)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotGetInfo)
        b = QPushButton("Close", self)
        b.move(260, 420)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.close)

    def slotSystemClick(self):
        button = self.sender()
        s = StorageDetails(self, button.host)
        s.show()

    def makeButton(self, x, y, name, value, qp, qb):
        p = QPixmap(80, 15)
        p.fill()
        qp.begin(p)
        qp.fillRect(0,0, int(value*0.8), 15, qb)
        qp.end()
        l = QLabel(name, self)
        l.move(x-50, y)
        l.show()
        b = QPushButton(self)
        b.setPixmap(p)
        b.move(QPoint(x, y))
        b.host  = name
        b.value = value
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotSystemClick)
        return b

    def updateButton(self, button, value, qp, qb):
        p = QPixmap(80, 15)
        p.fill()
        qp.begin(p)
        qp.fillRect(0,0, int(value*0.8), 15, qb)
        qp.end()
        button.value = value
        button.setPixmap(p)
        button.repaint()

    def getSystemButtonValue(self, name, ldir):
        try:
            reload(GetStorageCapacity)
        except:
            import GetStorageCapacity
        return GetStorageCapacity.main(name, ldir)

    def getColour(self, value):
        qbred    = QBrush(QColor('red'))
        qbyellow = QBrush(QColor('yellow'))
        qbgreen  = QBrush(QColor('green'))
        qbpurple = QBrush(QColor('cyan'))
        if value < 0: qb = qbred
        elif value < 50: qb = qbgreen
        elif value <75: qb = qbyellow
        else: qb = qbred
        return qb

    def slotGetInfo(self):
        self.statusBar().message("Retrieving data...")
        self.setCursor(Qt.busyCursor)
        qp = QPainter()
        for i in range(self.config['nrLioffNodes']):
            name  = "lioff%03d" % (i+1)
            if name == "lioff019" or name == "lioff020": continue
            if i > 20:
                value = self.getSystemButtonValue(name, "/data")
            else:
                value = self.getSystemButtonValue(name, "/local")
            qb    = self.getColour(value)
            if len(self.Buttons) < self.config['nrLioffNodes']:
                self.Buttons.append(self.makeButton(80 + 200 * int(i/10), 35+35*(i%10+1), name, value, qp, qb))
            else:
                self.updateButton(self.Buttons[i], value, qp, qb)

        self.unsetCursor()
        self.statusBar().message("Finished update: " + time.ctime())
        ##r = os.spawnl(os.P_WAIT, 'GetStorageCapacity.py -rlifs004')
