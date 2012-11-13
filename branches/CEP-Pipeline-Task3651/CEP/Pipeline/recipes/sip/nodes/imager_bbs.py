# LOFAR AUTOMATIC IMAGING PIPELINE
# imager_bbs
# Wouter Klijn 2012
# klijn@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.subprocessgroup import SubProcessGroup

class imager_bbs(LOFARnodeTCP):
    """
    imager_bbs node performs a bbs run for each of measuremt sets supplied in 
    the  mapfile at ms_list_path. Calibration is done on the sources in 
    the sourcedb in the mapfile sky_list_path. Solutions are stored in the 
    parmdb_list_path
    
    1. Load the mapfiles
    2. For each measurement set to calibrate start a subprocess
    3. Check if the processes finished correctly
    """
    def run(self, bbs_executable, parset, ms_list_path, parmdb_list_path,
             sky_list_path):
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
        ms_list = eval(open(ms_list_path).read())
        parmdb_list = eval(open(parmdb_list_path).read())
        sky_list = eval(open(sky_list_path).read())
        source_db = sky_list[0]["file"]
        try:
            bbs_process_group = SubProcessGroup(self.logger)
            # *****************************************************************
            # 2. start the bbs executable with data
            for (measurement_set, parmdm) in zip(
                                                ms_list, parmdb_list):

                command = [
                    bbs_executable,
                    "--sourcedb={0}".format(source_db),
                    "--parmdb={0}".format(parmdm) ,
                    measurement_set,
                    parset]
                self.logger.error(command)

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

        return 0


if __name__ == "__main__":
    _JOBID, _JOBHOST, _JOBPORT = sys.argv[1:4]
    sys.exit(imager_bbs(_JOBID, _JOBHOST, _JOBPORT).run_with_stored_arguments())
