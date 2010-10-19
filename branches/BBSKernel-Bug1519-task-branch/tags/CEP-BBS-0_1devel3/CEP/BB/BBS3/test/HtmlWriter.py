#!/usr/bin/env python
"""
This module contains the html structures
The structure of a generated html document is:
document
- 1 stackedbargraph
- 1 timertable
- 1 testinfoparagraph
- (per test) testparagraph
-- (per interval) parmtracetable
--- (per parameter) parmtracerow
---- (per interval) parmtracecel (is not a seperate object in this file)
"""
import sys
import time
import TestData
import OutputParser
import pdb
from forgetHTML import *

class BBSTestResultDocument(SimpleDocument):
   """
   This document holds the results of a testrun.
   It contains a bargraph and table with all the times,
   some info about the host and the detailed output of the parameters
   """
   def __init__(self, title):
      SimpleDocument.__init__(self, title)
      self.testinfo = TestData.TestInfo()
      self.mysbg = StackedBarGraph()
      self.mytimertable = TimerTable()
      self.mytests = []
   def addTest(self, name, testrun):
      timers = testrun.getTotalTimes()
      self.mysbg.addTimeBar(name, timers)
      self.mytimertable.addTimeRow(name, timers)

      tp = TestParagraph(name, testrun)
      self.mytests.append(tp)
   def __str__(self):
      now = map(str, time.localtime()) # convert all int to a string
      nowstring = now[2] + "-" + now[1] + "-" + now[0] + " at " + now[3] + ":" + now[4]
      self.body.append(Header("Generated on " + nowstring, 2))
      self.body.append(Ruler())
      #self.body.append(self.mysbg)
      #self.body.append(self.mytimertable)
      self.body.append(TestInfoParagraph(self.testinfo))
      for testrun in self.mytests:
         pdb.set_trace()
         self.body.append(Block(Text('testrun: ' + str(testrun))))
         self.body.append(testrun)
      self.body.append(Ruler())
      self.body.append("End of BBSTest Results")
      return SimpleDocument.__str__(self)

class StackedBarGraph:
   def __init__(self):
      self.mymatrix = matrix(defaultValue = 0)
   def addTimeBar(self, name, timeDict):
      for timeName in timeDict:
         if not "total" in timeName:
            if isinstance(timeDict[timeName], TestData.Timer):
               self.mymatrix.set(name, timeName, timeDict[timeName].getTotal())
   def __str__(self):
      data = self.mymatrix.getRowsWithLabelAsList()
      names = list()
      for name in self.mymatrix.colnrs:
         names.append(name)
            
      dl = DataList()
      dl.segment_names = names
      dl.load_tuples(data)
      chart = StackedBarChart(dl)
      chart.title = "Timings"
      return chart.__str__()
   
class TimerTable:
   def __init__(self):
      self.mymatrix = matrix(defaultValue = 0)
   def addTimeRow(self, name, timeDict):
      for timeName in timeDict:
         if isinstance(timeDict[timeName], TestData.Timer):
            # this is a NSTimer
            time = timeDict[timeName]
            self.mymatrix.set(name, timeName, time.getTotal())
         else:
            # this is a real/system/user timer
            timeusr = timeDict[timeName]
            self.mymatrix.set(name, timeName, timeusr.getShortStr())
   def __str__(self):
      data = self.mymatrix.getRowsWithLabelAsList()
      names = ["fileName"]
      for name in self.mymatrix.colnrs:
         names.append(name)

      t = Table("Timers")
      t.heading = names
      t.body = data
      return t.__str__()    

class TimerTableWithGraph:
   def __init__(self):
      self.rows = dict()
   def addTimeRow(self, name, timeDict):
      self.rows[name] = timeDict
   def getColumns(self):
      myColumns = list()
      for row in self.rows:
         for key in timeDict:
            if not key in myColumns:
               myColumns.add(key)
   def getTotals(self):
      myTotals = dict()
      for row in self.rows:
         total = 0
         for key in timeDict:
            if isinstance(timers[column], TestData.Timer):
               total += timers[column].getTotal()
         myTotals[row] = total
      return myTotals
   def makeColors(self, columns):
      rsteps = map(lambda x:x/len(columns), range(0,len(columns)))
      gsteps = map(lambda x: (x + 1/3) % 1, rsteps)
      bsteps = map(lambda x: (x + 2/3) % 1, rsteps)
      r = map(lambda x: 4 * (x - 0.5)^2, rsteps);
      g = map(lambda x: 4 * (x - 0.5)^2, gsteps);
      b = map(lambda x: 4 * (x - 0.5)^2, bsteps);
      colors = dict()
      for column in colums:
         colors[column] = (r,g,b)
      return colors
   def writeImage(self, name, cols, row):
      colors = makeColors(self, myColumns)
      start = 0
      for col in cols:
         pass
   def writeLegendImage(self, cols):
      colors = makeColors(self, myColumns)
      
   def __str__(self):
      # create a unique list of timer names
      myColumns = getColumns(self)
            
      myTable = Table("Timers")
      for rowKey in self.rows:
         myTableRow = list()
         myTableRow.append(rowKey)
         timers = self.rows[rowKey]
         for column in myColumns:
            if column in timers:
               timer = timers[column]
               if isinstance(timer, TestData.Timer):
                  myTableRow.append(timer.getTotal())
               elif itsinstance(timer, TestData.TimerUSR):
                  myTableRow.append(timer.getShortStr())
               else:
                  myTableRow.append("unknown timer")
            else:
               myTableRow.append("not timed")
         myTable.append(myTableRow)

      # calc the total time for all tests
      totals = getTotals(self)
      scalefactor = 1/max(totals)
      myGraph = Table("Timers")
      for rowKey in self.rows:
         myTableRow = list()
         myTableRow.append(rowKey)
         myTableRow.append(totals(rowKey))
         #myTableRow.append(Image(writeImage(???)))
         myGraph.append(myTableRow)
      myTableRow = list()         
      myTableRow.append("Average")
      myTableRow.append(sum(totals)/len(totals))
      # myTableRow.append(Image(writeLegendImage(???)))
      myGraph.append(myTableRow)
      return myGraph.__str__() + myTable.__str__()    

