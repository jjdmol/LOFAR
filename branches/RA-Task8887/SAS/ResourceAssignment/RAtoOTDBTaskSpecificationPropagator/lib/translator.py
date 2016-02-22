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
ResourceAssigner inserts/updates tasks and assigns resources to it based on incoming parset.
"""

import logging
from datetime import datetime
import time

from lofar.messaging.RPC import RPC, RPCException
from lofar.parameterset import parameterset

from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME as RADB_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_SERVICENAME as RADB_SERVICENAME
from lofar.sas.resourceassignment.resourceassignmentestimator.config import DEFAULT_BUSNAME as RE_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentestimator.config import DEFAULT_SERVICENAME as RE_SERVICENAME

logger = logging.getLogger(__name__)

class ResourceAssigner():
    def __init__(self,
                 radb_busname=RADB_BUSNAME,
                 radb_servicename=RADB_SERVICENAME,
                 radb_broker=None,
                 re_busname=RE_BUSNAME,
                 re_servicename=RE_SERVICENAME,
                 re_broker=None,
                 ssdb_busname='lofar.system',
                 ssdb_servicename='SSDBService',
                 ssdb_broker=None,
                 broker=None):
        """
        ResourceAssigner inserts/updates tasks in the radb and assigns resources to it based on incoming parset.
        :param radb_busname: busname on which the radb service listens (default: lofar.ra.command)
        :param radb_servicename: servicename of the radb service (default: RADBService)
        :param radb_broker: valid Qpid broker host (default: None, which means localhost)
        :param re_busname: busname on which the resource estimator service listens (default: lofar.ra.command)
        :param re_servicename: servicename of the resource estimator service (default: ResourceEstimation)
        :param re_broker: valid Qpid broker host (default: None, which means localhost)
        :param ssdb_busname: busname on which the ssdb service listens (default: lofar.system)
        :param ssdb_servicename: servicename of the radb service (default: SSDBService)
        :param ssdb_broker: valid Qpid broker host (default: None, which means localhost)
        :param broker: if specified, overrules radb_broker, re_broker and ssdb_broker. Valid Qpid broker host (default: None, which means localhost)
        """
        if broker:
            radb_broker = broker
            re_broker = broker
            ssdb_broker = broker

        self.radbrpc = RARPC(servicename=radb_servicename, busname=radb_busname, broker=radb_broker)
        self.rerpc = RPC(re_servicename, busname=re_busname, broker=re_broker, ForwardExceptions=True)
        self.ssdbGetActiveGroupNames = RPC(ssdb_servicename+'.GetActiveGroupNames', busname=ssdb_busname, broker=ssdb_broker, ForwardExceptions=True)
        self.ssdbGetHostForGID = RPC(ssdb_servicename+'.GetHostForGID', busname=ssdb_busname, broker=ssdb_broker, ForwardExceptions=True)

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
        self.rerpc.open()
        self.ssdbGetActiveGroupNames.open()
        self.ssdbGetHostForGID.open()

    def close(self):
        """Close rpc connections to radb service and resource estimator service"""
        self.radbrpc.close()
        self.rerpc.close()
        self.ssdbGetActiveGroupNames.close()
        self.ssdbGetHostForGID.close()

    def doAssignment(self, sasId, parsets, status='prescheduled'):
        logger.info('doAssignment: sasId=%s parset=%s' % (sasId, parsets))

        #parse main parset...
        mainParsetDict = parsets[str(sasId)]
        mainParset = parameterset(mainParsetDict)
        momId = mainParset.getInt('Observation.momID', -1)
        taskType = mainParset.getString('Task.type', '')
        if taskType.lower() == 'observation':
            taskType = 'Observation'
        startTime = datetime.strptime(mainParset.getString('Observation.startTime'), '%Y-%m-%d %H:%M:%S')
        endTime = datetime.strptime(mainParset.getString('Observation.stopTime'), '%Y-%m-%d %H:%M:%S')

        ##check if task already present in radb
        #existingTask = self.radbrpc.getTask(otdb_id=sasId)

        #if existingTask:
            ##present, so update task and specification in radb
            #taskId = existingTask['id']
            #specificationId = existingTask['specification_id']
            #self.radbrpc.updateSpecification(specificationId, startTime, endTime, str(mainParsetDict))
            #self.radbrpc.updateTask(taskId, momId, sasId, status, taskType, specificationId)
        #else:
        #insert new task and specification in the radb
        specificationId = self.radbrpc.insertSpecification(startTime, endTime, str(mainParsetDict))['id']
        taskId = self.radbrpc.insertTask(momId, sasId, status, taskType, specificationId)['id']

        #analyze the parset for needed and available resources and claim these in the radb
        cluster = self.parseSpecification(mainParset)
        available = self.getAvailableResources(cluster)

        needed = self.getNeededResouces(mainParset)

        if self.checkResources(needed, available):
            claimed, resourceIds = self.claimResources(needed, taskId, startTime, endTime)
            if claimed:
                self.commitResourceClaimsForTask(taskId)
                self.radbrpc.updateTask(taskId, status='scheduled')
            else:
                self.radbrpc.updateTask(taskId, status='conflict')

    def parseSpecification(self, parset):
        # TODO: cluster is not part of specification yet. For now return CEP4. Add logic later.
        default = "cep2"
        cluster ="cep4"
        return cluster

    def getNeededResouces(self, parset):
        replymessage, status = self.rerpc(parset.dict(), timeout=10)
        logger.info('getNeededResouces: %s' % replymessage)
        stations = parset.getStringVector('Observation.VirtualInstrument.stationList', '')
        logger.info('Stations: %s' % stations)
        return replymessage

    def getAvailableResources(self, cluster):
        # Used settings
        groupnames = {}
        available    = {}
        while True:
            try:
                replymessage, status = self.ssdbGetActiveGroupNames()
                if status == 'OK':
                    groupnames = replymessage
                    logger.info('SSDBService ActiveGroupNames: %s' % groupnames)
                else:
                    logger.error("Could not get active group names from SSDBService: %s" % status)

                groupnames = {v:k for k,v in groupnames.items()} #swap key/value for name->id lookup
                logger.info('groupnames: %s' % groupnames)
                if cluster in groupnames.keys():
                    groupId = groupnames[cluster]
                    replymessage, status = self.ssdbGetHostForGID(groupId)
                    if status == 'OK':
                        available = replymessage
                        logger.info('available: %s' % available)
                    else:
                        logger.error("Could not get hosts for group %s (gid=%s) from SSDBService: %s" % (cluster, groupId, status))
                else:
                    logger.error("group \'%s\' not known in SSDBService active groups (%s)" % (cluster, ', '.join(groupnames.keys())))
                return available
            except KeyboardInterrupt:
                break
            except Exception as e:
                logger.warning("Exception while getting available resources. Trying again... " + str(e))
                time.sleep(0.25)

    def checkResources(self, needed, available):
        return True

    def claimResources(self, resources, taskId, startTime, endTime):
        #TEMP HACK
        cep4storage = resources['Observation']['total_data_size']
        resources = dict()
        resources['cep4storage'] = cep4storage

        resourceNameDict = {r['name']:r for r in self.radbrpc.getResources()}
        claimedStatusId = next(x['id'] for x in self.radbrpc.getResourceClaimStatuses() if x['name'].lower() == 'claimed')

        resourceClaimIds = []

        for r in resources:
            if r in resourceNameDict:
                resourceClaimIds.append(self.radbrpc.insertResourceClaim(resourceNameDict[r]['id'], taskId, startTime, endTime, claimedStatusId, 1, -1, 'anonymous', -1))

        success = len(resourceClaimIds) == len(resources)
        return success, resourceClaimIds

    def commitResourceClaimsForTask(self, taskId):
        self.radbrpc.updateResourceClaimsForTask(taskId, status='ALLOCATED')

