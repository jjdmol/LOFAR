# LOFAR IMAGING PIPELINE
# Prepare phase node 
# Wouter Klijn 
# 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import sys
import errno
import subprocess
import tempfile
import shutil
import os

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import  LOFARnodeTCP
from subprocess import CalledProcessError
import pyrap.tables as pt                                                       #@UnresolvedImport
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.group_data import load_data_map
from argparse import ArgumentError
from lofarpipe.support.lofarexceptions import PipelineException

# Some constant settings for the recipe
time_slice_dir_name = "time_slices"
collected_ms_dir_name = "s"

class SubProcessGroup(object):
        """
        A wrapper class for the subprocess module: allows fire and forget
        insertion of commands with a an optional sync/ barrier/ return
        """
        def __init__(self, logger = None):
            self.process_group = []
            self.logger = logger


        def run(self, cmd_in, unsave = False):
            """
            Add the cmd as a subprocess to the current group: The process is
            started!
            cmd can be suplied as a single string (white space seperated)
            or as a list of strings
            """

            if type(cmd_in) == type(""): #todo ugly
                cmd = cmd_in.split()
            elif type(cmd_in) == type([]):
                cmd = cmd_in
            else:
                raise Exception("SubProcessGroup.run() expects a string or" +
                    "list[string] as arguments suplied: {0}".format(type(cmd)))

            # Run subprocess
            process = subprocess.Popen(
                        cmd,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)
            # save the process
            self.process_group.append((cmd, process))

            # TODO: SubProcessGroup could saturate a system with to much 
            # concurent calss: artifical limit to 20 subprocesses
            if not unsave and (len(self.process_group) > 20):
                self.logger.error("Subprocessgroup could hang with more"
                    "then 20 concurent calls, call with unsave = True to run"
                     "with more than 20 subprocesses")
                raise PipelineException("Subprocessgroup could hang with more"
                    "then 20 concurent calls. Aborting")

            if self.logger == None:
                print "Subprocess started: {0}".format(cmd)
            else:
                self.logger.info("Subprocess started: {0}".format(cmd))

        def wait_for_finish(self):
            """
            Wait for all the processes started in the current group to end.
            Return the return status of a processes in an dict (None of no 
            processes failed 
            This is a Pipeline component: Of an logger is supplied the 
            std out and error will be suplied to the logger
            """
            collected_exit_status = []
            for cmd, process in self.process_group:
                # communicate with the process
                # TODO: This would be the best place to create a
                # non mem caching interaction with the processes!
                # TODO: should a timeout be introduced here to prevent never ending
                # runs?
                (stdoutdata, stderrdata) = process.communicate()
                exit_status = process.returncode

                # get the exit status
                if  exit_status != 0:
                    collected_exit_status.append((cmd, exit_status))

                # log the std out and err
                if self.logger != None:
                    self.logger.info(cmd)
                    self.logger.debug(stdoutdata)
                    self.logger.warn(stderrdata)
                else:
                    print cmd
                    print stdoutdata
                    print stderrdata

            if len(collected_exit_status) == 0:
                collected_exit_status = None
            return collected_exit_status


