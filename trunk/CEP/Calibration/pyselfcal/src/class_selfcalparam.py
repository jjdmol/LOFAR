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
import fpformat
import numpy as np

########################################################################
# Extra modules
########################################################################

#from lofar.selfcal import class_obsPreprocessing

import class_obsPreprocessing

########################################################################
## Define selfcalibration Parameters for cycles & prepare parsets
########################################################################



class selfCalParam:

    def __init__(self,obsDir,outputDir,listFiles,Files,NbFiles,nbChan,frequency,maxBaseline,integTimeOnechunk,observationIntegTime,nbCycle,ra_target,dec_target,outerfovclean,VLSSuse,preprocessIndex,FOV,nofPixelPerBeam,startResolution,endResolution,resolutionVector,startingFactor,robust,skyModel,UVmin,annulusRadius):

		################################################################	    
		# Initialization
		################################################################	
  
		self.obsDir					= obsDir
		self.outputDir				= outputDir
		self.listFiles 				= listFiles
		self.Files 					= Files
		self.NbFiles 				= NbFiles
		self.nbSB					= nbChan
		self.frequency 				= frequency
		self.maxBaseline 			= maxBaseline
		self.integTimeOnechunk 		= integTimeOnechunk
		self.observationIntegTime	= observationIntegTime
		self.nbCycle				= int(nbCycle)
		self.ra_target				= ra_target
		self.dec_target				= dec_target
		
		self.outerfovclean			= outerfovclean
		self.VLSSuse				= VLSSuse
		self.preprocessIndex		= preprocessIndex
		
		self.FOV					= FOV
		self.nofPixelPerBeam		= nofPixelPerBeam
		self.startResolution		= startResolution
		self.endResolution			= endResolution
		self.resolutionVector		= resolutionVector
		self.nbCycle				= nbCycle
		self.startingFactor			= startingFactor
		self.robust					= robust
		self.skyModel				= skyModel
		self.UVmin					= UVmin
		self.annulusRadius			= annulusRadius



	
    def selfCalParamFunc(self):
		

		################################################################	    
		#Load the Fov and correct if it is too large
		################################################################	    

		# Initialize extraction parameters
		thresh_isl				= 6
		thresh_pix				= 8
				
		
		if self.frequency > 1.9E8:
			
			print ''
			print '########### PreProcessing INFOS ########################'			
			print 'This is a High HBA observation. It requires a small field of view, and so a small annulus subraction.'
			print 'If FOV > 1.25 => FOV=1.25. If Annulus Radius >0.25 => Annulus Radius=0.25'
						
			if self.FOV > 1.25:
				self.FOV 			= 1.25
				
			
			self.thresh_isl	= 3
			self.thresh_pix	= 5				
				

			print 'Current FOV=%s degree'%(self.FOV,self.annulusRadius) 
			print 'Extraction Parameters changed for High HBA: thresh_isl=3 (instead 6); thresh_pix=5 (instead 8)'
			print ''
			print ''										
	
		else:
			print ''
			print '########### PreProcessing INFOS ########################'
			print 'This is a LOFAR HBA or LBA observation.'
			print 'If FOV > 15 => FOV=15 degree'
						
						
			if self.FOV > 15:
				self.FOV 			= 15
    
    
		################################################################	    
		#Load the nof pixel per beam, bad Resolution Factor, FOV  and initialize parameters
		################################################################	    
			
				
		pixPerBeam		= self.nofPixelPerBeam
		badResolFactor	= self.startingFactor
		fov				= self.FOV


		UVmax	= range(self.nbCycle)
		wmax	= range(self.nbCycle)
		robust	= range(self.nbCycle)
		pixsize	= range(self.nbCycle)
		
		nbpixel		= range(self.nbCycle)
		nbpixelTemp	= range(self.nbCycle)
		
		
		################################################################	    
		# define SELFCAL RUN PARAM: Resolution, UVmax, robust, pixel size etc ...
		################################################################	    


		#Adapt Number of pixel to right value for FT
	
		multipleOfTwo	= 2**(np.arange(15))
		multipleOfThree	= 3**(np.arange(10))
		multipleOfFive	= 5**(np.arange(6))
		
		matrixOfGoodNumber	= multipleOfTwo[:,np.newaxis,np.newaxis]*multipleOfThree[np.newaxis,:,np.newaxis]*multipleOfFive[np.newaxis,np.newaxis,:]
		matrixFlat			= matrixOfGoodNumber.ravel()
		matrixFlatSorted	= sorted(matrixFlat)
		
		

		# Case 0: no resolution vector, no start resolution and  no end resolution provided 
		
				
		if self.resolutionVector == ["none"] and self.startResolution == "none" and self.endResolution == "none":
		
			bestBeamresol	= float(fpformat.fix((3E8/self.frequency/self.maxBaseline)*180./3.14*3600.,0))
			bestPixelResol	= float(fpformat.fix(bestBeamresol/pixPerBeam,2))  

			kcycle	= range(self.nbCycle)
			
			for i in kcycle: 
			
				pixsize[i]	= float(fpformat.fix((badResolFactor*bestPixelResol)-(i*(badResolFactor*bestPixelResol-bestPixelResol)/(self.nbCycle-1)),3))

				nbpixelTemp[i]	= int(fov*3600./pixsize[i])
				nbpixel[i]		= int(matrixFlatSorted[np.searchsorted(matrixFlatSorted,nbpixelTemp[i])])
				
				if self.robust != "none":
					robust[i]	= self.robust
				else:	
					if self.nbCycle > 1:
						robust[i]	= float(fpformat.fix(1.0-(i*3.0/(self.nbCycle-1)),2))
					else:
						robust[i] = -2	
							
				UVmax[i]	= float(fpformat.fix((3E8/self.frequency)/(pixPerBeam*pixsize[i]/3600.*3.14/180.)/(1E3*3E8/self.frequency),3))
				wmax[i]		= float(fpformat.fix(UVmax[i]*(3E8/self.frequency)*1E3,3))




		# Case 1: no resolution vector,  start resolution and  end resolution provided
		
		if self.resolutionVector == ["none"] and self.startResolution != "none" and self.endResolution != "none":		
		
			bestBeamresol	= float(self.endResolution)
			bestPixelResol	= float(fpformat.fix(bestBeamresol/pixPerBeam,2))  

			worstBeamresol	= float(self.startResolution)
			worstPixelResol	= float(fpformat.fix(worstBeamresol/pixPerBeam,2))  


			kcycle	= range(self.nbCycle)
			
			for i in kcycle: 
			
				pixsize[i]	= float(fpformat.fix((worstPixelResol)-(i*(worstPixelResol-bestPixelResol)/(self.nbCycle-1)),3))

				nbpixelTemp[i]	= int(fov*3600./pixsize[i])
				nbpixel[i]		= int(matrixFlatSorted[np.searchsorted(matrixFlatSorted,nbpixelTemp[i])])
				
				if self.robust != "none":
					robust[i]	= self.robust
				else:	
					if self.nbCycle > 1:
						robust[i]	= float(fpformat.fix(1.0-(i*3.0/(self.nbCycle-1)),2))
					else:
						robust[i] = -2	
							
				UVmax[i]	= float(fpformat.fix((3E8/self.frequency)/(pixPerBeam*pixsize[i]/3600.*3.14/180.)/(1E3*3E8/self.frequency),3))
				wmax[i]		= float(fpformat.fix(UVmax[i]*(3E8/self.frequency)*1E3,3))		
		
		
		

		# Case 2: no resolution vector, no start resolution, just end resolution provided
		
		if self.resolutionVector == ["none"] and self.startResolution == "none" and self.endResolution != "none":
		
			bestBeamresol	= float(self.endResolution)
			bestPixelResol	= float(fpformat.fix(bestBeamresol/pixPerBeam,2))  

			kcycle	= range(self.nbCycle)
			
			for i in kcycle: 
			
				pixsize[i]	= float(fpformat.fix((badResolFactor*bestPixelResol)-(i*(badResolFactor*bestPixelResol-bestPixelResol)/(self.nbCycle-1)),3))

				nbpixelTemp[i]	= int(fov*3600./pixsize[i])
				nbpixel[i]		= int(matrixFlatSorted[np.searchsorted(matrixFlatSorted,nbpixelTemp[i])])
				
				if self.robust != "none":
					robust[i]	= self.robust
				else:	
					if self.nbCycle > 1:
						robust[i]	= float(fpformat.fix(1.0-(i*3.0/(self.nbCycle-1)),2))
					else:
						robust[i] = -2	
							
				UVmax[i]	= float(fpformat.fix((3E8/self.frequency)/(pixPerBeam*pixsize[i]/3600.*3.14/180.)/(1E3*3E8/self.frequency),3))
				wmax[i]		= float(fpformat.fix(UVmax[i]*(3E8/self.frequency)*1E3,3))		
		
		

		# Case 3: no resolution vector, no start resolution, no end resolution		

		if self.resolutionVector != ["none"] and self.startResolution == "none" and self.endResolution == "none":
		
			bestBeamresol	= float(self.resolutionVector[-1])
			bestPixelResol	= float(fpformat.fix(bestBeamresol/pixPerBeam,2))  

			kcycle	= range(self.nbCycle)
			
			for i in kcycle: 
			
				pixsize[i]	= float(fpformat.fix(self.resolutionVector[i]/pixPerBeam,3))

				nbpixelTemp[i]	= int(fov*3600./pixsize[i])
				nbpixel[i]		= int(matrixFlatSorted[np.searchsorted(matrixFlatSorted,nbpixelTemp[i])])
				
				if self.robust != "none":
					robust[i]	= self.robust
				else:	
					if self.nbCycle > 1:
						robust[i]	= float(fpformat.fix(1.0-(i*3.0/(self.nbCycle-1)),2))
					else:
						robust[i] = -2		
							
				UVmax[i]	= float(fpformat.fix((3E8/self.frequency)/(pixPerBeam*pixsize[i]/3600.*3.14/180.)/(1E3*3E8/self.frequency),3))
				wmax[i]		= float(fpformat.fix(UVmax[i]*(3E8/self.frequency)*1E3,3))	
		
		


		################################################################	    
		# WARNINGS
		################################################################	    
		
				
		memoryCriterion = self.nbSB*self.NbFiles*self.integTimeOnechunk/self.observationIntegTime	
		criterionLevel = 100000
		
		print '#################################################'
		print 'Input DATA caracteristics'
		print ''
		print 'You use data containing %s Subband'%(self.nbSB)
		print 'You have %s time chunk of %s  seconds'%(self.NbFiles,self.integTimeOnechunk)
		print 'And the observation integration time is every  %s seconds'%(self.observationIntegTime)
		print ''
		print 'Your Observation memory Criterion is: %s and the limit is %s'%(memoryCriterion,criterionLevel)
		print ''
		print '#################################################'	
		
		if memoryCriterion > criterionLevel:
		  
			print '#################################################'
			print ''
			print 'The quantity of data is very voluminous to be process on the node. Certainly the time computing will be extremly long, in addition the memory consumption could be huge !!' 
			print 'It is really recommended to reprocess (merging subbands phase) with less subbands or less time chunks.'
			print ''
			print '#################################################'
			
			
		if nbpixel[-1] > 12000:
				
			print '#################################################'
			print ''
			print 'The final number of pixel is greater than 15000, i.e the final image will be 15000x15000 pixels !!'
			print 'The Current final number of pixel is: %s pixels'%(nbpixel[-1])
			print 'It is very large! Certainly the time computing will be extremly long, in addition the memory consumption could be huge !!' 
			print 'It is really recommended to change your input parameters.' 
			print 'Current imaging parameters are: '
			print '      startResolution  = %s arcsec'%(self.startResolution) 
			print '      endResolution    = %s arcsec'%(self.endResolution) 
			print '      resolutionVector = %s arcsec'%(self.resolutionVector) 
			print '      startingFactor   = %s'%(badResolFactor) 
			print '      FOV              = %s degree'%(fov) 
			print '      nofPixelPerBeam  = %s pixel/beam'%(pixPerBeam) 
			print ''						
			print ''
			print '#################################################'		   
		   
		

		
		################################################################	    
		# Determine the Skymodel and its path
		################################################################	    
		
				
		SkymodelPath	= self.outputDir+'Skymodel/'
		GSMSkymodel		= SkymodelPath+'InitialSkymodel'

		if os.path.isdir(SkymodelPath) != True:
				cmd="""mkdir %s"""%(SkymodelPath)
				os.system(cmd)	



		# If a skyModel is provided
		if self.skyModel != "none":
			
			cmd	= 'cp %s %s'%(self.skyModel,GSMSkymodel)
			os.system(cmd)


		# If the Iniatial Skymodel is extracted from VLSS catalog
		if self.VLSSuse == 'yes':
				if self.frequency >1.9E8:
					# VLSS skymodel for High HBA		
					if os.path.isfile(GSMSkymodel) != True:	
						cmd="""gsm.py %s %s %s 3 0.5 0.01"""%(GSMSkymodel,self.ra_target,self.dec_target)
						print ''
						print cmd
						print ''
						os.system(cmd)						
				else:
					# VLSS skymodel for LBA/HBA		
					if os.path.isfile(GSMSkymodel) != True:	
						cmd="""gsm.py %s %s %s 10 0.5 0.01"""%(GSMSkymodel,self.ra_target,self.dec_target)
						print ''
						print cmd
						print ''
						os.system(cmd)					
					
			
		# If the Iniatial Skymodel is extracted a low resolution image			
		if self.VLSSuse == 'no':
			
				if self.outerfovclean == 'yes':
				
						cmd	= 'cp %sPreprocessDir/Skymodel/Skymodel_substraction%s_center %s'%(self.outputDir,self.preprocessIndex,GSMSkymodel)
						print ''
						print 'Annulus cleaning on, use the central sky model instead the VLSS one!'
						print cmd
						print ''
						os.system(cmd)
						
				
				

				if self.outerfovclean == 'no':
				
						preprocessDir	= '%sPreprocessDir/'%(self.outputDir)
						cmd="""mkdir %s"""%(preprocessDir)
						os.system(cmd)
						print ''
						print 'The PreProcess directory: %s\n has been created'%(preprocessDir)
						print ''

						preprocessImageDir	= '%sImage/'%(preprocessDir)
						cmd="""mkdir %s"""%(preprocessImageDir)
						os.system(cmd)
						print ''
						print 'The PreProcess Image directory: %s\n has been created'%(preprocessImageDir)
						print ''
					
						preprocessSkymodelDir	= '%sSkymodel/'%(preprocessDir)
						cmd="""mkdir %s"""%(preprocessSkymodelDir)
						os.system(cmd)
						print ''
						print 'The PreProcess Skymodel directory: %s\n has been created'%(preprocessSkymodelDir)
						print ''							
						

						preprocessBBSDir	= '%sBBS-Dir/'%(preprocessDir)
						cmd="""mkdir %s"""%(preprocessBBSDir)
						os.system(cmd)
						print ''
						print 'The PreProcess BBS directory: %s\n has been created'%(preprocessSkymodelDir)
						print ''
	
						
						i						=0
						initNofAnnulusSources	= 1000
						nIteration 				= 10000000
						
						if self.UVmin == "none":						
							if self.dec_target <= 35:
								UVmin = 0.1
							else:
								UVmin=0
								
						else:
							UVmin = self.UVmin
							
						
						obsPreprocess_Obj									= class_obsPreprocessing.obsPreprocessing(self.obsDir,preprocessDir,preprocessImageDir,preprocessSkymodelDir,preprocessBBSDir,i,self.listFiles,self.Files,self.NbFiles,self.frequency,UVmin,nIteration,self.ra_target,self.dec_target,initNofAnnulusSources,self.annulusRadius,self.FOV)
						obsPreprocess_Obj.obsPreprocessImagingFunc() 
						obsPreprocess_Obj.obsPreprocessSrcExtractionFunc()	
						initNofAnnulusSources,nb_annulus					= obsPreprocess_Obj.obsPreprocessAnnulusExtractionFunc()
						
						cmd	= 'cp %sPreprocessDir/Skymodel/Skymodel_substraction%s_center %s'%(self.outputDir,self.preprocessIndex,GSMSkymodel)
						print ''
						print 'Annulus cleaning on, use the central sky model instead the VLSS one!'
						print ''
						os.system(cmd)				
					
				
					
		
			
		
		#################################################	
		# Determine RMS_BOX (and bright's) to use
		#################################################				

		
		RMS_BOX		=range(2)
		RMS_BOX[0] 	= 80
		RMS_BOX[1] 	= 10
		
		RMS_BOX_Bright		= range(2) 
		RMS_BOX_Bright[0] 	= 40
		RMS_BOX_Bright[1] 	= 10



		 
		################################################################	    
		#Create the image directory 	
		################################################################	    
				
		
		ImagePathDir	= self.outputDir+'Image/'

		if os.path.isdir(ImagePathDir) != True:
			cmd="""mkdir %s"""%(ImagePathDir)
			os.system(cmd)	
		
		
		################################################################	    
		# Create the BBS Parset file for Phase calibration only 
		################################################################	    
		
		
		BBSDir		= self.outputDir+'BBS-Dir/'
		BBSParset	= BBSDir+'BBS-Parset-Phase-Only'
		
		if os.path.isdir(BBSDir) != True:
			cmd="""mkdir %s"""%(BBSDir)
			os.system(cmd)
		
		
		if os.path.isfile(BBSParset) != True:	
		
			fileBBS = open(BBSParset,'w')
		
			cmd1	= """Strategy.ChunkSize = 100\n"""
			cmd2	= """Strategy.Steps = [solve, correct]\n"""
			cmd3	= """Strategy.InputColumn = DATA\n"""
			cmd4	= '\n'
			cmd5	= """Step.solve.Operation = SOLVE\n"""
			cmd6	= """Step.solve.Model.Sources = []\n"""
			cmd7	= """Step.solve.Model.Gain.Enable = T\n"""
			cmd8	= """Step.solve.Model.Cache.Enable = T\n"""
			cmd9	= """Step.solve.Model.Beam.Enable = T\n"""
			cmd10	= """Step.solve.Model.Beam.UseChannelFreq = True\n"""
			cmd11	= """Step.solve.Model.Beam.Mode = ARRAY_FACTOR\n"""
			cmd12	= """Step.solve.Model.Ionosphere.Enable = F\n"""
			cmd13	= """Step.solve.Model.TEC.Enable = F\n"""
			cmd14	= """Step.solve.Model.Phasors.Enable = T # If solving for AMP or PHASE, or if in addition TEC is enabled. For TEC only, it's F.\n"""
			cmd15	= """Step.solve.Solve.Mode = COMPLEX  #Step.solve.Solve.Mode = COMPLEX\n"""
			cmd16	= """Step.solve.Solve.Parms = ["Gain:0:0:Phase:*", "Gain:1:1:Phase:*"]\n"""
			cmd17	= """Step.solve.Solve.CellSize.Freq = 0 # If not enough SNR, phase solve over entire band of concatenated data\n"""
			cmd18	= """Step.solve.Solve.CellSize.Time = 1\n"""
			cmd19	= """Step.solve.Solve.CellChunkSize = 100\n"""
			cmd20	= """Step.solve.Solve.PropagateSolutions = T\n"""
			cmd21	= """Step.solve.Solve.Options.MaxIter = 100\n"""
			cmd22	= """Step.solve.Solve.Options.EpsValue = 1e-9\n"""
			cmd23	= """Step.solve.Solve.Options.EpsDerivative = 1e-9\n"""
			cmd24	= """Step.solve.Solve.Options.ColFactor = 1e-9\n"""
			cmd25	= """Step.solve.Solve.Options.LMFactor = 1.0\n"""
			cmd26	= """Step.solve.Solve.Options.BalancedEqs = F\n"""
			cmd27	= """Step.solve.Solve.Options.UseSVD = T\n"""
			cmd28	= '\n'
			cmd29	= """Step.correct.Operation = CORRECT\n"""
			cmd30	= """Step.correct.Model.Sources = []\n"""
			cmd31	= """Step.correct.Model.Gain.Enable = T\n"""
			cmd32	= """Step.correct.Model.Beam.Enable = F\n"""
			cmd33	= """Step.correct.Model.Beam.UseChannelFreq = True\n"""
			cmd34	= """Step.correct.Model.TEC.Enable = F\n"""
			cmd35	= """Step.correct.Model.Phasors.Enable = T\n"""
			cmd36	= """Step.correct.Output.Column = CORRECTED_DATA\n"""
			cmd37	= """Step.correct.Output.WriteCovariance = T"""
			
			fileBBS.write(cmd1)
			fileBBS.write(cmd2)
			fileBBS.write(cmd3)
			fileBBS.write(cmd4)
			fileBBS.write(cmd5)
			fileBBS.write(cmd6)
			fileBBS.write(cmd7)
			fileBBS.write(cmd8)
			fileBBS.write(cmd9)
			fileBBS.write(cmd10)
			fileBBS.write(cmd11)
			fileBBS.write(cmd12)
			fileBBS.write(cmd13)
			fileBBS.write(cmd14)
			fileBBS.write(cmd15)
			fileBBS.write(cmd16)
			fileBBS.write(cmd17)
			fileBBS.write(cmd18)
			fileBBS.write(cmd19)
			fileBBS.write(cmd20)
			fileBBS.write(cmd21)
			fileBBS.write(cmd22)
			fileBBS.write(cmd23)
			fileBBS.write(cmd24)
			fileBBS.write(cmd25)
			fileBBS.write(cmd26)
			fileBBS.write(cmd27)
			fileBBS.write(cmd28)
			fileBBS.write(cmd29)
			fileBBS.write(cmd30)
			fileBBS.write(cmd31)
			fileBBS.write(cmd32)
			fileBBS.write(cmd33)
			fileBBS.write(cmd34)
			fileBBS.write(cmd35)
			fileBBS.write(cmd36)
			fileBBS.write(cmd37)
		
			fileBBS.close()
		
		################################################################
		# Print an overview of global parameters
		################################################################	    
		
		print ''
		print 'the calibration Field Of View is: %s degree'%(fov)
		print 'Cycle N |Nof pixel | Pixel size | Robust param | UVmax | wmax |'
		for i in kcycle:
			print 'Cycle N %s|'%(i),nbpixel[i],'|',pixsize[i],'|',robust[i],'|',UVmax[i],'|',wmax[i],'|'
		print ''	
		
				
			
		return ImagePathDir,pixsize,nbpixel,robust,UVmax,wmax,SkymodelPath,GSMSkymodel,RMS_BOX,RMS_BOX_Bright,BBSParset,thresh_isl,thresh_pix
		 
		
		
	

	  
	    
	  
