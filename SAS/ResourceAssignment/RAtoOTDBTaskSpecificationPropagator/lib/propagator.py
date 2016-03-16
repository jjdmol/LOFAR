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
import datetime
import time

from lofar.messaging.RPC import RPC, RPCException
from lofar.parameterset import parameterset

from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC as RADBRPC
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME as RADB_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_SERVICENAME as RADB_SERVICENAME

from lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.otdbrpc import OTDBRPC
from lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.otdbrpc import DEFAULT_BUSNAME as OTDB_BUSNAME
from lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.otdbrpc import DEFAULT_SERVICENAME as OTDB_SERVICENAME
from lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.translator import RAtoOTDBTranslator

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

        self.radbrpc = RADBRPC(busname=radb_busname, servicename=radb_servicename, broker=radb_broker) ## , ForwardExceptions=True hardcoded in RPCWrapper right now
        self.otdbrpc = OTDBRPC(busname=otdb_busname, servicename=otdb_servicename, broker=otdb_broker) ## , ForwardExceptions=True hardcoded in RPCWrapper right now
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

    def doTaskConflict(self, ra_id, otdb_id, mom_id):
        logger.info('doTaskConflict: otdb_id=%s mom_id=%s' % (otdb_id, mom_id))
        if not otdb_id:
            logger.warning('doTaskConflict no valid otdb_id: otdb_id=%s' % (otdb_id,))
            return
        self.otdbrpc.taskSetStatus(otdb_id, 'conflict')

    def doTaskScheduled(self, ra_id, otdb_id, mom_id):
        logger.info('doTaskScheduled: otdb_id=%s mom_id=%s' % (otdb_id, mom_id))
        if not otdb_id:
            logger.warning('doTaskScheduled no valid otdb_id: otdb_id=%s' % (otdb_id,))
            return
        ra_info = self.getRAinfo(ra_id)
        otdb_info = self.translator.CreateParset(mom_id, ra_info)
        self.setOTDBinfo(otdb_id, otdb_info, 'scheduled')
    
    def getRAinfo(self, ra_id):
        info = {}
        task = self.radbrpc.getTask(ra_id)
        claims = self.radbrpc.getResourceClaims(task_id=ra_id, extended=True)
        for claim in claims:
            print claim
        #resource_ids = [x['resource_id'] for x in claims]
        #all_resources = self.radbrpc.getResources()
        #resources = [all_resources[id] for id in resource_ids]
        info["starttime"] = task["starttime"] + datetime.timedelta(hours=1)
        info["endtime"] = task["endtime"] + datetime.timedelta(hours=1)
        info["status"] = task["status"]
        return info
    
    def setOTDBinfo(self, otdb_id, otdb_info, otdb_status):
        self.otdbrpc.taskSetSpecification(otdb_id, otdb_info)
        #self.otdbrpc.taskPrepareForScheduling(otdb_id, otdb_info["LOFAR.ObsSW.Observation.startTime"], otdb_info["LOFAR.ObsSW.Observation.stopTime"])
        self.otdbrpc.taskSetStatus(otdb_id, otdb_status)
