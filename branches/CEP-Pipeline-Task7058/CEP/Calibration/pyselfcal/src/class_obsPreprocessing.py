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



class obsPreprocessing:

    def __init__(self,obsDir,preprocessDir,preprocessImageDir,preprocessSkymodelDir,preprocessBBSDir,i,listFiles,Files,NbFiles,frequency,UVmin,nIteration,ra_target,dec_target,initNofAnnulusSources,annulusRadius,FOV):

		################################################################	    
		# Initialization
		################################################################	

    
		self.obsDir					= obsDir
		self.preprocessDir			= preprocessDir
		self.preprocessImageDir		= preprocessImageDir
		self.preprocessSkymodelDir	= preprocessSkymodelDir
		self.preprocessBBSDir		= preprocessBBSDir
		self.BBSParset				= ''
		self.i						= i
		self.listFiles				= listFiles
		self.Files					= Files
		self.NbFiles				= NbFiles
		self.frequency				= frequency
		self.UVmin					= UVmin
		self.nIteration				= nIteration
		self.ra_target				= ra_target
		self.dec_target				= dec_target
		self.initNofAnnulusSources	= initNofAnnulusSources
		self.annulusRadius			= annulusRadius
		self.FOV					= FOV
		self.OriginalFOV			= FOV
			
		
		################################################################	    
		# Default parameter for HBA and LBA obs => changes for High HBA
		################################################################	    
				
		self.thresh_isl	= 6
		self.thresh_pix	= 8
		
		
		################################################################	    
		# Create the initial directory
		################################################################	    
		
		self.IterDir		= self.preprocessDir+'Iter%s/'%(self.i)
		cmd="""mkdir %s"""%(self.IterDir)
		os.system(cmd)									
					
		#Copy data from observation directory
		if self.i==0:
			cmd=""" cp -r %s* %s"""%(self.obsDir,self.IterDir)
			
			print ''
			print cmd
			os.system(cmd)	
			print ''

		else:
			cmd=""" cp -r %s %s"""%(self.preprocessDir+'Iter%s/*sub%s'%(self.i-1,self.i-1),self.IterDir)
			print ''
			print cmd 
			os.system(cmd)			
			print ''
			
		
		# Concatenate Initial Time chunks 
		self.list0	= sorted(glob.glob(('%s*')%(self.IterDir)))
		self.initialConcatMS	= '%stimeConcatMS'%(self.IterDir)
		
		
		if os.path.isdir(self.initialConcatMS) != True:
			pt.msconcat(self.list0, self.initialConcatMS, concatTime=True)
	  
	  
		################################################################	    
		# Determine the fov for calibration step
		################################################################	    
		
		if self.frequency > 1.9E8:
			
			print ''
			print '########### PreProcessing INFOS ########################'			
			print 'This is a High HBA observation. It requires a small field of view, and so a small annulus subraction.'
			print 'If FOV > 1.25 => FOV=1.25. If Annulus Radius >0.25 => Annulus Radius=0.25'
						
			if self.FOV > 1.25:
				self.FOV 			= 1.25
				self.OriginalFOV	= 1.25
				
			if self.annulusRadius > 0.25:
				self.annulusRadius = 0.25

			print 'FOV = FOV (%s) + 2 x Annulus Radius(%s)'%(self.FOV,self.annulusRadius)
				
			self.FOV = self.OriginalFOV + 2*self.annulusRadius				
			self.thresh_isl	= 3
			self.thresh_pix	= 5				
				

			print 'Current FOV=%s degree and Annulus Radius=%s degree'%(self.FOV,self.annulusRadius) 
			print 'Extraction Parameters changed for High HBA: thresh_isl=3 (instead 6); thresh_pix=5 (instead 8)'
			print ''
			print ''										
	
		else:
			print ''
			print '########### PreProcessing INFOS ########################'
			print 'This is a LOFAR HBA or LBA observation.'
			print 'If FOV > 15 => FOV=15. If Annulus Radius >5 => Annulus Radius=5'
			print 'If FOV + 2 x Annulus Radius >10 => FOV=15 and Annulus Radius=2.5'			
						
			if self.FOV > 15:
				self.FOV 			= 15
				self.OriginalFOV	= 15
				
			if self.annulusRadius > 5:
				self.annulusRadius = 5
							
			if (self.FOV + self.annulusRadius) >15:
				self.FOV 			= 10
				self.OriginalFOV	= 10
				self.annulusRadius 	= 2.5	
							
			print 'FOV = FOV (%s) + 2 x Annulus Radius(%s)'%(self.FOV,self.annulusRadius)

			self.FOV = self.FOV + 2*self.annulusRadius

			print 'Current TOTAL FOV=%s degree and Annulus Radius=%s degree'%(self.FOV,self.annulusRadius) 			
			print ''		
		
		################################################################	    
		# Extra param for preprocessing:
		################################################################	    
		
		self.pixPerBeam	= 4
		self.pixsize 	= 22.5
		self.UVmax		= float(fpformat.fix((3E8/self.frequency)/(self.pixPerBeam*self.pixsize/3600.*3.14/180.)/(1E3*3E8/self.frequency),3))
		self.wmax		= float(fpformat.fix(self.UVmax*(3E8/self.frequency)*1E3,3))
		
		
		################################################################	    
		# Adapt Number of pixel to right value for FT
		################################################################	    
		
		multipleOfTwo	= 2**(np.arange(15))
		multipleOfThree	= 3**(np.arange(10))
		multipleOfFive	= 5**(np.arange(6))
		
		matrixOfGoodNumber	= multipleOfTwo[:,np.newaxis,np.newaxis]*multipleOfThree[np.newaxis,:,np.newaxis]*multipleOfFive[np.newaxis,np.newaxis,:]
		matrixFlat			= matrixOfGoodNumber.ravel()
		matrixFlatSorted	= sorted(matrixFlat)
		
		nbpixelTemp		= int(self.FOV*3600./self.pixsize)
		self.nbpixel	= int(matrixFlatSorted[np.searchsorted(matrixFlatSorted,nbpixelTemp)])
		
		print ''
		print 'To obtain quicker an optimized image, the nof pixel changed from %s to %s'%(nbpixelTemp,self.nbpixel) 
		print 'It is for optimize the Fouriier Transform during the imaging process'
		print ''

		
		################################################################	    
		# Check if UVmin is > UVmax
		################################################################	    
		
		if self.UVmin > self.UVmax:

			print 'Selected UVmin (%s) > UVmax (%s) at Low resolution for preprocessing !!'%(self.UVmin,self.UVmax)
			print 'New UVmin = UVmax/2'
			self.UVmin = float(fpformat.fix(self.UVmax/2.0,3))
			print 'New UVmin = %s (in klambda)'%(self.UVmin)
			print ''
		print '########################################################'			
		print ''
		print ''
		



	####################################################################		
	# Imaging
	####################################################################		
	
    def obsPreprocessImagingFunc(self):
    

		threshold = 0.005
		if self.nbpixel%2 ==1:
				self.nbpixel = self.nbpixel+1
				
				
					
		#Imaging now with the image 
		cmd_image='awimager ms=%s image=%sImage_substraction%s weight=briggs robust=1 npix=%s cellsize=%sarcsec data=CORRECTED_DATA padding=1.18 niter=%s stokes=I operation=mfclark timewindow=300 UVmin=%s UVmax=%s wmax=%s fits threshold=%sJy'%(self.initialConcatMS,self.preprocessImageDir,self.i,self.nbpixel,self.pixsize,self.nIteration,self.UVmin,self.UVmax,self.wmax,threshold) 
		
		print ''
		print cmd_image
		os.system(cmd_image)
		print ''
		
			



	####################################################################		
	# Source Extraction
	####################################################################		
		
    def obsPreprocessSrcExtractionFunc(self):
			
		
		#extract the source model with pybdsm
		print ''
		print 'extraction by pybdsm: bdsm.process_image %s,adaptive_rms_box=True,advanced_opts=True,detection_image=%s,thresh_isl=%s,thresh_pix=%s,rms_box=(%s,%s),rms_box_bright=(%s,%s),adaptive_thresh=30,blank_limit=1E-4,atrous_do=True,psf_vary_do=True,psf_stype_only=False,psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3'%('%sImage_substraction%s.restored.corr'%(self.preprocessImageDir,self.i),'%sImage_substraction%s.restored'%(self.preprocessImageDir,self.i),self.thresh_isl,self.thresh_pix,80,10,40,10)
		print ''		
		
		#extract the source model with pybdsm
		img	=  bdsm.process_image('%sImage_substraction%s.restored.corr'%(self.preprocessImageDir,self.i),adaptive_rms_box='True',advanced_opts='True',detection_image='%sImage_substraction%s.restored'%(self.preprocessImageDir,self.i),thresh_isl='%s'%(self.thresh_isl),thresh_pix='%s'%(self.thresh_pix),rms_box=(80,10),rms_box_bright=(40,10),adaptive_thresh=30,blank_limit=1E-4,atrous_do='True',ini_method='curvature',psf_vary_do='True',psf_stype_only='False',psf_snrcut=5,psf_snrcutstack=5,psf_snrtop=0.3) 
				
		#write bbs catalog
		img.write_catalog(outfile="""%sSkymodel_substraction%s"""%(self.preprocessSkymodelDir,self.i),catalog_type='gaul',format='bbs',correct_proj='True',srcroot='source')

		#write ds9 catalog
		img.write_catalog(outfile="""%sSkymodel_substraction%s.reg"""%(self.preprocessSkymodelDir,self.i),catalog_type='gaul',format='ds9',correct_proj='True',srcroot='source')


		#write fits catalog
		img.write_catalog(outfile="""%sSkymodel_substraction%s.fits"""%(self.preprocessSkymodelDir,self.i),catalog_type='gaul',format='fits',correct_proj='True',srcroot='source')





	####################################################################		
	# Annulus Extraction
	####################################################################		
		
    def obsPreprocessAnnulusExtractionFunc(self):
			
		##define the radius (fov/2)
		if self.frequency > 1.9E8:
			radiusFOV	= self.OriginalFOV/2.0
		else:
			radiusFOV	= self.OriginalFOV/2.0
				
		
		# Output Skymodel 
		Skymodel		= """%sSkymodel_substraction%s"""%(self.preprocessSkymodelDir,self.i)
		out_annulus		= """%sSkymodel_substraction%s_annulus"""%(self.preprocessSkymodelDir,self.i)
		out_center		= """%sSkymodel_substraction%s_center"""%(self.preprocessSkymodelDir,self.i)				

		file_annulus	= open(out_annulus,'w')
		file_center		= open(out_center,'w')	
		
		cmd1	='format = Name, Type, Ra, Dec, I, Q, U, V, MajorAxis, MinorAxis, Orientation, ReferenceFrequency=\'%s\', SpectralIndex=\'[]\'\n'%(self.frequency)
		cmd2	='\n'
		
		file_annulus.write(cmd1)
		file_annulus.write(cmd2)
		file_center.write(cmd1)	
		file_center.write(cmd2)	

		inlist		= open(Skymodel,'r').readlines()
		nbsrc		= len(inlist)
		i=0
			
		ra_cat_hexa		= range(nbsrc-2)
		dec_cat_hexa	= range(nbsrc-2)	

		j=0
		for line in inlist:
				if j >= 2:
						linesplit				= line.split(',')
						ra_cat_hexa[j-2] 		= linesplit[2]
						dec_cat_hexa[j-2] 		= linesplit[3]

						ra		= ra_cat_hexa[j-2].split(':')
						dec		= dec_cat_hexa[j-2].split('.')
						dec[3]	= '0.'+dec[3]
						dec[2]	= float(dec[2])+float(dec[3])
						del dec[-1]
						
						
						ra_deg	= (float(float(ra[0])+((float(ra[1])+(float(ra[2])/60.))/60.)))*360.0/24.0
						
						if dec[0][1] == '-':
							dec_deg	= float(dec[0])-((float(dec[1])-(float(dec[2])/60.))/60.)
						if dec[0][1] == '+':	
							dec_deg	= float(dec[0])+((float(dec[1])+(float(dec[2])/60.))/60.)	
					
						ra_rad			= ra_deg*3.14/180.
						dec_rad			= dec_deg*3.14/180.

						ra_target_rad	= self.ra_target*3.14/180.
						dec_target_rad	= self.dec_target*3.14/180.
						
						distance	= (math.acos(math.sin(dec_rad)*math.sin(dec_target_rad)+math.cos(dec_rad)*math.cos(dec_target_rad)*math.cos(ra_target_rad-ra_rad)))*180/3.14
						
						
						
						if distance <= radiusFOV:
									file_center.write(line)									
															
						if distance > radiusFOV:
									file_annulus.write(line)
									
				j=j+1													


		#Close Sky models		
		file_annulus.close()
		file_center.close()	

		inlist_annulus	= open(out_annulus,'r').readlines()
		nb_annulus		= len(inlist_annulus)
	
	
		# check number of sources in the annulus
		if nb_annulus >2:
	
			nb_annulus = nb_annulus-2
			if self.i == 0:
				self.initNofAnnulusSources = nb_annulus
				
			
		return self.initNofAnnulusSources,nb_annulus
			



	####################################################################		
	# BBS Substraction  & NDPPP
	####################################################################		
		

    def obsPreprocessCreateBBSFunc(self):


		# Create NDPPP Iteration Directory
		self.NDPPPDir	= self.preprocessDir+"""NDPPP_Iter%s/"""%(self.i)
		if os.path.isdir(self.NDPPPDir) != True:
				cmd="""mkdir %s"""%(self.NDPPPDir)
				os.system(cmd)		
		
		
		

		# Create the BBS Parset file for annulus substraction
		
		self.BBSParset	= self.preprocessBBSDir+'BBS_substraction_Iter%s'%(self.i)

		Skymodel		= """%sSkymodel_substraction%s"""%(self.preprocessSkymodelDir,self.i)
		out_annulus		= """%sSkymodel_substraction%s_annulus"""%(self.preprocessSkymodelDir,self.i)
		out_center		= """%sSkymodel_substraction%s_center"""%(self.preprocessSkymodelDir,self.i)	



		#found annulus names		
		inlist_annulus	= open(out_annulus,'r').readlines()
		nb_annulus		= len(inlist_annulus)-2
		nameAnnulus		= range(nb_annulus)
		
		j = 0
		for line in inlist_annulus:
			if j >= 2:
				linesplit		= line.split(',')
				nameAnnulus[j-2]	= linesplit[0]
			j=j+1




		#found center names
		inlist_center	= open(out_center,'r').readlines()
		nb_center		= len(inlist_center)-2		
		nameCenter		= range(nb_center)			
		j = 0
		
		for line in inlist_center:
			if j >= 2:
				linesplit		= line.split(',')
				nameCenter[j-2]	= linesplit[0]
			j=j+1				
		

		 
		cmd1	= 'Strategy.InputColumn = DATA\n'
		cmd2	= 'Strategy.UseSolver = F\n'
		cmd3	= 'Strategy.Steps = [subtractfield, add_annulus, solve_annulus, subtract_annulus, add_center]\n'
		cmd4	= 'Strategy.ChunkSize = 100\n'
		cmd5	= 'Strategy.Baselines = [CR]S*&\n'
		cmd6	= '\n'
		cmd7	= 'Step.subtractfield.Operation = SUBTRACT\n'
		cmd8	= 'Step.subtractfield.Model.Sources = []\n'
		cmd9	= 'Step.subtractfield.Model.Beam.Enable = T\n'
		cmd10	= 'Step.subtractfield.Model.Beam.Mode = ARRAY_FACTOR\n'
		cmd11	= '\n'
		cmd12	= 'Step.add_annulus.Operation = ADD\n'
		cmd13	= 'Step.add_annulus.Model.Sources = %s\n'%(nameAnnulus)
		cmd14	= 'Step.add_annulus.Model.Beam.Enable = T\n'
		cmd15	= 'Step.add_annulus.Model.Beam.Mode = ARRAY_FACTOR\n'
		cmd16	= '\n'
		cmd17	= 'Step.solve_annulus.Operation = SOLVE\n'
		cmd18	= 'Step.solve_annulus.Model.Sources = %s\n'%(nameAnnulus)
		cmd19	= 'Step.solve_annulus.Model.Cache.Enable = T\n'
		cmd20	= 'Step.solve_annulus.Model.Beam.Enable = T\n'
		cmd21	= 'Step.solve_annulus.Model.Beam.Mode = ARRAY_FACTOR\n'
		cmd22	= 'Step.solve_annulus.Model.Phasors.Enable = F\n'
		cmd23	= 'Step.solve_annulus.Solve.Mode = COMPLEX\n'
		cmd24	= 'Step.solve_annulus.Solve.Parms = ["Gain:*:*:*:*"]\n'
		cmd25	= 'Step.solve_annulus.Solve.CellSize.Freq = 0\n'
		cmd26	= 'Step.solve_annulus.Solve.CellSize.Time = 5\n'
		cmd27	= 'Step.solve_annulus.Solve.CellChunkSize = 10\n'
		cmd28	= 'Step.solve_annulus.Solve.Options.MaxIter = 50\n'
		cmd29	= 'Step.solve_annulus.Solve.Options.EpsValue = 1e-9\n'
		cmd30	= 'Step.solve_annulus.Solve.Options.EpsDerivative = 1e-9\n'
		cmd31	= 'Step.solve_annulus.Solve.Options.ColFactor = 1e-9\n'
		cmd32	= 'Step.solve_annulus.Solve.Options.LMFactor = 1.0\n'
		cmd33	= 'Step.solve_annulus.Solve.Options.BalancedEqs = F\n'
		cmd34	= 'Step.solve_annulus.Solve.Options.UseSVD = T\n'
		cmd35	= '\n'
		cmd36	= 'Step.subtract_annulus.Operation = SUBTRACT\n'
		cmd37	= 'Step.subtract_annulus.Model.Sources = %s\n'%(nameAnnulus)
		cmd38	= 'Step.subtract_annulus.Model.Phasors.Enable = F\n'
		cmd39	= 'Step.subtract_annulus.Model.Beam.Enable = T\n'
		cmd40	= 'Step.subtract_annulus.Model.Beam.Mode = ARRAY_FACTOR\n'
		cmd41	= '\n'
		cmd42	= 'Step.add_center.Operation = ADD\n'
		cmd43	= 'Step.add_center.Model.Sources = %s\n'%(nameCenter)
		cmd44	= 'Step.add_center.Model.Beam.Enable = T\n'
		cmd45	= 'Step.add_center.Model.Beam.Mode = ARRAY_FACTOR\n'
		cmd46	= 'Step.add_center.Output.Column = CORRECTED_DATA\n'


		bbsFile = open(self.BBSParset,'w')
		
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
		bbsFile.write(cmd11)
		bbsFile.write(cmd12)
		bbsFile.write(cmd13)
		bbsFile.write(cmd14)
		bbsFile.write(cmd15)
		bbsFile.write(cmd16)
		bbsFile.write(cmd17)	
		bbsFile.write(cmd18)	
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
		bbsFile.write(cmd41)		
		bbsFile.write(cmd42)		
		bbsFile.write(cmd43)
		bbsFile.write(cmd44)		
		bbsFile.write(cmd45)			
		bbsFile.write(cmd46)
		
		bbsFile.close()			
		

		os.chdir(self.NDPPPDir)
		

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
			  skymodel_k	= """%sSkymodel_substraction%s"""%(self.preprocessSkymodelDir,self.i)
			  param_k		= """%sNDPPP_timeChunk%s_Iter%s"""%(self.NDPPPDir,j,self.i)
			  
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
		
		if self.i==0:
			cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,'%s%s'%(self.IterDir,files_k),self.BBSParset, skymodel_k)
			print ''
			print cmd_cal
			print ''
			os.system(cmd_cal)

		if self.i>0:
			cmd_cal="""calibrate-stand-alone -f -t %s %s %s %s"""%(core_index,'%s%s_sub%s'%(self.IterDir,files_k,self.i-1),self.BBSParset, skymodel_k)
			print ''
			print cmd_cal
			print ''
			os.system(cmd_cal)

									
		#Create NDPPP parsetFile
		file = open(param_k,'w')
		
		if self.i==0:
						
			cmd1 ="""msin = %s%s\n"""%(self.IterDir,files_k)
			cmd2 ="""msout = %s%s_sub%s\n"""%(self.IterDir,files_k,self.i)

		if self.i>0:
						
			cmd1 ="""msin = %s%s_sub%s\n"""%(self.IterDir,files_k,self.i-1)	
			cmd2 ="""msout = %s%s_sub%s\n"""%(self.IterDir,files_k,self.i)							
						
							

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
		file.write(cmd8)
		file.write(cmd9)
		file.write(cmd10)
		file.write(cmd11)
		
		file.close()			
		
		#Run NDPPP
		cmd_NDPPP = """NDPPP %s"""%(param_k)
		print ''
		print cmd_NDPPP
		print ''
		os.system(cmd_NDPPP)
		
		
		
		# Copy CORRECTED DATA Column to DATA column		
		self.copy_data("""%s%s_sub%s"""%(self.IterDir,files_k,self.i))	
		
		


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
		
		
