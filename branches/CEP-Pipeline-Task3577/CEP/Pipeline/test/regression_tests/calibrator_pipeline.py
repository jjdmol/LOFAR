import math
import sys
import numpy


from lofarpipe.recipes.helpers.WritableParmDB import WritableParmDB, list_stations
from lofarpipe.recipes.helpers.ComplexArray import ComplexArray, RealImagArray, AmplPhaseArray


def compare_two_parmdb(infile_1, infile_2, max_delta):
        """
        """
        # Create copy of the input file
        # delete target location
        if not os.path.exists(infile_1):
            message = "The supplied parmdb path is not available on"
            "the filesystem: {0}".format(infile_1)
            self.logger.error(message)
            raise Exception(message)

        if not os.path.exists(infile_2):
            message = "The supplied parmdb path is not available on"
            "the filesystem: {0}".format(infile_2)
            self.logger.error(message)
            raise Exception(message)

        # copy both instrument tables (might not be needed, allows reuse of 
        # existing code
        shutil.copytree(infile_1, infile_1 + "_copy")
        shutil.copytree(infile_2, infile_2 + "_copy")

        # Create a local WritableParmDB
        parmdb_1 = WritableParmDB(infile_1)
        parmdb_2 = WritableParmDB(infile_1)

        #get all stations in the parmdb
        stations_1 = list_stations(parmdb_1)
        stations_2 = list_stations(parmdb_1)
        try:
            for station_1, station_2 in zip(stations_1, stations_2):
                # compare the station names
                if station_1 != station_2:
                    print  "the station found in the parmdb are not the same!\n"
                    print "{0} != {1}".format(station_1, station_2)
                    return False

                print "Processing station {0}".format(station_1)

                # till here implemented
                polarization_data_1, type_pair_1 = \
                   _read_polarisation_data_and_type_from_db(parmdb, station_1)

                polarization_data_2, type_pair_2 = \
                   _read_polarisation_data_and_type_from_db(parmdb, station_1)

                if type_pair_1 != type_pair_2:
                    print  "the types found in the parmdb for station {0}are not the same!\n".format(stations_1)
                    print "{0} != {1}".format(type_pair_1, type_pair_2)
                    return False

                # Convert the raw data to the correct complex array type
                complex_array_1 = _convert_data_to_ComplexArray(
                            polarization_data_1, type_pair_1)

                complex_array_2 = _convert_data_to_ComplexArray(
                            polarization_data_2, type_pair_1)

                # convert to magnitudes
                amplitudes_1 = complex_array_1.amp[:-1]
                amplitudes_2 = complex_array_2.amp[:-1]

                for val_1, val_2 in zip(amplitudes_1, amplitudes_1):
                    if numpy.abs(val_1 - val_2) > max_delta:
                        print "Warning found different gains in the instrument table!"
                        print "station: {0}".format(station_1)
                        print "{0} != {1}".format(val_1, val_2)
                        print amplitudes_1
                        print amplitudes_2
                        return False

        finally:
            # remove create temp files
            shutil.rmtree(infile_1 + "_copy")
            shutil.rmtree(infile_2 + "_copy")
        return True


def _read_polarisation_data_and_type_from_db(parmdb, station):
        all_matching_names = parmdb.getNames("Gain:*:*:*:{0}".format(station))
        """
        Read the polarisation data and type from the db.
        """
        # get the polarisation_data eg: 1:1
        # This is based on the 1 trough 3th entry in the parmdb name entry
        pols = set(":".join(x[1:3]) for x in  (x.split(":") for x in all_matching_names))

        # Get the im or re name, eg: real. Sort for we need a known order
        type_pair = sorted(set(x[3] for x in  (x.split(":") for x in all_matching_names)))

        #Check if the retrieved types are valid
        sorted_valid_type_pairs = [sorted(RealImagArray.keys),
                                    sorted(AmplPhaseArray.keys)]

        if not type_pair in sorted_valid_type_pairs:
            print "The parsed parmdb contained an invalid array_type:"
            print "{0}".format(type_pair)
            print "valid data pairs are: {0}".format(
                                                    sorted_valid_type_pairs)
            raise Exception(
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

def _convert_data_to_ComplexArray(data, type_pair):
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
            print "Incorrect data type pair provided: {0}".format(
                                            type_pair)
            raise Exception(
                "Invalid data type retrieved from parmdb: {0}".format(type_pair))
        return complex_array


if __name__ == "__main__":
    parmdb_1, parmdb_2, max_delta = None, None, None
    # Parse parameters from command line
    error = False
    try:
        parmdb_1, parmdb_2, max_delta = sys.argv[1:3]
    except:
        print "usage: python {0} parmdb_1_path "\
            " parmdb_2_path [max_delta (type=float)]".format(sys.argv[0])
        sys.exit(1)

    max_delta = None
    try:
        max_delta = float(sys.argv[3])
    except:
        max_delta = 0.0001

    print "using max delta: {0}".format(max_delta)

    if not error:
        data_equality = compare_two_parmdb(infile_1, infile_2, max_delta)

        if not data_equality:
            print "Regression test failed: exiting with exitstatus 1"
            print " parmdb data equality = : {0}".format(image_equality)
            sys.exit(1)

        print "Regression test Succeed!!"
        sys.exit(0)




