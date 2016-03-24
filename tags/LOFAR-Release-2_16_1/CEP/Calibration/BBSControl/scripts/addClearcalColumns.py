!#/usr/bin/env python
#
# Script that creates (if not already present) MODEL_DATA and CORRECTED_DATA
# columns in a MS
#
# These columns are expected to be present by the casapy imager task, and
# if any of the two is missing any existing MODEL_DATA or CORRECTED_DATA
# will be overwritten, thus destroying the result of a previous BBS run
#
# File:           addClearcalColumns.py
# Author:         Sven Duscha (duscha@astron.nl)
# Date:           2011-02-10
# Last change:    2011-02-10


import sys
import os
import pyrap.tables as pt

# This script does not do any command argument parsing with getopt
# Simply: 1st argument (sys.arg[1]) is the MS to add the columns to
if sys.argc != 2:
   print "usage: ", sys.argv[0], "<MSfile>"
   sys.exit(0)
else:
   filename = sys.argv[1]


# Check if we got a gds file or a single MS, i.e. a directory
# or if it is a gds file, i.e. text file
#
if os.path.isdir(filename)       # directory, i.e. single MS
   addClearcalColumns(filename)
else:                            # gds
   executeGDS(filename)


# Check if a particular column is in the table
#
# returns true/false accordingy
#
def hasColumn(table, name):
   if name is in table.colnames():
      return True
   else:
      return False


# High-level function to add the necessary MODEL_DATA
# and/or CORRECTED_DATA columns to a MS
#
def addClearcalColumns(filename):
   # Open MS for read/write
   table=pt.table(msfile, readonly=False)

   # Check if MODEL_DATA column is already present
   if hasColumn(table, "MODEL_DATA") == False:      
      print "addClearcalColumns() adding MODEL_DATA"     # DEBUG
      table.addcolumn("MODEL_DATA")

      nFreq=getNFreqs(table)
      channelSelection=np.zeros(2, dtype=int)     # array to write CHANNEL_SELECTION keyword data
      channelSelection[1]=nFreq                   # [0,nFreq]

      # put Keyword: "CHANNEL_SELECTION"
      table.putkeyword("CHANNEL_SELECTION", channelSelection)

   # Check if CORRECTED_DATA column is already present
   if hasColumn(table, "CORRECTED_DATA") == False:
      print "addClearcalColumns() adding CORRECTED_DATA"     # DEBUG
      table.addcolumn("CORRECTED_DATA")


# Add MODEL_DATA and CORRECTED_DATA columns to all files
# specified in the gds
#
def executeGDS(filename):
   print "executeGDS()"      # DEBUG
   
   MSfilenames=parseGD(filename)
   multipleFiles(MSfilenames)
   
   
# Helper function to get the number of frequencies in 
# the MS
#   
def getNFreqs(table):
   return pt.tablecolumn(tab, 'DATA').getdesc()['shape'][0]


# Parse gds file to get list of MS filenames
#
# returns list of individual MS filenames
#
def parseGDS(gdsFilename):
   print "parseGDS()"   # DEBUG

   MSfilenames=[]

   gds_fd=open(gdsFilename, 'r')
   lines=gds_fd.readlines()          # read in entire gds file

   for line in lines:                # get "NParts" (number of parts)
      if line.find("NParts"):
         numParts=
      else:
         raise ValueError
         
   # Look for Partx.Name, e.g. Part0.FileName, and add to list of filenames
   for line in lines:
      if line.find("Part") and line.find(".Filename"):
         pos=line.find("=")+2       # because it is " = " with space
         MSfilename.append(line[pos:])

   print "parseGDS() MSfilenames = ", MSfilenames    # DEBUG

   return MSfilenames    # return list of filenames


# Add columns to multiple files in list MSfilenames        
#        
def multipleFiles(MSfilenames):
   if !isinstance(MSfilename, list):
      print "multipleFiles() MSfilenames is not a list"
   else:
      for filename in MSfilenames:
         addClearcalColumns(filename)
