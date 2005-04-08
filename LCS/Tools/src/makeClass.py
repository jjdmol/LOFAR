#! /usr/bin/env python
#
# makeClass.py: Script to make default class files in a Package/srcdir in the
# LOFAR development tree. normal class files, main program and templates
# are covered
#
# Usage:
#        ./makeClass [-h] [-t list [-d] | -m] [ClassName]
# Args:
#       ClassName 	    The name of the Class that will be created
#       h,--help            usage
#       t,--templated list  This is an automated templated class,
#                           list can contain a comma seperated list
#                           with the template parameters. Example:
#                           makeClass -t T,U className
#       d,--diy             Do it yourself (manual template instanciation)
#                           Only together with -t
#       m,--main            This is a main class
#
# Revisions:
#
#  26-01-2005   Initial Release.
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
import re
from datetime import date

def openFile(name,mode):
  try:
    file = open (name,mode)
  except IOError, message:
    sys.exit("Error opening file: %s" % message)
  return file

def replacePackageAndClassName(readFile,writeFile,packageName,
                               className,subDirName):
  aLine=readFile.readline()
  year=`date.today().year`
  while aLine != "":
    #set start of copyright year
    if aLine.find("%YEAR%") > -1:
      aLine = str.replace(aLine,"%YEAR%",year)


    # replace SUB with Subdir when needed
    if aLine.find("%SUB%") > -1:
      if subDirName != "":
        aLine = str.replace(aLine,"%SUB%",subDirName+"/")
      else:
        aLine = str.replace(aLine,"%SUB%",subDirName)

    # replace SUBUPPER with Subdir in uppercase when needed
    if aLine.find("%SUBUPPER%") > -1:
      if subDirName != "":
        aLine = str.replace(aLine,"%SUBUPPER%",subDirName.upper()+"_")
      else:
        aLine = str.replace(aLine,"%SUBUPPER%",subDirName.upper())

    # replace PACKAGE with real name
    if aLine.find("%PACKAGE%") > -1:
      aLine = str.replace(aLine,"%PACKAGE%",packageName)

    # replace PACKAGEUPPER with uppercase Package name
    if aLine.find("%PACKAGEUPPER%") > -1:
      aLine = str.replace(aLine,"%PACKAGEUPPER%",packageName.upper())

    # replace CLASS with real name
    if aLine.find("%CLASS%") > -1:
      aLine = str.replace(aLine,"%CLASS%",className)
      
    # replace CLASSUPPER with uppercase classname
    if aLine.find("%CLASSUPPER%") > -1:
      aLine = str.replace(aLine,"%CLASSUPPER%",className.upper())
      
    writeFile.write(aLine)
    aLine=readFile.readline()
  
