#!/usr/bin/env python

########################################################################
#                                                                      #
# Created by N.Vilchez (vilchez@astron.nl)                             #
# 14/11/2014                                                           #
#                                                                      #
########################################################################


########################################################################
# IMPORT general modules
########################################################################

import sys, glob, os
import pyrap.tables as pt
import numpy as np
import fpformat
import math
import threading
import time

########################################################################
#import Lofar modules
########################################################################

import lofar.bdsm as bdsm

########################################################################
# Preprocessing (annulus out the FOV) independant directrion subtraction
########################################################################



class selfcalPeeling:

    def __init__(self,obsDir,outputDir,preprocessDir,preprocessIndex,peelingDir,peelingDataDir,peelingImageDir,peelingSkymodelDir,peelingBBSDir,peelingNDPPPDir,listFiles,Files,NbFiles,frequency,UVmin,nIteration,ra_target,dec_target,outerfovclean,skymodel2Peel,FOV):

		################################################################	    
		# Initialization
		################################################################	

    
		self.obsDir					= obsDir
		self.outputDir				= outputDir
		self.preprocessDir			= preprocessDir
		self.preprocessIndex		= preprocessIndex
		self.peelingDir				= peelingDir
		self.peelingDataDir			= peelingDataDir
		self.peelingImageDir		= peelingImageDir
		self.peelingSkymodelDir		= peelingSkymodelDir
		self.peelingBBSDir			= peelingBBSDir
		self.peelingNDPPPDir		= peelingNDPPPDir
		
		self.BBSParset				= '%sBBSPeelingParset'%(self.peelingBBSDir)	
		self.GSMSkymodel			= ''
			
		self.listFiles				= listFiles
		self.Files					= Files
		self.NbFiles				= NbFiles
		self.frequency				= frequency
		self.UVmin					= UVmin
		self.nIteration				= nIteration
		self.ra_target				= ra_target
		self.dec_target				= dec_target
		self.outerfovclean			= outerfovclean
		self.skymodel2Peel			= skymodel2Peel
		self.FOV					= FOV
		
				
		
		################################################################	    
		# Default parameter for HBA and LBA obs => changes for High HBA
		################################################################	    
				
		self.thresh_isl	= 6
		self.thresh_pix	= 8
		
		
		################################################################	    
		# Copy the data
		################################################################	    
		

		# copy outerfovcleaned data (annulus substraction)
		if self.outerfovclean == 'yes': 

			print ''							
			cmd=""" cp -r %s %s"""%("""%sIter%s/*sub%s"""%(self.preprocessDir,self.preprocessIndex,self.preprocessIndex),self.peelingDataDir)
			print cmd
			print ''
			os.system(cmd)		
			print 'toto'
		
		
		# copy original data
		if self.outerfovclean =='no':
					
					print ''
					cmd=""" cp -r %s* %s"""%(self.obsDir,self.peelingDataDir)
					print cmd
					print ''
					os.system(cmd)	
	
					
					# check if CORRECTED_DATA column exists, if not take DATA 
					# column, and anyway copy CORRECTED_DATA column to DATA 
					# column
								
					for MS in self.Files:		
							# Copy CORRECTED DATA Column to DATA column		
							try:				
									
									t = pt.table("""%s%s"""%(self.peelingDataDir,MS), readonly=False, ack=True)
									data = t.getcol('CORRECTED_DATA')
									t.close()								
							except:
									print ''
									print 'There are no CORRECTED DATA COLUMN !!'
									print 'Selfcal will copy by default your DATA column to the CORRECTED_DATA column'
									self.copy_data("""%s%s"""%(self.peelingDataDir,MS))
									print ''
							
							
							self.copy_data_invert("""%s%s"""%(self.peelingDataDir,MS))	




