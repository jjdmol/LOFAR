import math
import sys

def validate_image_equality(image_1_path, image_2_path, max_delta):
    import pyrap.images as pim

    # get the difference between the two images
    im = pim.image("{0} - {1}".format(image_1_path, image_2_path))
    im.saveas("difference.IM2")
    # get the stats of the image
    stats_dict = im.statistics()
    return_value = compare_image_statistics(stats_dict, max_delta)

    if not return_value:
        print "\n\n\n"
        print "*"*30
        print "Statistics of the produced image:"
        im = pim.image("{0}".format(image_1_path))
        stats_dict_single_image = im.statistics()
        print stats_dict_single_image
        print "\n\n\n"
        print "Statistics of the compare image:"
        im = pim.image("{0}".format(image_2_path))
        stats_dict_single_image = im.statistics()
        print stats_dict_single_image
        print "\n\n\n"
        print "difference between produced image and the baseline image:"
        print "maximum delta: {0}".format(max_delta)
        print stats_dict
        print "*"*30

    return return_value


def _test_against_maxdelta(value, max_delta, name):
    if math.fabs(value) > max_delta:
        print "Dif found: '{0}' difference >{2}<is larger then " \
            "the maximum accepted delta: {1}".format(name, max_delta, value)
        return True
    return False

def compare_image_statistics(stats_dict, max_delta=0.0001):

    return_value = False
    found_incorrect_datapoint = False
    for name, value in stats_dict.items():

        if name == "rms":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 300, name)
        elif name == "medabsdevmed":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 200, name)
        elif name == "minpos":
            pass
            # this min location might move 100 points while still being the same image
        elif name == "min":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 2000, name)
        elif name == "maxpos":
            pass
            # this max location might move 100 points while still being the same image
        elif name == "max":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 1500, name)
        elif name == "sum":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 200000, name)
        elif name == "quartile":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 4000, name)
        elif name == "sumsq":
            # tested with sum already
            pass

        elif name == "median":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta, name)
        elif name == "npts":
            pass    # cannot be tested..
        elif name == "sigma":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 300, name)
        elif name == "mean":
            found_incorrect_datapoint = _test_against_maxdelta(
                float(value[0]), max_delta * 3, name)

        # if we found an incorrect datapoint in this run or with previous
        # results: results in true value if any comparison failed
        return_value = return_value or found_incorrect_datapoint

    return not return_value



# from here sourcelist compare functions
def validate_source_list_files(source_list_1_path, source_list_2_path, max_delta):
    # read the sourcelist files
    fp = open(source_list_1_path)
    sourcelist1 = fp.read()
    fp.close()

    fp = open(source_list_2_path)
    sourcelist2 = fp.read()
    fp.close()

    # convert to dataarrays
    sourcelist_data_1 = convert_sourcelist_as_string_to_data_array(sourcelist1)
    sourcelist_data_2 = convert_sourcelist_as_string_to_data_array(sourcelist2)

    return compare_sourcelist_data_arrays(sourcelist_data_1, sourcelist_data_2, max_delta)


def convert_sourcelist_as_string_to_data_array(source_list_as_string):
    #split in lines
    source_list_lines = source_list_as_string.split("\n")
    entries_array = []

    #get the format line
    format_line_entrie = source_list_lines[0]

    # get the format entries
    entries_array.append([format_line_entrie.split(",")[0].split("=")[1].strip()])
    for entry in format_line_entrie.split(',')[1:]:
        entries_array.append([entry.strip()])

    # scan all the lines for the actual data

    for line in sorted(source_list_lines[2:]):  # try sorting based on name (should work :P)
        # if empty
        if line == "":
            continue
        # add the data entries
        for idx, entrie in enumerate(line.split(",")):
            entries_array[idx].append(entrie.strip())

    return entries_array

def easyprint_data_arrays(data_array1, data_array2):
    print "All data as red from the sourcelists:"
    for (first_array, second_array) in zip(data_array1, data_array2):
        print first_array
        print second_array