def addTemplates(type,readFile,writeFile,className,packageName,templateList,autoTemplate,subDirName):
  aLine=readFile.readline()
  year=`date.today().year`
  
  while aLine != "":
    #set start of copyright year
    if aLine.find("%YEAR%") > -1:
      aLine = str.replace(aLine,"%YEAR%",year)
      
    # replace SUB with Subdir when needed
    if aLine.find("%SUB%") > -1:
      if subDirName != "":
        aLine = str.replace(aLine,"%SUB%",subDirName+"/")
      else:
        aLine = str.replace(aLine,"%SUB%",subDirName)

    # replace SUBUPPER with Subdir in uppercase when needed
    if aLine.find("%SUBUPPER%") > -1:      
      if subDirName != "":
        aLine = str.replace(aLine,"%SUBUPPER%",subDirName.upper()+"_")
      else:
        aLine = str.replace(aLine,"%SUBUPPER%",subDirName.upper())

    # replace PACKAGE with real name
    if aLine.find("%PACKAGE%") > -1:
      aLine= str.replace(aLine,"%PACKAGE%",packageName)
      
    # replace PACKAGEUPPER with uppercase Package name
    if aLine.find("%PACKAGEUPPER%") > -1:
      aLine = str.replace(aLine,"%PACKAGEUPPER%",packageName.upper())

    # replace CLASS with real name
    if aLine.find("%CLASS%") > -1:
      aLine = str.replace(aLine,"%CLASS%",className)
      
    # replace CLASSUPPER with uppercase classname
    if aLine.find("%CLASSUPPER%") > -1:
      aLine = str.replace(aLine,"%CLASSUPPER%",className.upper())
      
    # Check if !diy, template and .h file, if so include tcc in header file
    if aLine.find("Include tcc file here when needed") > -1 and \
           type =="h" and autoTemplate == 1:
      if subDirName != "":
        writeFile.write("\n#include <"+packageName+"/"+subDirName+"/"+className+".tcc>\n")
      else:
        writeFile.write("\n#include <"+packageName+"/"+className+".tcc>\n")
      aLine=""
    elif aLine.find("Include tcc file here when needed") > -1:
      aLine =""
    # find place to inserttemplates depending on filetype
    if type=="h" and aLine.find("insert templates here") > -1:
      writeFile.write("\n    template< ")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write("typename "+templateList[i])
        i+=1
      writeFile.write(" >")
      writeFile.write("\n    class "+className)
      writeFile.write("\n    {")
      writeFile.write("\n    public:")
      writeFile.write("\n\n      "+className+"();")
      writeFile.write("\n      ~"+className+"();")
      writeFile.write("\n    private:")
      writeFile.write("\n      // Copying is not allowed")
      writeFile.write("\n      "+className+"(const "+className+"< ")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write(templateList[i])
        i+=1
      writeFile.write(" >& that);")
      writeFile.write("\n      "+className+"< ")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write(templateList[i])
        i+=1
      writeFile.write(" >& operator=(const "+className+"< ")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write(templateList[i])
        i+=1
      writeFile.write(" >& that);")
      writeFile.write("\n    };")
    elif (type=="tcc" ) and aLine.find("insert templates here") > -1:
      writeFile.write("\n    template< ")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write("typename "+templateList[i])
        i+=1
      writeFile.write(" >")
      writeFile.write("\n    "+className+"< ")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write(templateList[i])
        i+=1
      writeFile.write(">::"+className+"()")
      writeFile.write("\n    {")
      writeFile.write("\n  work to do")
      writeFile.write("\n    }")
      writeFile.write("\n")

      writeFile.write("\n    template<")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write("typename "+templateList[i])
        i+=1
      writeFile.write(" >")
      writeFile.write("\n    "+className+"<")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write(templateList[i])
        i+=1
      writeFile.write(">::~"+className+"()")
      writeFile.write("\n    {")
      writeFile.write("\n  work to do")
      writeFile.write("\n    }")
      writeFile.write("\n")
    elif (type=="diy" ) and aLine.find("insert templates here") > -1:
      writeFile.write("\n  //template class "+className+"< ")
      i=0
      while i < len(templateList):
        if i > 0:
          writeFile.write(", ")
        writeFile.write(templateList[i])
        i+=1
      writeFile.write(" >;\n")
    else:
      writeFile.write(aLine)
    aLine=readFile.readline()

def makeDefaultClass(lofarDir,className,packageName,srcDir,subDirName):
  # default.h file
  readFile=openFile(lofarDir+"/LCS/Tools/src/templates/header.h_template","r")
  writeFile=openFile(className+".h","w")
  replacePackageAndClassName(readFile,writeFile,packageName,className,subDirName)
  writeFile.close()
  readFile.close()
  addToMakefile("h",className,srcDir,subDirName)
  #default.cc file
  readFile=openFile(lofarDir+"/LCS/Tools/src/templates/header.cc_template","r")
  writeFile=openFile(className+".cc","w")
  replacePackageAndClassName(readFile,writeFile,packageName,className,subDirName)
  writeFile.close()
  readFile.close()
  addToMakefile("cc",className,srcDir,subDirName)

def makeTemplatedClass(lofarDir,className,packageName,templateList,autoTemplate,srcDir,subDirName):
  #default h file
  readFile=openFile(lofarDir+"/LCS/Tools/src/templates/templated_header.h_template","r")
  writeFile=openFile(className+".h","w")
  addTemplates("h",readFile,writeFile,className,packageName,templateList,autoTemplate,subDirName)
  writeFile.close()
  readFile.close()
  addToMakefile("h",className,srcDir,subDirName)


  if autoTemplate==0:
    #default diy-tcc template file
    readFile=openFile(lofarDir+"/LCS/Tools/src/templates/templated_header.tcc_template","r")
    writeFile=openFile(className+".tcc","w")
    addTemplates("tcc",readFile,writeFile,className,packageName,templateList,autoTemplate,subDirName)
    writeFile.close()
    readFile.close()
    addToMakefile("tcc",className,srcDir,subDirName)

    #default diy-cc template file
    readFile=openFile(lofarDir+"/LCS/Tools/src/templates/templated_header.cc_template","r")
    writeFile=openFile(className+".cc","w")
    addTemplates("diy",readFile,writeFile,className,packageName,templateList,autoTemplate,subDirName)
    writeFile.close()
    readFile.close()
    addToMakefile("diy",className,srcDir,subDirName)
  else:
    #default tcc file
    readFile=openFile(lofarDir+"/LCS/Tools/src/templates/templated_header.tcc_template","r")
    writeFile=openFile(className+".tcc","w")
    addTemplates("tcc",readFile,writeFile,className,packageName,templateList,autoTemplate,subDirName)
    writeFile.close()
    readFile.close()
    addToMakefile("tcc",className,srcDir,subDirName)

