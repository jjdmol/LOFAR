#!/usr/bin/env python
#
# Python script that tries to check, if a BBS sky model file
# is syntactically correct
#
# File:           checkBBSskymodel.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2011-02-18
# Last change:    2011-02-24
#
# This file parses a BBS sky model file .sky or .model and tries
# to find potential errors in it and hint users to the possible cause
# It does not look for physical correctness of the given parameters, though
#

import sys
import os
import re      # regular expressions for string checking

# Display usage information for this script
#
def usage():
   print "Usage: ", sys.argv[0], "<skymodelfile>"
   print

#********************************************
#
# Main function   
#
#********************************************

def main():
   if len(sys.argv) != 2:     # take only one sys arg: filename of file to check
      usage()
   else:
      filename=sys.argv[1]    # filename is first argument

   try:   
      sky_fh=open(filename, 'r')
   except:
      raise IOError

   # DEBUG Testing colours
   #print bcolors.WARNING + "WARNING"
   #print bcolors.OKBLUE + "OKBLUE"
   #print bcolors.OKGREEN + "OKGREEN"
   #print bcolors.WARNING + "WARNING"
   #print bcolors.FAIL + "FAIL"
   #print bcolors.ENDC + "ENDC"


   lines=sky_fh.readlines()
   correct=parseSkymodel(lines)
 
   # Indicate result of the test
   if correct==True:
      print bcolors.OKGREEN + "skymodel " + filename + " passed test." 
      print bcolors.ENDC
   else:
      print bcolors.FAIL + "skymodel " + filename + " contains above errors." 
      print bcolors.ENDC
 
# Parse the required format line (also check if it is on one line)
#
def parseFormat(line):
   #print "parseFormat()"   # DEBUG

   minNumFields=15         # Definitions for a correct format line
   correct=True            # error indicator
   #line=[]

   # Check for # format line at the beginning (must be one line)
   correct=checkFormatOneLine(line)  # checks 0:5 line
   
   #print "correct = ", correct   # DEBUG
   
   #line=lines[0]   # then only look at first line
   #print "parseFormat() type(line) = ", type(line), " line = ",line
   
   # Check for required fields
   # (Name, Type, Ra, Dec, I, Q, U, V, ReferenceFrequency='60e6', SpectralIndexDegree='0', SpectralIndex:0='0.0', MajorAxis, MinorAxis, Orientation) = format
   if line.find("Name")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "format line missing 'Name'"
      correct=False
   if line.find("Type")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "format line missing 'Type'"
      correct=False
   if line.find("Type")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "format line missing 'Type'"
      correct=False
   if line.find("Ra")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "format line missing 'Ra'"
      correct=False
   if line.find("Dec")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "format line missing 'Dec'"
      correct=False
   if line.find("I")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "format line missing 'Q'"
      correct=False
   if line.find("Q")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "format line missing 'Q'"
      correct=False
   if line.find("U")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing 'U'"
      correct=False
   if line.find("V")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing 'V'"
      correct=False
   if line.find("ReferenceFrequency")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing 'ReferenceFrequency'"
      correct=False
   if line.find("SpectralIndexDegree")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing 'SpectralIndexDegree'"
      correct=False
   if line.find("SpectralIndex")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing 'SpectralIndex'"
      correct=False
   if line.find("MajorAxis")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing MajorAxis"
      correct=False
   if line.find("MinorAxis")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing MinorAxis"
      correct=False
   if line.find("Orientation")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing Orientation"
      correct=False
      
   # Check if there is a =format et the end
   if line.find("= format")==-1:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " format line missing '= format' at the end"
      correct=False

   # Check if all parameters are separated with commas
   splits=line.split(",")
   # num of commas must be equal to (num of splits) - 4 (the beginning and ending splits are not parameters)
   if getNumComma(line) < len(splits)-4:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " missing ',' in format line"
      print line
      correct=False



   #print "parseFormat() correct = ", correct    # DEBUG
      
   return correct
   


