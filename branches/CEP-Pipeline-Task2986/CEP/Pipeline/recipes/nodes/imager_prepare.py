# LOFAR IMAGING PIPELINE
# Prepare phase 
# of the imaging pipeline: node (also see master recipe)
# Responcible for
# 
# 1. Collect the Measurement Sets (MSs): copy to the  current node
# 2. Start dppp: Combines the data from subgroups into single set
# 3. Concatenate the time slice measurment sets, to a virtual ms (thus 
#    combining information in both time and frequency  
#
# Wouter Klijn 
# 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import sys
import errno
import os.path

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import  LOFARnodeTCP
from subprocess import CalledProcessError
import pyrap.tables as pt                                                       #@UnresolvedImport
from lofarpipe.support.utilities import create_directory

# Some constant settings for the recipe
log4CPlusName = "imager_prepare_node"
time_slice_dir = "time_slices"
collected_ms_dir = "subband_mss"

class imager_prepare(LOFARnodeTCP):
    def run(self, init_script, parset, working_dir, ndppp, output_measurement_set,
            slices_per_image, subbands_per_image, input_map_repr):
        with log_time(self.logger):
            create_directory(working_dir)
            # TODO: Load the input file names ( will be performed by marcel code )
            self.logger.info(eval(input_map_repr))

            input_map = eval(input_map_repr)
            target_dir_for_collected_ms = os.path.join(working_dir,
                                collected_ms_dir)

            #Copy the input files (caching included for testing purpose)
            missing_files = self._cached_copy_input_files(
                            target_dir_for_collected_ms, input_map, True)
            if len(missing_files): self.logger.info(repr(missing_files))

            #run dppp: collect indif frequencies into larger group
            group_measurements_collected = \
                self._run_dppp(working_dir, time_slice_dir, slices_per_image,
                    input_map, subbands_per_image, missing_files, collected_ms_dir,
                    parset, ndppp, init_script, log4CPlusName)

            # Add imaging columns to each timeslice
            # ndppp fails if not present
            for ms in group_measurements_collected:
                pt.addImagingColumns(ms)                                        #@UnresolvedImport

            # Perform the (virtual) concatenation of the timeslices
            self._concat_timeslices(group_measurements_collected,
                                    output_measurement_set)

            #return succes
            self.outputs["time_slices"] = group_measurements_collected
            self.outputs["completed"] = "true"

        return 0


    def _cached_copy_input_files(self, target_dir_for_collected_ms,
                                 input_map, cached = False):
        """
        Perform a optionally cached copy of the input ms:
        For testing purpose the output, the missing_files can be saved
        allowing the skip of this copy 
        """
        # TODO: Remove the cached copy for the real version
        missing_files = []
        temp_missing = os.path.join(target_dir_for_collected_ms, "temp_missing")
        create_directory(target_dir_for_collected_ms)
        if not cached:
            #Collect all files and copy to current node
            missing_files = self._copy_input_files(target_dir_for_collected_ms,
                                                   input_map)

            fp = open(temp_missing, 'w')
            fp.write(repr(missing_files))
            fp.close()
        else:
            fp = open(temp_missing)
            missing_files = eval(fp.read())
            fp.close()

        return missing_files


    def _copy_input_files(self, target_dir_for_collected_ms, input_map):
        """
        Collect all the measurement sets in a single directory:
        The measurement sets are located on different nodes on the cluster.
        This function collects all the file in the input map in the sets_dir
        Return value is a set of missing files
        """
        create_directory(target_dir_for_collected_ms)
        missing_files = []

        #loop all measurement sets
        for idx, (node, path) in enumerate(input_map):
            self.logger.info("copy file: {0}".format(path))
            # construct copy command
            # TODO: rsync return value disappears on the node:
            # For testing purpose the sterr is send to null to prevent
            # errors: subprocess would be a candidate for improvement
            copy_command = "rsync -r -q {0}:{1} {2} 2> /dev/null ".format(node, path,
                                            target_dir_for_collected_ms)
            exit_status = os.system(copy_command)

            #if copy failed log the missing file
            if  exit_status != 0:
                missing_files.append(path.split('/')[-1])
                self.logger.info("Failed loading file: {0}".format(path))

        # return the missing files (for 'logging'
        return set(missing_files)


    def _run_dppp(self, working_dir, group_dir, slices_per_image, input_map,
                  subbands_per_image, missing_files, sets_dir, parset, ndppp,
                  init_script, log4CPlusName):
        """
        Run NDPPP:  
        Create dir for grouped measurements, assure clean workspace
        Create input map for ndppp: Remove missing files
        CAll ndppp in save mode. Actual parameters are located in 
        temp_parset_filename
        """
        #create the directory to save subband group data sets
        group_measurement_directory = os.path.join(working_dir, group_dir)
        group_measurements_collected = []
        create_directory(group_measurement_directory)

        # assure empty dir
        os.system("rm -rf {0}/*".format(group_measurement_directory))

        for idx_time_slice in range(slices_per_image):
            #collect the subband for this timeslice in a single list
            input_map_subgroup = \
                input_map[(idx_time_slice * subbands_per_image): \
                             ((idx_time_slice + 1) * subbands_per_image)]

            #get the path on the remote machine. get the directory names
            input_subgroups = map(lambda x: x.split("/")[-1] ,
                                          list(zip(*input_map_subgroup)[1]))

            #join with the group_measurement_directory
            ndppp_input_ms = map(lambda x: os.path.join(working_dir,
                         sets_dir, x), input_subgroups)

            output_ms_name = "time_slice_{0}.dppp.ms".format(idx_time_slice)
            group_measurements_collected.append(
                os.path.join(working_dir, group_dir, output_ms_name))

            # create the directory to save group measurement set for this 
            # group
            group_ms_path = os.path.join(group_measurement_directory,
                                         output_ms_name)

            # Update the parset with calculated parameters
            patchDictionary = {'uselogger': 'True', # enables log4cplus
                               'msin': repr(ndppp_input_ms),
                               'msout':group_ms_path
                               }
            temp_parset_filename = None
            try:
                temp_parset_filename = patch_parset(parset, patchDictionary)
            except Exception, e:
                self.logger.error("failed loading and updating the " +
                                  "parset: {0}".format(parset))

            #run ndppp
            cmd = [ndppp, temp_parset_filename]

            try:
                environment = read_initscript(self.logger, init_script)
                with CatchLog4CPlus(working_dir, self.logger.name +
                                    "." + os.path.basename(log4CPlusName),
                                    os.path.basename(ndppp)) as logger:
                        catch_segfaults(cmd, working_dir, environment,
                                        logger, cleanup = None)

            except CalledProcessError, e:
                return 1
            except Exception, e:
                return 1
            finally:
                os.unlink(temp_parset_filename)

            # TODO: temp fix ger: allows aw imaging in later stages  
            #pt.addImagingColumns(os.path.join(working_dir, group_dir, output_ms_name))

        return group_measurements_collected


    def _concat_timeslices(self, group_measurements_collected,
                                    output_file_path):
        """
        Msconcat to combine the time slices in a single ms:
        It is a virtual ms, a ms with symbolic links to actual data is created!                 
        """
        pt.msconcat(group_measurements_collected,
                               output_file_path, concatTime = True)


if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(
        imager_prepare(jobid, jobhost, jobport).run_with_stored_arguments())