def makeMainClass(lofarDir,className,packageName,srcDir,subDirName):
  readFile=openFile(lofarDir+"/LCS/Tools/src/templates/main.cc_template","r")
  writeFile=openFile(className+"Main.cc","w")
  replacePackageAndClassName(readFile,writeFile,packageName,className,subDirName)
  writeFile.close()
  readFile.close()
  addToMakefile("maincc",className+"Main",srcDir,subDirName)


def addToMakefile(type,className,srcDir,subDirName):
  hPattern=re.compile('^([ \t]*)INSTHDRS[ \t]*=.*$',re.IGNORECASE)
  ccPattern=re.compile('^(.*)_la_SOURCES[ \t]*=.*$',re.IGNORECASE)
  mainccPattern=re.compile('^(.*)bin_PROGRAMS[ \t]*=.*$',re.IGNORECASE)
  tccPattern=re.compile('^([ \t]*)TCCHDRS[ \t]*=.*$',re.IGNORECASE)
  os.rename(srcDir+"/Makefile.am",srcDir+"/Makefile.am.old")
  readFile=openFile(srcDir+"/Makefile.am.old","r")
  writeFile=openFile(srcDir+"/Makefile.am","w")
  searchEnd=0
  aLine=readFile.readline()

  while aLine != "":
    if subDirName != "":
      extendedClassName=subDirName+"/"+className
    else:
      extendedClassName=className
      
    if type == "h":
      # find INSTHDRS to start inserting headerfiles
      if hPattern.search(aLine):
        #find / to see if the line allready contains another header
        front,end = aLine.split("=")
        if re.search("[a-zA-Z]",end):
          writeFile.write(front+" = "+extendedClassName+".h \\\n")
          writeFile.write("\t"+end)
        elif end.find('\\'):
          writeFile.write(front+" = "+extendedClassName+".h \\\n")
        else :
          writeFile.write(front+" = "+extendedClassName+".h\n")
          
      else:
        writeFile.write(aLine)

    elif type == "cc" or type == "diy":
      # find _la_SOURCES to start inserting sourcefiles
      if ccPattern.search(aLine):
        #find / to see if the line allready contains another source
        front,end = aLine.split("=")
        if re.search("[a-zA-Z]",end):
          writeFile.write(front+" = "+extendedClassName+".cc \\\n")
          writeFile.write("\t\t"+end)
        elif end.find('\\'):
          writeFile.write(front+" = "+extendedClassName+".cc \\\n")
        else :
          writeFile.write(front+" = "+extendedClassName+".cc\n")
          
      else:
        writeFile.write(aLine)

    elif type == "maincc":
      # find bin_PROGRAMS to start inserting mainsourcefiles
      if mainccPattern.search(aLine):        
        #find / to see if the line allready contains another source
        front,end = aLine.split("=")
        if re.search("[a-zA-Z]",end):
          writeFile.write(front+" = "+extendedClassName+" \\\n")
          writeFile.write("\t"+end)
          searchEnd=1
        elif end.find('\\'):
          writeFile.write(front+" = "+extendedClassName+" \\\n")
          searchEnd=1
        else :
          writeFile.write(front+" = "+className+"\n")
          writeFile.write(className+"_SOURCES = "+extendedClassName+".cc\n")

      elif searchEnd > 0:
        # there have been other mainprograms, so we need to look
        # for the end of all programName_SOURCES line to include our
        # new mainprogram SOURCES line
        if aLine.find("_SOURCES") < 0:
          writeFile.write(className+"_SOURCES = "+extendedClassName+".cc\n")
          searchEnd=0
        writeFile.write(aLine)  
      else:
        writeFile.write(aLine)

    elif type == "tcc":
      # find TCCHDRS to start inserting templatefiles
      if tccPattern.search(aLine):
        #find / to see if the line allready contains another source
        front,end = aLine.split("=")
        if re.search("[a-zA-Z]",end):
          writeFile.write(front+" = "+extendedClassName+".tcc \\\n")
          writeFile.write("\t"+end)
        elif end.find('\\'):
          writeFile.write(front+" = "+extendedClassName+".tcc \\\n")
        else :
          writeFile.write(front+" = "+extendenClassName+".tcc\n")
          
      else:
        writeFile.write(aLine)

    else:
      writeFile.write(aLine)
    
    aLine=readFile.readline()

  writeFile.close()
  readFile.close()
  os.unlink(srcDir+"/Makefile.am.old")
  
  
  