class TestInfoParagraph(Table):
   """
   This class holds all the information about a test run
   (like hostname, number of processors, date etc) in a
   html table
   """
   def __init__(self, TestInfo):
      Table.__init__(self, "information about testrun")
      self.heading = []
      self.body = list()
      for key in TestInfo.getTags():
         linelist = TestInfo[key]
         line = ''.join(linelist);
         self.body.append([key, Pre(Small(line))])

class TestParagraph(Paragraph):
   def __init__(self, name, testrun):
      tdd = testrun.getTraceDicts()
      self.name = name
      self.settings = testrun.settings
      self.myPTraceTables = list()
      for int in range(0,len(tdd)):
         self.myPTraceTables.append(ParmTraceTable(Strong("Interval " + str(int)), tdd[int]))
      Paragraph.__init__(self)
   def __str__(self):
      mypar = Block()
      mypar.append(Header(self.name, 2))
      mypar.append(Small(Href("#home", "back to top")))
      mypar.append(Header("Settings:", 3))
      mypar.append(self.settings)
      mypar.append(P())
      mypar.append(Href(self.name.split("/")[-1], "BBSTest output"))
      mypar.append(P())
      for pt in self.myPTraceTables:
         mypar.append(pt)
      return mypar.__str__()      

class ParmTraceTable(SimpleTable):
   def __init__(self, name, traceDict):
      SimpleTable.__init__(self, border=1, cellpadding=5, cellspacing=1)
      #self.append(Caption(name))
      header = TableRow()
      header.append(TableHeader("Parameter name"))
      header.append(TableHeader("real value"))
      header.append(TableHeader("Q (small is bad)"))
      header.append(TableHeader("ParameterValues"))
      self.append(header)
      for name in traceDict:
         self.append(ParmTraceRow(traceDict[name]))      

class ParmTraceRow(TableRow):
   def __init__(self, pTrace):
      TableRow.__init__(self)
      self.append(TableCell(Strong(pTrace.name)))
      realValue = pTrace.getRealValue()
      if realValue == None:
         self.append(TableCell())
         self.append(TableCell())
      else:
         self.append(TableCell(Strong(', '.join(pTrace.getRealValue()))))
         self.append(TableCell(Strong(pTrace.getQuality())))         
      for value in pTrace.valueList:
         self.append(TableCell(', '.join(str(value))))

def sortedkeys(origdict):
   """
   function to return the keys from a dict in sorted order
   """
   keys = origdict.keys()
   keys.sort()
   return keys

class matrix:
   """
   This class represents a 2D variant of a dict.
   A dict of dicts doesn't garantuee that the values with the same
   column index are actually in the same column. The method getRowsWithLabelAsList
   does return all the columns aligned.

   """
   def __init__(self, defaultValue):
      self.rows = dict()
      self.colnrs = dict()
      self.defaultValue = defaultValue
   def set(self, row, column, value):
      if not self.colnrs.has_key(column):
         self.colnrs[column] = len(self.colnrs) + 1
      if not self.rows.has_key(row):
         self.rows[row] = dict()
      self.rows[row][self.colnrs[column]] = value
   def get(self, row, column):
      return self.rows[row][self.colnrs[column]]
   def getRowsWithLabelAsList(self):
      l = list()
      sortedrowkeys = sortedkeys(self.rows)
      for rowName in sortedrowkeys:
         t = [Href("#"+rowName, rowName)]
         for colName in self.colnrs:
            colnr = self.colnrs[colName]
            row = self.rows[rowName]
            if row.has_key(colnr):
               t.append(self.rows[rowName][self.colnrs[colName]])
            else:
               t.append(self.defaultValue)
         l.append(t)
      return l

if __name__ == "__main__":
   try:
      f = open('testDefault.out', 'rb')
   except:
      f = None

   if not f:
      d = BBSTestResultDocument('HtmlWriter testdocument')
      tr=TestData.TestRun()
      tr.add(TestData.Interval())
      tr.children[0].add(TestData.Iteration())
      tr.children[0].children[0].add(TestData.Parameter('testparm', 2.5))
   else:
      d = BBSTestResultDocument('HtmlWriter testdocument from testDefault')
      op = OutputParser.OutputParser('testDefault')
      op.parseFileByName('testDefault.out')
      tr = op.itsTestRun
      #      tr.doPrint(' ')
      
   d.addTest('test', tr)
   open('output.html', 'w').write(str(d))
