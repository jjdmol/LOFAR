#! /usr/bin/env python
#
# makePackage.py: Script to make an initial Packagedir in the LOFAR
# development tree
#
# Usage:
#        ./makePackageDir [PackageName]
# Args:
#       PackageName	The name of the Package that will be created
#
#
# Revisions:
#
#  25-01-2005   Initial Release.
#
#
#  Copyright (C) 2005
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  $Id$

#
# import all packages we need
#
import os
import sys
import getopt

def usage():
  print "usage: "+sys.argv[0]+" [-h] [-s] packageName [packageName...]"
  print "args:  -h,--help                    - print usage"
  print "args:  -s,--super                   - super(toplevel) package"  
  print "       packageName [packageName...] - name of the package to be created."
  sys.exit(2)

def openFile(name,mode):
  try:
    file = open (name,mode)
  except IOError, message:
    sys.exit("Error opening file: %s" % message)
  return file

def createDir(name):
  try:
    file = os.mkdir(name)
  except IOError, message:
    sys.exit("Error creating directory: %s" % message)

def createPackageDoc(packageName,dirLevel):
  if os.path.isfile(packageName):
    return
  else:
    writeFile=openFile(packageName+"/package.doc","w")
    if dirLevel>0:
      baseGroup=os.path.basename(os.environ["PWD"])
      writeFile.write("// \ingroup "+baseGroup+"\n")
    writeFile.write("// \defgroup "+packageName+" "+packageName+" Description\n")
    writeFile.close()

def replacePackageName(readFile,writeFile,packageName,dirLevel):
  levelString=""
  for i in range(0,dirLevel):
    levelString +="../"
  aLine=readFile.readline()
  while aLine != "":
    newLine=aLine
    if aLine.find("%PACKAGE%") > -1:
      newLine = str.replace(aLine,"%PACKAGE%",packageName)
    if aLine.find("%PACKAGELOWER%") > -1:
      newLine = str.replace(aLine,"%PACKAGELOWER%",packageName.lower())
    if aLine.find("%LEVEL%") > -1:
      newLine = str.replace(aLine,"%LEVEL%",levelString)
    writeFile.write(newLine)
    aLine=readFile.readline()
  
def createConfigureIn(lofarDir,packageName,dirLevel):
  #
  # Create configure.in in Package dir
  #
  readFile=openFile(lofarDir+"/templates/package_configure.in_template","r")
  writeFile=openFile(packageName+"/configure.in","w")
  replacePackageName(readFile,writeFile,packageName,dirLevel)
  writeFile.close()
  readFile.close()

def createLofarconfIn(lofarDir,packageName):
  #
  # Create lofarconf.in in (super) Package dir
  #
  readFile=openFile(lofarDir+"/templates/package_lofarconf.in_template","r")
  writeFile=openFile(packageName+"/lofarconf.in","w")
  aLine=readFile.readline()
  while aLine != "":
    writeFile.write(aLine)
    aLine=readFile.readline()
  writeFile.close()
  readFile.close()

def addToLofarconfIn(lofarDir,packageName):
  #
  # add new package to lofarconf.in in Package dir
  #

  os.rename("lofarconf.in","lofarconf.in.old")

  readFile=openFile("lofarconf.in.old","r")
  writeFile=openFile("lofarconf.in","w")
  aLine=readFile.readline()
  while aLine != "":
    writeFile.write(aLine)
    aLine=readFile.readline()
  writeFile.write(packageName+"\t\t\t#short description\n")
  writeFile.close()
  readFile.close()

def createBootstrap(lofarDir,packageName,dirLevel):
  #
  # Create Bootstrap in Package dir
  #
  readFile=openFile(lofarDir+"/templates/package_bootstrap_template","r")
  writeFile=openFile(packageName+"/bootstrap","w")
  replacePackageName(readFile,writeFile,packageName,dirLevel)
  writeFile.close()
  readFile.close()

def createMakefiles(lofarDir,packageName,srcDir,testDir,dirLevel):
  #
  # Create Makefile.am in Package dir
  #
  readFile=openFile(lofarDir+"/templates/package_makefile.am_template","r")
  writeFile=openFile(packageName+"/Makefile.am","w")
  replacePackageName(readFile,writeFile,packageName,dirLevel)
  writeFile.close()
  readFile.close()

  #
  # Create Makefile.am in srcDir
  #
  readFile=openFile(lofarDir+"/templates/src_makefile.am_template","r")
  writeFile=openFile(srcDir+"/Makefile.am","w")
  replacePackageName(readFile,writeFile,packageName,dirLevel)
  writeFile.close()
  readFile.close()

  #
  # Create Makefile.am in testDir
  #
  readFile=openFile(lofarDir+"/templates/test_makefile.am_template","r")
  writeFile=openFile(testDir+"/Makefile.am","w")
  replacePackageName(readFile,writeFile,packageName,dirLevel)
  writeFile.close()
  readFile.close()


def main(argv):
  packageName = "None"
  baseDir = os.environ["PWD"]
  super=0
  #
  # get Lofar base dir
  #
  file= os.popen("echo $PWD | sed -e 's%/LOFAR/.*%/LOFAR%'")
  lofarDir=str.replace(file.readline(),"\n","")
  file.close()

  # find out the directory sublevel
  dirLevel=len(baseDir.split('/'))-len(lofarDir.split('/'))

  try:
    opts, args = getopt.getopt(argv, "hs",["help","super"])
  except getopt.GetoptError:
    usage()
  for opt, arg in opts:
    if opt in ("-h", "--help"):
      usage()
    elif opt in ("-s", "--super"):
      super = 1
    

  if len(args) <= 0 and packageName == "None":
    usage()
    
  for packageName in args:
    
    #
    # print LOFAR Package and basetree
    #
    if super ==0:
      print "Trying to set up Package: " + baseDir + "/" + packageName +"\n"
    else:
      print "Trying to set up Super Package: " + baseDir + "/" + packageName +"\n"
      
    #
    # Check of given package name allready exists in the working directory as
    # directory or as file
    #
    if os.path.isdir(packageName) | os.path.isfile(packageName):
      print "Sorry, that name allready exists. Please take another one\n"
      sys.exit(1)

    #
    # if configure.in exists in the root directory it is forbidden to extend
    # this directory with (sub)packages
    #
    if os.path.isfile("configure.in"):
      print "Sorry, it is not allowed to create subpackages in this packagedir\n"
      sys.exit(1)

    #
    # Create the directory
    #
    createDir(packageName)
    
    if super == 0:
      srcDir = packageName+"/src"
      testDir = packageName+"/test"
    
      #
      # create the src and test directories in package
      #
      createDir(srcDir)
      createDir(testDir)
    
      #
      # Create all initial files from templates
      #
      createMakefiles(lofarDir,packageName,srcDir,testDir,dirLevel)
      
      #
      # Create all initial files from templates
      #
      createConfigureIn(lofarDir,packageName,dirLevel)
      
      #
      # Create all initial files from templates
      #
      createBootstrap(lofarDir,packageName,dirLevel)
      
    else:
      #
      # create lofarconf.in
      #
      createLofarconfIn(lofarDir,packageName)

    #
    # Add package to lofarconf.in
    #
    addToLofarconfIn(lofarDir,packageName)

    createPackageDoc(packageName,dirLevel)
  
#
# this is the main entry
#
if __name__ == "__main__":
  main(sys.argv[1:])
  print "Done"
