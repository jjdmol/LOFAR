#!/usr/bin/env python

# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: assignment.py 1580 2015-09-30 14:18:57Z loose $

"""
RAtoOTDBTaskSpecificationPropagator gets a task to be scheduled in OTDB,
reads the info from the RA DB and sends it to OTDB in the correct format.
"""

import logging

#from lofar.parameterset import parameterset

logger = logging.getLogger(__name__)

""" Prefix that is common to all parset keys, depending on the exact source. """
PREFIX="LOFAR.ObsSW."
##TODO use this.

class RAtoOTDBTranslator():
    def __init__(self):
        """
        RAtoOTDBTranslator translates values from the RADB into parset keys to be stored in an OTDB Tree
        """

    def CreateParset(self, mom_id, ra_info):
        logger.info('CreateParset: start=%s, end=%s' % (ra_info['starttime'], ra_info['endtime']))

        parset = {}
        #parset[PREFIX+'Observation.momID'] = str(mom_id)
        parset[PREFIX+'Observation.startTime'] = ra_info['starttime'].strftime('%Y-%m-%d %H:%M:%S')
        parset[PREFIX+'Observation.stopTime'] = ra_info['endtime'].strftime('%Y-%m-%d %H:%M:%S')

        if "stations" in ra_info.keys():
            parset[PREFIX+'Observation.VirtualInstrument.stationList'] = stations
        return parset






