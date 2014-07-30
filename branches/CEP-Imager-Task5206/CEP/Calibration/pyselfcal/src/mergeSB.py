#!/usr/bin/env python



# IMPORT general modules
import sys,os,commands,glob,time
import getopt

import pyrap.tables as pt
import numpy as np

# Import local modules (classes)
from lofar.selfcal import class_obspar
from lofar.selfcal import class_mergesubbands
    


#############################
# check the Launch
#############################


def main(initparameterlist):

	try:
	      opts, args = getopt.getopt(sys.argv[1:], "hooc:",     ["help", "obsDir=", "outputDir=", "column2Merge="])
      
	except getopt.GetoptError as err:
	      print "Usage: mergeSB.py --obsDir= --outputDir= --column2Merge="
	      print "toto"
	      sys.exit(2)
	      
	         	
	for par1, par2 in opts:
		
		if par1 in ("--help"):
			print ""
        		print "Usage: mergeSB.py --obsDir= --outputDir= --column2Merge="
			print ""
        		sys.exit(2)
        
        	
		elif par1 in ("--obsDir"):
			initparameterlist[0]=par2
		elif par1 in ("--outputDir"):
			initparameterlist[1]=par2		
		elif par1 in ("--column2Merge"):
			initparameterlist[2]=par2								
		else:
        		print("Option {} Unknown".format(par1))
        		sys.exit(2)


        		
        # Check parameters		
 	if initparameterlist[0] == "none" or initparameterlist[1] == "none" or initparameterlist[2] == "none":
		print ""
		print "MISSING Parameters"	
        	print "Usage: mergeSB.py --obsDir= --outputDir= --column2Merge="
		print ""
        	sys.exit(2)       	
 
 		
 	return initparameterlist 





    
##############################    
# Main Program
##############################
    


if __name__=='__main__':
  
  
    ######################    
    #Inputs
    ######################  
    initparameterlist=range(3)
    
    initparameterlist[0]	= "none"	# Observation Directory
    initparameterlist[1]	= "none"	# Output Directory
    initparameterlist[2]	= "none"	# Number Of cycle for the selfcal loop



    # Read and check parameters	
    initparameterlist = main(initparameterlist); 
  
    obsDir		= initparameterlist[0]
    outputDir	= initparameterlist[1]
    colum2Merge	= initparameterlist[2]
    
    
    ######################    
    #Warnings
    ######################
    
    
    ### WARNINGS on the obsDir 
    if obsDir[-1] != '/':
	obsDir = obsDir+'/'
	
    if os.path.isdir(obsDir) != True:
	print ""
	print "The observation directory do not exists ! Check it Please."
	print ""
	sys.exit(2)  
   
   
    ### WARNINGS on the outputDir 
    if outputDir[-1] != '/':
	outputDir = outputDir+'/'
	
    if os.path.isdir(outputDir) != True:
	cmd="""mkdir %s"""%(outputDir)
	os.system(cmd)
	print ""
	print """The output directory do not exists !\n%s has been created"""%(outputDir)
	print ""
	
	
    ######################   
    ## End of warnings
    ######################
   
   
   
   
    ######################   
    #Main code started Now !!!!
    ######################

    #######################################################################################################################
    
    print ''
    print '###########################################################'
    print 'Start Observation directory global parameters determination'
    print '###########################################################\n'
  
    # Observation Directory Parameter determination
    obsPar_Obj						= class_obspar.observationParam(obsDir)
    listFiles,obsType,IDs,SAPId,NbSB,StartSB,EndSB	= obsPar_Obj.obsParamFunc()    

    print """		IDs used:%s\n"""%(IDs)
    print """		Number of subbands used:%s\n"""%(NbSB)
    print """		Starting Subband:%s   Ending Subband:%s\n"""%(StartSB,EndSB)
    
    print '#########################################################'
    print 'End Observation directory global parameters determination'
    print '#########################################################\n'
    

 
    print '#######################################################################################################################'
    
	
    print '###############################'
    print 'Start time chunks concatenation'
    print '###############################\n'  	
	

    # Merge the subbands by time chunks now
    mergeSubbands_Obj	= class_mergesubbands.mergeSubbands(obsDir,outputDir,colum2Merge,listFiles,IDs,obsType,SAPId,NbSB,StartSB,EndSB)
    mergeSubbands_Obj.mergeSubbandsFunc()

    print '################################'
    print 'End of time chunks concatenation'
    print '################################\n'    

    
    #######################################################################################################################