# Crude check if format line is on one line
#
def checkFormatOneLine(lines):
   index=0
   correct=True
   formatKeywords=['Name', 'Type', 'Ra,', 'Dec,', 'I,', 'Q,', 'U,', 'V,', 'ReferenceFrequency', 'SpectralIndexDegree', 'SpectralIndex:0', 'MajorAxis', 'MinorAxis,Orientation', 'format']

   #if lines[0][0]!='#':
   #   print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "first line must be format line and start with a #"
   #   print "Should be: # (Name, Type, Ra, Dec, I, Q, U, V, ReferenceFrequency='60e6', SpectralIndexDegree='0', SpectralIndex:0='0.0', MajorAxis, MinorAxis, Orientation) = format"
   #   print "Yours is: ", line
   #   correct=False

   # We do this that we test the first five lines (AFTER top line) for keywords
   # that should be in the first line!
   for line in lines[1:5]:
      for keyword in formatKeywords:
         if keyword in line and line[0]!="#":   # ignore comments
            #print "keyword = ", keyword , "line[0] = ", line[0]        # DEBUG
            #print "line = ", line                                      # DEBUG
            print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "first line must be format line and MUST be one line" 
            print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + "line No. " + bcolors.FAIL + str(index) + bcolors.ENDC + ": " + line
            correct=False
            return correct
      index=index+1
      
   return correct
   
   
# High-level function to parse the sky model file
#
def parseSkymodel(lines):
   #print "parseSkymodel()"   # DEBUG
   
   correct=False             # indicate that skymodel file is deemed correct
   lineIndex=1;              # line index to indicate in which line we have an error
   minNumFields=8            # 8 fields: Name, Type, RA, Dec, I, Q, U, V

   # Do checks
   #
   correct=checkFormatOneLine(lines)
   if correct==False:
      return correct
   correct=parseFormat(lines[0])     # parse the first line of the skymodel file (format line MUST be first line)
   if correct==False:
      return correct
 
   # Check source definition lines
   for line in lines:
      #print "line: ", line   # DEBUG
      
      # keep a list of known sources we already encountered in the file
      fileSources=[]
      
      # Generic rules
      # Skip line if we encounter a comment '#'
      if line.find('#') != -1 or line=='':
         lineIndex=lineIndex+1
         continue                # continue with next line   
      
      # Check if we have minimum number of fields
      fields=line.split(',')     
      stripall(fields)
      
      numFields=len(fields)
      # Skip empty lines
      if numFields <= 1:      # empty line
         lineIndex=lineIndex+1
         continue

      if numFields < minNumFields:
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + ") missing required field"
         print line
         correct=False

      # Check if we have enough commata
      numComma=getNumComma(line)      
      #print "numComma = ", numComma, " numFields = ", numFields
      if numComma < (numFields-1):
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + "): missing ',' in"
         print line           
         correct=False
      

      # Check if we have no duplicate source definition
      sourceName=fields[0].strip(',').strip()  # get sourceName without ',' and without whitespaces
      
      # Check if it is not one of the defined types (how else can we recognize 'wrong'
      # source identifiers, users are practically allowed to use an number or character...
      #
      if sourceName in ['POINT', 'GAUSSIAN', 'SHAPELET']:   # SHAPELET is still experimental
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + ") invalid source identifier '" +  sourceName, "'"
         print line
         correct=False
         
      if sourceName in fileSources:
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + "): duplicate source definition '" + sourceName+ "'"
         print line
         correct=False
      else:      
         fileSources.append(sourceName)

      correct=checkPositionFields(line, lineIndex)

      fields=stripall(fields)
      type=fields[1].strip()             # get source type (second entry)

      # Scan parameters for specific types of sources     
      if type == "POINT":
         #print "POINT source"
         checkPointSource(line)
      elif type == "GAUSSIAN":
         #print "GAUSSIAN source"
         checkGaussianSource(line)
      elif type == "SHAPELET":
         print "SHAPELET source"
      else:                # if we encounter an unknown source type...
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + "): unknown source type"
         print line
         correct=False
         
      # If we did all the tests on that specific source definition line
      lineIndex=lineIndex+1   # increment line index to show error for right line

      #print "parseSkymodel() correct = ", correct    # DEBUG
      return correct

   
