# Prepare fase of the LOFAR AWIMAGER RECIPE
# collect datasets and start prepropressing fases
# Wouter Klijn 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------

from __future__ import with_statement
#import os
import sys
#import tempfile  
#import shutil  
import os.path
#import math

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import get_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import  LOFARnodeTCP
import pyrap.tables as pt                   #@UnresolvedImport
from subprocess import CalledProcessError

 
class prepare_imager(LOFARnodeTCP):
    def run(self, executable, initScript, parset, workingDir):        
        log4CPlusName = "prepare_imagerRecipe" 
        
        # Time execution of this job 
        # todo do we need executable?
        #maybee multiple
        with log_time(self.logger):
            if not os.access(executable, os.X_OK):
                self.logger.error("Could not find executable: {0}".format(
                                                                executable))         
                return 1

#            # Calculate AWImager parameters that depend on measurement set      
#            maxbaseline = get_parset(parset).getInt('maxbaseline')
#            cellSize, npix, wMax, wProjPlanes = \
#                self._calcParamFromMeasurement(measurementSet, maxbaseline)
#            cellSizeFormatted = str(int(round(cellSize))) + 'arcsec'
            
            # Update the parset (supplied to awimager) with calculated parameters
#            patchDictionary = {'uselogger': 'True',  # enables log4cpluscd log
#                               'ms': measurementSet,  
#                               'cellsize': cellSizeFormatted,
#                               'npix': str(npix),
#                               'wmax': str(wMax),
#                               'wprojplanes':str(wProjPlanes)
#                                }      
            patchDictionary = {}
            try:
                tempParsetFilename = patch_parset(parset, patchDictionary)
            except Exception, e:
                self.logger.error("failed loading and updating the parset: {0}",
                                  format(parset))
                self.logger.error(e)
                return 1
            
            # The command and parameters to be run
            cmd = [executable, tempParsetFilename]
                              
            try:
                environment = read_initscript(self.logger, initScript)               
                with CatchLog4CPlus(workingDir,
                    self.logger.name + "." + os.path.basename(log4CPlusName),
                    os.path.basename(executable)) as logger:                      
                        catch_segfaults(cmd, workingDir, environment, logger,
                                    cleanup=None)
            
            # Thrown by catch_segfault
            except CalledProcessError, e:                
                self.logger.error(str(e))
                return 1
            
            except Exception, e:
                self.logger.error(str(e))
                return 1
            
            finally:
                # Cleanup of temporary parameterset
                os.unlink(tempParsetFilename)

        return 0
    
#    def _nearestCeiledPower2(self, value):
#        '''
#        Return int value of  the nearest Ceiled power of 2 for the 
#        suplied argument
#        
#        '''
#        return int(pow(2, math.ceil(math.log(value, 2)))) 
#    
#    def _calcParamFromMeasurement(self, measurementSet, 
#                                            baselineLimit):                   
#        ardSecInDegree = 3600
#        arcSecInRad = (180.0 / math.pi) * ardSecInDegree
#        
#        # Calculate the cellSize         
#        maxBaseline = pt.taql('CALC sqrt(max([select sumsqr(UVW[:2]) from ' + \
#            '{0} where sumsqr(UVW[:2]) <{1} giving as memory]))'.format( \
#            measurementSet, baselineLimit *
#            baselineLimit))[0]
#            
#        waveLength = pt.taql('CALC C()/REF_FREQUENCY ' + \
#            'from {0}/SPECTRAL_WINDOW'.format(measurementSet))[0]
#
#        cellSize = (1.0/3) * (waveLength/float(maxBaseline)) * arcSecInRad            
#                         
#        # Calculate the number of pixels in x and y dim
#        #    fov and diameter depending on the antenna name
#        fov = None
#        stationDiameter = None
#        antennaName = pt.taql('select NAME from {0}/ANTENNA'.format( \
#                          measurementSet)).getcell('NAME',0)
#        if antennaName.count('HBA'):
#            fov = 6  #(degrees)
#            stationDiameter = 35 #(meters)
#        elif antennaName.count('LBA'):
#            fov = 3
#            stationDiameter = 30
#        else:
#            self.logger.error('unknow antenna type for antenna: {0}').format( \
#                                                                antennaName)
#            return 1
#           
#        npix = (ardSecInDegree * fov) / cellSize
#        npix = self._nearestCeiledPower2(npix)
#            
#        # Get the max w with baseline < 10000
#        wMax = pt.taql('CALC max([select UVW[2] from ' + \
#            '{0} where sumsqr(UVW[:2]) <{1} giving as memory])'.format(  
#            measurementSet, baselineLimit*baselineLimit))[0]
#                
#        # Calculate number of projection planes
#        wProjPlanes = (maxBaseline * waveLength) / (stationDiameter ** 2)
#        wProjPlanes = int(round(wProjPlanes))
#        
#        return cellSize, npix, wMax, wProjPlanes
    

if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(prepare_imager(jobid, jobhost, jobport).run_with_stored_arguments())
