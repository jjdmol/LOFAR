#                                                         LOFAR IMAGING PIPELINE
#
#                                    Node recipe to export calibration solutions
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import os
import shutil
import sys
import tempfile
import numpy

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import read_initscript, create_directory
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarexceptions import PipelineRecipeFailed

from lofarpipe.recipes.helpers.WritableParmDB import WritableParmDB, list_stations
from lofarpipe.recipes.helpers.ComplexArray import ComplexArray, RealImagArray, AmplPhaseArray

class ParmExportCal(LOFARnodeTCP):
<<<<<<< .mine

    def run(self, infile, outfile, executable, initscript, sigma):
=======

    def run(self, infile, outfile, executable, initscript):
>>>>>>> .r21050
        # Time execution of this job
        with log_time(self.logger):
            if os.path.exists(infile):
                self.logger.info("Processing %s" % infile)
            else:
                self.logger.error(
                    "Instrument model file %s does not exist" % infile
                )
                return 1

        # Check if executable exists and is executable.
        if not os.access(executable, os.X_OK):
            self.logger.error("Executable %s not found" % executable)
            return 1

        # Create output directory (if it doesn't already exist)
        create_directory(os.path.dirname(outfile))

        # Initialize environment
        env = read_initscript(self.logger, initscript)

        try:
            temp_dir = tempfile.mkdtemp()
            with CatchLog4CPlus(
                temp_dir,
                self.logger.name + '.' + os.path.basename(infile),
                os.path.basename(executable)
            ) as logger:
                catch_segfaults(
                    [executable, '-in', infile, '-out', outfile],
                    temp_dir,
                    env,
                    logger
                )
        except Exception, excp:
            self.logger.error(str(excp))
            return 1
        finally:
            shutil.rmtree(temp_dir)

<<<<<<< .mine
        #From here new parmdb implementation!!
        self._filter_stations_parmdb(infile, outfile)
=======
        return 1 #return 1 to allow rerunning of this script
>>>>>>> .r21050

        return 1



    def _filter_stations_parmdb(self, infile, outfile):
        # Create copy of the input file
        # delete target location
        shutil.rmtree(outfile)
        self.logger.debug("cleared target path for filtered parmdb: \n {0}".format(
                                outfile))
        # copy
        shutil.copytree(infile, outfile)
        self.logger.debug("Copied raw parmdb to target locations: \n {0}".format(
                                infile))

        # Create a local WritableParmDB
        parmdb = WritetableParmdb(outfile)

        #get all stations in the parmdb
        stations = list_stations(parmdb)

        for station in stations:
            self.logger.debug("Processing station {0}".format(station))

            # till here implemented
            polarization_data, type_pair = \
               self._read_polarisation_data_and_type_from_db(parmdb, station)
            corected_data = self._swap_outliers_with_median(polarization_data,
                                                  type_pair, sigma)
            self._write_corrected_data(parmdb, station,
                                       polarization_data, corected_data)

    def _read_polarisation_data_and_type_from_db(self, parmdb, station):
        all_matching_names = parmdb.getNames("Gain:*:*:*:{0}".format(station))

        # get the polarisation_data eg: 1:1
        # This is based on the 1 trough 3th entry in the parmdb name entry
        pols = set(":".join(x[1:3]) for x in  (x.split(":") for x in names))

        # Get the im or re name, eg: real. Sort for we need a known order
        type_pair = sorted([x[3] for x in  (x.split(":") for x in names)])

        #Check if the retrieved types are valid
        sorted_valid_type_pairs = [sorted(RealImagArray.keys),
                                    sorted(AmplPhaseArray.keys)]
        if not type_pair in sorted_valid_type_pairs:
            self.logger.error("The parsed parmdb contained an invalid array_type:")
            self.logger.error("{0}".format(type_pair))
            self.logger.error("valid data pairs are: {0}".format(
                                                    sorted_valid_type_pairs))
            raise PipelineRecipeFailed(
                    "Invalid data type retrieved from parmdb: {0}".format(
                                                type_pair))

        polarisation_data = dict()
        #for all polarisation_data in the parmdb (2 times 2)
        for polarization in pols:
            data = []
            #for the two types
            for key in type_pair:
                query = "Gain:{0}:{1}:{2}".format(polarization, key, station)
                #append the retrieved data (resulting in dict to arrays
                data.append(parmdb.getValuesGrid(query)[query])
            polarisation_data[polarization] = data

        #return the raw data and the type of the data
        return polarisation_data, type_pair


    def _swap_outliers_with_median(self, polarization_data, type_pair, sigma):
        corrected_polarization_data = dict()
        for pol, data in polarization_data.iteritems():
            # Convert the raw data to the correct complex array type
            complex_array = self._convert_data_to_ComplexArray(data, type_pair)

            # get the data as amplitude from the amplitude array, skip last entry
            amplitudes = complex_array.amp[:-1]

            # calculate the statistics
            median = numpy.median(amplitudes)
            stddev = numpy.std(amplitudes)
            # Swap outliers with median version of the data
            corrected = numpy.where(
                   numpy.abs(amplitudes - median) > sigma * stddev,
                   median,
                   amplitudes
                   )
            # assign the corect data back to the complex_array
            complex_array.amp = numpy.concatenate((corrected, complex_array.amp[-1:]))
            # collect all corrected data
            corrected_polarization_data[pol] = complex_array

        return corrected_polarization_data

    def _convert_data_to_ComplexArray(self, data, type_pair):
        if sorted(type_pair) == sorted(RealImagArray.keys):
            # The type_pair is in alphabetical order: Imag on index 0
            complex_array = RealImagArray(data[1]["values"], data[0]["values"])
        elif sorted(type_pair) == sorted(AmplPhaseArray.keys):
            complex_array = AmplPhaseArray(data[0]["values"], data[1]["values"])
        else:
            self.logger.error("Incorrect data type pair provided: {0}".format(
                                            type_pair))
            raise PipelineRecipeFailed(
                "Invalid data type retrieved from parmdb: {0}".format(type_pair))
        return complex_array

    def _write_corrected_data(self, parmdb, station, polarization_data,
                               corected_data):
        for pol, data in polarization_data.iteritems():
            if not pol in corected_data:
                error_message = "Requested polarisation type is unknown:" \
                        "{0} \n valid polarisations: {1}".format(pol, corected_data.keys())
                self.logger.error(error_message)
                raise PipelineRecipeFailed(error_message)

            corrected_data = corected_data[pol]
            #get the "complex" converted data from the complex array
            for component, value in corrected_data.writeable.iteritems():
                #Collect all the data needed to write an array 
                name = "Gain:{0}:{1}:{2}".format(pol, component, station)
                freqscale = data[0]['freqs'][0]
                freqstep = data[0]['freqwidths'][0]
                timescale = data[0]['times'][0]
                timestep = data[0]['timewidths'][0]
                #call the write function on the parmdb
                parmdb.setValues(name, value, freqscale, freqstep, timescale,
                                 timestep)



if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(ParmExportCal(jobid, jobhost, jobport).run_with_stored_arguments())
