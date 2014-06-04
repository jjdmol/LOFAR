#!/usr/bin/env python



# IMPORT general modules

import sys
import glob
import os
import pyrap.tables as pt
import numpy as np
import fpformat



######################################################################
## Define observation directory parameters (NbTimechunk, NbSB etc ...)
######################################################################




class observationParam:

    def __init__(self,obsDir):
    
	self.obsDir	= obsDir

 
 
	#####################################################
	# Observation parameter for data to merge in subbands
	##################################################### 
 
 
    
    def obsParamFunc(self):
    
    
	##############################
	# generate the list of files
	listFiles	= sorted(glob.glob(self.obsDir+'*'))
	NbFiles		= len(listFiles)
	
	
	##############################	
	# check type of observation: intermediate data or final data product not the same nomenclature
	splinter1	= self.obsDir
	splinter2	= '_uv.'
	
	File0split1	= listFiles[0].split(splinter1)
	File0_1		= File0split1[1]
	
	File0split2	= File0_1.split(splinter2)
	File0_2		= File0split2[1]
	
	
	if File0_2 == 'MS.dppp':
	    obsType = 'intermediateData'
	if File0_2 == 'dppp.MS':
	    obsType = 'finalData'
	 
	 
	if obsType != 'intermediateData' and  obsType != 'finalData':
	    print ''
	    print 'The observation format is not valid:\nMust be Lxxxxxx_SBxxx_uv.dppp.MS or Lxxxxxx_SAPxxx_SBxxx_uv.MS.dppp\n'
	    print ''
	    sys.exit(2)
	    
	    
	    
	##############################	    
	# Check if intermediate and final data product are mixed
	
	kobsType	= range(NbFiles)
	i=0
	
	for line in listFiles:
	    File0split1	= listFiles[i].split(splinter1)
	    File0_1		= File0split1[1]
	    
	    File0split2		= File0_1.split(splinter2)
	    kobsType[i]		= File0split2[1]	  
	    i=i+1
	
	krange	= range(NbFiles-1)
	i=0
	for i in krange:
	    if kobsType[i] != kobsType[i+1]:
		print ''
		print 'Observation directory contains intermediate data (Lxxxxxx_SAPxxx_SBxxx_uv.MS.dppp) and \nfinal data products (Lxxxxxx_SBxxx_uv.dppp.MS)\n\nIt is not allowed to merge this 2 kind of data'
		print ''
		sys.exit(2)
	
	      
	##############################	    
	#Determine IDs and SBs
	
	Files		= range(NbFiles)
	NameFiles	= range(NbFiles)
	
	IDsTemp		= range(NbFiles)
	SBsTemp		= range(NbFiles)	
	SBs			= range(NbFiles)
	
	SAPs		= range(NbFiles)
	
	i=0
	
	if obsType == 'intermediateData':
	  
	    for line in listFiles:
		splinter1		= self.obsDir
		linesplit		= line.split(splinter1)
		Files[i]		= linesplit[1]
		
		splinter2		= '_uv.MS.dppp'
		filesplit		= Files[i].split(splinter2)
		NameFiles[i]		= filesplit[0]
		
		splinter3		= 'SB'
		NameFilesplit		= NameFiles[i].split(splinter3)
		SBsTemp[i]		= NameFilesplit[1]
		SBs[i]			= float(SBsTemp[i])
		
		splinter4		= '_SAP'
		filesplit2		= Files[i].split(splinter4)
		IDsTemp[i]		= filesplit2[0]
		
		splinter5		= '_SAP'
		splinter6		= '_SB'
		NameFilesplit1		= NameFiles[i].split(splinter5)
		NameFilesplit2		= NameFilesplit1[1].split(splinter6)
		SAPs[i]			= NameFilesplit2[0]		
				
		i = i+1	
	
	# Check if there are several SAPs in the same directory
	if obsType == 'intermediateData':	
	      i=0
	      for i in krange:
		  if SAPs[i] != SAPs[i+1]:
		      print ''
		      print 'Several different SAPs found in the same directory\nIt is not allowed to merge several different SAPs\n'
		      print ''
		      sys.exit(2)			
				
	
	if obsType == 'finalData':
	  
	    for line in listFiles:
		splinter1		= self.obsDir
		linesplit		= line.split(splinter1)
		Files[i]		= linesplit[1]
		
		splinter2		= '_uv.dppp.MS'
		filesplit		= Files[i].split(splinter2)
		NameFiles[i]		= filesplit[0]
		
		splinter3		= 'SB'
		NameFilesplit		= NameFiles[i].split(splinter3)
		SBsTemp[i]		= NameFilesplit[1]
		SBs[i]			= float(SBsTemp[i])
		
		splinter4		= '_SB'
		filesplit2		= Files[i].split(splinter4)
		IDsTemp[i]		= filesplit2[0]
		
		i = i+1		
		
		
	# Subbands determination
	NbSB		= int(max(SBs)-min(SBs)+1)
	StartSB		= int(min(SBs))
	EndSB		= int(max(SBs))
	
	
	
	# SAP Id determination
	if obsType == 'intermediateData':
	      SAPId	= SAPs[0]
	else:
	      SAPId	= ''
	      
	

	# IDs determination

	IDs 	= [IDsTemp[0]]
	
	i = 0
	for i in krange:
	    
	    if IDsTemp[i] != IDsTemp[i+1]:
			IDs.append(IDsTemp[i+1])
				 		    
		
	print 'List Of Files to merge: %s\n'%(listFiles)
	print ''	
	print 'Number of subbands: %s\n'%(NbSB)
	print ''
	print 'First subband: %s, Last subband: %s'%(StartSB,EndSB)
		
		    
	
	#Return value
	return listFiles,obsType,IDs,SAPId,NbSB,StartSB,EndSB
	
	
	