########################################################################	
# No Skymodel provided => extract the strongest source from VLSS
########################################################################	


    def selfCalPeelingVLSS(self):

		########################
		# Extract a GSM Skymodel
		self.GSMSkymodel		= self.peelingSkymodelDir+'InitialSkymodel2Peel'

		# If the Iniatial Skymodel is extracted from VLSS catalog
		if self.frequency >1.9E8:
				# VLSS skymodel for High HBA		
				cmd="""gsm.py %s %s %s 3 0.5 0.01"""%(self.GSMSkymodel,self.ra_target,self.dec_target)
				print ''
				print cmd
				print ''
				os.system(cmd)						
		else:
				# VLSS skymodel for LBA/HBA		
				cmd="""gsm.py %s %s %s %s 0.5 0.01"""%(self.GSMSkymodel,self.ra_target,self.dec_target,self.FOV)
				print ''
				print cmd
				print ''
				os.system(cmd)		


		#######################
		# Extract a GSM Skymodel and 
		# find strongest source

		inlistGSM 		= open(self.GSMSkymodel,'r').readlines()
		nbGSMlines		= len(inlistGSM)

		Name			= range(nbGSMlines-3)
		I				= range(nbGSMlines-3)
				
		i=0
		
		for line in inlistGSM:
			if i >= 3:
				linesplit		= line.split(',')
				Name[i-3]		= linesplit[0]		
				I[i-3]			= linesplit[4]												
			i=i+1	

		highestFluxName	= Name[I.index(max(I))]
		Name.remove(highestFluxName)

		print highestFluxName

		#######################
		# Create the peeling parset 
	
	
		cmd1	= 'Strategy.InputColumn = DATA\n'
		cmd2	= 'Strategy.UseSolver = F\n'
		cmd3	= 'Strategy.Steps = [subtractfield, add, solve, subtract, add_sources]\n'
		cmd4	= 'Strategy.ChunkSize = 100\n'
		cmd5	= 'Strategy.Baselines = [CR]S*&\n'
		cmd6	= '\n'
		cmd7	= 'Step.subtractfield.Operation = SUBTRACT\n'
		cmd8	= 'Step.subtractfield.Model.Sources = []\n'
		cmd9	= 'Step.subtractfield.Model.Beam.Enable = T\n'
		cmd10	= 'Step.subtractfield.Model.Beam.Mode = ARRAY_FACTOR\n'
		#cmd10b	= 'Step.subtractfield.Output.Column = CORRECTED_DATA\n'
		cmd11	= '\n'
		cmd12	= 'Step.add.Operation = ADD\n'
		cmd13	= 'Step.add.Model.Sources = [%s]\n'%(highestFluxName)
		cmd14	= 'Step.add.Model.Beam.Enable = T\n'
		cmd15	= 'Step.add.Model.Beam.Mode = ARRAY_FACTOR\n'
		#cmd15b	= 'Step.add1.Output.Column = MODEL_DATA\n'
		cmd16	= '\n'
		cmd17	= 'Step.solve.Operation = SOLVE\n'
		cmd18	= 'Step.solve.Model.Sources = %s\n'%(highestFluxName)
		cmd18b	= 'Step.solve.Model.DirectionalGain.Enable = T\n'
		cmd19	= 'Step.solve.Model.Cache.Enable = T\n'
		cmd20	= 'Step.solve.Model.Beam.Enable = T\n'
		cmd21	= 'Step.solve.Model.Beam.Mode = ARRAY_FACTOR\n'
		cmd22	= 'Step.solve.Model.Phasors.Enable = F\n'
		cmd23	= 'Step.solve.Solve.Mode = COMPLEX\n'
		cmd24	= 'Step.solve.Solve.Parms = ["DirectionalGain:0:0:*","DirectionalGain:1:1:*"]\n'
		cmd25	= 'Step.solve.Solve.CellSize.Freq = 0\n'
		cmd26	= 'Step.solve.Solve.CellSize.Time = 5\n'
		cmd27	= 'Step.solve.Solve.CellChunkSize = 10\n'
		cmd28	= 'Step.solve.Solve.Options.MaxIter = 150\n'
		cmd29	= 'Step.solve.Solve.Options.EpsValue = 1e-9\n'
		cmd30	= 'Step.solve.Solve.Options.EpsDerivative = 1e-9\n'
		cmd31	= 'Step.solve.Solve.Options.ColFactor = 1e-9\n'
		cmd32	= 'Step.solve.Solve.Options.LMFactor = 1.0\n'
		cmd33	= 'Step.solve.Solve.Options.BalancedEqs = F\n'
		cmd34	= 'Step.solve.Solve.Options.UseSVD = T\n'
		cmd35	= '\n'
		cmd36	= 'Step.subtract.Operation = SUBTRACT\n'
		cmd37	= 'Step.subtract.Model.Sources = %s\n'%(highestFluxName)
		cmd38	= 'Step.subtract.Model.Phasors.Enable = F\n'
		cmd39	= 'Step.subtract.Model.Beam.Enable = T\n'
		cmd40	= 'Step.subtract.Model.Beam.Mode = ARRAY_FACTOR\n'
		#cmd40b	= 'Step.subtract.Output.Column=CORRECTED_DATA\n'
		cmd41	= '\n'
		cmd42	= 'Step.add_sources.Operation = ADD\n'
		cmd43	= 'Step.add_sources.Model.Sources = %s\n'%(Name)
		cmd44	= 'Step.add_sources.Model.Beam.Enable = T\n'
		cmd45	= 'Step.add_sources.Model.Beam.Mode = ARRAY_FACTOR\n'
		cmd45b	= 'Step.add_sources.DirectionalGain.Enable = F\n'
		cmd46	= 'Step.add_sources.Output.Column = CORRECTED_DATA\n'

			
		bbsFile 		= open(self.BBSParset,'w')
		
		
		bbsFile.write(cmd1)
		bbsFile.write(cmd2)
		bbsFile.write(cmd3)
		bbsFile.write(cmd4)
		bbsFile.write(cmd5)
		bbsFile.write(cmd6)
		bbsFile.write(cmd7)
		bbsFile.write(cmd8)
		bbsFile.write(cmd9)
		bbsFile.write(cmd10)
		#bbsFile.write(cmd10b)
		bbsFile.write(cmd11)
		bbsFile.write(cmd12)
		bbsFile.write(cmd13)
		bbsFile.write(cmd14)
		bbsFile.write(cmd15)
		#bbsFile.write(cmd15b)
		bbsFile.write(cmd16)
		bbsFile.write(cmd17)	
		bbsFile.write(cmd18)
		bbsFile.write(cmd18b)	
		bbsFile.write(cmd19)		
		bbsFile.write(cmd20)		
		bbsFile.write(cmd21)		
		bbsFile.write(cmd22)		
		bbsFile.write(cmd23)	
		bbsFile.write(cmd24)	
		bbsFile.write(cmd25)		
		bbsFile.write(cmd26)		
		bbsFile.write(cmd27)		
		bbsFile.write(cmd28)		
		bbsFile.write(cmd29)		
		bbsFile.write(cmd30)		
		bbsFile.write(cmd31)		
		bbsFile.write(cmd32)		
		bbsFile.write(cmd33)		
		bbsFile.write(cmd34)		
		bbsFile.write(cmd35)		
		bbsFile.write(cmd36)
		bbsFile.write(cmd37)		
		bbsFile.write(cmd38)		
		bbsFile.write(cmd39)	
		bbsFile.write(cmd40)
		#bbsFile.write(cmd40b)		
		bbsFile.write(cmd41)		
		bbsFile.write(cmd42)		
		bbsFile.write(cmd43)
		bbsFile.write(cmd44)		
		bbsFile.write(cmd45)
		bbsFile.write(cmd45b)			
		bbsFile.write(cmd46)
		
		bbsFile.close()			
		

		os.chdir(self.peelingNDPPPDir)	
	
	
	
		# Run  BBS
	
		max_threads = 8
		delay 		= 5
		

		# Parrallelization on time chunks
	
		jrange	= range(self.NbFiles)
		counter	= 0
		
		for j in jrange:
					
			  # Wait until one thread has finished
			  while threading.activeCount() > max_threads:
				  time.sleep(delay)
			  
			  # Set argument typle		  
			  
			  index		= '%s'%(j)					
			  files_k		= self.Files[j]
			  skymodel_k	= self.GSMSkymodel
			  param_k		= """%sNDPPP_timeChunk%s"""%(self.peelingNDPPPDir,j)
			  
			  args = (index,files_k,skymodel_k,param_k)
			  
			  
			  # Process data
			  t = threading.Thread(target=self.process, args=args)
			  t.start()				
			  
			  
			  # Wait some time before next run is started
			  time.sleep(delay)
				
				

		# end of the parralelization 	
		# Wait until all threads have finished
		
		while threading.activeCount() > 1:
			  time.sleep(delay)					


	####################################################################
	# Parrallelization (BBS & NDPPP) function
	####################################################################


    def process(self,index,files_k,skymodel_k,param_k):
				
				
		k = int(index)				
		
		
		print ''
		print '##############################################'
		print 'Start the Run of BBS & NDPPP Annulus Source substraction on Time chunk %s'%(k)
		print '##############################################\n'					
		
		if self.NbFiles <=2:
			core_index=8
		else: 
			core_index=1	
		
		

		#Run calibration & Transfer DATA from  CORRECTED DATA column to DATA column and erase CORRECTED DATA Column					
		cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,'%s%s'%(self.peelingDataDir,files_k),self.BBSParset, skymodel_k)
		print ''
		print cmd_cal
		print ''
		os.system(cmd_cal)	




		#Create NDPPP parsetFile
		file = open(param_k,'w')
						
		cmd1 ="""msin = %s%s\n"""%(self.peelingDataDir,files_k)
		cmd2 ="""msout = %s%s_peeled\n"""%(self.peelingDataDir,files_k)						
							

		cmd3  ="""msin.autoweight = false\n"""
		cmd4  ="""msin.forceautoweight = false\n"""
		cmd5  ="""msin.datacolumn = CORRECTED_DATA\n"""
		cmd6  ="""steps=[preflag,count,flag1]\n"""
		cmd6a ="""preflag.type=preflagger\n"""
		cmd6b ="""preflag.corrtype=auto\n"""
		cmd6c ="""preflag.baseline= [DE*, FR*, SE*, UK*]\n"""
		cmd6d ="""preflag.amplmin = 1e-30 \n"""		
		
		cmd7  ="""flag1.type=aoflagger\n"""
		cmd8  ="""flag1.threshold=1\n"""
		cmd9  ="""flag1.freqwindow=1\n"""
		cmd10 ="""flag1.timewindow=1\n"""
		cmd11 ="""flag1.correlations=[0,1,2,3]   # only flag on XX and YY [0,3]"""

		
		file.write(cmd1)
		file.write(cmd2)
		file.write(cmd3)
		file.write(cmd4)
		file.write(cmd5)				
		file.write(cmd6)
		file.write(cmd6a)
		file.write(cmd6b)
		file.write(cmd6c)
		file.write(cmd6d)		
		file.write(cmd7)
		#file.write(cmd8)
		#file.write(cmd9)
		#file.write(cmd10)
		#file.write(cmd11)
		
		file.close()			
		
		#Run NDPPP
		cmd_NDPPP = """NDPPP %s"""%(param_k)
		print ''
		print cmd_NDPPP
		print ''
		os.system(cmd_NDPPP)
		
		
		
		# Copy CORRECTED DATA Column to DATA column		
		self.copy_data("""%s%s"""%(self.peelingDataDir,files_k))	
		
		

		print ''
		print '##############################################'
		print 'End of the Run BBS & NDPPP Annulus Source substraction on Time chunk %s'%(k)
		print '##############################################\n'							
		print ''




		
	####################################################################	
	# Additionnal Internal function
	####################################################################	
		
	
    def copy_data(self,inms):
		
			## Create corrected data colun and Put data to corrected data column 
			t = pt.table(inms, readonly=False, ack=True)
			data = t.getcol('DATA')
			pt.addImagingColumns(inms, ack=True)
			t.putcol('CORRECTED_DATA', data)
			t.close()		
		
    def copy_data_invert(self,inms):
		
			## Create corrected data colun and Put data to corrected data column 
			t = pt.table(inms, readonly=False, ack=True)
			data = t.getcol('CORRECTED_DATA')
			pt.addImagingColumns(inms, ack=True)
			t.putcol('DATA', data)
			t.close()			
