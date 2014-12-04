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



class selfCalRun:


	####################################################################
	# Preparation of the Iteration run
	####################################################################

    def __init__(self,i,obsDir,outputDir,nbCycle,listFiles,Files,NbFiles,BBSParset,SkymodelPath,GSMSkymodel,ImagePathDir,UVmin,UVmax,wmax,pixsize,nbpixel,robust,nIteration,RMS_BOX,RMS_BOX_Bright,thresh_isl,thresh_pix,outerfovclean,VLSSuse,preprocessIndex,mask,maskDilatation):
    
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
	self.wmax			= wmax
	self.pixsize		= pixsize
	self.nbpixel		= nbpixel
	self.robust			= robust
	self.nIteration		= nIteration
	self.RMS_BOX		= RMS_BOX
	self.RMS_BOX_Bright	= RMS_BOX_Bright
	self.thresh_isl		= thresh_isl
	self.thresh_pix		= thresh_pix
	
	self.outerfovclean	= outerfovclean
	self.VLSSuse		= VLSSuse
	self.preprocessIndex= preprocessIndex
	self.mask			= mask
	self.maskDilatation	= maskDilatation
	

	
	####################################################################
	## Directory creation and Data copy for the selfCal loop
	####################################################################
	
	if self.i < self.nbCycle:
		
			# Create The Iteration Directory
			self.IterDir		= self.outputDir+"""Iter%s/"""%(self.i)
			if os.path.isdir(self.IterDir) != True:
				cmd="""mkdir %s"""%(self.IterDir)
				os.system(cmd)									

			
			# copy original data
			if self.outerfovclean =='no':
					
					print ''
					cmd=""" cp -r %s* %s"""%(self.obsDir,self.IterDir)
					print cmd
					print ''
					os.system(cmd)							
									
			# copy data from PreProcessing directory		
			if self.outerfovclean =='yes':

					print ''							
					cmd=""" cp -r %s %s"""%("""%sPreprocessDir/Iter%s/*sub%s"""%(self.outputDir,self.preprocessIndex,self.preprocessIndex),self.IterDir)
					print cmd
					print ''
					os.system(cmd)							
									


			# Create NDPPP Iteration Directory
			self.NDPPPDir	= self.outputDir+"""NDPPP_Iter%s/"""%(self.i)
			if os.path.isdir(self.NDPPPDir) != True:
					cmd="""mkdir %s"""%(self.NDPPPDir)
					os.system(cmd)



	####################################################################
	## Directory creation and Data copy Final Image
	####################################################################
	if self.i == self.nbCycle:
		
			# Create The Iteration Directory
			self.IterDir		= self.outputDir+"""Final_Iter/"""
			if os.path.isdir(self.IterDir) != True:
				cmd="""mkdir %s"""%(self.IterDir)
				os.system(cmd)	
							
			
			# copy original data
			if self.outerfovclean =='no':
					
					print ''
					cmd=""" cp -r %s* %s"""%(self.obsDir,self.IterDir)
					print cmd
					print ''
					os.system(cmd)							
									
			# copy data from PreProcessing directory		
			if self.outerfovclean =='yes':

					print ''							
					cmd=""" cp -r %s %s"""%("""%sPreprocessDir/Iter%s/*sub%s"""%(self.outputDir,self.preprocessIndex,self.preprocessIndex),self.IterDir)
					print cmd
					print ''
					os.system(cmd)				
			
			#if os.listdir(self.IterDir) == []:
			#		cmd=""" cp -r %s %s"""%(self.outputDir+"""Iter%s/*Iter%s"""%(self.i-1,self.i-1),self.IterDir)
			#		print cmd 
			#		os.system(cmd)			
			#else:
			#		print ''
			#		print """Data for iteration number %s has been copied before, so no need to copy again!\n"""%(self.i)
			#		print ''					
						


			# Create NDPPP Iteration Directory
			self.NDPPPDir	= self.outputDir+"""NDPPP_Final_Iter%s/"""%(self.i)
			if os.path.isdir(self.NDPPPDir) != True:
					cmd="""mkdir %s"""%(self.NDPPPDir)
					os.system(cmd)


	####################################################################
	# Change the execution directory to have the calibration log in the 
	# NDPPP directory
	####################################################################
	os.chdir(self.NDPPPDir)
	






