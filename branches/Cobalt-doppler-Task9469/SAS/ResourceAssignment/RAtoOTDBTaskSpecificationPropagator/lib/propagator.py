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

from lofar.sas.otdb.otdbrpc import OTDBRPC
from lofar.sas.otdb.config import DEFAULT_OTDB_SERVICE_BUSNAME, DEFAULT_OTDB_SERVICENAME
from lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.translator import RAtoOTDBTranslator

from lofar.mom.momqueryservice.momqueryrpc import MoMQueryRPC
from lofar.mom.momqueryservice.config import DEFAULT_MOMQUERY_BUSNAME, DEFAULT_MOMQUERY_SERVICENAME

logger = logging.getLogger(__name__)


class RAtoOTDBPropagator():
    def __init__(self,
                 radb_busname=RADB_BUSNAME,
                 radb_servicename=RADB_SERVICENAME,
                 radb_broker=None,
                 otdb_busname=DEFAULT_OTDB_SERVICE_BUSNAME,
                 otdb_servicename=DEFAULT_OTDB_SERVICENAME,
                 mom_busname=DEFAULT_MOMQUERY_BUSNAME,
                 mom_servicename=DEFAULT_MOMQUERY_SERVICENAME,
                 otdb_broker=None,
                 mom_broker=None,
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
            mom_broker  = broker

        self.radbrpc = RADBRPC(busname=radb_busname, servicename=radb_servicename, broker=radb_broker) ## , ForwardExceptions=True hardcoded in RPCWrapper right now
        self.otdbrpc = OTDBRPC(busname=otdb_busname, servicename=otdb_servicename, broker=otdb_broker) ## , ForwardExceptions=True hardcoded in RPCWrapper right now
        self.momrpc = MoMQueryRPC(busname=mom_busname, servicename=mom_servicename, broker=mom_broker)
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
        self.momrpc.open()

    def close(self):
        """Close rpc connections to radb service and resource estimator service"""
        self.radbrpc.close()
        self.otdbrpc.close()
        self.momrpc.close()

    def doTaskConflict(self, otdb_id):
        logger.info('doTaskConflict: otdb_id=%s' % (otdb_id,))
        if not otdb_id:
            logger.warning('doTaskConflict no valid otdb_id: otdb_id=%s' % (otdb_id,))
            return
        try:
            self.otdbrpc.taskSetStatus(otdb_id, 'conflict')
        except Exception as e:
            logger.error(e)

    def doTaskScheduled(self, ra_id, otdb_id, mom_id):
        try:
            logger.info('doTaskScheduled: ra_id=%s otdb_id=%s mom_id=%s' % (ra_id, otdb_id, mom_id))
            if not otdb_id:
                logger.warning('doTaskScheduled no valid otdb_id: otdb_id=%s' % (otdb_id,))
                return
            ra_info = self.getRAinfo(ra_id)

            logger.info('RA info for ra_id=%s otdb_id=%s: %s' % (ra_id, otdb_id, ra_info))

            # check if this is a CEP4 task, or an old CEP2 task
            # at this moment the most simple check is to see if RA claimed (CEP4) storage
            # TODO: do proper check on cluster/storage/etc
            if not ra_info['storage']:
                logger.info("No (CEP4) storage claimed for ra_id=%s otdb_id=%s, skipping otdb specification update." % (ra_id, otdb_id))
                return

            #get mom project name
            try:
                project = self.momrpc.getProjectDetails(mom_id)
                logger.info(project)
                project_name = "_".join(project[str(mom_id)]['project_name'].split())
            except (RPCException, KeyError) as e:
                logger.error('Could not get project name from MoM for mom_id %s: %s' % (mom_id, str(e)))
                logger.info('Using \'unknown\' as project name.')
                project_name = 'unknown'

            otdb_info = self.translator.CreateParset(otdb_id, ra_info, project_name)
            logger.debug("Parset info for OTDB: %s" %otdb_info)
            self.setOTDBinfo(otdb_id, otdb_info, 'scheduled')
        except Exception as e:
            logger.error(e)
            self.doTaskConflict(otdb_id)

    def getRAinfo(self, ra_id):
        info = {}
        info["storage"] = {}
        task = self.radbrpc.getTask(ra_id)
        claims = self.radbrpc.getResourceClaims(task_ids=ra_id, extended=True, include_properties=True)
        for claim in claims:
            logger.debug("Processing claim: %s" % claim)
            if claim['resource_type_name'] == 'storage':
                info['storage'] = claim
        info["starttime"] = task["starttime"]
        info["endtime"] = task["endtime"]
        info["status"] = task["status"]
        return info

    def setOTDBinfo(self, otdb_id, otdb_info, otdb_status):
        try:
            logger.info('Setting specticication for otdb_id %s: %s' % (otdb_id, otdb_info))
            self.otdbrpc.taskSetSpecification(otdb_id, otdb_info)
            self.otdbrpc.taskPrepareForScheduling(otdb_id, otdb_info["LOFAR.ObsSW.Observation.startTime"], otdb_info["LOFAR.ObsSW.Observation.stopTime"])
            logger.info('Setting status (%s) for otdb_id %s' % (otdb_status, otdb_id))
            self.otdbrpc.taskSetStatus(otdb_id, otdb_status)
        except Exception as e:
            logger.error(e)
            self.doTaskConflict(otdb_id)
