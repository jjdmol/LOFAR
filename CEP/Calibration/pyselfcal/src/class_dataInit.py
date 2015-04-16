#!/usr/bin/env python

########################################################################
#                                                                      #
# Created by N.Vilchez (vilchez@astron.nl)                             #
# 12/02/2015                                                           #
#                                                                      #
########################################################################


########################################################################
# IMPORT general modules
########################################################################

import sys,os,glob
import pyrap.tables as pt
import numpy as np
import pyfits
import math
import time
import threading

########################################################################
#import Lofar modules
########################################################################

import lofar.bdsm as bdsm

########################################################################
## Define selfcalibration strategy, iteration, levels, perpare parsets
########################################################################



class dataInit:
	
	####################################################################
	# Preparation of the Data
	####################################################################

    def __init__(self,obsDir,outputDir,dataDir,Files,NbFiles):
    
			self.obsDir 	= obsDir
			self.outputDir	= outputDir
			self.dataDir	= dataDir
			
			self.Files		= Files
			self.NbFiles	= NbFiles
			
    def dataInitFunc(self):			

			
			# copy original data
			print ''
			cmd=""" cp -r %s* %s"""%(self.obsDir,self.dataDir)
			print cmd
			print ''
			os.system(cmd)	
	
					
			# check if CORRECTED_DATA column exists, if not take DATA 
			# column, and anyway copy CORRECTED_DATA column to DATA 
			# column
								
			for MS in self.Files:		
					# Copy CORRECTED DATA Column to DATA column		
					try:				
									
									t = pt.table("""%s%s"""%(self.dataDir,MS), readonly=False, ack=True)
									data = t.getcol('CORRECTED_DATA')
									t.close()								
							
					except:
									print ''
									print 'There are no CORRECTED DATA COLUMN !!'
									print 'Selfcal will copy by default your DATA column to the CORRECTED_DATA column'
									self.copy_data("""%s%s"""%(self.dataDir,MS))
									print ''
							
																				
									


			# Create NDPPP Inital Flag Directory
			self.NDPPPDir	= self.outputDir+"""NDPPP_Initial_Flag/"""
			if os.path.isdir(self.NDPPPDir) != True:
					cmd="""mkdir %s"""%(self.NDPPPDir)
					os.system(cmd)
					



			#############################################################
			# Change the execution directory to have the calibration log in the 
			# NDPPP directory
			#############################################################
			os.chdir(self.NDPPPDir)
	

			#############################################################	
			# NDPPP Runs on each time chunks
			#############################################################

	
			#############################################################
			# Parrallelization on time chunks
			#############################################################
		
			max_threads = 8
			delay 		= 2
			
		
			jrange	= range(self.NbFiles)
			counter	= 0
		
			for j in jrange:
						
				  # Wait until one thread has finished
				  while threading.activeCount() > max_threads:
					  time.sleep(delay)
				  
				  # Set argument typle
				  
				  index_initFlag		= '%s'%(j)					
				  files_initFlag		= self.Files[j]
				  param_initFlag		= """%sNDPPP_Init_timeChunk%s"""%(self.NDPPPDir,j)
				  
				  args = (index_initFlag,files_initFlag,param_initFlag)
				  
				  
				  # Process data
				  t = threading.Thread(target=self.process, args=args)
				  t.start()				
				  
				  
				  # Wait some time before next run is started
				  time.sleep(delay)
					
					
			# end of the parralelization, Wait until all threads have finished
				
			while threading.activeCount() > 1:
				  time.sleep(delay)					



	####################################################################
	# Parallelized NDPPP function
	####################################################################
	
    def process(self,index_initFlag,files_initFlag,param_initFlag):
				
				
			k = int(index_initFlag)				
		
			print ''
			print '##############################################'
			print 'Initial Flagging with NDPPP on Time chunk %s'%(k)
			print '##############################################\n'					
									
										
			#Create NDPPP parsetFile
			
			InitFlagfile = open(param_initFlag,'w')
																
			cmd1 	="""msin = %s%s\n"""%(self.dataDir,files_initFlag)
			cmd2 	="""msout=.\n"""								
			cmd3  	="""msin.autoweight = false\n"""
			cmd4  	="""msin.forceautoweight = false\n"""
			cmd5  	="""msin.datacolumn = CORRECTED_DATA\n"""
			cmd6  	="""steps=[preflag,count,flag1]\n"""
			cmd7 	="""preflag.type=preflagger\n"""
			cmd8 	="""preflag.corrtype=auto\n"""
			cmd9 	="""preflag.baseline= [DE*, FR*, SE*, UK*]\n"""
			cmd10 	="""preflag.amplmin = 1e-30 \n"""
			cmd11  	="""flag1.type=aoflagger\n"""
			
			InitFlagfile.write(cmd1)
			InitFlagfile.write(cmd2)
			InitFlagfile.write(cmd3)
			InitFlagfile.write(cmd4)
			InitFlagfile.write(cmd5)				
			InitFlagfile.write(cmd6)
			InitFlagfile.write(cmd7)
			InitFlagfile.write(cmd8)
			InitFlagfile.write(cmd9)
			InitFlagfile.write(cmd10)
			InitFlagfile.write(cmd11)

			
			InitFlagfile.close()			
			
			#Run Initial flaging with NDPPP
			
			cmd_NDPPP = """NDPPP %s"""%(param_initFlag)
			print ''
			print cmd_NDPPP
			print ''
			os.system(cmd_NDPPP)
				
			print ''
			print '##############################################'
			print 'End of initial Flagging with NDPPP NDPPP on Time chunk %s'%(k)
			print '##############################################\n'

					

	####################################################################	
	# Additionnal Internal function
	####################################################################	
		
	
    def copy_data(self,inms):
		
			# Create corrected data colun and Put data to corrected data column 
			t = pt.table(inms, readonly=False, ack=True)
			data = t.getcol('DATA')
			pt.addImagingColumns(inms, ack=True)
			t.putcol('CORRECTED_DATA', data)
			t.close()	


