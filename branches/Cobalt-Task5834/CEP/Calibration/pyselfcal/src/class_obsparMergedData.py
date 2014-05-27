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




class observationMergedDataParam:

    def __init__(self,obsDir):
    
	self.obsDir	= obsDir

 
 
	############################################################################################################
	# Observation parameter for merged data
	############################################################################################################	
	
    def obsParamMergedDataFunc(self):
    
		##############################
		# generate the list of files
		listFiles	= sorted(glob.glob(self.obsDir+'*'))
		NbFiles		= len(listFiles)
		
		if NbFiles == 1:
			k			= range(NbFiles)
		else:
			k			= range(NbFiles-1)			
		
		
		k1			= range(NbFiles)		
		Files		= range(NbFiles)	
		 
		i = 0 
		  
		for line in listFiles:
			
				splinter1		= self.obsDir
				linesplit		= line.split(splinter1)
				Files[i]		= linesplit[1]				
				i = i+1			
	
		
		
					
		# check about the number of channels of the data
		for i in k: 

			tab00	= pt.table(listFiles[i]+'/SPECTRAL_WINDOW')
			freq0		= tab00.getcell('REF_FREQUENCY',0)
			nbchan0		= tab00.getcell('NUM_CHAN',0)
			
			if NbFiles == 1:			
				freq1		= freq0
				nbchan1		= nbchan0
			else:
				tab01	= pt.table(listFiles[i+1]+'/SPECTRAL_WINDOW')			
				freq1		= tab01.getcell('REF_FREQUENCY',0)
				nbchan1		= tab01.getcell('NUM_CHAN',0)				

			
			
			if freq0 != freq1:
				
				print ''
				print '#################' 
				print """Time chunk %s and %s have not the same reference frequency! \n%s MHz and %s MHz  respectively"""%(Files[i],Files[i+1],freq0/1E6,freq1/1E6)
				print '#################'
				print ''
				sys.exit(2)
				
			if nbchan0 != nbchan1:
				print ''
				print '#################' 
				print """Time chunk %s and %s have not the same number of channels! \n%s and %s channels respectively"""%(Files[i],Files[i+1],nbchan0,nbchan1)
				print '#################'
				print ''
				sys.exit(2)				
		

		# Determine observation parameters: time for One time chunk, integration time during the observation, UVmin to apply if under +35 degree
		tabstart0	= pt.table(listFiles[0]+'/OBSERVATION')
		start0		= tabstart0.getcell('LOFAR_OBSERVATION_START',0)
		end0		= tabstart0.getcell('LOFAR_OBSERVATION_END',0)
		trange		= tabstart0.getcell('TIME_RANGE',0)
		
		
		tabtime		= pt.table(listFiles[0])
		tinterval	= tabtime.getcell('INTERVAL',0)		
		
		tabtarget	= pt.table(listFiles[0]+'/FIELD')
		coords		= tabtarget.getcell('REFERENCE_DIR',0)
		target		= coords[0]*180./3.14
		
		
		UVmin=0
		if target[1] <= 35:
		    UVmin=0.1
		    
		ra_target	= target[0]+360.
		dec_target	= target[1]
		
		
		# determine number of channels and the reference frequency, integtime for a time chunk and observation integration time
		frequency				= freq1
		nbChan					= nbchan1
		integTimeOnechunk		= end0-start0
		observationIntegTime	= tinterval
			

		# Determine the longuest baseline of the data set 
		dist = range(NbFiles) 			
		for i in k1: 
				tab0 	= pt.table(listFiles[i], readonly=False, ack=True)
				pos0	= tab0.getcol('UVW')
				dist[i] = max(pos0[:,0]**2+pos0[:,1]**2)**0.5 	
				
		maxBaseline	= max(dist)


		print 'List Of Files: %s\n'%(listFiles)
		print ''	
		print 'Number of subbands (1 channel/subband): %s\n'%(nbChan)
		print ''
		print 'Averange Frequency: %s MHz\n'%(fpformat.fix(frequency/1E6,3))
		print ''
		print 'Longuest baseline in the data: %s meters\n'%(maxBaseline) 
		print ''
		print 'Target coordinates: ra=%s deg, dec=%s deg'%(ra_target,dec_target)
		print ''
		
		
		
		
		
		return 	listFiles,Files,NbFiles,nbChan,frequency,maxBaseline,integTimeOnechunk,observationIntegTime,UVmin,ra_target,dec_target
					
								
	
	
	
	
	
	

 
 
