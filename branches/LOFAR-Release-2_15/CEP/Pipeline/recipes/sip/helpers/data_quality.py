#                                                        LOFAR IMAGING PIPELINE
#
#                                              Helper function used for ms
#                                             quality validation and filtering
#                                                            Wouter Klijn: 2015
#                                                               klijn@astron.nl
# -----------------------------------------------------------------------------
import sys
import shutil
import os

from lofarpipe.support.subprocessgroup import SubProcessGroup
from lofarpipe.support.utilities import create_directory

def run_rficonsole(rficonsole_executable, temp_dir,
                    input_ms_list, logger, resourceMonitor):
    """
    _run_rficonsole runs the rficonsole application on the supplied
    timeslices in time_slices.
    This functionality has also been implemented in BBS. 
    """

    # loop all measurement sets
    rfi_temp_dir = os.path.join(temp_dir, "rfi_temp_dir")
    create_directory(rfi_temp_dir)

    try:
        rfi_console_proc_group = SubProcessGroup(logger=logger,
                                       usageStats=resourceMonitor)
        for time_slice in input_ms_list:
            # Each rfi console needs own working space for temp files
            temp_slice_path = os.path.join(rfi_temp_dir,
                os.path.basename(time_slice))
            create_directory(temp_slice_path)

            # construct copy command
            logger.info(time_slice)
            command = [rficonsole_executable, "-indirect-read",
                        time_slice]
            logger.info("executing rficonsole command: {0}".format(
                                                            " ".join(command)))

            # Add the command to the process group
            rfi_console_proc_group.run(command, cwd = temp_slice_path)
                

        # wait for all to finish
        if rfi_console_proc_group.wait_for_finish() != None:
            raise Exception("an rfi_console_proc_group run failed!")

    finally:
        shutil.rmtree(rfi_temp_dir)


def filter_bad_stations(input_ms_list,
        asciistat_executable, statplot_executable, msselect_executable,
        logger, resourceMonitor):
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
    logger.info("Filtering bad stations")
    logger.debug("Collecting statistical properties of input data")
    asciistat_output = []
    asciistat_proc_group = SubProcessGroup(logger=logger,
                                           usageStats=resourceMonitor)
    for ms in input_ms_list:
        output_dir = ms + ".filter_temp"
        create_directory(output_dir)
        asciistat_output.append((ms, output_dir))

        cmd_string = "{0} -i {1} -r {2}".format(asciistat_executable,
                        ms, output_dir)
        asciistat_proc_group.run(cmd_string)

    if asciistat_proc_group.wait_for_finish() != None:
        raise Exception("an ASCIIStats run failed!")

    # Determine the station to remove
    logger.debug("Select bad stations depending on collected stats")
    asciiplot_output = []
    asciiplot_proc_group = SubProcessGroup(logger)
    for (ms, output_dir) in asciistat_output:
        ms_stats = os.path.join(output_dir, os.path.split(ms)[1] + ".stats")

        cmd_string = "{0} -i {1} -o {2}".format(statplot_executable,
                                                  ms_stats, ms_stats)
        asciiplot_output.append((ms, ms_stats))
        asciiplot_proc_group.run(cmd_string)

    if asciiplot_proc_group.wait_for_finish() != None:
        raise Exception("an ASCIIplot run failed!")

    # remove the bad stations
    logger.debug("Use ms select to remove bad stations")
    msselect_output = {}
    msselect_proc_group = SubProcessGroup(logger)
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
    for input_ms in input_ms_list:
        filtered_list_of_ms.append(msselect_output[input_ms])

    return filtered_list_of_ms

