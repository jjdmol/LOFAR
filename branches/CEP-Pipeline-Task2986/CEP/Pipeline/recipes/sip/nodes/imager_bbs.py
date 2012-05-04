# LOFAR AUTOMATIC IMAGING PIPELINE
# imager_bbs
# Wouter Klijn 2012
# klijn@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys
import time
import subprocess

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import log_process_output
from lofarpipe.support.group_data import load_data_map

class imager_bbs(LOFARnodeTCP):
    """
    imager_bbs node performs a bbs based on the supplied parset it is a shallow
    wrapper around bbs
    It starts bbs on a new subprocess and logs the output aborting on failure   
    """
    def run(self, bbs_executable, parset, ms_list_path, parmdb_list_path,
             sky_list_path):
        # read in the mapfiles to data maps: The master recipe added the single
        # path to a mapfilem which allows usage of default data methods (load_data_map)
        node, ms_list = load_data_map(ms_list_path)[0]
        node, parmdb_list = load_data_map(parmdb_list_path)[0]
        node, sky_list = load_data_map(sky_list_path)[0]

        self.logger.debug("Starting imager_bbs Node")
        try:
            process_list = []
            # *****************************************************************
            # Collect the input data, run the bbs executable with data
            for (ms, parmdm, sky) in zip(ms_list, parmdb_list, sky_list):
                command = [
                    bbs_executable,
                    "--sourcedb={0}".format(sky),
                    "--parmdb={0}".format(parmdm) ,
                    ms,
                    parset]

                # Spawn a subprocess and connect the pipelines
                bbs_process = subprocess.Popen(
                        command,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)

                process_list.append(bbs_process)

            # *****************************************************************
            # check if all the runs have finished correctly
            bbs_failed_run = False
            while(True):
                finished = True
                for idx, bbs_process in enumerate(process_list):
                    #If the process has not finished
                    if bbs_process.poll() == None:
                        finished = False  #set stopper to False

                    if bbs_process.poll() > 0:
                        self.logger.error(
                            "Failed bbs run detected at idx {0} in the input.".format(
                            idx))
                        bbs_failed_run = True

                        # Stop checking: we have a failed run.  
                        break

                # check if finished
                if finished:
                    break

                # wait for a second and try again 
                time.sleep(1)

            # *****************************************************************
            # Collect output, wait for processes if needed!
            for bbs_process in process_list:
                sout, serr = bbs_process.communicate()

            # Log the output
                log_process_output("imager_bbs", sout, serr, self.logger)
        except OSError, e:
            self.logger.error("Failed to execute bbs: {0}".format(str(e)))
            return 1

        if bbs_failed_run == True:
            return 1

        return 0


if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_bbs(jobid, jobhost, jobport).run_with_stored_arguments())