def compare_sourcelist_data_arrays(data_array1, data_array2, max_delta=0.0001):
    """
    Ugly function to compare two sourcelists.
    It needs major refactoring, but for a proof of concept it works
    """
    print "######################################################"
    found_incorrect_datapoint = False
    for (first_array, second_array) in zip(data_array1, data_array2):

        # first check if the format string is the same, we have a major fail if this happens
        if first_array[0] != second_array[0]:
            print "******************* problem:"
            print "format strings not equal: {0} != {1}".format(first_array[0], second_array[0])
            found_incorrect_datapoint = True

        # Hard check on equality of the name of the found sources
        if first_array[0] == "Name":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                if entrie1 != entrie2:
                    print "The sourcelist entrie names are not the same: \n{0} !=\n {1}".format(entrie1, entrie2)
                    found_incorrect_datapoint = True

        # Hard check on equality of the type of the found sources
        elif first_array[0] == "Type":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                if entrie1 != entrie2:
                    print "The sourcelist entrie types are not the same: {0} != {1}".format(entrie1, entrie2)
                    found_incorrect_datapoint = True

        # soft check on the Ra: convert to float and compare the values
        elif first_array[0] == "Ra":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_array = entrie1.split(":")
                entrie1_as_float = float(entrie1_as_array[0]) * 3600 + float(entrie1_as_array[1]) * 60 + float(entrie1_as_array[2])# float("".join(entrie1.split(":")))
                entrie2_as_array = entrie2.split(":")
                entrie2_as_float = float(entrie2_as_array[0]) * 3600 + float(entrie2_as_array[1]) * 60 + float(entrie2_as_array[2])
                if not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 10000) :
                    print "we have a problem Ra's are not the same within max_delta: {0} != {1}  max_delta_ra = {2}".format(
                                                entrie1, entrie2, max_delta * 10000)
                    found_incorrect_datapoint = True
        elif first_array[0] == "Dec":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_array = entrie1.strip("+").split(".")
                entrie1_as_float = float(entrie1_as_array[0]) * 3600 + float(entrie1_as_array[1]) * 60 + \
                    float("{0}.{1}".format(entrie1_as_array[2], entrie1_as_array[3]))
                entrie2_as_array = entrie2.strip("+").split(".")
                entrie2_as_float = float(entrie2_as_array[0]) * 3600 + float(entrie2_as_array[1]) * 60 + \
                    float("{0}.{1}".format(entrie2_as_array[2], entrie2_as_array[3]))
                if not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 10000) :
                    print "Dec's are not the same within max_delta: {0} != {1}  max_delta_ra = {2}".format(
                                                entrie1, entrie2, max_delta * 10000)
                    found_incorrect_datapoint = True

        elif first_array[0] == "I":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 2000):
                    print "I's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 1000)
                    found_incorrect_datapoint = True


        elif first_array[0] == "Q":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 1000):
                    print "Q's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 1000)
                    found_incorrect_datapoint = True
        elif first_array[0] == "U":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 1000):
                    print "Q's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 1000)
                    found_incorrect_datapoint = True

        elif first_array[0] == "V":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 1000):
                    print "V's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 1000)
                    found_incorrect_datapoint = True

        elif first_array[0] == "MajorAxis":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 60000):
                    print "MajorAxis's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 50000)
                    found_incorrect_datapoint = True

        elif first_array[0] == "MinorAxis":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 30000):
                    print "MinorAxis's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 30000)
                    found_incorrect_datapoint = True

        elif first_array[0] == "Orientation":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 70000):
                    print "Orientation's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 10000)
                    found_incorrect_datapoint = True

        elif first_array[0].split("=")[0].strip() == "ReferenceFrequency":
            for (entrie1, entrie2) in zip(first_array[1:], second_array[1:]):
                entrie1_as_float = float(entrie1)
                entrie2_as_float = float(entrie2)
                if  not math.fabs(entrie1_as_float - entrie2_as_float) < (max_delta * 10000000):
                    print "Orientation's are not the same within max_delta {0} != {1}   max_delta_I = {2}  ".format(
                                entrie1_as_float, entrie2_as_float, max_delta * 10000000)
                    found_incorrect_datapoint = True
        elif first_array[0].split("=")[0].strip() == "SpectralIndex":
            # Not known yet what will be in the spectral index: therefore do not test it
            pass
        else:
            print "unknown format line entrie found: delta fails"
            print first_array[0]
            found_incorrect_datapoint = True

    if found_incorrect_datapoint:
        print "######################################################"
        print "compared the following data arrays:"
        easyprint_data_arrays(data_array1, data_array2)
        print "######################################################"


    # return  inverse of found_incorrect_datapoint to signal delta test success   
    return not found_incorrect_datapoint


# Test data:
source_list_as_string = """
format = Name, Type, Ra, Dec, I, Q, U, V, MajorAxis, MinorAxis, Orientation, ReferenceFrequency='6.82495e+07', SpectralIndex='[]'

/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i3_s3_g3, GAUSSIAN, 14:58:34.711, +71.42.19.636, 3.145e+01, 0.0, 0.0, 0.0, 1.79857e+02, 1.49783e+02, 1.24446e+02, 6.82495e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i2_s2_g2, GAUSSIAN, 15:09:52.818, +70.48.01.625, 2.321e+01, 0.0, 0.0, 0.0, 2.23966e+02, 1.09786e+02, 1.32842e+02, 6.82495e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i4_s4_g4, GAUSSIAN, 14:53:10.634, +69.29.31.920, 1.566e+01, 0.0, 0.0, 0.0, 1.25136e+02, 4.72783e+01, 6.49083e+01, 6.82495e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i0_s0_g0, POINT, 15:20:15.370, +72.27.35.077, 1.151e+01, 0.0, 0.0, 0.0, 0.00000e+00, 0.00000e+00, 0.00000e+00, 6.82495e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i1_s1_g1, POINT, 15:15:15.623, +66.54.31.670, 4.138e+00, 0.0, 0.0, 0.0, 0.00000e+00, 0.00000e+00, 0.00000e+00, 6.82495e+07, [0.000e+00]

"""

