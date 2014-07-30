#!/usr/bin/env python



# IMPORT general modules

import sys
import time
import glob
import os, commands
import pyrap.tables as pt
import numpy as np
import optparse
import pyfits

import threading

#import Lofar modules
import lofar.bdsm as bdsm


#######################################################################
## Define selfcalibration strategy, iteration, levels, perpare parsets
#######################################################################


class selfCalRun:


    def __init__(self,i,obsDir,outputDir,nbCycle,listFiles,Files,NbFiles,BBSParset,SkymodelPath,GSMSkymodel,ImagePathDir,UVmin,UVmax,wmax,pixsize,nbpixel,robust,nIteration,RMS_BOX):
    
	self.i				= i
	self.j				= 0
	self.obsDir			= obsDir
	self.outputDir		= outputDir
	self.nbCycle		= nbCycle
	self.listFiles		= listFiles
	self.Files			= Files
	self.NbFiles		= NbFiles
	self.BBSParset		= BBSParset
	self.SkymodelPath	= SkymodelPath
	self.GSMSkymodel	= GSMSkymodel
	self.ImagePathDir	= ImagePathDir
	self.UVmin			= UVmin
	self.UVmax			= UVmax
	self.wmax		= wmax
	self.pixsize		= pixsize
	self.nbpixel		= nbpixel
	self.robust			= robust
	self.nIteration		= nIteration
	self.RMS_BOX		= RMS_BOX



	##################################
	# Preparation of the Iteration run
	##################################
	
	##################################	
	## Dir for the selfCal loop
	##################################	
	
	if self.i < self.nbCycle:
		
			# Create The Iteration Directory
			self.IterDir		= self.outputDir+"""Iter%s/"""%(self.i)
			if os.path.isdir(self.IterDir) != True:
				cmd="""mkdir %s"""%(self.IterDir)
				os.system(cmd)									
				
			#Copy data from observation directory or from the previous iteration
			if self.i==0:
				if os.listdir(self.IterDir) == []:
						cmd=""" cp -r %s* %s"""%(self.obsDir,self.IterDir)
						os.system(cmd)
						print cmd
						
				else:
					print ''
					print """Data for iteration number %s has been copied before, so no need to copy again!\n"""%(self.i)
					print ''
								
							
			if self.i != 0:
				if os.listdir(self.IterDir) == []:
						cmd=""" cp -r %s %s"""%(self.outputDir+"""Iter%s/*Iter%s"""%(self.i-1,self.i-1),self.IterDir)
						print cmd 
						os.system(cmd)
						
				else:
					print ''
					print """Data for iteration number %s has been copied before, so no need to copy again!\n"""%(self.i)
					print ''					
						

			# Create NDPPP Iteration Directory
			self.NDPPPDir	= self.outputDir+"""NDPPP_Iter%s/"""%(self.i)
			if os.path.isdir(self.NDPPPDir) != True:
					cmd="""mkdir %s"""%(self.NDPPPDir)
					os.system(cmd)

	##################################
	## Dir for the selfCal Final Image
	##################################
	if self.i == self.nbCycle:
		
			# Create The Iteration Directory
			self.IterDir		= self.outputDir+"""Final_Iter/"""
			if os.path.isdir(self.IterDir) != True:
				cmd="""mkdir %s"""%(self.IterDir)
				os.system(cmd)	
				
							
			if os.listdir(self.IterDir) == []:
					cmd=""" cp -r %s %s"""%(self.outputDir+"""Iter%s/*Iter%s"""%(self.i-1,self.i-1),self.IterDir)
					print cmd 
					os.system(cmd)

						
			else:
					print ''
					print """Data for iteration number %s has been copied before, so no need to copy again!\n"""%(self.i)
					print ''					
						

			# Create NDPPP Iteration Directory
			self.NDPPPDir	= self.outputDir+"""NDPPP_Final_Iter%s/"""%(self.i)
			if os.path.isdir(self.NDPPPDir) != True:
					cmd="""mkdir %s"""%(self.NDPPPDir)
					os.system(cmd)


	###################################################################################
	# Change the execution directory to have the calibration log in the NDPPP* directory
	###################################################################################	
	os.chdir(self.NDPPPDir)
	
	
  
    def selfCalRunFuncCalibBBSNDPPP(self):


	####################################################################	
	# Calibration & NDPPP Runs on each time chunks
	####################################################################
	
	
	##################################
	# Run  BBS
	##################################				
	
	max_threads = 8
	delay 		= 5
	
	##################################
	# Parrallelization on time chunks
	##################################
	
	
	jrange	= range(self.NbFiles)
	counter	= 0
	
	for j in jrange:
				
		  # Wait until one thread has finished
		  while threading.activeCount() > max_threads:
		      time.sleep(delay)
		  
		  # Set argument typle
		  
		  index		= '%s'%(j)					
		  files_k		= self.Files[j]
		  skymodel_k	= """%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i)
		  param_k		= """%sNDPPP_timeChunk%s_Iter%s"""%(self.NDPPPDir,j,self.i)
		  
		  args = (index,files_k,skymodel_k,param_k)
		  
		  
		  # Process data
		  t = threading.Thread(target=self.process, args=args)
		  t.start()				
		  
		  
		  # Wait some time before next run is started
		  time.sleep(delay)
			
			
	############################
	# end of the parralelization 	
	# Wait until all threads have finished
	while threading.activeCount() > 1:
		  time.sleep(delay)					



    def process(self,index,files_k,skymodel_k,param_k):
				
				
		k = int(index)				
		
		print ''
		print '##############################################'
		print """Start the Run BBS & NDPPP on Time chunk %s"""%(k)
		print '##############################################\n'					
		
		
		if self.NbFiles <=2:
			core_index=8
		else: 
			core_index=1		
		
		################
		#Run calibration & Transfer DATA from  CORRECTED DATA column to DATA column and erase CORRECTED DATA Column					
		
		if self.i ==0:
			cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,""" %s%s"""%(self.IterDir,files_k),self.BBSParset,self.GSMSkymodel)
			print ''
			print cmd_cal
			print ''
			os.system(cmd_cal)


		else:			
			cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,"""%s%s_Iter%s"""%(self.IterDir,files_k,self.i-1), self.BBSParset , skymodel_k )
			print ''
			print cmd_cal
			print ''
			os.system(cmd_cal)
					
	
									
		#Create NDPPP parsetFile
		file = open(param_k,'w')
		
		if self.i==0:
						
			cmd1 ="""msin = %s%s\n"""%(self.IterDir,files_k)
			cmd2 ="""msout = %s%s_Iter%s\n"""%(self.IterDir,files_k,self.i)

		if self.i>0:
						
			cmd1 ="""msin = %s%s_Iter%s\n"""%(self.IterDir,files_k,self.i-1)	
			cmd2 ="""msout = %s%s_Iter%s\n"""%(self.IterDir,files_k,self.i)							
						
							

		cmd3  ="""msin.autoweight = false\n"""
		cmd4  ="""msin.forceautoweight = false\n"""
		cmd5  ="""msin.datacolumn = CORRECTED_DATA\n"""
		cmd6  ="""steps=[flag1]\n"""
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
		self.copy_data("""%s%s_Iter%s"""%(self.IterDir,files_k,self.i))	
		
		
		
		
		print ''
		print '##############################################'
		print """End of the Run BBS & NDPPP on Time chunk %s"""%(k)
		print '##############################################\n'							
		

		
	####################################################################	
	# Concatenate in Time and Imaging 
	####################################################################	

		
    def selfCalRunFuncImaging(self):

		#Concatenate Calibrated Time chunks 
		listOfFiles	= sorted(glob.glob("""%s*_Iter%s"""%(self.IterDir,self.i)))
		pt.msconcat(listOfFiles, """%sAll_Iteration_number_%s"""%(self.IterDir,self.i), concatTime=True)
		
		
		#Determine background threshold
		threshold = 0
		
		if self.i == 0: 
			# First Iteration threshold: 10xtheorical threshold ~ 5mJy
			threshold = 0.005
		else: 
			
			previousImage2Convert	= '%sImage_%sarcsec_Iter%s.restored'%(self.ImagePathDir,self.pixsize[self.i-1],self.i-1)				
			previousImagePath 		= '%sImage_%sarcsec_Iter%s.restored.fits'%(self.ImagePathDir,self.pixsize[self.i-1],self.i-1)
			
			cmd	= 'image2fits in=%s out=%s'%(previousImage2Convert,previousImagePath)
			print ''
			print cmd
			print ''
			os.system(cmd)
			
			
			# open a FITS file 
			fitsImage	= pyfits.open(previousImagePath) 
			scidata 	= fitsImage[0].data 
			
			dataRange	= range(fitsImage[0].shape[2])
			sortedData	=  range(fitsImage[0].shape[2]**2)
			
			for i in dataRange:
				for j in dataRange:
					sortedData[i*fitsImage[0].shape[2]+j]	=  scidata[0,0,i,j]
			
			sortedData 		= sorted(sortedData)
			
			# Percent of faintest data to use to determine 5sigma value : use 5%			
			dataPercent		= int(fitsImage[0].shape[2]*0.05)
			
			fiveSigmaData	= sum(sortedData[0:dataPercent])/dataPercent	
			threshold		= abs(fiveSigmaData)/5.0*2.335/2.0
			

		
		if self.i < self.nbCycle:
		
			if self.nbpixel[self.i]%2 ==1:
				self.nbpixel[self.i] = self.nbpixel[self.i]+1
					
			#Imaging now with the image 
			cmd_image="""awimager ms=%s image=%sImage_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i],self.i,self.robust[self.i],self.nbpixel[self.i],self.pixsize[self.i],self.nIteration, self.UVmin,self.UVmax[self.i],self.wmax[self.i],threshold) 
			print ''
			print cmd_image
			print ''
			os.system(cmd_image)	
		
		
		if self.i == self.nbCycle:
		

			
			
			if self.nbpixel[self.i-1]%2 ==1:
				self.nbpixel[self.i-1] = self.nbpixel[self.i-1]+1
					
			#Imaging now with the image 
			cmd_image="""awimager ms=%s image=%sFinal_Image_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i-1],self.i,self.robust[self.i-1],self.nbpixel[self.i-1],self.pixsize[self.i-1],self.nIteration, self.UVmin,self.UVmax[self.i-1],self.wmax[self.i-1],threshold) 
			print ''
			print cmd_image
			print ''
			os.system(cmd_image)		


	####################################################################	
	# Extract The Sky model
	####################################################################	


    def selfCalRunFuncSrcExtraction(self):
		
		if self.i < self.nbCycle:		
		
				#extract the source model with pybdsm
				print ''
				print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=6,thresh_pix=8,rms_box=(%s,%s),blank_limit=1E-4,atrous_do=True'%("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),"""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),self.RMS_BOX[0],self.RMS_BOX[1])
				print ''		
		
				#extract the source model with pybdsm
				img	=  bdsm.process_image("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),thresh_isl=6,thresh_pix=8,rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),blank_limit=1E-4,atrous_do='True') 
				
				#write bbs catalog
				img.write_catalog(outfile="""%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

				#write ds9 catalog
				img.write_catalog(outfile="""%sSkymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


				#write fits catalog
				img.write_catalog(outfile="""%sSkymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')



		if self.i == self.nbCycle:
		  
				#extract the source model with pybdsm
				print ''
				print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=6,thresh_pix=8,rms_box=(%s,%s),blank_limit=1E-4,atrous_do=True'%("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),"""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),self.RMS_BOX[0],self.RMS_BOX[1])
				print ''		  
		  

				#extract the source model with pybdsm
				img	=  bdsm.process_image("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),thresh_isl=6,thresh_pix=8,rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),blank_limit=1E-4,atrous_do='True') 
				
				#write bbs catalog
				img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

				#write ds9 catalog
				img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


				#write fits catalog
				img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')







	####################################################################


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
		
		
    def delete_data(self,inms):		
			
			## Put corrected data in data column and erase corrected data
			t = pt.table(inms, readonly=False, ack=True)
			data = t.getcol('CORRECTED_DATA')
			t.putcol('DATA', data)
			pt.removeImagingColumns(inms)
			t.close()		
