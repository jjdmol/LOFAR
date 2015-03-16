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
import os,sys,glob
import pyrap.tables as pt 
import numpy as np



########################################################################
# Merge Subbands per time chunks
########################################################################


class mergeSubbands:

    def __init__(self,obsDir,outputDir,colum2Merge,listFiles,IDs,obsType,SAPId,NbSB,StartSB,EndSB,nofChannel2Merge):

	####################################################################
	# Initialization
	####################################################################
    
	self.obsDir 			= obsDir
	self.outputDir			= outputDir
	self.colum2Merge		= colum2Merge
	self.listFiles			= listFiles
	self.IDs				= IDs
	self.obsType			= obsType
	self.SAPId				= SAPId
	self.NbSB				= NbSB
	self.StartSB			= StartSB
	self.EndSB				= EndSB
	self.nofChannel2Merge	= nofChannel2Merge
  
    def mergeSubbandsFunc(self):

	####################################################################
	# Determine the number of channel per subbands
	####################################################################
    	
	
	tab 	= pt.table(self.listFiles[0], readonly=False, ack=True)
	data 	= tab.getcol('DATA')
	weight	= tab.getcol('WEIGHT')
	
	NbChannelPerSubband	= data.size/weight.size
	freqStep			= NbChannelPerSubband
	
	
	# Check the nof channel per subbands
	
	if self.nofChannel2Merge != 'none':
		
			if self.nofChannel2Merge > NbChannelPerSubband:
				
					print '#################################################'
					print ''
					print 'The required number of channel per subband is greater than the current number of channel per subband in the data!'
					print 'So the number of channel per subband is set to the maximum possible value: %s channel per subband'%(NbChannelPerSubband)
					print ''
					print '#################################################'
					
					freqStep = 1
					
			else: 
			
				freqStep = int(NbChannelPerSubband/self.nofChannel2Merge)
			
	
	
	# Define the time chunks loop
	k 	= range(len(self.IDs)) 
	kSB	= range(self.NbSB)
	

	####################################################################
	# NDPPP Warning checks
	####################################################################	
	
	# define the output for the NDPPP parsets 
	outputNDPPPParsetDir	= """%sNDPPP-SUMSB-Parset/"""%(self.outputDir)

	### WARNINGS on the outputNDPPPParsetDir 
	if os.path.isdir(outputNDPPPParsetDir) != True:
	    cmd="""mkdir %s"""%(outputNDPPPParsetDir)
	    os.system(cmd)
	    
	  
	  
	  
	# define the output for the merged data    
	outputMergedDataDir	= """%sMergedDATA/"""%(self.outputDir)	    
	
	### WARNINGS on the outputMergedDataDir 
	if os.path.isdir(outputMergedDataDir) != True:
	    cmd="""mkdir %s"""%(outputMergedDataDir)
	    os.system(cmd)
	    	    
	    

	####################################################################
	# Parsets generation
	####################################################################	
	
	# Loop on the time chunks
	for i in k:			
		
		param="""%s/NDPPP_avg_%s.parset"""%(outputNDPPPParsetDir,self.IDs[i])
		file = open(param,'w')
	
		listTimeChunk	= []
		
		
		# intermediate data
		if self.obsType == 'intermediateData':
			
		      for j in kSB:
			    listTimeChunk.append("""%s%s"""%(self.obsDir,self.IDs[i])+'_SAP'+self.SAPId+'_SB'+format(self.StartSB+j,'03d')+'_uv.MS.dppp')
			    
		 
		 
		# final data products
		if self.obsType == 'finalData':
		      
		      for j in kSB:
			    listTimeChunk.append("""%s%s"""%(self.obsDir,self.IDs[i])+'_SB'+format(self.StartSB+j,'03d')+'_uv.dppp.MS')
		
		cmd1="""msin = %s\n"""%(listTimeChunk)
		cmd2="""msout = %s%s_SB%s_%s\n"""%(outputMergedDataDir,self.IDs[i],format(self.StartSB,'03d'),format(self.EndSB,'03d'))
				
		
		cmd3="""msin.missingdata = true\n"""
		cmd4="""msin.orderms=False\n"""
		cmd5 ="""msin.autoweight = false\n"""
		cmd6 ="""msin.forceautoweight = false\n"""
		cmd7 ="""msin.datacolumn = %s\n"""%(self.colum2Merge)
		
		cmd8 ="""steps=[preflag,count,avg1,flag1]\n"""
		
		cmd9 ="""preflag.type=preflagger\n"""
		cmd10 ="""preflag.corrtype=auto\n"""

		cmd11 ="""preflag.baseline= [DE*, FR*, SE*, UK*]\n"""
		cmd12 ="""preflag.amplmin = 1e-30 \n"""		

		cmd13 ="""avg1.type=squash\n"""
		cmd14 ="""avg1.freqstep = %s\n"""%(freqStep)
		cmd15 ="""avg1.timestep=1\n"""
		
		cmd16 ="""flag1.type=aoflagger\n"""
		
		#cmd17 ="""flag1.type=madflagger\n"""
		#cmd18 ="""flag1.threshold=4\n"""
		#cmd19 ="""flag1.freqwindow=1\n"""
		#cmd21="""flag1.timewindow=1\n"""
		#cmd22 ="""flag1.correlations=[0,1,2,3]   # only flag on XX and YY [0,3]\n"""
		

	
	
		file.write(cmd1)
		file.write(cmd2)
			
		file.write(cmd3)			
		file.write(cmd4)
		file.write(cmd5)
		file.write(cmd6)
		file.write(cmd7)
		
		file.write(cmd8)
		
		file.write(cmd9)
		file.write(cmd10)
		
		file.write(cmd11)		
		file.write(cmd12)
		
		file.write(cmd13)
		file.write(cmd14)
		file.write(cmd15)
		
		file.write(cmd16)		
		
		#file.write(cmd17)
		#file.write(cmd18)
		#file.write(cmd19)
		#file.write(cmd20)
		#file.write(cmd21)
		
		
			
		file.close()
		
		
		
	####################################################################
	# Running NDPPP for frequency concatenation
	####################################################################		
		
		if os.path.isdir("""%s%s_SB%s_%s"""%(outputMergedDataDir,self.IDs[i],format(self.StartSB,'03d'),format(self.EndSB,'03d'))) != True:
		      
		      cmd="""NDPPP %s"""%(param)
		      os.system(cmd)
		      
		      
		      print ''
		      print '#############'
		      if  os.path.isdir("""%s%s_SB%s_%s"""%(outputMergedDataDir,self.IDs[i],format(self.StartSB,'03d'),format(self.EndSB,'03d'))) != True:
			
					print """Time chunk ID:%s concatenation in subbands FAILED"""%(self.IDs[i])
					print '#############\n'		      
		      
		      else:
					print """Time chunk ID:%s has been concatenated in subbands"""%(self.IDs[i])
					print '#############\n'		
					self.copy_data("""%s%s_SB%s_%s"""%(outputMergedDataDir,self.IDs[i],format(self.StartSB,'03d'),format(self.EndSB,'03d')))
					print ''	
		      
		else:
		      print '#############'
		      print """%s%s_SB%s_%s\n"""%(outputMergedDataDir,self.IDs[i],format(self.StartSB,'03d'),format(self.EndSB,'03d'))+"""exits, don t need to concatenate %s in Subbands"""%(self.IDs[i])
		      print '#############\n'
	


		



	####################################################################
	# Function for copy data from DATA column to CORRECTED DATA column
	####################################################################
	
    def copy_data(self,inms):
		
			## Create corrected data colun and Put data to corrected data column 
			
			print inms
			t = pt.table(inms, readonly=False, ack=True)
			data = t.getcol('DATA')
			pt.addImagingColumns(inms, ack=True)
			t.putcol('CORRECTED_DATA', data)
			t.close()		
		
	      

	
	
	
	