source_list_as_string2 = """
format = Name, Type, Ra, Dec, I, Q, U, V, MajorAxis, MinorAxis, Orientation, ReferenceFrequency='6.82495e+07', SpectralIndex='[]'

/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i3_s3_g3, GAUSSIAN, 14:58:34.711, +71.42.19.636, 3.146e+01, 0.0, 0.0, 0.0, 1.79857e+02, 1.49783e+02, 1.24446e+02, 6.82496e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i2_s2_g2, GAUSSIAN, 15:09:52.818, +70.48.01.625, 2.321e+01, 0.0, 0.0, 0.0, 2.23966e+02, 1.09786e+02, 1.32842e+02, 6.82495e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i4_s4_g4, GAUSSIAN, 14:53:10.634, +69.29.31.920, 1.566e+01, 0.0, 0.0, 0.0, 1.25136e+02, 4.72783e+01, 6.49083e+01, 6.82495e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i0_s0_g0, POINT, 15:20:15.370, +72.27.35.077, 1.151e+01, 0.0, 0.0, 0.0, 0.00000e+00, 0.00000e+00, 0.00000e+00, 6.82495e+07, [0.000e+00]
/data/scratch/klijn/out/awimage_cycle_0/image.restored_w0_i1_s1_g1, POINT, 15:15:15.623, +66.54.31.670, 4.138e+00, 0.0, 0.0, 0.0, 0.00000e+00, 0.00000e+00, 0.00000e+00, 6.82495e+07, [0.000e+00]

"""
#entries_array = convert_sourcelist_as_string_to_data_array(source_list_as_string)
#entries_array2 = convert_sourcelist_as_string_to_data_array(source_list_as_string2)

#print compare_sourcelist_data_arrays(entries_array, entries_array2, 0.0001)

image_data = {'rms': [ 0.], 'medabsdevmed':[ 0.], 'minpos': [0, 0, 0, 0]
              , 'min':[ 0.], 'max': [ 0.],
        'quartile': [ 0.], 'sumsq': [ 0.], 'median': [ 0.], 'npts':[ 65536.],
        'maxpos': [0, 0, 0, 0], 'sigma': [ 0.], 'mean': [ 0.]}


    #{'rms': array([ 0.52093363]), 'medabsdevmed': array([ 0.27387491]), 'minpos': array([156, 221,   0,   0], 
    #dtype=int32), 'min': array([-2.26162958]), 'max': array([ 24.01361465]), 'sum': array([ 1355.46549538]), 
    #'quartile': array([ 0.54873329]), 'sumsq': array([ 17784.62525496]), 'median': array([ 0.00240479]),
    # 'npts': array([ 65536.]), 'maxpos': array([148, 199,   0,   0], dtype=int32),
    # 'sigma': array([ 0.52052685]), 'mean': array([ 0.02068276])}

image_data = {'rms': [ 0.52093363], 'medabsdevmed': [ 0.27387491], 'minpos': [[156, 221, 0, 0], "int32"],
              'min': [-2.26162958], 'max': [ 24.01361465], 'sum': [ 1355.46549538],
                'quartile' : [ 0.54873329], 'sumsq': [ 17784.62525496], 'median': [ 0.00240479],
                'npts': [ 65536.], 'maxpos':[ [148, 199, 0, 0], "int32"],
                'sigma': [ 0.52052685], 'mean': [ 0.02068276]}

# print compare_image_statistics(image_data)



if __name__ == "__main__":
    source_list_1, source_list_2, image_1, image_2, max_delta = None, None, None, None, None
    # Parse parameters from command line
    error = False
    try:
        source_list_1, source_list_2, image_1, image_2 = sys.argv[1:5]
    except:
        print "usage: python {0} source_list_1_path "\
            " source_list_2_path image_1_path image_2_path (max_delta type=float)".format(sys.argv[0])
        sys.exit(1)

    max_delta = None
    try:
        max_delta = float(sys.argv[5])
    except:
        max_delta = 0.0001

    print "using max delta: {0}".format(max_delta)

    if not error:
        image_equality = validate_image_equality(image_1, image_2, max_delta)
        sourcelist_equality = validate_source_list_files(source_list_1, source_list_2, max_delta)
        if not (image_equality and sourcelist_equality):
            print "Regression test failed: exiting with exitstatus 1"
            print " image_equality: {0}".format(image_equality)
            print " sourcelist_equality: {0}".format(sourcelist_equality)
            sys.exit(1)

        print "Regression test Succeed!!"
        sys.exit(0)


