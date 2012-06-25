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

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.lofarexceptions import PipelineException


class copier(LOFARnodeTCP):
    """
    Node script for copying files between nodes. See master script for full public interface
    """
    def run(self, working_directory, source_mapfile, target_mapfile, shared_target_dir):
        # Time execution of this job
        with log_time(self.logger):
            try:
                source_map = load_data_map(source_mapfile)
                target_map = load_data_map(target_mapfile)
            except Exception, e:
                self.logger.error("An error occured during loading of"
                 "mapfiles")
                raise e

            return self._copy_all_sources_to_target(working_directory,
                         source_map, target_map, shared_target_dir)

    def _copy_all_sources_to_target(self, working_directory, source_map,
                            target_map, shared_target_dir):
        """
        Read the mapfiles, construct the target dir if needed and call 
        the actual copy function.
        """
        # combine the two lists to get the copy pairs
        for source_pair, target_pair in zip(source_map, target_map):
            source_node, source_path = source_pair
            target_node, target_path = target_pair

            # use the shared_target_dir for the target path:
            if shared_target_dir != "":
                # create the new target path based on target_path plus filename
                # if not abs construct target path with working dir,
                # relpath and file name
                if not os.path.isabs(shared_target_dir):
                    target_path = os.path.join(working_directory,
                        shared_target_dir, os.path.basename(target_path))

                else:
                    target_path = os.path.join(
                    shared_target_dir, os.path.basename(target_path))

            self._copy_single_file_using_rsync(
                                source_node, source_path, target_path)

        return 0

    def _copy_single_file_using_rsync(self, source_node, source_path,
                                      target_path):
        # assure that target dir exists (rsync creates it but..
        # an error in the python code will throw a nicer error
        create_directory(os.path.dirname(target_path))

        #check if the targat_path is writable for the current proc
        if not os.access(os.path.dirname(target_path), os.W_OK):
            message = "No write acces to target path: {0}".format(target_path)
            self.logger.error(message)
            raise IOError(message)



        # construct copy command
        command = ["rsync", "-r", "{0}:{1}".format(source_node, source_path) ,
                               "{0}".format(target_path)]

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
            message = "Failed to (rsync) copy file: {0} on node: {1} \n to: {2}".format(
                                                    source_path, source_node, target_path)
            self.logger.warn(message)
            self.logger.error(stderrdata)
            raise PipelineException(message)

        self.logger.debug(stdoutdata)
        return 0



if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(copier(jobid, jobhost, jobport).run_with_stored_arguments())
