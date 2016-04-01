# LOFAR IMAGING PIPELINE
# Prepare phase node
# Wouter Klijn
# 2012
# klijn@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys
import shutil
import os
import subprocess
import copy
import pyrap.tables as pt   # order of pyrap import influences the type
                            # conversion binding
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import  LOFARnodeTCP
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.data_map import DataMap
from lofarpipe.support.subprocessgroup import SubProcessGroup
from lofarpipe.recipes.helpers.data_quality import run_rficonsole, filter_bad_stations

# Some constant settings for the recipe
_time_slice_dir_name = "time_slices"


class imager_prepare(LOFARnodeTCP):
    """
    Steps perform on the node:
    
    1. Create directories and assure that they are empty.
    2. Collect the Measurement Sets (MSs): copy to the current node.
    3. Start dppp: Combines the data from subgroups into single timeslice.
    4. Flag rfi (toggle by parameter)
    5. Add addImagingColumns to the casa ms.
    6. Filter bad stations. Find station with repeated bad measurement and
       remove these completely from the dataset.
    7. Add measurmentset tables
    8. Perform the (virtual) concatenation of the timeslices
    """
    def run(self, environment, parset, working_dir, processed_ms_dir,
            ndppp_executable, output_measurement_set,
            time_slices_per_image, subbands_per_group, input_ms_mapfile,
            asciistat_executable, statplot_executable, msselect_executable,
            rficonsole_executable, do_rficonsole, add_beam_tables, globalfs):
        """
        Entry point for the node recipe
        """
        self.environment.update(environment)
        self.globalfs = globalfs
        with log_time(self.logger):
            input_map = DataMap.load(input_ms_mapfile)

            #******************************************************************
            # 1. Create the directories used in this recipe
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
            # 2. Copy the input files
            # processed_ms_map will be the map containing all the 'valid'
            # input ms
            processed_ms_map = self._copy_input_files(
                            processed_ms_dir, input_map)

            #******************************************************************
            # 3. run dppp: collect frequencies into larger group
            time_slices_path_list = \
                self._run_dppp(working_dir, time_slice_dir,
                    time_slices_per_image, processed_ms_map, subbands_per_group,
                    processed_ms_dir, parset, ndppp_executable)

            # If no timeslices were created, bail out with exit status 1
            if len(time_slices_path_list) == 0:
                self.logger.error("No timeslices were created.")
                self.logger.error("Exiting with error state 1")
                return 1

            self.logger.debug(
                    "Produced time slices: {0}".format(time_slices_path_list))

            #***********************************************************
            # 4. run rfi_concole: flag datapoints which are corrupted
            if (do_rficonsole):
                run_rficonsole(rficonsole_executable, time_slice_dir,
                                 time_slices_path_list, self.logger,
                                self.resourceMonitor )

            #******************************************************************
            # 5. Add imaging columns to each timeslice
            # ndppp_executable fails if not present
            for time_slice_path in time_slices_path_list:
                pt.addImagingColumns(time_slice_path)
                self.logger.debug(
                "Added imaging columns to time_slice: {0}".format(
                                                            time_slice_path))

            #*****************************************************************
            # 6. Filter bad stations
            time_slice_filtered_path_list = filter_bad_stations(
                time_slices_path_list, asciistat_executable,
                statplot_executable, msselect_executable,
                self.logger, self.resourceMonitor)

            #*****************************************************************
            # 7. Add measurementtables
            if add_beam_tables:
                self._add_beam_tables(time_slice_filtered_path_list)

            #******************************************************************
            # 8. Perform the (virtual) concatenation of the timeslices
            self._concat_timeslices(time_slice_filtered_path_list,
                                    output_measurement_set)

            # *****************************************************************
            # Write the actually used ms for the created dataset to the input 
            # mapfile
            processed_ms_map.save(input_ms_mapfile)

            # return
            self.outputs["time_slices"] = \
                time_slices_path_list

        return 0

    def _add_beam_tables(self, time_slices_path_list):
        beamtable_proc_group = SubProcessGroup(self.logger)
        for ms_path in time_slices_path_list:
            self.logger.debug("makebeamtables start")
            cmd_string = "makebeamtables ms={0} overwrite=true".format(ms_path)
            self.logger.debug(cmd_string)
            beamtable_proc_group.run(cmd_string)

        if beamtable_proc_group.wait_for_finish() != None:
            # TODO: Exception on error: make time_slices_path_list a mapfile
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
        for input_item, processed_item in zip(input_map, processed_ms_map):
            # fill the copied item with the correct data
            processed_item.host = self.host
            processed_item.file = os.path.join(
                    processed_ms_dir, os.path.basename(input_item.file))

            stderrdata = None
            # If we have to skip this ms
            if input_item.skip == True:
                exit_status = 1  
                stderrdata = "SKIPPED_FILE"

            else:
                # use cp the copy if machine is the same ( localhost) 
                # make sure data is in the correct directory. for now:
                # working_dir/[jobname]/subbands
                # construct copy command
                command = ["rsync", "-r", "{0}:{1}".format(
                                input_item.host, input_item.file),
                                   "{0}".format(processed_ms_dir)]
                if self.globalfs or input_item.host == "localhost":
                    command = ["cp", "-r", "{0}".format(input_item.file),
                                           "{0}".format(processed_ms_dir)]

                self.logger.debug("executing: " + " ".join(command))

                # Spawn a subprocess and connect the pipes
                # The copy step is performed 720 at once in that case which 
                # might saturate the cluster.
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
                processed_item.skip = True
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
        # TODO: cpu limited is static at this location
        environment['OMP_NUM_THREADS'] = str(8)
        self.logger.debug("Using %s threads for ndppp" % 8)
        with CatchLog4CPlus(working_dir, self.logger.name +
             "." + os.path.basename("imager_prepare_ndppp"),
                  os.path.basename(ndppp)) as logger:
            catch_segfaults(cmd, working_dir, environment,
                   logger, cleanup = None, usageStats=self.resourceMonitor)

    def _get_nchan_from_ms(self, file):
        """
        Wrapper for pt call to retrieve the number of channels in a ms

        Uses Pyrap functionality throws 'random' exceptions.
        """

        # open the datasetassume same nchan for all sb
        table = pt.table(file)  # 
           
        # get the data column, get description, get the 
        # shape, first index returns the number of channels
        nchan = str(pt.tablecolumn(table, 'DATA').getdesc()["shape"][0])

        return nchan

    def _run_dppp(self, working_dir, time_slice_dir_path, slices_per_image,
                  processed_ms_map, subbands_per_image, collected_ms_dir_name, 
                  parset, ndppp):
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
            output_ms_name = "time_slice_{0}.dppp.ms".format(idx_time_slice)

            # construct time slice name
            time_slice_path = os.path.join(time_slice_dir_path,
                                         output_ms_name)

            # convert the datamap to a file list: Add nonfalid entry for
            # skipped files: ndppp needs the incorrect files there to allow 
            # filling with zeros           
            ndppp_input_ms = []
            nchan_known = False

            for item in processed_ms_map[start_slice_range:end_slice_range]:
                if item.skip:
                    ndppp_input_ms.append("SKIPPEDSUBBAND")
                else:
                    # From the first non skipped filed get the nchan
                    if not nchan_known:
                        try:
                            # We want toAutomatically average the number 
                            # of channels in the output to 1, get the current
                            # nr of channels
                            nchan_input = self._get_nchan_from_ms(item.file)
                            nchan_known = True

                        # corrupt input measurement set
                        except Exception, e:
                            self.logger.warn(str(e))
                            item.skip = True
                            ndppp_input_ms.append("SKIPPEDSUBBAND")
                            continue

                    ndppp_input_ms.append(item.file)
            
            # if none of the input files was valid, skip the creation of the 
            # timeslice all together, it will not show up in the timeslice 
            # mapfile
            if not nchan_known:
                continue
           
            # TODO/FIXME: dependency on the step name!!!!
            ndppp_nchan_key = "avg1.freqstep"  
            
            # Join into a single string list of paths.
            msin = "['{0}']".format("', '".join(ndppp_input_ms))
            
            # Update the parset with computed parameters
            patch_dictionary = {'uselogger': 'True',  # enables log4cplus
                               'msin': msin,
                               'msout': time_slice_path,
                               ndppp_nchan_key:nchan_input}


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
            # and the input ms should have the skip  set
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



if __name__ == "__main__":
    _jobid, _jobhost, _jobport = sys.argv[1:4]
    sys.exit(
        imager_prepare(_jobid, _jobhost, _jobport).run_with_stored_arguments())
