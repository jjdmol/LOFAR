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
#import Statistics modules
########################################################################

#from lofar.selfcal import LSM_module.pyvo.pyvo
#from lofar.selfcal import module.lsmtool as lsmtool

#LSM Tools
import LSM_module.pyvo.pyvo
from LSM_module.module import lsmtool as lsmtool

#Sarrvesh tool
import optparse
import lofar.parmdb as lp
import matplotlib.pyplot as plt
import numpy
from os import makedirs
from os.path import exists

########################################################################
## Define selfcalibration strategy, iteration, levels, perpare parsets
########################################################################



class selfCalRun:


	####################################################################
	# Preparation of the Iteration run
	####################################################################

    def __init__(self,i,dataDir,outputDir,nbCycle,listFiles,Files,NbFiles,SkymodelPath,GSMSkymodel,ImagePathDir,UVmin,UVmax,wmax,pixsize,nbpixel,robust,nIteration,RMS_BOX,RMS_BOX_Bright,thresh_isl,thresh_pix,outerfovclean,VLSSuse,preprocessIndex,mask,maskDilation,ra_target,dec_target,FOV):
    
	self.i				= i
	self.j				= 0
	self.dataDir		= dataDir
	self.outputDir		= outputDir
	self.nbCycle		= nbCycle
	self.listFiles		= listFiles
	self.Files			= Files
	self.NbFiles		= NbFiles
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
	self.maskDilation	= maskDilation
	
	self.ra_target		= ra_target
	self.dec_target		= dec_target
	self.FOV			= FOV
	
	self.statisticsSkymodelCurrent 	= ''
	self.statisticsSkymodelPrevious = ''	
	self.rmsclipped					= 0
	

	
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
					cmd=""" cp -r %s* %s"""%(self.dataDir,self.IterDir)
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


			# Create Statistics Iteration Directory
			self.statDir	= self.outputDir+"""Stats_Iter%s/"""%(self.i)
			if os.path.isdir(self.statDir) != True:
					cmd="""mkdir %s"""%(self.statDir)
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
					cmd=""" cp -r %s* %s"""%(self.dataDir,self.IterDir)
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
			self.NDPPPDir	= self.outputDir+"""NDPPP_Final_Iter%s/"""%(self.i)
			if os.path.isdir(self.NDPPPDir) != True:
					cmd="""mkdir %s"""%(self.NDPPPDir)
					os.system(cmd)


			# Create Statistics Iteration Directory
			self.statDir	= self.outputDir+"""Stats_Final_Iter%s/"""%(self.i)
			if os.path.isdir(self.statDir) != True:
					cmd="""mkdir %s"""%(self.statDir)
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
		  
		  index_process			= '%s'%(j)					
		  files_process			= self.Files[j]
		  
		  if self.i == 0:
				skymodel_process		= """%s"""%(self.GSMSkymodel)
		  else:
				skymodel_process		= """%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i)
		  
		  param1_process		= """%sGaincal_OnlyPhase_Predict_timeChunk%s_Iter%s"""%(self.NDPPPDir,j,self.i)
		  param2_process		= """%sGaincal_OnlyPhase_Applycal_timeChunk%s_Iter%s"""%(self.NDPPPDir,j,self.i)
		  param3_process		= """%sNDPPP_timeChunk%s_Iter%s"""%(self.NDPPPDir,j,self.i)

		  
		  args = (index_process,files_process,skymodel_process,param1_process,param2_process,param3_process)
		  
		  
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
	
    def process(self,index_process,files_process,skymodel_process,param1_process,param2_process,param3_process):
				
				
		k = int(index_process)				
		
		print ''
		print '##############################################'
		print 'Start the Run BBS & NDPPP on Time chunk %s'%(k)
		print '##############################################\n'					
		
		
		# Create MS/sky model from skymodel in bbs format
		
		cmd_sourcedb = """makesourcedb in=%s  out=%s%s/sky format='<'"""%(skymodel_process,self.IterDir,files_process)		
		print ''
		print 'Create sourcedb in the MS for MS:%s\n'%(files_process)
		print cmd_sourcedb
		print ''
		os.system(cmd_sourcedb)
		print ''
								
		
		
		# Create parset Gaincal Predict parset
		
		fileGaincal1 = open(param1_process,'w')
		
		if self.outerfovclean =='yes':
				cmd1	= """msin=%s%s_sub%s\n"""%(self.IterDir,files_process,self.preprocessIndex)
				cmd7	= """gaincal.sourcedb=%s%s_sub%s/sky\n"""%(self.IterDir,files_process,self.preprocessIndex)
				cmd10	= """gaincal.parmdb=%s%s_sub%s/instrument\n"""%(self.IterDir,files_process,self.preprocessIndex)
		
		if self.outerfovclean =='no':		
				cmd1	= """msin=%s%s\n"""%(self.IterDir,files_process)
				cmd7	= """gaincal.sourcedb=%s%s/sky\n"""%(self.IterDir,files_process)
				cmd10	= """gaincal.parmdb=%s%s/instrument\n"""%(self.IterDir,files_process)
						
		cmd2	= """msin.datacolumn=CORRECTED_DATA\n"""
		cmd3	= """msout=.\n"""
		cmd4	= """steps=[gaincal]\n"""
		cmd5	= """gaincal.type=gaincal\n"""
		cmd6	= """gaincal.caltype=phaseonly\n"""
		cmd8	= """gaincal.usebeammodel=false\n"""
		cmd9	= """gaincal.tolerance=1.e-5\n"""
				
		fileGaincal1.write(cmd1)
		fileGaincal1.write(cmd2)
		fileGaincal1.write(cmd3)
		fileGaincal1.write(cmd4)
		fileGaincal1.write(cmd5)
		fileGaincal1.write(cmd6)
		fileGaincal1.write(cmd7)
		fileGaincal1.write(cmd8)
		fileGaincal1.write(cmd9)
		fileGaincal1.write(cmd10)
		
		fileGaincal1.close()


		# Create Gaincal Applycal parset
		
		fileGaincal2 = open(param2_process,'w')
		
		if self.outerfovclean =='yes':
				cmd1	= """msin=%s%s_sub%s\n"""%(self.IterDir,files_process,self.preprocessIndex)
				cmd8	= """applycal.parmdb=%s%s_sub%s/instrument\n"""%(self.IterDir,files_process,self.preprocessIndex)		
		if self.outerfovclean =='no':
				cmd1	= """msin=%s%s\n"""%(self.IterDir,files_process)
				cmd8	= """applycal.parmdb=%s%s/instrument\n"""%(self.IterDir,files_process)		
						
		cmd2	= """msin.datacolumn=CORRECTED_DATA\n"""
		cmd3	= """msout=.\n"""
		cmd4	= """msout.datacolumn=CORRECTED_DATA\n"""
		cmd5	= """steps=[applycal]\n"""
		cmd6	= """applycal.type=applycal\n"""
		cmd7	= """applycal.correction=gain\n"""
		
			
		fileGaincal2.write(cmd1)
		fileGaincal2.write(cmd2)
		fileGaincal2.write(cmd3)
		fileGaincal2.write(cmd4)
		fileGaincal2.write(cmd5)
		fileGaincal2.write(cmd6)
		fileGaincal2.write(cmd7)
		fileGaincal2.write(cmd8)

		fileGaincal2.close()		
		
		
				
		#Create NDPPP flaging parset
			
		fileFlag1	= open(param3_process,'w')
		
		if self.outerfovclean =='yes':
				cmd1 ="""msin = %s%s_sub%s\n"""%(self.IterDir,files_process,self.preprocessIndex)
								
		if self.outerfovclean =='no':
				cmd1 ="""msin = %s%s\n"""%(self.IterDir,files_process)
							
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

		
		fileFlag1.write(cmd1)
		fileFlag1.write(cmd2)
		fileFlag1.write(cmd3)
		fileFlag1.write(cmd4)
		fileFlag1.write(cmd5)				
		fileFlag1.write(cmd6)
		fileFlag1.write(cmd7)
		fileFlag1.write(cmd8)
		fileFlag1.write(cmd9)
		fileFlag1.write(cmd10)
		fileFlag1.write(cmd11)

		fileFlag1.close()			
	
	
		# Run calibration and flaging using NDPPP (Gaincal with Predict step)
	
		#Run NDPPP Gaincal 1 : Predict 		
		cmd_NDPPP1 = """NDPPP %s"""%(param1_process)
		print ''
		print cmd_NDPPP1
		print ''
		os.system(cmd_NDPPP1)

		
		#Run NDPPP Gaincal 2 : Applycal 		
		cmd_NDPPP1 = """NDPPP %s"""%(param2_process)
		print ''
		print cmd_NDPPP1
		print ''
		os.system(cmd_NDPPP1)

		
		#Run NDPPP: Flagging 		
		cmd_NDPPP1 = """NDPPP %s"""%(param3_process)
		print ''
		print cmd_NDPPP1
		print ''
		os.system(cmd_NDPPP1)		



		print ''
		print '##############################################'
		print 'End of the Run calibration with NDPPP on Time chunk %s'%(k)
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
				listOfFiles	= sorted(glob.glob("""%s*"""%(self.IterDir)))
				print ''
				print listOfFiles
				print ''
				pt.msconcat(listOfFiles, """%sAll_Iteration_number_%s"""%(self.IterDir,self.i), concatTime=True)
		


		################################################################			
		#Determine background threshold
		################################################################	

		threshold = 0
		
		if self.i == 0: 
			# First Iteration threshold: 10xtheorical threshold ~ 5mJy
			threshold = 0.05
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
							cmd_image="""awimager ms=%s image=%sImage_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy mask=%s"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i],self.i,self.robust[self.i],self.nbpixel[self.i],self.pixsize[self.i],self.nIteration, self.UVmin,self.UVmax[self.i],self.wmax[self.i],threshold,"""%sMask_Iter%s.casa"""%(self.SkymodelPath,self.i+1)) 
							print ''
							print cmd_image
							print ''
							os.system(cmd_image)	
						
						
						if self.i == self.nbCycle:
								
							if self.nbpixel[self.i-1]%2 ==1:
								self.nbpixel[self.i-1] = self.nbpixel[self.i-1]+1
									
							#Imaging now with the image 
							cmd_image="""awimager ms=%s image=%sFinal_Image_%sarcsec_Iter%s weight=briggs robust=%s npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits="" threshold=%sJy mask=%s"""%("""%sAll_Iteration_number_%s"""%(self.IterDir,self.i),self.ImagePathDir,self.pixsize[self.i-1],self.i,self.robust[self.i-1],self.nbpixel[self.i-1],self.pixsize[self.i-1],self.nIteration, self.UVmin,self.UVmax[self.i-1],self.wmax[self.i-1],threshold,"""%sFinal_Mask_Iter%s.casa"""%(self.SkymodelPath,self.i+1)) 
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
							print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature_do=True)#,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sTemporary_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),"""%sTemporary_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
							print ''		
					
							#extract the source model with pybdsm
							img	=  bdsm.process_image("""%sTemporary_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sTemporary_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature')#,psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
							
							#write bbs catalog
							img.write_catalog(outfile="""%sTemporary_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True',clobber='True')

							#write ds9 catalog
							img.write_catalog(outfile="""%sTemporary_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True',clobber='True')


							#write fits catalog
							img.write_catalog(outfile="""%sTemporary_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True',clobber='True')

							# Extract the mask in fits format
							img.export_image(outfile="""%sMask_Iter%s.casa"""%(self.SkymodelPath,self.i+1),img_format='casa',img_type='island_mask',mask_dilation=self.maskDilation)


					if self.i == self.nbCycle:
					  
							#extract the source model with pybdsm
							print ''
							print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature)#,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sTemporary_Final_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),"""%sTemporary_Final_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
							print ''		  
					  

							#extract the source model with pybdsm
							img	=  bdsm.process_image("""%sTemporary_Final_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sTemporary_Final_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature')#,psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
							
							#write bbs catalog
							img.write_catalog(outfile="""%sTemporary_Final_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True',clobber='True')

							#write ds9 catalog
							img.write_catalog(outfile="""%sTemporary_Final_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True',clobber='True')


							#write fits catalog
							img.write_catalog(outfile="""%sTemporary_Final_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True',clobber='True')
							
							
							# Extract the mask in fits format
							img.export_image(outfile="""%sFinal_Mask_Iter%s.casa"""%(self.SkymodelPath,self.i+1),img_format='casa',img_type='island_mask',mask_dilation=self.maskDilation)


				# 1: cleaning with mask			
				if stepProcess == 1:							

					if self.i < self.nbCycle:

						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature)#,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),"""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		
				
						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature')#,psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True',clobber='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True',clobber='True')


						#write fits catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True',clobber='True')
						
						
						# Store the path of the Skymodel and value to exploit
						#Skymodel path
						self.statisticsSkymodelCurrent 		= """%sPybdsm_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1)
						if self.i !=0:
							self.statisticsSkymodelPrevious	= """%sPybdsm_Skymodel_Iter%s"""%(self.SkymodelPath,self.i)					
						# Values
						self.rmsclipped						= img.clipped_rms
						self.Mean							= img.clipped_mean
						self.TotalFlux						= img.total_flux_gaus
						
						
						# Copy pybdsm extracted Skymodel as a save
						cmd_cp	= """cp %sSkymodel_Iter%s  %sPybdsm_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1,self.SkymodelPath,self.i+1)
						print ''
						print cmd_cp
						print ''
						os.system(cmd_cp)
						
						# convert dot model to BBS format
						convert_cmd="""casapy2bbs.py --mask=%sMask_Iter%s.casa %sImage_%sarcsec_Iter%s.model.corr  %sSkymodel_Iter%s"""%(self.SkymodelPath,self.i+1,self.ImagePathDir,self.pixsize[self.i],self.i,self.SkymodelPath,self.i+1)
						print ''
						print convert_cmd
						print ''						
						os.system(convert_cmd)						
						


					if self.i == self.nbCycle:
				  
						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature)#,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),"""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		  
				  

						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature')#,psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True',clobber='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True',clobber='True')

						#write fits catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True',clobber='True')


						# Store the path of the Skymodel and value to exploit
						# Skymodel path
						self.statisticsSkymodelCurrent 	= """%sPybdsm_Final_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1)
						self.statisticsSkymodelPrevious	= """%sPybdsm_Skymodel_Iter%s"""%(self.SkymodelPath,self.i)
						# Values
						self.rmsclipped						= img.clipped_rms
						self.Mean							= img.clipped_mean
						self.TotalFlux						= img.total_flux_gaus					


						# Copy pybdsm extracted Skymodel as a save
						cmd_cp	= """cp %sFinal_Skymodel_Iter%s  %sPybdsm_Final_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1,self.SkymodelPath,self.i+1)
						print ''
						print cmd_cp
						print ''
						os.system(cmd_cp)
						
						# convert dot model to BBS format						
						convert_cmd="""casapy2bbs.py --mask=%sFinal_Mask_Iter%s.casa %sFinal_Image_%sarcsec_Iter%s.model.corr  %sFinal_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1,self.ImagePathDir,self.pixsize[self.i-1],self.i,self.SkymodelPath,self.i+1)
						print ''
						print convert_cmd
						print ''
						os.system(convert_cmd)	
						
						
		################################################################
		# no use of  mask 
		################################################################
		else: 

				if self.i < self.nbCycle:
				
						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature)#,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),"""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		
				
						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sImage_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sImage_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature')#,psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True',clobber='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True',clobber='True')


						#write fits catalog
						img.write_catalog(outfile="""%sSkymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True',clobber='True')


						# Store the path of the Skymodel and value to exploit
						# Skymodel path
						self.statisticsSkymodelCurrent 		= """%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i+1)
						if self.i !=0:
							self.statisticsSkymodelPrevious	= """%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i)					
						# Values
						self.rmsclipped						= img.clipped_rms
						self.Mean							= img.clipped_mean
						self.TotalFlux						= img.total_flux_gaus


				if self.i == self.nbCycle:
				  
						#extract the source model with pybdsm
						print ''
						print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,ini_method=curvature)#,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),"""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),self.thresh_isl,self.thresh_pix,self.RMS_BOX[0],self.RMS_BOX[1],self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1])
						print ''		  
				  

						#extract the source model with pybdsm
						img	=  bdsm.process_image("""%sFinal_Image_%sarcsec_Iter%s.restored.corr"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),adaptive_rms_box='True',advanced_opts='True',detection_image="""%sFinal_Image_%sarcsec_Iter%s.restored"""%(self.ImagePathDir,self.pixsize[self.i-1],self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(self.RMS_BOX[0],self.RMS_BOX[1]),rms_box_bright=(self.RMS_BOX_Bright[0],self.RMS_BOX_Bright[1]),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature')#,psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
						
						#write bbs catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='bbs',correct_proj='True',clobber='True')

						#write ds9 catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.reg"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='ds9',correct_proj='True',clobber='True')


						#write fits catalog
						img.write_catalog(outfile="""%sFinal_Skymodel_Iter%s.fits"""%(self.SkymodelPath,self.i+1),catalog_type='gaul',format='fits',correct_proj='True',clobber='True')


						# Store the path of the Skymodel and value to exploit
						# Skymodel path
						self.statisticsSkymodelCurrent 	= """%sFinal_Skymodel_Iter%s"""%(self.SkymodelPath,self.i+1)
						self.statisticsSkymodelPrevious	= """%sSkymodel_Iter%s"""%(self.SkymodelPath,self.i)
						# Values
						self.rmsclipped						= img.clipped_rms
						self.Mean							= img.clipped_mean
						self.TotalFlux						= img.total_flux_gaus


########################################################################	
# Extract Sky model Statistics and Instrument table Statistics
########################################################################	


    def selfCalRunFuncStatistics(self):
		
		
			############################################################
			# Extract Statistics from D.Rafferty LSMtool Module
			############################################################
		
			# Create stats directory without flux cut 
			plotsGSMDir_noFluxCut				= self.statDir+'LSMTools/GSM_Comparison_noFluxCut/'
			plotNVSSDir_noFluxCut				= self.statDir+'LSMTools/NVSS_Comparison_noFluxCut/'

			cmd1 = 'mkdir -p %s'%(plotsGSMDir_noFluxCut)
			cmd2 = 'mkdir -p %s'%(plotNVSSDir_noFluxCut)
			os.system(cmd1)
			os.system(cmd2)
			
			if self.i != 0:
					plotPreviousIterDir_noFluxCut		= self.statDir+'LSMTools/Iter%s_Iter%s_Comparison_noFluxCut/'%(self.i-1,self.i)
					cmd3 ='mkdir -p %s'%(plotPreviousIterDir_noFluxCut)
					os.system(cmd3)
					
					
			# Create stats directory with flux cut I>500 mJy
			plotsGSMDir_500mJyCut				= self.statDir+'LSMTools/GSM_Comparison_500mJyCut/'
			plotNVSSDir_500mJyCut				= self.statDir+'LSMTools/NVSS_Comparison_500mJyCut/'

			cmd1 = 'mkdir -p %s'%(plotsGSMDir_500mJyCut)
			cmd2 = 'mkdir -p %s'%(plotNVSSDir_500mJyCut)
			os.system(cmd1)
			os.system(cmd2)
			
			if self.i != 0:
					plotPreviousIterDir_500mJyCut		= self.statDir+'LSMTools/Iter%s_Iter%s_Comparison_500mJyCut/'%(self.i-1,self.i)
					cmd3 ='mkdir -p %s'%(plotPreviousIterDir_500mJyCut)
					os.system(cmd3)

		
					
			# Load Skymodels					
			pybdsm_model_current 	= lsmtool.load(self.statisticsSkymodelCurrent)
			if self.i != 0:		
				pybdsm_model_previous	= lsmtool.load(self.statisticsSkymodelPrevious)
							
			gsm_model 				= lsmtool.load('gsm', VOPosition=[self.ra_target, self.dec_target], VORadius='%s degree'%(self.FOV))
			nvss_model 				= lsmtool.load('nvss', VOPosition=[self.ra_target, self.dec_target], VORadius='%s degree'%(self.FOV))		
					
					
					
			# Run comparison no flux cut		
			pybdsm_model_current.compare(gsm_model, outDir='%s'%(plotsGSMDir_noFluxCut), radius='2 arcmin')		
			pybdsm_model_current.compare(nvss_model, outDir='%s'%(plotNVSSDir_noFluxCut), radius='2 arcmin')
			if self.i != 0:
					pybdsm_model_current.compare(pybdsm_model_previous, outDir='%s'%(plotPreviousIterDir_noFluxCut), radius='2 arcmin')		


			# Run comparison with flux cut I>500mJy
			pybdsm_model_current.select('I > 500 mJy')
			if self.i != 0:
				pybdsm_model_previous.select('I > 500 mJy')

			pybdsm_model_current.compare(gsm_model, outDir='%s'%(plotsGSMDir_500mJyCut), radius='2 arcmin', excludeByFlux=True)		
			pybdsm_model_current.compare(nvss_model, outDir='%s'%(plotNVSSDir_500mJyCut), radius='2 arcmin', excludeByFlux=True)
			if self.i != 0:
					pybdsm_model_current.compare(pybdsm_model_previous, outDir='%s'%(plotPreviousIterDir_500mJyCut), radius='2 arcmin', excludeByFlux=True)	

		
			############################################################
			# Extract Instrument parmdb plots from Sarrvesh Module
			############################################################


			# generate instruments table plots 
			
			list_instrument_MS 	= range(len(self.Files))
			list_MS 			= range(len(self.Files))
			
			for k in range(len(self.Files)):
				if self.outerfovclean =='no':
					list_instrument_MS[k] 	= """%s%s/instrument"""%(self.IterDir,self.Files[k])
					list_MS[k]				= """%s"""%(self.Files[k])	
				if self.outerfovclean =='yes':
					list_instrument_MS[k] 	= """%s%s_sub%s/instrument"""%(self.IterDir,self.Files[k],self.preprocessIndex)
					list_MS[k]				= """%s_sub%s"""%(self.Files[k],self.preprocessIndex)
				
				
				parmdbOutputDir = """%s/Parmdb_Plots/%s/"""%(self.statDir,list_MS[k])		
				cmd 			= """mkdir -p %s"""%(parmdbOutputDir)				
				os.system(cmd)
				
				self.makePhasePlots("""%s"""%(list_instrument_MS[k]),"""%s"""%(parmdbOutputDir))
				
				

			############################################################
			# Other Stats extracted with pybdsm
			############################################################
			
			pybdsmStatDir		= """%sPybdsm_Stats/"""%(self.statDir)
			cmd_pybdsmStats 	= """mkdir -p %s"""%(pybdsmStatDir)
			
			os.system(cmd_pybdsmStats) 
			
			pybdsmStatFile		= """%sExtractedStats"""%(pybdsmStatDir)
			
			filepybdsm			= open(pybdsmStatFile,'w')
			
			cmd_pybdsm	= """
******************************************************************************
Informations extracted with pybdsm

- The (3-sigma) clipped rms = %s Jy 

- The Mean flux on the image = %s Jy

- The Total Flux on the image = %s Jy

						"""%(self.rmsclipped,self.Mean,self.TotalFlux)
			filepybdsm.write(cmd_pybdsm)
			filepybdsm.close()						
						
						
						
						
						

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
		
    def copy_data_invert(self,inms):
		
			## Create corrected data colun and Put data to corrected data column 
			t = pt.table(inms, readonly=False, ack=True)
			data = t.getcol('CORRECTED_DATA')
			pt.addImagingColumns(inms, ack=True)
			t.putcol('DATA', data)
			t.close()	


	####################################################################	
	# Sarrvesh Tool Internal function
	####################################################################	


    def normalize(self,phase):
		"""
		Normalize phase to the range [-pi, pi].
		"""
		# Convert to range [-2*pi, 2*pi].
		out = numpy.fmod(phase, 2.0 * numpy.pi)
		# Convert to range [-pi, pi]
		out[out < -numpy.pi] += 2.0 * numpy.pi
		out[out > numpy.pi] -= 2.0 * numpy.pi
		return out

    def makePhasePlots(self,instTable, outDir):
		"""
		For a given instrument table, this function plots the phase solutions for each station with respect to the first station
		
		To do: Internally the code now assumes that the phase solutions were derived using BBS with phasors=True in the SOLVE step. In later versions, check the form in which the gain solutions are stored and plot accordingly.
		
		Written by Sarrvesh S. Sridhar
		corrected by Nicolas Vilchez 
		
		v1.0 created 11 Feb 2014
		"""
		# Read the input instrument table
		if instTable == '':
			print 'An instrument table must be specified.'; return
		try:
			thisTable 		= lp.parmdb(instTable)
		except:
			print 'Error: Unable to read the instrument table. Will skip plotting the solutions.'
			return 
		# Create the output directory
		if not exists(outDir):
			makedirs(outDir)
		# Get the list of gain terms in this instrument table
		gTabList 		= thisTable.getNames(parmnamepattern='Gain*Phase*')
		nGainTerms		= len(gTabList)
		# Get the name of the first station
		refStationName	= gTabList[0].split(':')[-1]
		refValues 		= thisTable.getValuesGrid(parmnamepattern=gTabList[0])[gTabList[0]]['values']
		# Get the common time axis for all stations
		timeAxis		= thisTable.getValuesGrid(parmnamepattern=gTabList[0])[gTabList[0]]['times']
		timeSinceStart 	= timeAxis - timeAxis[0]
		del timeAxis
		# Plot the phase for each entry in the gain table
		for i in range(1,nGainTerms):
			thisStationName = gTabList[i].split(':')[-1]
			elementName		= 'G'+str(gTabList[i].split(':')[1])+str(gTabList[i].split(':')[2])
			print 'Plotting the phase solutions in', elementName, 'for station', thisStationName
			values = thisTable.getValuesGrid(parmnamepattern=gTabList[i])[gTabList[i]]['values']
			plt.figure(1)
			plt.plot(timeSinceStart, self.normalize(values-refValues), 'ro')
			plt.xlabel('Time (seconds)')
			plt.ylabel('Phase (radians)')
			fileName = outDir+'/'+thisStationName+'_'+elementName+'.png'
			plt.title('Station: '+thisStationName+' '+elementName)
			plt.grid(True)
			plt.savefig(fileName, bbox_inches='tight')
			plt.close()