def usage():
  print "usage: "+sys.argv[0]+" [-h] [-m | -t list [-d]] className [className...]"
  print "args:  -h,--help               - print usage"
  print "       -m,--main               - make main class" 
  print "       -t,--templated list     - automated templated class"
  print "                                 list can contain a comma seperated list"
  print "                                 with the template parameters. Example:"
  print "                                 makeClass -t T,U className"
  print "       -d,--diy                - Do it yourself (manual template "
  print "                                 instanciation) Only together with -t"
  print "       className [className...]- name of the class(es) to be created."
  sys.exit(2)

def main(argv):
  noMain=1
  noTemplated=1
  autoTemplate=1
  className = "None"
  #
  # get Lofar base dir
  #
  file= os.popen("echo $PWD | sed -e 's%/LOFAR/.*%/LOFAR%'")
  lofarDir=str.replace(file.readline(),"\n","")
  file.close()
  baseDir = os.environ["PWD"]
  subDirName = ""  
  packageName = ""
  srcDir = ""
  os.path.basename(os.path.dirname(baseDir));
  
  # look if we are in a subdir within src
  if baseDir.find("src") > -1 :
    if os.path.basename(os.path.dirname(baseDir)) == "src":
      srcDir,subDirName=os.path.split(baseDir)
      packageName=os.path.basename(os.path.dirname(srcDir))
    elif os.path.split(baseDir)[1] != "src":
      print "Sorry, only one level of subdirs is allowed in src."
      usage()
    else:
      packageName=os.path.basename(os.path.dirname(baseDir))
      srcDir=baseDir
  else:
    print "You have to be in the srcdir or one of its subdirs to run this program."
    usage()

  try:
    opts, args = getopt.getopt(argv, "hdmt:",
                               ["help","diy","templated=","main"])
  except getopt.GetoptError:
    usage()
  for opt, arg in opts:
    if opt in ("-h", "--help"):
      usage()
    elif opt in ("-m", "--main"):
      noMain = 0
    elif opt in ("-t", "--templated"):
      noTemplated = 0
      templateList = str.split(arg,',')
    elif opt in ("-d", "--diy"):
      autoTemplate = 0

  if len(args) <= 0 and className == "None":
    usage()
    

  if noTemplated==0 and noMain==0:
    print "Sorry, no facility to generate a templated mainfile (yet)."
    usage()
  if len(sys.argv) < 1:
    usage()
  if autoTemplate==0 and noTemplated==1:
    print "Diy only makes sense in templated class."
    print "I will forget you gave this option, and continue.."
    

  #
  # Make a backup from the Original Makefile
  #
  os.system("cp "+srcDir+"/Makefile.am "+srcDir+"/Makefile.am.save")


  for className in args:

    #
    # print info
    #
    if noMain and noTemplated: 
      print "Trying to set up default class " + className + " for package " + packageName
    if noMain and noTemplated==0:
      print "Trying to set up default templated class " + className + " for package " + packageName
      if templateList == "":
        print "No templates provided, so only default template class will be created."
    if noMain==0:
      print "Trying to set up main class " + className + " for package " + packageName

    #
    # Check of given class name allready exists in the working directory as
    # directory or as file
    #
  
    if noMain and noTemplated: 
      if os.path.isfile(className+".h") or os.path.isfile(className+".cc"):
        print "Sorry, that class allready exists. Please take another name"
        sys.exit(1)
    if noMain and noTemplated==0:
      if autoTemplate==0:
        ext=".cc"
      else:
        ext=".tcc"
      if os.path.isfile(className+ext):
        print "Sorry, that name allready exists. Please take another one"
        sys.exit(1)
    if noMain==0:
      if os.path.isfile(className+"Main.cc"):
        print "Sorry, that name allready exists. Please take another one"
        sys.exit(1)
      if os.path.isfile(className+".h") == 0 or os.path.isfile(className+".cc") == 0:
        print "WARNING: the base classes for whom you are creating a Mainprogram"
        print "         are not available yet."
        print "         please remember that you have to create them.\n"

    #
    # Create all initial files from templates
    #
    if noMain and noTemplated:
      makeDefaultClass(lofarDir,className,packageName,srcDir,subDirName)
    if noMain and noTemplated==0:
      makeTemplatedClass(lofarDir,className,packageName,templateList,autoTemplate,srcDir,subDirName)
    if noMain==0:
      makeMainClass(lofarDir,className,packageName,srcDir,subDirName)

#
# this is the main entry
#
if __name__ == "__main__":
  main(sys.argv[1:])
  print "Done"
