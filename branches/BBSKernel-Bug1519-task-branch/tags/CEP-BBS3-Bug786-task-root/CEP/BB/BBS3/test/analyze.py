#!/usr/local/bin/python
"""
This module analysis one outputdirectory and recreates the menu.html
by looking for files named output.html in the subdirectories of the
current directory.
"""
import sys
import os
sys.path.append('./HTMLgen')
import HTMLgen
from OutputParser import OutputParser, GeneralInfoParser
from htmloutput import BBSTestResultDocument

def createMenu():
   # create html document
   document = HTMLgen.BasicDocument()
   document.append(HTMLgen.Heading(2,"BBS Test Results"))
   document.append(HTMLgen.Heading(3,"Available test results:"))

   # look for output.html in subdirectories
   curdir = os.curdir
   subdirs = list()
   for filename in os.listdir(curdir):
      longname = curdir + "/" + filename + "/output.html"
      if os.path.isfile(longname):
         # create a list of tuples of (name, relative path of output.html)
         subdirs.append((filename,longname))

   # create a link from each subdir
   for subdir in subdirs:
     document.append(HTMLgen.Href(subdir[1], subdir[0], target = "output"))
     document.append(HTMLgen.BR())

   open("menu.html", 'w').write(str(document))

def analyzeDir(dirName):
   # create HTML document
   document = BBSTestResultDocument()

   # parse the file with general info
   try:
     fileName = dirName + "/testgeneralinfo.out"
     gp = GeneralInfoParser()
     gp.parseFileByName(fileName)
     document.testinfo = gp.getInfo()
   except:
     print ("could not find general info, continuing")

   # parse the other .out files
   for fileName in os.listdir(dirName):
      
      # take all files in the directory
      if fileName.startswith('test'):
         extension = fileName.split('.')[-1]
         if extension == 'out':
            # we have a file that is called test*.out
            if fileName != "testgeneralinfo.out":
               print("now analyzing " + fileName)
               op = OutputParser(name = fileName)
               op.parseFileByName(dirName + "/" + fileName)
               document.addTest(fileName, op.itsTestRun)

   # write the html document
   open(dirName + "/output.html", 'w').write(str(document))

if __name__ == "__main__":
   if len(sys.argv) == 1:
      print "No subdirs given on the commandline"
   else:
      for dir in sys.argv[1:]:
         print "Analyzing " + dir + " ..."
         if os.path.isdir(sys.argv[1]):
            analyzeDir(dir.rstrip('/'))
         else:
            print "Error: " + dir + " is not a valid directory"

   # recreate index
   print "Recreating menu ..."
   createMenu()   

	
