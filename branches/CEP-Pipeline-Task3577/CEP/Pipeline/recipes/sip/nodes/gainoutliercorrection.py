#                                                         LOFAR IMAGING PIPELINE
#
#                                    Node recipe to find outliers in the parmdb
#                                    and swap these with the median
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import os
import shutil
import sys
import tempfile
import numpy
import errno

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import create_directory, delete_directory
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarexceptions import PipelineRecipeFailed

from lofarpipe.recipes.helpers.WritableParmDB import WritableParmDB, list_stations
from lofarpipe.recipes.helpers.ComplexArray import ComplexArray, RealImagArray, AmplPhaseArray

class gainoutliercorrection(LOFARnodeTCP):
    """
    Perform a gain outlier correction on the provided parmdb.
    The functionality is based on the edit_parmdb script of John Swinbank.
    
    Outliers in the gain are swapped with the median. resulting gains 
    are written back to the supplied ms:
    
    1. Select correction correction method
    2. Call parmexportcal for gain correction
    3. use gainoutliercorrect from Swinbank
       Step are summarized in the functions of this recipe

    """
    def run(self, infile, outfile, executable, environment, sigma):
        self.environment.update(environment)
        # Time execution of this job
        with log_time(self.logger):
            if os.path.exists(infile):
                self.logger.info("Processing %s" % infile)
            else:
                self.logger.error(
                    "Instrument model file %s does not exist" % infile
                )
                return 1
        # Create output directory (if it doesn't already exist)
        create_directory(os.path.dirname(outfile))
        # ********************************************************************
        # 1. Select correction method
        if not os.access(executable, os.X_OK) and sigma != None:
            # If the executable is not accesable and we have a sigma:
            # use the 'local' functionality (edit parmdb)

        # *********************************************************************
        # 3. use gainoutliercorrect from Swinbank
            self._filter_stations_parmdb(infile, outfile, sigma)
            return 0

        # else we need an executable
        # Check if exists and is executable.
        if not os.access(executable, os.X_OK):
            self.logger.error("Executable %s not found" % executable)
            return 1

        # ********************************************************************
        # 2. Call parmexportcal for gain correction
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
                    self.environment,
                    logger
                )
        except Exception, excp:
            self.logger.error(str(excp))
            return 1
        finally:
            shutil.rmtree(temp_dir)

        return 0

    def _filter_stations_parmdb(self, infile, outfile, sigma):
        """
        Performs a gain outlier correction of the infile parmdb with
        the corrected parmdb written to outfile.
        Outliers in the gain with a distance of median of sigma times std
        are replaced with the mean. The last value of the complex array
        is skipped (John Swinbank: "I found it [the last value] was bad when 
        I hacked together some code to do this")
        """
        sigma = float(sigma)
        # Create copy of the input file
        # delete target location
        if not os.path.exists(infile):
            message = "The supplied parmdb path is not available on"
            "the filesystem: {0}".format(infile)
            self.logger.error(message)
            raise PipelineRecipeFailed(message)

        delete_directory(outfile)

        self.logger.debug("cleared target path for filtered parmdb: \n {0}".format(
                                outfile))

        # copy
        shutil.copytree(infile, outfile)
        self.logger.debug("Copied raw parmdb to target locations: \n {0}".format(
                                infile))

        # Create a local WritableParmDB
        parmdb = WritableParmDB(outfile)


        #get all stations in the parmdb
        stations = list_stations(parmdb)

        for station in stations:
            self.logger.debug("Processing station {0}".format(station))

            # till here implemented
            polarization_data, type_pair = \
               self._read_polarisation_data_and_type_from_db(parmdb, station)

            corected_data = self._swap_outliers_with_median(polarization_data,
                                                  type_pair, sigma)
            #print polarization_data
            self._write_corrected_data(parmdb, station,
                                       polarization_data, corected_data)
        return parmdb, corected_data

    def _read_polarisation_data_and_type_from_db(self, parmdb, station):
        """
        Read the polarisation data and type from the db.
        """
        all_matching_names = parmdb.getNames("Gain:*:*:*:{0}".format(station))

        # get the polarisation_data eg: 1:1
        # This is based on the 1 trough 3th entry in the parmdb name entry
        pols = set(":".join(x[1:3]) for x in  (x.split(":") for x in all_matching_names))

        # Get the im or re name, eg: real. Sort for we need a known order
        type_pair = sorted(set(x[3] for x in  (x.split(":") for x in all_matching_names)))

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
        """
        Perform the actual find and replace of the outliers
        Calculation are perform with complex arithmatic therefore the
        2d arrays are converted to complex value array of 1 d
        """
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
        """
        Performs a conversion of a 2d array to a 1d complex valued array.
        with real/imag values or with amplitude phase values
        """
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
        """
        Use pyparmdb to write (now corrected) data to the parmdb
        """
        for pol, data in polarization_data.iteritems():
            if not pol in corected_data:
                error_message = "Requested polarisation type is unknown:" \
                        "{0} \n valid polarisations: {1}".format(pol, corected_data.keys())
                self.logger.error(error_message)
                raise PipelineRecipeFailed(error_message)

            corrected_data_pol = corected_data[pol]
            #get the "complex" converted data from the complex array
            for component, value in corrected_data_pol.writeable.iteritems():
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
    sys.exit(gainoutliercorrection(jobid, jobhost, jobport).run_with_stored_arguments())
