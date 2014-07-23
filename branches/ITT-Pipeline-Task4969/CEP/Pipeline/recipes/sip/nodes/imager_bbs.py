# LOFAR AUTOMATIC IMAGING PIPELINE
# imager_bbs
# Wouter Klijn 2012
# klijn@astron.nl
# Nicolas Vilchez, 2014
# vilchez@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys
import os

import pyrap.tables as pt

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.subprocessgroup import SubProcessGroup
from lofarpipe.support.data_map import MultiDataMap

class imager_bbs(LOFARnodeTCP):
    """
    imager_bbs node performs a bbs run for each of measuremt sets supplied in 
    the  mapfile at ms_list_path. Calibration is done on the sources in 
    the sourcedb in the mapfile sky_list_path. Solutions are stored in the 
    parmdb_list_path
    
    1. Load the mapfiles
    2. For each measurement set to calibrate start a subprocess
    3. Check if the processes finished correctly
    4. (added by Nicolas vilchez) concat in time the final MS
    5. (added by N.Vilchez) copy time slives directory to a new one       
    """
    
    def run(self, bbs_executable, parset, ms_list_path, parmdb_list_path,
             sky_list_path, concat_ms_path, major_cycle):
        """
        imager_bbs functionality. Called by framework performing all the work
        """
        self.logger.debug("Starting imager_bbs Node")
        # *********************************************************************
        # 1. Load mapfiles
        # read in the mapfiles to data maps: The master recipe added the single
        # path to a mapfilem which allows usage of default data methods 
        # (load_data_map)
        # TODO: Datamap
        ms_map = MultiDataMap.load(ms_list_path)
        parmdb_map = MultiDataMap.load(parmdb_list_path)
        sky_list = MultiDataMap.load(sky_list_path)
        source_db = sky_list[0].file[0] # the sourcedb is the first file entry
      
        try:
            bbs_process_group = SubProcessGroup(self.logger)
            # *****************************************************************
            # 2. start the bbs executable with data
            for (measurement_set, parmdm) in zip(ms_map[0].file,
                                                parmdb_map[0].file):
                command = [
                    bbs_executable,
                    "--sourcedb={0}".format(source_db),
                    "--parmdb={0}".format(parmdm) ,
                    measurement_set,
                    parset]
                self.logger.info("Executing bbs command: {0}".format(" ".join(
                            command)))
                bbs_process_group.run(command)

            # *****************************************************************
            # 3. check status of the processes
            if bbs_process_group.wait_for_finish() != None:
                self.logger.error(
                            "Failed bbs run detected Aborting")
        except OSError, exception:
            self.logger.error("Failed to execute bbs: {0}".format(str(
                                                                    exception)))
            return 1
            
        # *********************************************************************
        # 4. Concat in time after bbs calibration your MSs using 
        #    msconcat (pyrap.tables module) (added by N.Vilchez)        
        # this step has te be performed on this location. because the bbs run 
        # might add additional columns not present in the original ms
        # and therefore not produced in the concat done in the prepare phase
        # redmine issue #6021     
        pt.msconcat(ms_map[0].file,concat_ms_path, concatTime=True)                 
 
        # *********************************************************************
        # 5. copy time slives directory to a new one  
        # This is done for debugging purpose: The copy is not used for anything
        # The actual selfcal steps are done in place
        #  (added by N.Vilchez)                   
        # THe save location is created relative to the concat.ms
        # we could also use the self.scratch_directory from the toplevel recipe
        # this would need an aditional ingredient
        # This is a 'debugging' step and should never ever cause a failure of \
        # the pipeline
        try:
            working_dir = os.path.dirname(concat_ms_path)
            time_slice_dir = os.path.join(working_dir, 'time_slices')
            time_slice_copy_dir = os.path.join(working_dir, 
              'time_slices_cycle_{0}'.format(major_cycle))

            cmd = "cp -r {0} {1}".format(time_slice_dir, time_slice_copy_dir) 
            os.system(cmd)     
        except:
          self.logger.error(
               "Debug copy of temporary files failed: continue operations")
          pass # Do nothing       

        return 0


if __name__ == "__main__":
    _JOBID, _JOBHOST, _JOBPORT = sys.argv[1:4]
    sys.exit(imager_bbs(_JOBID, _JOBHOST, _JOBPORT).run_with_stored_arguments())
