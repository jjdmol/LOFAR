import sys
import numpy
import pylab
import lofar.parmdb
import pyrap.tables

def fetch(db, elements, sources, stations):
    parmRe = {}
    parmIm = {}
    
    # Seem to be a limit to the number of elements in {} in pyparmdb??
    for station in stations:
        parmRegexRe = "Gain:{%s}:Real:%s:{%s}" % (",".join(elements), station, ",".join(sources))
        parmRegexIm = "Gain:{%s}:Imag:%s:{%s}" % (",".join(elements), station, ",".join(sources))

        tmp = db.getValuesGrid(parmRegexRe)
        del tmp["_grid"]
        parmRe.update(tmp)

        tmp = db.getValuesGrid(parmRegexIm)
        del tmp["_grid"]
        parmIm.update(tmp)
    
    ampl = None
    for i in range(0, len(elements)):
        for j in range(0, len(sources)):
            for k in range(0, len(stations)):
                name = "Gain:%s:Real:%s:%s" % (elements[i], stations[k], sources[j])
                re = numpy.array(parmRe[name])
                name = "Gain:%s:Imag:%s:%s" % (elements[i], stations[k], sources[j])
                im = numpy.array(parmIm[name])
       
                tmp = numpy.sqrt(re * re + im * im)
                tmp = tmp.transpose()
               
                if ampl is None:
                    ampl = numpy.zeros((len(elements), len(sources), len(stations), len(tmp[0])))

                ampl[i,j,k] = tmp[0]

    return ampl
    
def flag(msName, dbName, half_window, threshold, storeFlags=True, updateMain=False, cutoffLow=None, cutoffHigh=None, debug=False):
    """
    Solution based flagging.
    
    msName:         name of the measurement to flag
    dbName:         name of solution parameter database
    half_window:    half the size of the window used in the flagging algorithm
    threshold:      threshold for the flagging algorithm (median of the absolute
                    distance to the median); typical values 2, 3, 4
    storeFlags:     (default True) if set to False, the flags will not be
                    written to the measurement
    updateMain:     (default False) if set to True, both the FLAG and the
                    FLAG_ROW column will be updated if set to False, only the
                    FLAG_ROW column will be updated
    cutoffLow:      (default None) if set, all values less than or equal to
                    cutoffLow will be flagged
    cutoffHigh:     (default None) if set, all values greater than or equal to
                    cutoffHigh will be flagged
    debug:          (default False) if set to True, a plot is generated for each
                    station that show what has been flagged
    """

    # Read station names from MS.
    antennaTable = pyrap.tables.table("%s/ANTENNA" % msName)
    stations = antennaTable.getcol("NAME")
    antennaTable.done()
    del antennaTable
    
    # Open main MS table.
    ms = None
    if storeFlags:
        ms = pyrap.tables.table(msName, readonly=False)
    
    # Open solution database.
    db = lofar.parmdb.parmdb(dbName)
    
    # Get solutions from solution database.
    elements = ["11","22"]
    sources = ["CasA","CygA"]
    ampl = fetch(db, elements, sources, stations)
    
    # Get the number of time samples.
    n_samples = ampl.shape[3]
    
    # Flag based on solutions.
    for stat in range(0, len(stations)):
        # Allocate flag array.
        flags = numpy.zeros(n_samples, bool)
    
        for src in range(0, len(sources)):
            for el in range(0, len(elements)):
                data = ampl[el, src, stat, :]
                assert(len(data) == n_samples)
                
                data_padded = numpy.zeros(n_samples + 2 * half_window)
                data_padded[half_window:half_window + n_samples] = data
                data_padded[0:half_window] = data[half_window:0:-1]
                data_padded[len(data_padded) - half_window + 1:len(data_padded)] = data[n_samples:n_samples-half_window:-1]

                for i in range(0, n_samples):
                    # Compute median of the absolute distance to the median.
                    window = data_padded[i:i+2*half_window]
                    median = numpy.median(window)
                    q = 1.4826 * numpy.median(numpy.abs(window - median))
            
                    # Flag sample if it is more than 1.4826 * threshold * the
                    # median distance away from the median.
                    if abs(data[i] - median) > (threshold * q):
                        flags[i] = True
                        data[i] = median

                if not cutoffLow is None:
                    flags[data <= cutoffLow] = True
                    
                if not cutoffHigh is None:
                    flags[data >= cutoffHigh] = True

                if debug:
                    pylab.clf()
                    pylab.plot(data_padded[half_window:len(data)+half_window])
                    pylab.plot(data)
                    pylab.show()

#                raw_input("Please press enter to continue...")
        
        print "(%.2f%%) %s" % (100.0 * numpy.sum(flags) / n_samples, stations[stat])
        sys.stdout.flush()

        if storeFlags:                
            stationTable = ms.query("ANTENNA1 == %d || ANTENNA2 == %d" % (stat, stat), sortlist="TIME,ANTENNA1,ANTENNA2")
            baselineIter = pyrap.tables.tableiter(stationTable, ["ANTENNA1", "ANTENNA2"])
            for baseline in baselineIter:
                assert(baseline.nrows() == len(flags))
                
                # Update row flags
                msRowFlags = baseline.getcol("FLAG_ROW")
                msRowFlags |= flags
                baseline.putcol("FLAG_ROW", msRowFlags)
                
                # Update main flags
                if updateMain:
                    msFlags = baseline.getcol("FLAG")
                    for i in range(0, n_samples):
                        msFlags[i, :, :] |= flags[i]
                    baseline.putcol("FLAG", msFlags)

