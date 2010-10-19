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

class CopyData(QDialog):
    def __init__(self, parent):
        QDialog.__init__(self, parent, "Storage Details", True)
        self.location = QLineEdit("/data/L2008_06100", self)
        self.location.setMinimumWidth(190)
        self.location.move(100, 15)
        l = QLabel("Dataset location:", self)
        l.move(15, 15)
        l.setMaximumWidth(80)
        self.resize(300, 150)
        b = QPushButton("Copy", self)
        b.move(100, 55)
        b.show()
        b = QPushButton("Close", self)
        b.move(100, 90)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.accept)

    def getDetails(self, name):
        try:
            reload(GetStorageDetails)
        except:
            import GetStorageDetails
        return GetStorageDetails.main(name)