# Check RA, Dec fields (they must be the third and fourth field)
#
def checkPositionFields(line, lineIndex):
   #print "checkPositionFields()"    # DEBUG

   index=0           # index string was searched up to
   correct=True      # result of this check
   colons=0          # number of colons found
   dots=0            # number of dots found
   RA=""             # Right Ascension string
   Dec=""            # Declination string
   type="Hours"      # type of coordinates (defaults to Hours)

   fields=line.split(',')
   fields=stripall(fields)
   
   RA=fields[2]                            # Assume RA is third field
   Dec=fields[3]                           # Assume Dec is fourth field
   
   # RA
   # We need either Hours:Mins:Seconds or Degrees.min.arcmins in RA
   for iter in re.finditer(':', RA):
      colons=colons+1
   for iter in re.finditer('\.', RA):
      dots=dots+1
    
   #print "colons = ", colons, " dots = ", dots # DEBUG
   
   # Determine what user 'intended' to enter
   #
   if colons > dots and not (colons==2 and dots==1):    # hours
      if colons!=3:
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + "): error in RA definition " + str(RA)
         print line
         correct=False

   else:                               # degrees
      #print "User defined degrees"   # DEBUG
      if colons!=2 and dots==1:
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + "): error in RA definition " + str(RA)
         print line
         correct=False
  
   # Declination is only allowed to be in degrees
   dots=0         # need to reset this before checking declination
   for iter in re.finditer('\.', Dec):
      dots=dots+1
   if dots!=3:
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + "): error in Dec definition " + str(Dec)
         print line
         correct=False
   
   return correct


# TODO: Check RA, Dec, I,Q,U,V for physical meaningful values?
# Meaningful limits for the spectral indices?
#
# Check a POINT source for its criteria
#
def checkPointSource(line):
   #print "checkPointSource()"    # DEBUG
   
   check=False
   minNumFields=8
   
   # Check if we have the minimum number of fields
   # This might differ for this specific type of source
   # from the minNumFields checked for in the generic check
   #
   fields=line.split(',')
   numFields=len(fields)
   
   #print "numFields = ", numFields, " minNumFields = ", minNumFields   # DEBUG
   
   if numFields < minNumFields:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + ") missing required field '"
   
   # Strip off the first four fields that already were checked before
   fields=fields[4:]
   # Check remaining fields if they are numerical (no check
   # physical meaningful values is done for now!)
   fields=stripall(fields)
   for field in fields:
      if isnumeric(field) == False:
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + ") must be numeric: " + str(field)
         print line
         check=False
         
   return check
 
 
# Check a Gaussian source for its criteria
# TODO: What else can we check here for consistency (MajorAxis > MinorAxis???)
#
def checkGaussianSource(line):
   #print "checkGaussianSource()" # DEBUG
   
   correct=False
   
   # Check if we have the minimum number of fields      
   minNumFields=14
   
   # Check if we have the minimum number of fields
   # This might differ for this specific type of source
   # from the minNumFields checked for in the generic check
   #
   fields=line.split(',')
   numFields=len(fields)
   if numFields < minNumFields:
      print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + ") missing required field '"
   
   # Strip off the first four fields that already were checked before
   fields=fields[4:]
   # Check remaining fields if they are numerical (no check
   # physical meaningful values is done for now!)
   for field in fields:
      field=field.strip(',').strip()
      if isnumeric(field) == False:
         print bcolors.FAIL + "checkBBSskymodel: " + bcolors.ENDC + " (line " + str(lineIndex) + ") must be numeric: " + str(field)
         print line
         correct=False
   
   return correct
   

#************************************
#
# Helper functions
#
#************************************


# Determine if a string is numeric (is there really no built-in function
# in python for that???)
#
def isnumeric(value):
   try:
      a=float(value)
      return True
   except:
      return False


# Get the number of commas on the line
#
def getNumComma(line):
   numComma=0
   for m in re.finditer(',', line):
      numComma=numComma+1
   return numComma



# Strip a list of fields from ',' and whitespaces
#
def stripall(fields):
   return [field.strip(',').strip() for field in fields]


# Class to call escape sequences to set terminal colours
#
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''

   
# main function entry point
#
if __name__ == "__main__":
   main()