########################################################################	
# Calibration & NDPPP Runs on each time chunks
########################################################################
	
  
    def selfCalRunFuncCalibBBSNDPPP(self):
	
	####################################################################
	# Run  BBS & Parrallelization on time chunks
	####################################################################
	
	max_threads = 8
	delay 		= 2
		
	
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
			
			
	# end of the parralelization, Wait until all threads have finished
		
	while threading.activeCount() > 1:
		  time.sleep(delay)					



	####################################################################
	# Parrallelization (BBS & NDPPP) function
	####################################################################
	
    def process(self,index,files_k,skymodel_k,param_k):
				
				
		k = int(index)				
		
		print ''
		print '##############################################'
		print 'Start the Run BBS & NDPPP on Time chunk %s'%(k)
		print '##############################################\n'					
		
		
		if self.NbFiles <=2:
			core_index=8
		else: 
			core_index=1		
		

		#Run calibration 				

		if self.outerfovclean =='yes':
		
				if self.i ==0:
					cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,""" %s%s_sub%s"""%(self.IterDir,files_k,self.preprocessIndex),self.BBSParset,self.GSMSkymodel)
					print ''
					print cmd_cal
					print ''
					os.system(cmd_cal)


				else:			
					cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,"""%s%s_sub%s"""%(self.IterDir,files_k,self.preprocessIndex), self.BBSParset , skymodel_k )
					print ''
					print cmd_cal
					print ''
					os.system(cmd_cal)
	
					
		if self.outerfovclean =='no':
		
				if self.i ==0:
					cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,""" %s%s"""%(self.IterDir,files_k),self.BBSParset,self.GSMSkymodel)
					print ''
					print cmd_cal
					print ''
					os.system(cmd_cal)


				else:			
					cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,"""%s%s"""%(self.IterDir,files_k), self.BBSParset , skymodel_k )
					print ''
					print cmd_cal
					print ''
					os.system(cmd_cal)
					
						
									
		#Create NDPPP parsetFile
		
		file = open(param_k,'w')
		
		if self.outerfovclean =='yes':
		

				cmd1 ="""msin = %s%s_sub%s\n"""%(self.IterDir,files_k,self.preprocessIndex)
				cmd2 ="""msout = %s%s_sub%s_Iter%s\n"""%(self.IterDir,files_k,self.preprocessIndex,self.i)
				
					
		if self.outerfovclean =='no':
		
				cmd1 ="""msin = %s%s\n"""%(self.IterDir,files_k)
				cmd2 ="""msout = %s%s_Iter%s\n"""%(self.IterDir,files_k,self.i)

								

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
		
		
		if self.outerfovclean =='no':
		
				# Copy (Calibrated)DATA from DATA column to CORRECTED 
				# DATA Column for imaging		
				
				self.copy_data("""%s%s_Iter%s"""%(self.IterDir,files_k,self.i))	
		
		if self.outerfovclean =='yes':
		
				# Copy (Calibrated)DATA from DATA column to CORRECTED 
				# DATA Column for imaging		
				
				self.copy_data("""%s%s_sub%s_Iter%s"""%(self.IterDir,files_k,self.preprocessIndex,self.i))	
		

		
		print ''
		print '##############################################'
		print 'End of the Run BBS & NDPPP on Time chunk %s'%(k)
		print '##############################################\n'							
				







