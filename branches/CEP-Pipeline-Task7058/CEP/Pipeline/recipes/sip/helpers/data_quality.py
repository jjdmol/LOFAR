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

def run_rficonsole(rficonsole_executable, temp_dir,
                    input_ms_list, logger, resourceMonitor):
    """
    _run_rficonsole runs the rficonsole application on the supplied
    timeslices in time_slices.

    """

    # loop all measurement sets
    rfi_temp_dir = os.path.join(temp_dir, "rfi_temp_dir")
    create_directory(rfi_temp_dir)

    try:
        rfi_console_proc_group = SubProcessGroup(logger,
                                            resourceMonitor)
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
        logger):
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
    asciistat_proc_group = SubProcessGroup(logger)
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
