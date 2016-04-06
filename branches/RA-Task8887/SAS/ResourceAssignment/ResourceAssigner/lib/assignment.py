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
from datetime import datetime, timedelta
import time
import collections

from lofar.common.util import humanreadablesize
from lofar.messaging.RPC import RPC, RPCException
from lofar.parameterset import parameterset

from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME as RADB_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_SERVICENAME as RADB_SERVICENAME

from lofar.sas.resourceassignment.resourceassignmentestimator.config import DEFAULT_BUSNAME as RE_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentestimator.config import DEFAULT_SERVICENAME as RE_SERVICENAME

from lofar.sas.systemstatus.service.SSDBrpc import SSDBRPC
from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_BUSNAME
from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_SERVICENAME

logger = logging.getLogger(__name__)

class ResourceAssigner():
    def __init__(self,
                 radb_busname=RADB_BUSNAME,
                 radb_servicename=RADB_SERVICENAME,
                 radb_broker=None,
                 re_busname=RE_BUSNAME,
                 re_servicename=RE_SERVICENAME,
                 re_broker=None,
                 ssdb_busname=DEFAULT_SSDB_BUSNAME,
                 ssdb_servicename=DEFAULT_SSDB_SERVICENAME,
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
        self.ssdbrpc = SSDBRPC(servicename=ssdb_servicename, busname=ssdb_busname, broker=ssdb_broker)

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
        self.ssdbrpc.open()

    def close(self):
        """Close rpc connections to radb service and resource estimator service"""
        self.radbrpc.close()
        self.rerpc.close()
        self.ssdbrpc.close()

    def doAssignment(self, specification_tree):
        logger.info('doAssignment: specification_tree=%s' % (specification_tree))

        otdb_id = specification_tree['otdb_id']
        taskType = specification_tree.get('task_type', '').lower()
        status = specification_tree.get('state', 'prescheduled').lower()

        #parse main parset...
        mainParset = parameterset(specification_tree['specification'])
        momId = mainParset.getInt('Observation.momID', -1)
        startTime = datetime.strptime(mainParset.getString('Observation.startTime'), '%Y-%m-%d %H:%M:%S')
        endTime = datetime.strptime(mainParset.getString('Observation.stopTime'), '%Y-%m-%d %H:%M:%S')

        #check if task already present in radb
        existingTask = self.radbrpc.getTask(otdb_id=otdb_id)

        if existingTask:
            #present, delete it, and create a new task
            taskId = existingTask['id']
            self.radbrpc.deleteTask(taskId)
            specificationId = existingTask['specification_id']
            self.radbrpc.deleteSpecification(specificationId)

        #insert new task and specification in the radb
        logger.info('doAssignment: insertSpecification startTime=%s endTime=%s' % (startTime, endTime))
        specificationId = self.radbrpc.insertSpecification(startTime, endTime, str(mainParset))['id']
        logger.info('doAssignment: insertSpecification specificationId=%s' % (specificationId,))

        logger.info('doAssignment: insertTask momId=%s sasId=%s status=%s taskType=%s specificationId=%s' % (momId, otdb_id, status, taskType, specificationId))
        taskId = self.radbrpc.insertTask(momId, otdb_id, status, taskType, specificationId)['id']
        logger.info('doAssignment: insertTask taskId=%s' % (taskId,))

        needed = self.getNeededResouces(specification_tree)
        logger.info('doAssignment: getNeededResouces=%s' % (needed,))

        if not str(otdb_id) in needed:
            logger.error("no otdb_id %s found in estimator results %s" % (otdb_id, needed))
            return

        if not taskType in needed[str(otdb_id)]:
            logger.error("no task type %s found in estimator results %s" % (taskType, needed[str(otdb_id)]))
            return

        try:
            #analyze the parset for needed and available resources and claim these in the radb
            available = self.getAvailableResources('cep4')
            logger.info('doAssignment: getAvailableResources=%s' % (available,))
        except Exception as e:
            logger.warning("Exception while getting available resources: %s" % str(e))
            return

        main_needed = needed[str(otdb_id)]
        if self.checkResources(main_needed, available):
            task = self.radbrpc.getTask(taskId)
            claimed, claim_ids = self.claimResources(main_needed, task)
            if claimed:
                self.radbrpc.updateTaskAndResourceClaims(taskId, claim_status='allocated')
                claims = self.radbrpc.getResourceClaims(task_id=taskId)
                if len(claim_ids) == len([x for x in claims if x['status'] == 'allocated']):
                    self.radbrpc.updateTask(taskId, status='scheduled')
                else:
                    self.radbrpc.updateTask(taskId, status='conflict')
            else:
                self.radbrpc.updateTask(taskId, status='conflict')

        self.processPredecessors(specification_tree)

    def processPredecessors(self, specification_tree):
        try:
            predecessor_trees = specification_tree['predecessors']

            if predecessor_trees:
                otdb_id = specification_tree['otdb_id']
                task = self.radbrpc.getTask(otdb_id=otdb_id)

                for predecessor_tree in predecessor_trees:
                    pred_otdb_id = predecessor_tree['otdb_id']
                    predecessor_task = self.radbrpc.getTask(otdb_id=pred_otdb_id)
                    if predecessor_task:
                        self.radbrpc.insertTaskPredecessor(task['id'], predecessor_task['id'])
                    self.processPredecessors(predecessor_tree)

        except Exception as e:
            logger.error(e)

    def getNeededResouces(self, specification_tree):
        replymessage, status = self.rerpc({"specification_tree":specification_tree}, timeout=10)
        logger.info('getNeededResouces: %s' % replymessage)
        #stations = replymessage['observation']['stations']
        ##stations = parset.getStringVector('Observation.VirtualInstrument.stationList', '')
        ##logger.info('Stations: %s' % stations)
        return replymessage

    def getAvailableResources(self, cluster, update_radb=True):
        # find out which resources are available
        # and what is their capacity
        # For now, only look at CEP4 storage
        # Later, also look at stations up/down for short term scheduling

        #get all active groupnames, find id for cluster group
        groupnames = self.ssdbrpc.getactivegroupnames()
        cluster_group_id = next(k for k,v in groupnames.items() if v == cluster)

        # for CEP4 cluster, do hard codes lookup of first and only node
        node_info = self.ssdbrpc.gethostsforgid(cluster_group_id)['nodes'][0]

        if update_radb:
            storage_resources = self.radbrpc.getResources(resource_types='storage', include_availability=True)
            cep4_storage_resource = next(x for x in storage_resources if 'cep4' in x['name'])
            active = node_info['statename'] == 'Active'
            total_capacity = node_info['totalspace']
            available_capacity = total_capacity - node_info['usedspace']

            logger.info("Updating resource availability of %s (id=%s) to active=%s available_capacity=%s total_capacity=%s" %
                        (cep4_storage_resource['name'], cep4_storage_resource['id'], active, available_capacity, total_capacity))

            self.radbrpc.updateResourceAvailability(cep4_storage_resource['id'],
                                                    active=active,
                                                    available_capacity=available_capacity,
                                                    total_capacity=total_capacity)

        return node_info

    def checkResources(self, needed, available):
        # For now, only check cep4 storage
        total_needed_storage = sum(x['storage']['total_size'] for x in needed.values() if 'storage' in x)
        total_available_storage = available['totalspace']

        logger.info("%senough storage resources available: needed=%s, available=%s" %
                    ('not ' if total_needed_storage >= total_available_storage else '',
                     humanreadablesize(total_needed_storage),
                     humanreadablesize(total_available_storage)))
        return total_needed_storage < total_available_storage

    def claimResources(self, needed_resources, task):
        logger.info('claimResources: task %s needed_resources=%s' % (task, needed_resources))

        # get the needed resources for the task type
        needed_resources_for_task_type = needed_resources[task['type']]

        # get db lists
        rc_property_types = {rcpt['name']:rcpt['id'] for rcpt in self.radbrpc.getResourceClaimPropertyTypes()}
        resource_types = {rt['name']:rt['id'] for rt in self.radbrpc.getResourceTypes()}
        resources = self.radbrpc.getResources()

        # loop over needed_resources -> resource_type -> claim (and props)
        # flatten the tree dict to a list of claims (with props)
        claims = []
        for resource_type_name, needed_claim_for_resource_type in needed_resources_for_task_type.items():
            if resource_type_name in resource_types:
                logger.info('claimResources: processing resource_type: %s' % resource_type_name)
                db_resource_type_id = resource_types[resource_type_name]
                db_resources_for_type = [r for r in resources if r['type_id'] == db_resource_type_id]

                # needed_claim_for_resource_type is a dict containing exactly one kvp of which the value is an int
                # that value is the value for the claim
                needed_claim_value = next((v for k,v in needed_claim_for_resource_type.items() if isinstance(v, int)))

                # FIXME: right now we just pick the first resource from the 'cep4' resources.
                # estimator will deliver this info in the future
                db_cep4_resources_for_type = [r for r in db_resources_for_type if 'cep4' in r['name'].lower()]

                if db_cep4_resources_for_type:
                    claim = {'resource_id':db_cep4_resources_for_type[0]['id'],
                            'starttime':task['starttime'],
                            'endtime':task['endtime'] + datetime.timedelta(days=31),
                            'status':'claimed',
                            'claim_size':needed_claim_value}

                    # if the needed_claim_for_resource_type dict contains more kvp's,
                    # then the subdict contains groups of properties for the claim
                    if len(needed_claim_for_resource_type) > 1:
                        claim['properties'] = []
                        needed_prop_groups = next((v for k,v in needed_claim_for_resource_type.items() if isinstance(v, collections.Iterable)))

                        def processProperties(propertiesDict, sap_nr=None):
                            for prop_type_name, prop_value in propertiesDict.items():
                                if prop_type_name in rc_property_types:
                                    rc_property_type_id = rc_property_types[prop_type_name]
                                    property = {'type':rc_property_type_id, 'value':prop_value}
                                    if sap_nr is not None:
                                        property['sap_nr'] = sap_nr
                                    claim['properties'].append(property)
                                else:
                                    logger.error('claimResources: unknown prop_type:%s' % prop_type_name)

                        for group_name, needed_prop_group in needed_prop_groups.items():
                            if group_name == 'saps':
                                for sap_dict in needed_prop_group:
                                    processProperties(sap_dict['properties'], sap_dict['sap_nr'])
                            else:
                                processProperties(needed_prop_group)

                    logger.info('claimResources: created claim:%s' % claim)
                    claims.append(claim)
            else:
                logger.error('claimResources: unknown resource_type:%s' % resource_type_name)

        logger.info('claimResources: inserting %d claims in the radb' % len(claims))
        claim_ids = self.radbrpc.insertResourceClaims(task['id'], claims, 1, 'anonymous', -1)['ids']
        logger.info('claimResources: %d claims were inserted in the radb' % len(claim_ids))
        return len(claim_ids) == len(claims), claim_ids

