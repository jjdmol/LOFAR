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

from lofar.sas.resourceassignment.RAtoOTDBTaskSpecificationPropagator.rpc import RARPC
from lofar.sas.resourceassignment.RAtoOTDBTaskSpecificationPropagator.config import DEFAULT_BUSNAME as RADB_BUSNAME
from lofar.sas.resourceassignment.RAtoOTDBTaskSpecificationPropagator.config import DEFAULT_SERVICENAME as RADB_SERVICENAME
from lofar.sas.otdb.otdbservice.config import DEFAULT_BUSNAME as OTDB_BUSNAME
from lofar.sas.otdb.otdbservice.config import DEFAULT_SERVICENAME as OTDB_SERVICENAME
from lofar.sas.resourceassignment.RAtoOTDBTaskSpecificationPropagator.rpc import RAtoOTDBTranslator

logger = logging.getLogger(__name__)


class RAtoOTDBPropagator():
    def __init__(self,
                 radb_busname=RADB_BUSNAME,
                 radb_servicename=RADB_SERVICENAME,
                 radb_broker=None,
                 otdb_busname=OTDB_BUSNAME,
                 otdb_servicename=OTDB_SERVICENAME,
                 otdb_broker=None,
                 broker=None):
        """
        RAtoOTDBPropagator updates tasks in the OTDB after the ResourceAssigner is done with them.
        :param radb_busname: busname on which the radb service listens (default: lofar.ra.command)
        :param radb_servicename: servicename of the radb service (default: RADBService)
        :param radb_broker: valid Qpid broker host (default: None, which means localhost)
        :param otdb_busname: busname on which the OTDB service listens (default: lofar.otdb.command)
        :param otdb_servicename: servicename of the OTDB service (default: OTDBService)
        :param otdb_broker: valid Qpid broker host (default: None, which means localhost)
        :param broker: if specified, overrules radb_broker and otdb_broker. Valid Qpid broker host (default: None, which means localhost)
        """
        if broker:
            radb_broker = broker
            otdb_broker = broker

        self.radbrpc = RARPC(servicename=radb_servicename, busname=radb_busname, broker=radb_broker)
        self.otdbrpc = RPC(otdb_servicename, busname=otdb_busname, broker=otdb_broker, ForwardExceptions=True)
        self.translator = RAtoOTDBTranslator()

    def __enter__(self):
        """Internal use only. (handles scope 'with')"""
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Internal use only. (handles scope 'with')"""
        self.close()

    def open(self):
        """Open rpc connections to radb service and resource estimator service"""
        self.radbrpc.open()
        self.otdbrpc.open()

    def close(self):
        """Close rpc connections to radb service and resource estimator service"""
        self.radbrpc.close()
        self.otdbrpc.close()

    def doPropagation(self, raId, otdbId, momId, status): #status has no default.
        logger.info('doPropagation: otdbId=%s momId=%s' % (sasId, momId))
        
        if not otdbId:
            logger.warning('doPropagation no valid otdbId: otdbId=%s' % (sasId, momId))
            return
        
        if status == 'conflict':
            self.otdbrpc.UpdateTreeStatus(otdbId, 'conflict')
        elif status == 'scheduled':
            RAinfo = self.getRAinfo(raId)
            OTDBinfo = self.translator.doTranslation(RAinfo)
            self.setOTDBinfo(otdbId, OTDBinfo, 'scheduled')
        else:
            logger.warning('doPropagation received unknown status: %s' % (status,))
    
    def getRAinfo(raId):
        info = {}
        task = self.radbrpc.GetTask(raId)
        info.update(task)
    
    def setOTDBinfo(otdbId, OTDBinfo, OTDBstatus)
        r = self.otdbrpc.UpdateTreeKey(otdbId, OTDBinfo('OTDBkeys'))
        if r:
            r = self.otdbrpc.UpdateTreeStatus(otdbId, OTDBstatus)
