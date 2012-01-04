# LOFAR AWIMAGER RECIPE
# Wouter Klijn 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------

from __future__ import with_statement
import os
import sys
import tempfile  
import shutil  
import os.path 
import math

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import get_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import catch_segfaults
from lofar.parameterset import parameterset 
import pyrap.tables as pt
from subprocess import CalledProcessError 


class AWImager(LOFARnodeTCP):
    def run(self, executable, init_script, parset, working_dir, measurement_set):
        self.logger.info("Start AWImager run: client")   
        log4CPlusName = "AWImagerRecipe" 
        if not os.access(executable, os.X_OK):
            self.logger.error("Could not find executable: {0}".format(
                                                             executable))        
            return 1    
        
        # Time execution of this job
        with log_time(self.logger):
            # Calculate AWImager parameters that depend on measurement set                 
            cell_size, npix, w_max, w_proj_planes = \
                self._calc_par_from_measurement(measurement_set, parset)          
            
            # Update the parset with calculated parameters
            patch_dictionary = {'uselogger': 'True',  # enables log4cpluscd log
                               'ms': measurement_set,  
                               'cellsize': cell_size,
                               'npix': npix,
                               'wmax': w_max,
                               'wprojplanes':w_proj_planes
                               }     
            try:
                temp_parset_filename = patch_parset(parset, patch_dictionary)
            except Exception, e:
                self.logger.error("failed loading and updating the parset: {0}",
                                  format(parset))
                self.logger.error(e)
                return 1
            
            # The command and parameters to be run
            cmd = [executable, temp_parset_filename]
            
            #run awimager
            try:
                environment = read_initscript(self.logger, init_script) 
                with CatchLog4CPlus(working_dir, self.logger.name + "." + os.path.basename(log4CPlusName), os.path.basename(executable)) as logger:                                                          
                        catch_segfaults(cmd, working_dir, environment, logger,
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
                os.unlink(temp_parset_filename)

        return 0
    
    def _nearest_ceiled_power2(self, value):
        '''
        Return int value of  the nearest Ceiled power of 2 for the 
        suplied argument
        
        '''
        return int(pow(2, math.ceil(math.log(value, 2)))) 
    
    def _calc_par_from_measurement(self, measurement_set, parset):
        """
        calculate and format some parameters that are determined runtime based
        on values in the measurement set:
        1: The cellsize
        2: The npixels in a each of the two dimension of the image
        3. What columns use to determine the maximum baseline
        4. The number of projection planes (if > 512 the ask George heald 
        
        """
        baseline_limit = get_parset(parset).getInt('maxbaseline')
                           
        ard_sec_in_degree = 3600
        arc_sec_in_rad = (180.0 / math.pi) * ard_sec_in_degree
        
        # Calculate the cell_size         
        max_baseline = pt.taql('CALC sqrt(max([select sumsqr(UVW[:2]) from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory]))'.format( \
            measurement_set, baseline_limit *
            baseline_limit))[0]
            
        #waveLength = pt.taql('CALC C()/REF_FREQUENCY from {0}/SPECTRAL_WINDOW'.format(measurement_set))[0]
            
        t=pt.table(measurement_set)
        t1 = pt.table(t.getkeyword("SPECTRAL_WINDOW"))
        freq =t1.getcell("REF_FREQUENCY",0)
        waveLength = pt.taql('CALC C()') / freq
        t1.close()

        cell_size = (1.0/3) * (waveLength/float(max_baseline)) * arc_sec_in_rad            
                         
        # Calculate the number of pixels in x and y dim
        #    fov and diameter depending on the antenna name
        fov = None
        station_diameter = None
        antenna = pt.table(t.getkeyword("ANTENNA"))
        antenna_name = antenna.getcell('NAME',0)
        antenna.close()
        t.close()
                          
        if antenna_name.count('HBA'):
            fov = 6  #(degrees)
            station_diameter = 35 #(meters)
        elif antenna_name.count('LBA'):
            fov = 3
            station_diameter = 30
        else:
            self.logger.error('unknow antenna type for antenna: {0}').format( \
                                                                antenna_name)
            return 1
           
        npix = (ard_sec_in_degree * fov) / cell_size
        npix = self._nearest_ceiled_power2(npix)
            
        # Get the max w with baseline < 10000
        w_max = pt.taql('CALC max([select UVW[2] from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory])'.format(  
            measurement_set, baseline_limit * baseline_limit))[0]
                
        # Calculate number of projection planes
        w_proj_planes = (max_baseline * waveLength) / (station_diameter ** 2)
        w_proj_planes = int(round(w_proj_planes))
        if w_proj_planes > 511:
            raise Exception("The number of projections planes for the current" +
                            "measurement set is to large.")  #FIXME: Ask george 
        cell_size_formatted = str(int(round(cell_size))) + 'arcsec'
        return cell_size_formatted, str(npix), str(w_max), str(w_proj_planes)
    

if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(AWImager(jobid, jobhost, jobport).run_with_stored_arguments())
