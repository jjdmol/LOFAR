#                                                        LOFAR IMAGING PIPELINE
#
#                                                        Copy from to nodes
#                                                            Wouter Klijn, 2012
#                                                               klijn@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import os
import sys
import subprocess
import errno

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.lofarexceptions import PipelineException


class copier(LOFARnodeTCP):
    """
    Node script for copying files between nodes. See master script for full public interface
    """
    def run(self, source_node, source_path, target_path):
        # Time execution of this job
        with log_time(self.logger):
            return self._copy_single_file_using_rsync(
                source_node, source_path, target_path)

    def _copy_single_file_using_rsync(self, source_node, source_path,
                                      target_path):
        # assure that target dir exists (rsync creates it but..
        # an error in the python code will throw a nicer error
        message = "No write acces to target path: {0}".format(
                                    os.path.dirname(target_path))
        # If not existing try to create dir catch no permission
        try:
            create_directory(os.path.dirname(target_path))
        except OSError, e:
            if e.errno == 13:  # No permision
                self.logger.error(message)
                raise IOError(message)
            else:
                raise e

        #check if the target_path is writable for the current proc
        if not os.access(os.path.dirname(target_path), os.W_OK):
            self.logger.error(message)
            raise IOError(message)


        # construct copy command: Copy to the dir
        command = ["rsync", "-r", "{0}:{1}".format(source_node, source_path),
                               "{0}".format(os.path.dirname(target_path))]

        self.logger.debug("executing: " + " ".join(command))
        #Spawn a subprocess and connect the pipes
        copy_process = subprocess.Popen(
                        command,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

        (stdoutdata, stderrdata) = copy_process.communicate()
        exit_status = copy_process.returncode
        #if copy failed log the missing file
        if  exit_status != 0:
            message = \
            "Failed to (rsync) copy file, command: \n {0}".format(" ".join(
                                            command))
            self.logger.warn(message)
            self.logger.error(stderrdata)
            return 1

        self.logger.debug(stdoutdata)

        # return the target path to signal success
        self.outputs["target"] = target_path
        return 0

if __name__ == "__main__":
    #  If invoked directly, parse command line arguments for logger information
    #                       and pass the rest to the run() method defined above
    # -------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(copier(jobid, jobhost, jobport).run_with_stored_arguments())