########################################################################	
# Imaging with AWimager
########################################################################
		
    def selfCalRunFuncImaging(self,stepProcess):


		################################################################	
		# Concatenate in Time and Imaging 
		################################################################	

		if stepProcess == 0:

				#Concatenate Calibrated Time chunks 
				listOfFiles	= sorted(glob.glob("""%s*_Iter%s"""%(self.IterDir,self.i)))
				pt.msconcat(listOfFiles, """%sAll_Iteration_number_%s"""%(self.IterDir,self.i), concatTime=True)
		


		################################################################			
		#Determine background threshold
		################################################################	

		threshold = 0
		
		if self.i == 0: 
			# First Iteration threshold: 10xtheorical threshold ~ 5mJy
			threshold = 0.005
		else: 
			
			previousImagePath 		= '%sImage_%sarcsec_Iter%s.fits'%(self.ImagePathDir,self.pixsize[self.i-1],self.i-1)
						
			# open a FITS file 	=> Estimate the awimager threshold		
			fitsImage = pyfits.open(previousImagePath)         #=> open fits image
			scidata   = fitsImage[0].data                      #=> load data
			scidata   = scidata.reshape((scidata.size,))       #=> flat the data in 1D array (before it was 4D array : freq,stokes, nx, ny)
			scidata   = scidata.ravel()						   #=> flat the data in 1D array (before it was 4D array : freq,stokes, nx, ny)
			scidata   = scidata[scidata<0]                     #=> select negative values (left part of the noise gaussian centered on 0)
			sigma     = math.sqrt(scidata.var())               #=> found sigma of that: squre root on the variance
			threshold = sigma*5.0                              #=> estimate the threshold which at 5 sigma)
						

		################################################################			
		#Imaging
		################################################################


		# Using a mask for cleaning 
		
		if self.mask == 'yes':

				# 0: Initial: needed for generate the mask
				if stepProcess == 0:
		
						if self.i < self.nbCycle:
						
							if self.nbpixel[self.i]%2 ==1:
								self.nbpixel[self.i] = self.nbpixel[self.i]+1
									
							#Imaging now with the image 
							cmd_image="""awimager ms=%s image=%sTemporary_Image_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i],self.i,self.robust[self.i],self.nbpixel[self.i],self.pixsize[self.i],self.nIteration, self.UVmin,self.UVmax[self.i],self.wmax[self.i],threshold) 
							print ''
							print cmd_image
							print ''
							os.system(cmd_image)	
						
						
						if self.i == self.nbCycle:
								
							if self.nbpixel[self.i-1]%2 ==1:
								self.nbpixel[self.i-1] = self.nbpixel[self.i-1]+1
									
							#Imaging now with the image 
							cmd_image="""awimager ms=%s image=%sTemporary_Final_Image_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i-1],self.i,self.robust[self.i-1],self.nbpixel[self.i-1],self.pixsize[self.i-1],self.nIteration, self.UVmin,self.UVmax[self.i-1],self.wmax[self.i-1],threshold) 
							print ''
							print cmd_image
							print ''
							os.system(cmd_image)
							
							
				# 1: cleaning with mask			
				if stepProcess == 1:									

						if self.i < self.nbCycle:
						
							if self.nbpixel[self.i]%2 ==1:
								self.nbpixel[self.i] = self.nbpixel[self.i]+1
									
							#Imaging now with the image 
							cmd_image="""awimager ms=%s image=%sImage_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy mask=%s"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i],self.i,self.robust[self.i],self.nbpixel[self.i],self.pixsize[self.i],self.nIteration, self.UVmin,self.UVmax[self.i],self.wmax[self.i],threshold,"""%sMask_Iter%s.fits"""%(self.SkymodelPath,self.i+1)) 
							print ''
							print cmd_image
							print ''
							os.system(cmd_image)	
						
						
						if self.i == self.nbCycle:
								
							if self.nbpixel[self.i-1]%2 ==1:
								self.nbpixel[self.i-1] = self.nbpixel[self.i-1]+1
									
							#Imaging now with the image 
							cmd_image="""awimager ms=%s image=%sFinal_Image_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy mask=%s"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i-1],self.i,self.robust[self.i-1],self.nbpixel[self.i-1],self.pixsize[self.i-1],self.nIteration, self.UVmin,self.UVmax[self.i-1],self.wmax[self.i-1],threshold,"""%sFinal_Mask_Iter%s.fits"""%(self.SkymodelPath,self.i+1)) 
							print ''
							print cmd_image
							print ''
							os.system(cmd_image)




		# Using no mask for cleaning 
		else: 

				if self.i < self.nbCycle:
				
					if self.nbpixel[self.i]%2 ==1:
						self.nbpixel[self.i] = self.nbpixel[self.i]+1
							
					#Imaging now with the image 
					cmd_image="""awimager ms=%s image=%sImage_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i],self.i,self.robust[self.i],self.nbpixel[self.i],self.pixsize[self.i],self.nIteration, self.UVmin,self.UVmax[self.i],self.wmax[self.i],threshold) 
					print ''
					print cmd_image
					print ''
					os.system(cmd_image)	
				
				
				if self.i == self.nbCycle:
						
					if self.nbpixel[self.i-1]%2 ==1:
						self.nbpixel[self.i-1] = self.nbpixel[self.i-1]+1
							
					#Imaging now with the image 
					cmd_image="""awimager ms=%s image=%sFinal_Image_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i-1],self.i,self.robust[self.i-1],self.nbpixel[self.i-1],self.pixsize[self.i-1],self.nIteration, self.UVmin,self.UVmax[self.i-1],self.wmax[self.i-1],threshold) 
					print ''
					print cmd_image
					print ''
					os.system(cmd_image)	






########################################################################	
# Extract The Sky model
########################################################################	


    def selfCalRunFuncSrcExtraction(self,stepProcess):
		
		################################################################
		# Using a mask for cleaning 
		################################################################
				
		if self.mask == 'yes':

				# 0: Initial: needed for generate the mask
				if stepProcess == 0:
		
					if self.i < self.nbCycle:		
					
							#extract the source model with pybdsm
							print ''
							print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sTemporary_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),"""%sTemporary_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
							print ''		
					
							#extract the source model with pybdsm
							img	=  bdsm.process_image("""%sTemporary_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sTemporary_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature',psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
							
							#write bbs catalog
							img.write_catalog(outfile="""%sTemporary_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

							#write ds9 catalog
							img.write_catalog(outfile="""%sTemporary_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


							#write fits catalog
							img.write_catalog(outfile="""%sTemporary_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')

							# Extract the mask in fits format
							img.export_image(outfile="""%sMask_Iter%s.fits"""%(self.SkymodelPath,self.i+1),img_format='fits',img_type='island_mask',mask_dilation=self.maskDilatation)


					if self.i == self.nbCycle:
					  
							#extract the source model with pybdsm
							print ''
							print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sTemporary_Final_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),"""%sTemporary_Final_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
							print ''		  
					  

							#extract the source model with pybdsm
							img	=  bdsm.process_image("""%sTemporary_Final_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sTemporary_Final_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature',psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
							
							#write bbs catalog
							img.write_catalog(outfile="""%sTemporary_Final_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

							#write ds9 catalog
							img.write_catalog(outfile="""%sTemporary_Final_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


							#write fits catalog
							img.write_catalog(outfile="""%sTemporary_Final_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')
							
							
							# Extract the mask in fits format
							img.export_image(outfile="""%sFinal_Mask_Iter%s.fits"""%(self.SkymodelPath,self.i+1),img_format='fits',img_type='island_mask',mask_dilation=self.maskDilatation)


				# 1: cleaning with mask			
				if stepProcess == 1:							

					if self.i < self.nbCycle:

						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),"""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		
				
						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature',psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


						#write fits catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')



					if self.i == self.nbCycle:
				  
						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),"""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		  
				  

						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature',psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


						#write fits catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')
						
						
						
		################################################################
		# no use of  mask 
		################################################################
		else: 

				if self.i < self.nbCycle:
				
						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),"""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		
				
						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature',psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


						#write fits catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')



				if self.i == self.nbCycle:
				  
						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),"""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		  
				  

						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature',psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True')


						#write fits catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True')







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
		
