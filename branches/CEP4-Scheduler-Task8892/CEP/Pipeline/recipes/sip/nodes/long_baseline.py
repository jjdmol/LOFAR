# LOFAR IMAGING PIPELINE
# long_baseline node
# Wouter Klijn
# 2014
# klijn@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys
import shutil
import os
import subprocess
import copy
import pyrap.tables as pt   # order of pyrap import influences the type conversion binding
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import  LOFARnodeTCP
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.data_map import DataMap
from lofarpipe.support.subprocessgroup import SubProcessGroup


# Some constant settings for the recipe
_time_slice_dir_name = "time_slices"


class long_baseline(LOFARnodeTCP):
    """
    Steps perform on the node:
    
    0. Create directories and assure that they are empty.
    1. Collect the Measurement Sets (MSs): copy to the current node.
    2. Start dppp: Combines the data from subgroups into single timeslice.
    3. Flag rfi. IF supplied executable is empty skipped
    4. Add addImagingColumns to the casa ms.
    5. Filter bad stations. Find station with repeated bad measurement and
       remove these completely from the dataset. Skipped if one of the
       exectutables is missing.
    6. optionally add beamtables
    7. Convert polarization
    8. Concatenate the time slice measurment sets, to a single virtual ms.
    """
    def run(self, environment, parset, working_dir, processed_ms_dir,
             ndppp_executable, output_measurement_set,
            subbandgroups_per_ms, subbands_per_subbandgroup, ms_mapfile,
            asciistat_executable, statplot_executable, msselect_executable,
            rficonsole_executable, add_beam_tables, globalfs, final_output_path):
        """
        Entry point for the node recipe
        """
        self.environment.update(environment)
        self.globalfs = globalfs

        with log_time(self.logger):
            input_map = DataMap.load(ms_mapfile)
            #******************************************************************
            # I. Create the directories used in this recipe
            create_directory(processed_ms_dir)
            create_directory(working_dir)

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
            # 1. Copy the input files
            processed_ms_map = self._copy_input_files(
                            processed_ms_dir, input_map)

            #******************************************************************
            # 2. run dppp: collect frequencies into larger group
            time_slices_path_list = \
                self._run_dppp(working_dir, time_slice_dir,
                    subbandgroups_per_ms, processed_ms_map, subbands_per_subbandgroup,
                    processed_ms_dir, parset, ndppp_executable)

            # If no timeslices were created, bail out with exit status 1
            if len(time_slices_path_list) == 0:
                self.logger.error("No timeslices were created.")
                self.logger.error("Exiting with error state 1")
                return 1

            self.logger.debug(
                    "Produced time slices: {0}".format(time_slices_path_list))

            #***********************************************************
            # 3. run rfi_concole: flag datapoints which are corrupted
            if False:
                self._run_rficonsole(rficonsole_executable, time_slice_dir,
                                     time_slices_path_list)

            #******************************************************************
            # 4. Add imaging columns to each timeslice
            # ndppp_executable fails if not present
            for time_slice_path in time_slices_path_list:
                pt.addImagingColumns(time_slice_path)
                self.logger.debug(
                "Added imaging columns to time_slice: {0}".format(
                                                            time_slice_path))

            #*****************************************************************
            # 5. Filter bad stations
            #if not(asciistat_executable == "" or
            #     statplot_executable == "" or
            #     msselect_executable == "" or True):
            if False:
                time_slice_filtered_path_list = self._filter_bad_stations(
                    time_slices_path_list, asciistat_executable,
                    statplot_executable, msselect_executable)
            else:
                # use the unfiltered list
                time_slice_filtered_path_list = time_slices_path_list

            #*****************************************************************
            # 6. Add measurmenttables
            if add_beam_tables:
                self.add_beam_tables(time_slice_filtered_path_list)

            #******************************************************************
            # 7. Perform Convert polarization:
            self._convert_polarization(time_slice_filtered_path_list)

            #******************************************************************
            # 8. Perform the (virtual) concatenation of the timeslices
            self._concat_timeslices(time_slice_filtered_path_list,
                                    output_measurement_set)

            #*****************************************************************
            # 9. Use table.copy(deep=true) to copy the ms to the correct
            # output location: create a new measurement set.
            self._deep_copy_to_output_location(output_measurement_set,
                                               final_output_path)

            # Write the actually used ms for the created dataset to the input 
            # mapfile
            processed_ms_map.save(ms_mapfile)



            #******************************************************************
            # return
            self.outputs["time_slices"] = \
                time_slices_path_list

        return 0

    def add_beam_tables(self, time_slices_path_list):
        beamtable_proc_group = SubProcessGroup(self.logger)
        for ms_path in time_slices_path_list:
            self.logger.debug("makebeamtables start")
            cmd_string = "makebeamtables ms={0} overwrite=true".format(ms_path)
            self.logger.debug(cmd_string)
            beamtable_proc_group.run(cmd_string)

        if beamtable_proc_group.wait_for_finish() != None:
            raise Exception("an makebeamtables run failed!")

        self.logger.debug("makebeamtables finished")

    def _copy_input_files(self, processed_ms_dir, input_map):
        """
        Collect all the measurement sets in a single directory:
        The measurement sets are located on different nodes on the cluster.
        This function collects all the file in the input map in the
        processed_ms_dir Return value is a set of missing files
        """
        processed_ms_map = copy.deepcopy(input_map)

        # loop all measurement sets
        for input_item, copied_item in zip(input_map, processed_ms_map):
            # fill the copied item with the correct data
            copied_item.host = self.host
            copied_item.file = os.path.join(
                    processed_ms_dir, os.path.basename(input_item.file))

            stderrdata = None
            # If we have to skip this ms
            if input_item.skip == True:
                exit_status = 1  
                stderrdata = "SKIPPED_FILE"

            else:
              # skip the copy if machine is the same (execution on localhost) 
              # make sure data is in the correct directory. 
              # for now: working_dir/[jobname]/subbands
              # construct copy command
              command = ["rsync", "-r", "{0}:{1}".format(
                              input_item.host, input_item.file),
                                 "{0}".format(processed_ms_dir)]
              if self.globalfs or input_item.host == "localhost":
                  command = ["cp", "-r", "{0}".format(input_item.file),
                                         "{0}".format(processed_ms_dir)]

              self.logger.debug("executing: " + " ".join(command))

              # Spawn a subprocess and connect the pipes
              # The copy step is performed 720 at once in that case which might
              # saturate the cluster.
              copy_process = subprocess.Popen(
                          command,
                          stdin = subprocess.PIPE,
                          stdout = subprocess.PIPE,
                          stderr = subprocess.PIPE)

              # Wait for finish of copy inside the loop: enforce single tread
              # copy
              (stdoutdata, stderrdata) = copy_process.communicate()

              exit_status = copy_process.returncode

            # if copy failed log the missing file and update the skip fields
            if  exit_status != 0:
                input_item.skip = True
                copied_item.skip = True
                self.logger.warning(
                            "Failed loading file: {0}".format(input_item.file))
                self.logger.warning(stderrdata)

            self.logger.debug(stdoutdata)

        return processed_ms_map


    def _dppp_call(self, working_dir, ndppp, cmd, environment):
        """
        Muckable function running the dppp executable.
        Wraps dppp with catchLog4CPLus and catch_segfaults
        """
        with CatchLog4CPlus(working_dir, self.logger.name +
             "." + os.path.basename("long_baseline_ndppp"),
                  os.path.basename(ndppp)) as logger:
            catch_segfaults(cmd, working_dir, environment,
                                  logger, cleanup = None)

    def _run_dppp(self, working_dir, time_slice_dir_path, slices_per_image,
                  processed_ms_map, subbands_per_image, collected_ms_dir_name, parset,
                  ndppp):
        """
        Run NDPPP:
        Create dir for grouped measurements, assure clean workspace
        Call with log for cplus and catch segfaults. Pparameters are
        supplied in parset
        """
        time_slice_path_list = []
        for idx_time_slice in range(slices_per_image):
            start_slice_range = idx_time_slice * subbands_per_image
            end_slice_range = (idx_time_slice + 1) * subbands_per_image
            # Get the subset of ms that are part of the current timeslice,
            # cast to datamap
            output_ms_name = "time_slice_{0}.dppp.ms".format(idx_time_slice)

            # construct time slice name
            time_slice_path = os.path.join(time_slice_dir_path,
                                         output_ms_name)

            # convert the datamap to a file list: Do not remove skipped files:
            # ndppp needs the incorrect files there to allow filling with zeros

            ndppp_input_ms = []
            for item in processed_ms_map[start_slice_range:end_slice_range]:
                if item.skip:
                    ndppp_input_ms.append("SKIPPEDSUBBAND")
                    # We need an entry in the list: ndppp will add zeros to
                    # pad missing subbands
                else:
                    ndppp_input_ms.append(item.file)

            # if none of the input files was valid, skip the creation of the 
            # timeslice all together, it will not show up in the timeslice 
            # mapfile
            if len(ndppp_input_ms) == 0:
                continue

            # Join into a single list of paths.
            msin = "['{0}']".format("', '".join(ndppp_input_ms))
            # Update the parset with computed parameters
            patch_dictionary = {'uselogger': 'True',  # enables log4cplus
                               'msin': msin,
                               'msout': time_slice_path}
            nddd_parset_path = time_slice_path + ".ndppp.par"
            try:
                temp_parset_filename = patch_parset(parset, patch_dictionary)
                shutil.copyfile(temp_parset_filename, nddd_parset_path)
            # Remove the temp file
            finally:
                os.remove(temp_parset_filename)

            try:
                nddd_parset_path = time_slice_path + ".ndppp.par"
                temp_parset_filename = patch_parset(parset, patch_dictionary)
                shutil.copy(temp_parset_filename, nddd_parset_path)
                self.logger.debug(
                            "Wrote a ndppp parset with runtime variables:"
                                  " {0}".format(nddd_parset_path))

            except Exception, exception:
                self.logger.error("failed loading and updating the " +
                                  "parset: {0}".format(parset))
                raise exception
            # remove the temp file
            finally:
                os.unlink(temp_parset_filename)

            # run ndppp
            cmd = [ndppp, nddd_parset_path]

            try:
                # Actual dppp call to externals (allows mucking)
                self._dppp_call(working_dir, ndppp, cmd, self.environment)
                # append the created timeslice on succesfull run
                time_slice_path_list.append(time_slice_path)

            # On error the current timeslice should be skipped
            except Exception, exception:
                for item in processed_ms_map[start_slice_range:end_slice_range]:
                    item.skip = True
                self.logger.warning(str(exception))
                continue

        return time_slice_path_list

    def _concat_timeslices(self, group_measurements_collected,
                                    output_file_path):
        """
        Msconcat to combine the time slices in a single ms:
        It is a virtual ms, a ms with symbolic links to actual data is created!
        """
        pt.msconcat(group_measurements_collected,
                               output_file_path, concatTime = True)
        self.logger.debug("Concatenated the files: {0} into the single measure"
            "mentset: {1}".format(
                ", ".join(group_measurements_collected), output_file_path))

    def _run_rficonsole(self, rficonsole_executable, time_slice_dir,
                        time_slices):
        """
        _run_rficonsole runs the rficonsole application on the supplied
        timeslices in time_slices.

        """

        # loop all measurement sets
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
                rfi_console_proc_group.run(command, cwd = temp_slice_path)

            # wait for all to finish
            if rfi_console_proc_group.wait_for_finish() != None:
                raise Exception("an rfi_console_proc_group run failed!")

        finally:
            shutil.rmtree(rfi_temp_dir)

    def _filter_bad_stations(self, time_slice_path_list,
            asciistat_executable, statplot_executable, msselect_executable):
        """
        A Collection of scripts for finding and filtering of bad stations:

        1. First a number of statistics with regards to the spread of the data
           is collected using the asciistat_executable.
        2. Secondly these statistics are consumed by the statplot_executable
           which produces a set of bad stations.
        3. In the final step the bad stations are removed from the dataset 
           using ms select

        REF: http://www.lofar.org/wiki/lib/exe/fetch.php?media=msss:pandeymartinez-week9-v1p2.pdf
        """
        # run asciistat to collect statistics about the ms
        self.logger.info("Filtering bad stations")
        self.logger.debug("Collecting statistical properties of input data")
        asciistat_output = []
        asciistat_proc_group = SubProcessGroup(self.logger)
        for ms in time_slice_path_list:
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
            ms_stats = os.path.join(
                            output_dir, os.path.split(ms)[1] + ".stats")

            cmd_string = "{0} -i {1} -o {2}".format(statplot_executable,
                                                     ms_stats, ms_stats)
            asciiplot_output.append((ms, ms_stats))
            asciiplot_proc_group.run(cmd_string)

        if asciiplot_proc_group.wait_for_finish() != None:
            raise Exception("an ASCIIplot run failed!")

        # remove the bad stations
        self.logger.debug("Use ms select to remove bad stations")
        msselect_output = {}
        msselect_proc_group = SubProcessGroup(self.logger)
        for ms, ms_stats  in asciiplot_output:
            # parse the .tab file containing the bad stations
            station_to_filter = []
            file_pointer = open(ms_stats + ".tab")

            for line in file_pointer.readlines():
                # skip headed line
                if line[0] == "#":
                    continue

                entries = line.split()
                # if the current station is bad (the last entry on the line)
                if entries[-1] == "True":
                    # add the name of station
                    station_to_filter.append(entries[1])

            # if this measurement does not contain baselines to skip do not
            # filter and provide the original ms as output
            if len(station_to_filter) == 0:
                msselect_output[ms] = ms
                continue

            ms_output_path = ms + ".filtered"
            msselect_output[ms] = ms_output_path

            # use msselect to remove the stations from the ms
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
        for input_ms in time_slice_path_list:
            filtered_list_of_ms.append(msselect_output[input_ms])

        return filtered_list_of_ms


    def _convert_polarization(self, time_slice_filtered_path_list):
        """
        # convert to circular polarization 
        # method based on input from Javier Moldon <moldon@astron.nl>
        """
        for time_slice in time_slice_filtered_path_list:
            #apply the polarization to each ms
            try:
                opened_ms=pt.taql(
                    "update {0}/ set DATA = mscal.stokes(DATA,'circ')".format(time_slice))
                opened_ms.close()

                opened_ms=pt.taql(
                    "update {0}/POLARIZATION set CORR_TYPE=[5,6,7,8]".format(time_slice))
                opened_ms.close()
                self.logger.info("Converted to circular polarization using taql")
            except Exception, exception:
                self.logger.error("Problem applying polarization to ms: {0}".format(
                    time_slice))
                raise exception

    def _deep_copy_to_output_location(self,output_measurement_set,
                                               final_output_path):
        ##################################

        table = pt.table(output_measurement_set)

        try:
          os.makedirs(os.path.dirname(final_output_path))
        except:
          pass # do nothing, the path already exists, we can output to this
               # location

        table.copy(final_output_path, deep=True)


if __name__ == "__main__":
    _jobid, _jobhost, _jobport = sys.argv[1:4]
    sys.exit(
        long_baseline(_jobid, _jobhost, _jobport).run_with_stored_arguments())
