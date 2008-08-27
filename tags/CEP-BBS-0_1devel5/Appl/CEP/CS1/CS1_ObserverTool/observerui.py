#!/usr/bin/env python

#############################################################################
# ObserverUI - PyQT application template for KDevelop
#
# Translated from C++ qmakeapp.cpp
# (qmakeapp.cpp - Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.)
#
# This file is part of an example program for Qt.  This example
# program may be used, distributed and modified without limitation.
#
#############################################################################

import sys, os, os.path, time
from qt import *
from StorageDetails import *
from ShowNodes import *
from GuessSize import *
from CopyData import *
from FindData import *
from ReshuffleData import *
from ShowGrid import *

VERSION = "0.23"

def load_QPixMap(fileName):
    f = open(os.path.join(sys.path[0],fileName),"r")
    result = QPixmap(f.readlines())
    f.close()
    return result

class ObserverUI(QMainWindow):
    """An application called ObserverUI."""

    def __init__(self):
        QMainWindow.__init__(self, None, "ObserverUI")
        self.initIcons()
        self.setup()
        self.initMenu()
        self.initMainWidget()
        self.setCaption(self.appTitle)
        b = QPushButton("&Update Info", self)
        b.move(30, 35)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotGetInfo)
        b = QPushButton("Show &Nodes", self)
        b.move(30, 70)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotShowNodes)
        b = QPushButton("Guess Size", self)
        b.move(30, 105)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotGuessSize)
        b = QPushButton("Copy Data", self)
        b.move(30, 140)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotCopyData)
        b = QPushButton("Reshuffle Data", self)
        b.move(30, 175)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotReshuffleData)
        b = QPushButton("Find Data", self)
        b.move(30, 210)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotFindData)
        b = QPushButton("GRID", self)
        b.move(30, 245)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotShowGrid)
        b = QPushButton("Close", self)
        b.move(30, 315)
        b.show()
        self.connect(b, SIGNAL("clicked()"), self.slotClose)

    def setup(self):

        self.appTitle = "ObserverUI version " + VERSION

        self.menu = [
            ('&Actions',
              [('&Update Info', self.slotGetInfo),
               ('Show &Nodes', self.slotShowNodes),
               ('Guess &Size', self.slotGuessSize),
               ('&Copy Data', self.slotCopyData),
               ('&Reshuffle Data', self.slotReshuffleData),
               ('&Find Data', self.slotFindData),
               ('&GRID functions', self.slotShowGrid),
               (None,),
               ('&Close', self.slotClose)]),
             ('&Help',
              [('&About', self.slotAbout),
               (None, ),
               ('What\'s this', self.slotWhatsThis)])
             ]

        self.fsButtons = []
        self.stButtons = []
        self.config    = {'nrListNodes':4, 'nrLifsNodes':12, 'nrLioffNodes':30}

    def initMainWidget(self):
        self.statusBar().message("Ready")
        self.resize(600, 550)

    def initIcons(self):
        self.openIcon = QIconSet( load_QPixMap("fileopen.pyxpm"))
        self.saveIcon = QIconSet( load_QPixMap("filesave.pyxpm"))
        self.printIcon = QIconSet( load_QPixMap("fileprint.pyxpm"))

    def initMenu(self):
        for (menuName, subMenu) in self.menu:
            menu = QPopupMenu(self)
            self.menuBar().insertItem( menuName, menu )
            for menuOption in subMenu:
                if len(menuOption)==1:
                    menu.insertSeparator()
                elif len(menuOption)==2:
                    menu.insertItem( menuOption[0], menuOption[1] )
                elif len(menuOption)==3:
                    menu.insertItem( menuOption[2], \
                                     menuOption[0], \
                                     menuOption[1] )

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
        qbpurple = QBrush(QColor('purple'))
        if value < 0: qb = qbpurple
        elif value < 50: qb = qbgreen
        elif value <75: qb = qbyellow
        else: qb = qbred
        return qb

    def slotGetInfo(self):
        self.statusBar().message("Retrieving data...")
        self.setCursor(Qt.busyCursor)
        qp = QPainter()
        for i in range(self.config['nrListNodes']):
            name  = "list%03d" % (i+1)
            value = self.getSystemButtonValue(name, "/data")
            qb    = self.getColour(value)
            if len(self.stButtons) < self.config['nrListNodes']:
                self.stButtons.append(self.makeButton(300, 35*(i+1), name, value, qp, qb))
            else:
                self.updateButton(self.stButtons[i], value, qp, qb)

            value = self.getSystemButtonValue(name, "/san/LOFAR")
            qb = self.getColour(value)
            self.san = self.makeButton(300, 35*(self.config['nrListNodes'] + 2 + i),
                                      'san00' + str(i+1), value, qp, qb)

        for i in range(self.config['nrLifsNodes']):
            name  = "lifs%03d" % (i+1)
            value = self.getSystemButtonValue(name, "/data")
            qb    = self.getColour(value)
            if len(self.fsButtons) < self.config['nrLifsNodes']:
                self.fsButtons.append(self.makeButton(470, 35*(i+1), name, value, qp, qb))
            else:
                self.updateButton(self.fsButtons[i], value, qp, qb)
        self.unsetCursor()
        self.statusBar().message("Finished update: " + time.ctime())
        ##r = os.spawnl(os.P_WAIT, 'GetStorageCapacity.py -rlifs004')

    def slotShowNodes(self):
        g = ShowNodes(self)
        g.show()

    def slotGuessSize(self):
        g = GuessSize(self)
        g.show()

    def slotCopyData(self):
        c = CopyData(self)
        c.show()

    def slotReshuffleData(self):
        c = ReshuffleData(self)
        c.show()

    def slotFindData(self):
        g = FindData(self)
        g.show()

    def slotShowGrid(self):
        g = ShowGrid(self)
        g.show()

    def slotClose(self):
        self.close()

    def slotWhatsThis(self):
        self.whatsThis()

    def slotAbout(self):
        QMessageBox.about(self, self.appTitle, \
                          "This is a simple application for Observer use\n")

    def closeEvent(self, closeEvent):
        yesNo = QMessageBox.warning(self, self.appTitle,
                                    "Do you want to quit\n",
                                    "Yes", "No", "",
                                    0, 1)
        if yesNo == 0:
            closeEvent.accept()
        else:
            closeEvent.ignore()

def main(args):
    app=QApplication(args)
    mainWindow = ObserverUI()
    os.chdir( str(app.applicationDirPath()))
    mainWindow.show()
    app.connect(app, SIGNAL("lastWindowClosed()"), app, SLOT("quit()"))
    app.exec_loop()

if __name__ == "__main__":
    main(sys.argv)
