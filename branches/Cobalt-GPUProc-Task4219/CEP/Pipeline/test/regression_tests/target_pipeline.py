import pyrap.tables as pt
import numpy
import sys

def load_and_compare_data_sets(ms1, ms2):
    # open the two datasets
    ms1 = pt.table(ms1)
    ms2 = pt.table(ms2)

    #get the amount of rows in the dataset
    n_row = len(ms1.getcol('DATA'))
    n_complex_vis = 4

    # create a target array with the same length as the datacolumn
    div_array = numpy.zeros((n_row, 1, n_complex_vis), dtype=numpy.complex64)
    ms1_array = ms1.getcol('DATA')
    ms2_array = ms2.getcol('DATA')

    div_max = 0
    for idx in xrange(n_row):
        for idy  in xrange(n_complex_vis):

            div_value = ms1_array[idx][0][idy] - ms2_array[idx][0][idy]
            if numpy.abs(div_value) > numpy.abs(div_max):
                div_max = div_value

            div_array[idx][0][idy] = div_value
    print "maximum different value between measurement sets: {0}".format(div_max)

    if div_max != 0:
        print "The measurement sets are contained a different value"
        print "failed delta test!"
        return False

    return True





if __name__ == "__main__":
    ms_1, mw_2 = None, None
    # Parse parameters from command line
    error = False
    print sys.argv
    try:
        ms_1, mw_2 = sys.argv[1:3]
    except Exception, e:
        print e
        print "usage: python {0} ms1 "\
            " ms2 ".format(sys.argv[0])
        print "target calibration is deterministic and should result in the same ms"
        sys.exit(1)

    if not error:
        print "regression test:"
        data_equality = load_and_compare_data_sets(ms_1, mw_2)

        if not data_equality:
            print "Regression test failed: exiting with exitstatus 1"
            sys.exit(1)

        print "Regression test Succeed!!"
        sys.exit(0)
