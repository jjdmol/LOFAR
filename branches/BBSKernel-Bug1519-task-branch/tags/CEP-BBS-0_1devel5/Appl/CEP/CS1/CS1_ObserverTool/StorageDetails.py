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

class StorageDetails(QDialog):
    def __init__(self, parent, host):
        QDialog.__init__(self, parent, "Storage Details", True)
        self.host = host
        height = 15
        lines = self.getDetails(host)
        sv = QScrollView(self)
        sv.enableClipper(True)
        sv.move(15, 15)
        sv.resize(270, 200)
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
        w.resize(250, height+60)
        self.resize(300, 260)
        b = QPushButton("Close", self)
        b.move(100, 220)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.accept)

    def getDetails(self, name):
        try:
            reload(GetStorageDetails)
        except:
            import GetStorageDetails
        return GetStorageDetails.main(name)