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
from datetime import datetime
import time

from lofar.messaging.RPC import RPC, RPCException
from lofar.parameterset import parameterset

from lofar.sas.resourceassignment.rotspservice.rpc import RARPC
from lofar.sas.resourceassignment.rotspservice.config import DEFAULT_BUSNAME as RADB_BUSNAME
from lofar.sas.resourceassignment.rotspservice.config import DEFAULT_SERVICENAME as RADB_SERVICENAME
from lofar.sas.otdb.otdbservice.config import DEFAULT_BUSNAME as OTDB_BUSNAME
from lofar.sas.otdb.otdbservice.config import DEFAULT_SERVICENAME as OTDB_SERVICENAME

logger = logging.getLogger(__name__)


class RAtoOTDBTranslator():
    def __init__(self):
        """
        RAtoOTDBTranslator translates values from the RADB into parset keys
        """

    def doTranslation(self, otdbId, momId, RAinfo):
        logger.info('doTranslation: start=%s, stop=%s' % (RAinfo['startTime'], RAinfo['stopTime']))

        #parse main parset...
        mainParsetDict = parsets[str(sasId)]
        parset = parameterset()
        momId = parset.add('Observation.momID', momId)
        if stations in RAinfo:
            parset.add('Observation.VirtualInstrument.stationList', stations)

        startTime = datetime.strptime(mainParset.getString('Observation.startTime'), '%Y-%m-%d %H:%M:%S')
        endTime = datetime.strptime(mainParset.getString('Observation.stopTime'), '%Y-%m-%d %H:%M:%S')





