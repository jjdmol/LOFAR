# LOFAR IMAGING PIPELINE
# Prepare phase node 
# Wouter Klijn 
# 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import sys
import shutil
import os
import subprocess

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import  LOFARnodeTCP
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.subprocessgroup import SubProcessGroup

import pyrap.tables as pt                                     #@UnresolvedImport

# Some constant settings for the recipe
_time_slice_dir_name = "time_slices"


class imager_prepare(LOFARnodeTCP):
    """
    Steps perform on the node:
    
    0. Create directories and assure that they are empty.
    1. Collect the Measurement Sets (MSs): copy to the  current node.
    2. Start dppp: Combines the data from subgroups into single timeslice.
    3. Flag rfi.
    4. Add addImagingColumns to the casa ms.
    5. Concatenate the time slice measurment sets, to a single virtual ms.
    6. Filter bad stations. Find station with repeated bad measurement and
       remove these completely from the dataset.
 
    **Members:** 
    """
    def run(self, environment, parset, working_dir, processed_ms_dir,
             ndppp_executable, output_measurement_set,
            time_slices_per_image, subbands_per_group, raw_ms_mapfile,
            asciistat_executable, statplot_executable, msselect_executable,
            rficonsole_executable):
        """
        Entry point for the node recipe
        """
        self.environment.update(environment)
        with log_time(self.logger):
            input_map = load_data_map(raw_ms_mapfile)

            #******************************************************************
            # I. Create the directories used in this recipe            
            create_directory(processed_ms_dir)

            # time slice dir_to_remove: assure empty directory: Stale data 
            # is problematic for dppp
            time_slice_dir = os.path.join(working_dir, _time_slice_dir_name)
            create_directory(time_slice_dir)
            for root, dirs, files in os.walk(time_slice_dir):
                for file_to_remove in files:
                    os.unlink(os.path.join(root, file_to_remove))
                for dir_to_remove in dirs:
                    shutil.rmtree(os.path.join(root, dir_to_remove))
            self.logger.debug("Created directory: {0}".format(time_slice_dir))
            self.logger.debug("and assured it is empty")

            #******************************************************************
            # 1. Copy the input files (caching included for testing purpose)
            missing_files = self._cached_copy_input_files(
                            processed_ms_dir, input_map,
                            skip_copy=False)
            if len(missing_files) != 0:
                self.logger.warn("A number of measurement sets could not be"
                                 "copied: {0}".format(missing_files))

            #******************************************************************
            # 2. run dppp: collect frequencies into larger group
            time_slices = \
                self._run_dppp(working_dir, time_slice_dir,
                    time_slices_per_image, input_map, subbands_per_group,
                    processed_ms_dir, parset, ndppp_executable)

            self.logger.debug("Produced time slices: {0}".format(time_slices))
            #***********************************************************
            # 3. run rfi_concole: flag datapoints which are corrupted
            self._run_rficonsole(rficonsole_executable, time_slice_dir,
                                 time_slices)

            #******************************************************************
            # 4. Add imaging columns to each timeslice
            # ndppp_executable fails if not present
            for ms in time_slices:
                pt.addImagingColumns(ms)
                self.logger.debug("Added imaging columns to ms: {0}".format(ms))

            #*****************************************************************
            # 5. Filter bad stations
            group_measurement_filtered = self._filter_bad_stations(
                time_slices, asciistat_executable,
                statplot_executable, msselect_executable)

            #******************************************************************
            # 6. Perform the (virtual) concatenation of the timeslices
            self._concat_timeslices(group_measurement_filtered,
                                    output_measurement_set)

            #******************************************************************
            # return 
            self.outputs["time_slices"] = group_measurement_filtered
            self.outputs["completed"] = "true"

        return 0

    def _cached_copy_input_files(self, processed_ms_dir,
                                 input_map, skip_copy=False):
        """
        Perform a optionalskip_copy copy of the input ms:
        For testing purpose the output, the missing_files can be saved
        allowing the skip of this step 
        """
        missing_files = []
        temp_missing = os.path.join(processed_ms_dir, "temp_missing")

        if not skip_copy:
            #Collect all files and copy to current node
            missing_files = self._copy_input_files(processed_ms_dir,
                                                   input_map)

            file_pointer = open(temp_missing, 'w')
            file_pointer.write(repr(missing_files))
            self.logger.debug(
                "Wrote file with missing measurement sets: {0}".format(
                                                            temp_missing))
            file_pointer.close()
        else:
            file_pointer = open(temp_missing)
            missing_files = eval(file_pointer.read())
            file_pointer.close()

        return missing_files

    def _copy_input_files(self, processed_ms_dir, input_map):
        """
        Collect all the measurement sets in a single directory:
        The measurement sets are located on different nodes on the cluster.
        This function collects all the file in the input map in the 
        processed_ms_dir Return value is a set of missing files
        """
        missing_files = []

        #loop all measurement sets
        for node, path in input_map:
            # construct copy command
            command = ["rsync", "-r", "{0}:{1}".format(node, path) ,
                               "{0}".format(processed_ms_dir)]

            self.logger.debug("executing: " + " ".join(command))

            # Spawn a subprocess and connect the pipes
            # DO NOT USE SUBPROCESSGROUP 
            # The copy step is performed 720 at once in that case which might 
            # saturate the cluster. 
            copy_process = subprocess.Popen(
                        command,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

            # Wait for finish of copy inside the loop: enforce single tread copy
            (stdoutdata, stderrdata) = copy_process.communicate()

            exit_status = copy_process.returncode

            #if copy failed log the missing file
            if  exit_status != 0:
                missing_files.append(path)
                self.logger.warning("Failed loading file: {0}".format(path))
                self.logger.warning(stderrdata)
            self.logger.debug(stdoutdata)

        # return the missing files (for 'logging')
        return set(missing_files)

    def _dppp_call(self, working_dir, ndppp, cmd, environment):
        """
        Muckable function running the dppp executable.
        Wraps dppp with catchLog4CPLus and catch_segfaults
        """
        with CatchLog4CPlus(working_dir, self.logger.name +
             "." + os.path.basename("imager_prepare_ndppp"),
                  os.path.basename(ndppp)) as logger:
            catch_segfaults(cmd, working_dir, environment,
                                  logger, cleanup=None)

    def _run_dppp(self, working_dir, time_slice_dir_path, slices_per_image,
                  input_map, subbands_per_image, collected_ms_dir_name, parset,
                  ndppp):
        """
        Run NDPPP:  
        Create dir for grouped measurements, assure clean workspace
        Call with log for cplus and catch segfaults. Pparameters are 
        supplied in parset
        """
        time_slice_path_collected = []
        for idx_time_slice in range(slices_per_image):
            # Get the subset of ms that are part of the current timeslice
            input_map_subgroup = \
                input_map[(idx_time_slice * subbands_per_image): \
                             ((idx_time_slice + 1) * subbands_per_image)]

            # get the filenames
            input_subgroups = map(lambda x: x.split("/")[-1] ,
                                          list(zip(*input_map_subgroup)[1]))

            # join with the group_measurement_directory to get the locations
            # on the local node
            ndppp_input_ms = map(lambda x: os.path.join(
                         collected_ms_dir_name, x), input_subgroups)

            output_ms_name = "time_slice_{0}.dppp.ms".format(idx_time_slice)

            # construct time slice name
            time_slice_path = os.path.join(time_slice_dir_path,
                                         output_ms_name)
            time_slice_path_collected.append(time_slice_path)

            msin = "['{0}']".format("', '".join(ndppp_input_ms))
            # Update the parset with computed parameters
            patch_dictionary = {'uselogger': 'True', # enables log4cplus
                               'msin': msin,
                               'msout':time_slice_path}
            nddd_parset_path = time_slice_path + ".ndppp.par"
            temp_parset_filename = patch_parset(parset, patch_dictionary)
            shutil.copy(temp_parset_filename, nddd_parset_path)

            try:
                nddd_parset_path = time_slice_path + ".ndppp.par"
                temp_parset_filename = patch_parset(parset, patch_dictionary)
                shutil.copy(temp_parset_filename, nddd_parset_path)
                self.logger.debug("Wrote a ndppp parset with runtime variables:"
                                  " {0}".format(nddd_parset_path))
                os.unlink(temp_parset_filename)

            except Exception, exception:
                self.logger.error("failed loading and updating the " +
                                  "parset: {0}".format(parset))
                raise exception

            #run ndppp
            cmd = [ndppp, nddd_parset_path]

            try:
                # Actual dppp call to externals (allows mucking)
                self._dppp_call(working_dir, ndppp, cmd, self.environment)

            except subprocess.CalledProcessError, exception:
                self.logger.error(str(exception))
                return 1
            except Exception, exception:
                self.logger.error(str(exception))
                return 1

        return time_slice_path_collected

    def _concat_timeslices(self, group_measurements_collected,
                                    output_file_path):
        """
        Msconcat to combine the time slices in a single ms:
        It is a virtual ms, a ms with symbolic links to actual data is created!                 
        """
        pt.msconcat(group_measurements_collected, #@UndefinedVariable
                               output_file_path, concatTime=True)
        self.logger.debug("Concatenated the files: {0} into the single measure"
            "mentset: {1}".format(
                ", ".join(group_measurements_collected), output_file_path))

    def _run_rficonsole(self, rficonsole_executable, time_slice_dir,
                        time_slices):
        """
        _run_rficonsole runs the rficonsole application on the supplied timeslices
        in time_slices.
        
        """

        #loop all measurement sets
        rfi_temp_dir = os.path.join(time_slice_dir, "rfi_temp_dir")
        create_directory(rfi_temp_dir)

        try:
            rfi_console_proc_group = SubProcessGroup(self.logger)
            for time_slice in time_slices:
                # Each rfi console needs own working space for temp files    
                temp_slice_path = os.path.join(rfi_temp_dir,
                    os.path.basename(time_slice))
                create_directory(temp_slice_path)

                # construct copy command
                self.logger.info(time_slice)
                command = [rficonsole_executable, "-indirect-read",
                            time_slice]
                self.logger.info("executing rficonsole command: {0}".format(
                            " ".join(command)))

                # Add the command to the process group
                rfi_console_proc_group.run(command, cwd=temp_slice_path)

            # wait for all to finish
            if rfi_console_proc_group.wait_for_finish() != None:
                raise Exception("an rfi_console_proc_group run failed!")

        finally:
            shutil.rmtree(rfi_temp_dir)

    def _filter_bad_stations(self, group_measurements_collected,
            asciistat_executable, statplot_executable, msselect_executable):
        """
        A Collection of scripts for finding and filtering of bad stations:

        1. First a number of statistics with regards to the spread of the data 
           is collected using the asciistat_executable.
        2. Secondly these statistics are consumed by the statplot_executable
           which produces a set of bad stations.
        3. In the final step the bad stations are removed from the dataset using
           ms select
           
        REF: http://www.lofar.org/wiki/lib/exe/fetch.php?media=msss:pandeymartinez-week9-v1p2.pdf
        """
        # run asciistat to collect statistics about the ms
        self.logger.info("Filtering bad stations")
        self.logger.debug("Collecting statistical properties of input data")
        asciistat_output = []
        asciistat_proc_group = SubProcessGroup(self.logger)
        for ms in group_measurements_collected:
            output_dir = ms + ".filter_temp"
            create_directory(output_dir)
            asciistat_output.append((ms, output_dir))

            cmd_string = "{0} -i {1} -r {2}".format(asciistat_executable,
                            ms, output_dir)
            asciistat_proc_group.run(cmd_string)

        if asciistat_proc_group.wait_for_finish() != None:
            raise Exception("an ASCIIStats run failed!")

        # Determine the station to remove
        self.logger.debug("Select bad stations depending on collected stats")
        asciiplot_output = []
        asciiplot_proc_group = SubProcessGroup(self.logger)
        for (ms, output_dir) in asciistat_output:
            ms_stats = os.path.join(output_dir, os.path.split(ms)[1] + ".stats")

            cmd_string = "{0} -i {1} -o {2}".format(statplot_executable,
                                                     ms_stats, ms_stats)
            asciiplot_output.append((ms, ms_stats))
            asciiplot_proc_group.run(cmd_string)

        if asciiplot_proc_group.wait_for_finish() != None:
            raise Exception("an ASCIIplot run failed!")

        #remove the bad stations
        self.logger.debug("Use ms select to remove bad stations")
        msselect_output = {}
        msselect_proc_group = SubProcessGroup(self.logger)
        for ms, ms_stats  in asciiplot_output:
            #parse the .tab file containing the bad stations
            station_to_filter = []
            file_pointer = open(ms_stats + ".tab")

            for line in file_pointer.readlines():
                #skip headed line
                if line[0] == "#":
                    continue

                entries = line.split()
                #if the current station is bad (the last entry on the line)
                if entries[-1] == "True":
                    #add the name of station
                    station_to_filter.append(entries[1])

            # if this measurement does not contain baselines to skip do not 
            # filter and provide the original ms as output
            if len(station_to_filter) == 0:
                msselect_output[ms] = ms
                continue

            ms_output_path = ms + ".filtered"
            msselect_output[ms] = ms_output_path

            #use msselect to remove the stations from the ms
            msselect_baseline = "!{0}".format(",".join(station_to_filter))
            cmd_string = "{0} in={1} out={2} baseline={3} deep={4}".format(
                            msselect_executable, ms, ms_output_path,
                            msselect_baseline, "False")
            msselect_proc_group.run(cmd_string)

        if msselect_proc_group.wait_for_finish() != None:
            raise Exception("an MSselect run failed!")

        filtered_list_of_ms = []
        # The order of the inputs needs to be preserved when producing the
        # filtered output!
        for input_ms in group_measurements_collected:
            filtered_list_of_ms.append(msselect_output[input_ms])

        self.logger.info(repr(filtered_list_of_ms))
        return filtered_list_of_ms


if __name__ == "__main__":
    _jobid, _jobhost, _jobport = sys.argv[1:4]
    sys.exit(
        imager_prepare(_jobid, _jobhost, _jobport).run_with_stored_arguments())
