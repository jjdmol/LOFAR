# solfetch.py:
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

import numpy

def fetch(db, stations, phasors=False, parm="Gain:0:0", direction=None,
    asPolar=True):
    """
    Fetch the value of a complex, station bound, parameter from a LOFAR
    parameter database. The parameter values for all station given in "stations"
    should be defined on the same grid.

    db:         A lofar.parmdb.parmdb instance.
    stations:   List of stations for which to retrieve the associated value
                (will be added as an infix / suffix to the parameter base name).
    phasors:    (default False) If set to true, use "Ampl", "Phase" infix
                instead of "Real", "Imag".
    parm:       (default "Gain:0:0") Base name of parameter to fetch.
    direction:  (default None) Source name added to the parameter name as a
                suffix.
    asPolar:    (default True) Return value as (amplitude, phase) if set to
                True, (real, imaginary) otherwise. Conversion is performed as
                needed, depending on the value of 'phasors'.
    """

    suffix = ""
    if direction:
        suffix = ":%s" % direction

    infix = ("Real", "Imag")
    if phasors:
        infix = ("Ampl", "Phase")

    el0 = []
    el1 = []
    for station in stations:
        fqname = "%s:%s:%s%s" % (parm, infix[0], station, suffix)
        el0.append(__fetch_value(db, fqname))

        fqname = "%s:%s:%s%s" % (parm, infix[1], station, suffix)
        el1.append(__fetch_value(db, fqname))

    el0 = numpy.array(el0)
    el1 = numpy.array(el1)

    if phasors and not asPolar:
        re = numpy.zeros(el0.shape)
        im = numpy.zeros(el1.shape)

        for i in range(0, len(stations)):
            re[i] = el0[i] * numpy.cos(el1[i])
            im[i] = el0[i] * numpy.sin(el1[i])

        return (re, im)

    if not phasors and asPolar:
        ampl = numpy.zeros(el0.shape)
        phase = numpy.zeros(el1.shape)

        for i in range(0, len(stations)):
            ampl[i] = numpy.sqrt(numpy.power(el0[i], 2) + numpy.power(el1[i], 2))
            phase[i] = numpy.arctan2(el1[i], el0[i])

        return (ampl, phase)

    return (el0, el1)

def __fetch_value(db, parm):
    tmp = db.getValuesGrid(parm)[parm]
    if type(tmp) is dict:
        return numpy.squeeze(tmp["values"])

    # Old parmdb interface.
    return numpy.squeeze(tmp)
