# solflag.py:
#
# Copyright (C) 2007
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

import sys
import math
import numpy
import pylab
import pyrap.tables
import lofar.parmdb
import solfetch

def flag(msName, dbName, half_window, threshold, sources=None, storeFlags=True,
    updateMain=True, cutoffLow=None, cutoffHigh=None, debug=False):
    """
    Solution based flagging.

    msName:         name of the measurement to flag
    dbName:         name of solution parameter database
    half_window:    half the size of the window used in the flagging algorithm
    threshold:      threshold for the flagging algorithm (median of the absolute
                    distance to the median); typical values 2, 3, 4
    sources:        (default None) for directional gains solutions, specify the
                    source directions that should be considered
    storeFlags:     (default True) if set to False, the flags will not be
                    written to the measurement
    updateMain:     (default True) if set to True, both the FLAG and the
                    FLAG_ROW column will be updated if set to False, only the
                    FLAG_ROW column will be updated
    cutoffLow:      (default None) if set, all values less than or equal to
                    cutoffLow will be flagged
    cutoffHigh:     (default None) if set, all values greater than or equal to
                    cutoffHigh will be flagged
    debug:          (default False) if set to True, a plot is generated for each
                    source - station combination that shows what has been flagged.
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
    elements = ["0:0", "1:1"]

    print "fetching solutions from %s..." % dbName,
    sys.stdout.flush()
    ampl = __fetch(db, elements, stations, sources)
    print "done."
    sys.stdout.flush()

    # Determine the number of directions.
    if sources is None:
        n_directions = 1
    else:
        n_directions = len(sources)

    # Get the number of time samples.
    n_samples = ampl.shape[-1]

    # Flag based on solutions.
    print "flagging..."
    sys.stdout.flush()

    for stat in range(0, len(stations)):
        # Allocate flag array for this station.
        flags = numpy.zeros(n_samples, bool)

        for src in range(0, n_directions):
            for el in range(0, len(elements)):
                # Create padded data array.
                sol = numpy.zeros(n_samples + 2 * half_window)
                sol[half_window:half_window + n_samples] = ampl[el][src][stat]

                for i in range(0, half_window):
                    # Mirror at left edge.
                    idx = min(n_samples - 1, half_window - i)
                    sol[i] = ampl[el][src][stat][idx]

                    # Mirror at right edge
                    idx = max(0, n_samples - 2 - i)
                    sol[n_samples + half_window + i] = ampl[el][src][stat][idx]

                sol_flag = numpy.zeros(n_samples + 2 * half_window, dtype=bool)

                # Thresholding.
                if not cutoffLow is None:
                    sol_flag[sol <= cutoffLow] = True

                if not cutoffHigh is None:
                    sol_flag[sol >= cutoffHigh] = True

                for i in range(half_window, half_window + n_samples):
                    # Compute median of the absolute distance to the median.
                    window = sol[i - half_window:i + half_window + 1]
                    window_flag = sol_flag[i - half_window:i + half_window + 1]
                    window_masked = window[~window_flag]

                    if len(window_masked) < math.sqrt(len(window)):
                        # Not enough data to get accurate statistics.
                        continue

                    median = numpy.median(window_masked)
                    q = 1.4826 * numpy.median(numpy.abs(window_masked - median))

                    # Flag sample if it is more than 1.4826 * threshold * the
                    # median distance away from the median.
                    if abs(sol[i] - median) > (threshold * q):
                        sol_flag[i] = True

                if debug:
                    # Get masked x-axis and solutions.
                    mask = ~sol_flag[half_window:half_window + n_samples]
                    x_axis = numpy.array(range(0, n_samples))
                    x_axis = x_axis[mask]

                    sol_masked = sol[half_window:half_window + n_samples]
                    sol_masked = sol_masked[mask]

                    fig_index = stat * n_directions + src + 1
                    pylab.figure(fig_index)
                    if el == 0:
                        pylab.clf()

                    pylab.subplot("21%d" % (el + 1))
                    pylab.plot(ampl[el][src][stat], 'r-')
                    pylab.plot(x_axis, sol_masked, 'go', markersize=4)

                # Merge flags based on the solutions for the current direction
                # into the station flags.
                flags = flags | sol_flag[half_window:half_window + n_samples]

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

    print "done."

def __fetch(db, elements, stations, directions=None):
    result = None

    if directions is None:
        for i in range(0, len(elements)):
            (ampl, phase) = solfetch.fetch(db, stations, parm="Gain:%s" %
                elements[i])

            # Allocate result array if necessary.
            if result is None:
                result = numpy.zeros((len(elements), 1, len(stations),
                    ampl.shape[-1]))

            # Copy solutions into result array.
            assert(result[i][0].shape == ampl.shape)
            result[i][0] = ampl
    else:
        for i in range(0, len(elements)):
            for j in range(0, len(directions)):
                # Fetch solutions.
                (ampl, phase) = solfetch.fetch(db, stations,
                    parm="DirectionalGain:%s" % elements[i],
                    direction=directions[j])

                # Allocate result array if necessary.
                if result is None:
                    result = numpy.zeros((len(elements), len(directions),
                        len(stations), ampl.shape[-1]))

                # Copy solutions into result array.
                assert(result[i][j].shape == ampl.shape)
                result[i][j] = ampl

    return result