class imager_prepare(LOFARnodeTCP):

    """
    Prepare phase node of the imaging pipeline: node (also see master recipe)
 
    1. Collect the Measurement Sets (MSs): copy to the  current node
    2. Start dppp: Combines the data from subgroups into single timeslice
    3. Flag rfi
    4. Add addImagingColumns to the casa images
    5  Filter bad stations: Find station with repeared bad measurement and
       remove these completely from the dataset
    6. Concatenate the time slice measurment sets, to a virtual ms 
    """
    def run(self, init_script, parset, working_dir, processed_ms_dir,
             ndppp_executable, output_measurement_set,
            time_slices_per_image, subbands_per_group, raw_ms_mapfile,
            asciistat_executable, statplot_executable, msselect_executable,
            rficonsole_executable):
        with log_time(self.logger):
            input_map = load_data_map(raw_ms_mapfile)

            #******************************************************************
            # Create the directories used in this recipe            
            create_directory(processed_ms_dir)

            # time slice dir: assure empty directory: Stale data is a problem
            time_slice_dir = os.path.join(working_dir, time_slice_dir_name)
            create_directory(time_slice_dir)
            for root, dirs, files in os.walk(time_slice_dir):
                for f in files:
                    os.unlink(os.path.join(root, f))
                for d in dirs:
                    shutil.rmtree(os.path.join(root, d))
            self.logger.debug("Created directory: {0}".format(time_slice_dir))
            self.logger.debug("and assured it is empty")

            #******************************************************************
            #Copy the input files (caching included for testing purpose)
            missing_files = self._cached_copy_input_files(
                            processed_ms_dir, input_map,
                            skip_copy = False)
            if len(missing_files) != 0:
                self.logger.warn("A number of measurement sets could not be"
                                 "copied: {0}".format(missing_files))

            #******************************************************************
            #run dppp: collect frequencies into larger group
            time_slices = \
                self._run_dppp(working_dir, time_slice_dir, time_slices_per_image,
                    input_map, subbands_per_group, processed_ms_dir,
                    parset, ndppp_executable, init_script)

            self.logger.debug("Produced time slices: {0}".format(time_slices))
            #***********************************************************
            # run rfi_concole: flag datapoints which are corrupted
            self._run_rficonsole(rficonsole_executable, time_slice_dir,
                                 time_slices)


            #******************************************************************
            # Add imaging columns to each timeslice
            # ndppp_executable fails if not present
            for ms in time_slices:
                pt.addImagingColumns(ms)                                        #@UndefinedVariable
                self.logger.debug("Added imaging columns to ms: {0}".format(ms))

            group_measurement_filtered = self._filter_bad_stations(
                time_slices, asciistat_executable,
                statplot_executable, msselect_executable)

            #******************************************************************
            # Perform the (virtual) concatenation of the timeslices
            self._concat_timeslices(group_measurement_filtered,
                                    output_measurement_set)

            #******************************************************************
            # return 
            self.outputs["time_slices"] = group_measurement_filtered
            self.outputs["completed"] = "true"

        return 0

    def _cached_copy_input_files(self, processed_ms_dir,
                                 input_map, skip_copy = False):
        """
        Perform a optionally skip_copy copy of the input ms:
        For testing purpose the output, the missing_files can be saved
        allowing the skip of this step 
        """
        # TODO: Remove the skip_copy copy for the real version
        missing_files = []
        temp_missing = os.path.join(processed_ms_dir, "temp_missing")

        if not skip_copy:
            #Collect all files and copy to current node
            missing_files = self._copy_input_files(processed_ms_dir,
                                                   input_map)

            fp = open(temp_missing, 'w')
            fp.write(repr(missing_files))
            self.logger.debug(
                "Wrote file with missing measurement sets: {0}".format(temp_missing))
            fp.close()
        else:
            fp = open(temp_missing)
            missing_files = eval(fp.read())
            fp.close()

        return missing_files


    def _copy_input_files(self, processed_ms_dir, input_map):
        """
        Collect all the measurement sets in a single directory:
        The measurement sets are located on different nodes on the cluster.
        This function collects all the file in the input map in the sets_dir
        Return value is a set of missing files
        """
        missing_files = []

        #loop all measurement sets
        for idx, (node, path) in enumerate(input_map):
            # construct copy command
            command = ["rsync", "-r", "{0}:{1}".format(node, path) ,
                               "{0}".format(processed_ms_dir)]

            self.logger.debug("executing: " + " ".join(command))
            #Spawn a subprocess and connect the pipes
            copy_process = subprocess.Popen(
                        command,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)

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


    def _run_dppp(self, working_dir, time_slice_dir_path, slices_per_image,
                  input_map, subbands_per_image, collected_ms_dir_name, parset,
                  ndppp, init_script):
        """
        Run NDPPP:  
        Create dir for grouped measurements, assure clean workspace
        Call with log for cplus and catch segfaults. Actual parameters are located in 
        temp_parset_filename
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
            patchDictionary = {'uselogger': 'True', # enables log4cplus
                               'msin': msin,
                               'msout':time_slice_path
                               }

            try:
                nddd_parset_path = time_slice_path + ".ndppp.par"
                temp_parset_filename = patch_parset(parset, patchDictionary)
                shutil.copy(temp_parset_filename, nddd_parset_path)
                self.logger.debug("Wrote a ndppp parset with runtime variables:"
                                  " {0}".format(nddd_parset_path))
                os.unlink(temp_parset_filename)

            except Exception, e:
                self.logger.error("failed loading and updating the " +
                                  "parset: {0}".format(parset))
                raise e

            #run ndppp
            cmd = [ndppp, nddd_parset_path]

            try:
                environment = read_initscript(self.logger, init_script)
                with CatchLog4CPlus(working_dir, self.logger.name +
                                    "." + os.path.basename("imager_prepare_ndppp"),
                                    os.path.basename(ndppp)) as logger:
                        catch_segfaults(cmd, working_dir, environment,
                                        logger, cleanup = None)

            except CalledProcessError, e:
                self.logger.error(str(e))
                return 1
            except Exception, e:
                self.logger.error(str(e))
                return 1

        return time_slice_path_collected


    def _concat_timeslices(self, group_measurements_collected,
                                    output_file_path):
        """
        Msconcat to combine the time slices in a single ms:
        It is a virtual ms, a ms with symbolic links to actual data is created!                 
        """
        pt.msconcat(group_measurements_collected, #@UndefinedVariable
                               output_file_path, concatTime = True)
        self.logger.debug("Concatenated the files: {0} into the single measure"
            "mentset: {1}".format(
                ", ".join(group_measurements_collected), output_file_path))

    def _run_rficonsole(self, rficonsole_executable, time_slice_dir,
                        group_measurements_collected):
        """
        _run_rficonsole runs the rficonsole application on the supplied timeslices
        in group_measurements_collected.
        """

        #loop all measurement sets
        temp_dir_path = os.path.join(time_slice_dir, "rfi_temp_dir")
        create_directory(temp_dir_path)
        try:
            processes = []
            for (idx, group_set) in enumerate(group_measurements_collected):
                # construct copy command
                self.logger.info(group_set)
                command = [rficonsole_executable, "-indirect-read",
                            group_set]
                self.logger.info("executing rficonsole command: {0}".format(
                            " ".join(command)))
                #Spawn a subprocess and connect the pipes
                copy_process = subprocess.Popen(
                            command,
                            cwd = temp_dir_path,
                            stdin = subprocess.PIPE,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.PIPE) #working dir == temp
                processes.append(copy_process)

            # wait for the processes to finish. We need to wait for all
            # so the order of the communicate calls does not matter 
            for proc in processes:
                (stdoutdata, stderrdata) = proc.communicate()
                #if copy failed log the missing file
                if  proc.returncode != 0:
                    self.logger.error(stdoutdata)
                    self.logger.error(stderrdata)
                    raise Exception("Error running rficonsole:")

                else:
                    self.logger.info(stdoutdata)
        finally:
            shutil.rmtree(temp_dir_path)


    def _filter_bad_stations(self, group_measurements_collected,
            asciistat_executable, statplot_executable, msselect_executable):
        """
        _filter_bad_stations performs three steps:
        1. First a number of statistics with regards to the spread of the data 
        is collected using the asciistat_executable
        2. Secondly these statistics are consumed by the statplot_executable
        which produces a set of bad stations.
        3. In the final step the bad stations are removed from the dataset using
        ms select
        ref: 
        http://www.lofar.org/wiki/lib/exe/fetch.php?media=msss:pandeymartinez-week9-v1p2.pdf
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
            fp = open(ms_stats + ".tab")

            for line in fp.readlines():
                #skip headed line
                if line[0] == "#":
                    continue

                entries = line.split()
                #if the current station is bad (the last entry on the line)
                if entries[-1] == "True":
                    #add the name of station
                    station_to_filter.append(entries[1])

            # if this measurement does not contain baselines to skip do not filter
            # and provide the original ms as output
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
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(
        imager_prepare(jobid, jobhost, jobport).run_with_stored_arguments())
