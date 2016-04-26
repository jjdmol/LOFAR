#!/usr/bin/python

import logging
import datetime
from lofar.messaging.RPC import RPC, RPCException, RPCWrapper
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME
from lofar.common.util import convertStringDigitKeysToInt

''' Simple RPC client for Service lofarbus.*Z
'''

logger = logging.getLogger(__name__)

class RARPCException(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return "RARPCException: " + str(self.message)

class RARPC(RPCWrapper):
    def __init__(self, busname=DEFAULT_BUSNAME,
                 servicename=DEFAULT_SERVICENAME,
                 broker=None):
        super(RARPC, self).__init__(busname, servicename, broker)

    def getResourceClaimStatuses(self):
        return self.rpc('GetResourceClaimStatuses')

    def getResourceClaimPropertyTypes(self):
        return self.rpc('GetResourceClaimPropertyTypes')

    def getResourceClaimProperties(self, claim_ids=None, task_id=None):
        return self.rpc('GetResourceClaimProperties', claim_ids=claim_ids, task_id=task_id)

    def insertResourceClaimProperty(self, claim_id, property_type, value):
        return self.rpc('InsertResourceClaimProperty', claim_id=claim_id, property_type=property_type, value=value)

    def getResourceClaims(self, claim_ids=None, lower_bound=None, upper_bound=None, task_id=None, status=None, resource_type=None, extended=False, include_properties=False):
        claims = self.rpc('GetResourceClaims', claim_ids=claim_ids,
                                               lower_bound=lower_bound,
                                               upper_bound=upper_bound,
                                               task_id=task_id,
                                               status=status,
                                               resource_type=resource_type,
                                               extended=extended,
                                               include_properties=include_properties)
        for claim in claims:
            claim['starttime'] = claim['starttime'].datetime()
            claim['endtime'] = claim['endtime'].datetime()
        return claims

    def getResourceClaim(self, id):
        resource_claim = self.rpc('GetResourceClaim', id=id)
        if resource_claim:
            resource_claim['starttime'] = resource_claim['starttime'].datetime()
            resource_claim['endtime'] = resource_claim['endtime'].datetime()
        return resource_claim


    def insertResourceClaim(self, resource_id, task_id, starttime, endtime, status, session_id, claim_size, username, user_id, properties=None):
        return self.rpc('InsertResourceClaim', resource_id=resource_id,
                                                    task_id=task_id,
                                                    starttime=starttime,
                                                    endtime=endtime,
                                                    status=status,
                                                    session_id=session_id,
                                                    claim_size=claim_size,
                                                    username=username,
                                                    user_id=user_id,
                                                    properties=properties)

    def insertResourceClaims(self, task_id, claims, session_id, username, user_id):
        return self.rpc('InsertResourceClaims', task_id=task_id,
                                                claims=claims,
                                                session_id=session_id,
                                                username=username,
                                                user_id=user_id)

    def deleteResourceClaim(self, id):
        return self.rpc('DeleteResourceClaim', id=id)

    def updateResourceClaim(self, id, resource_id=None, task_id=None, starttime=None, endtime=None, status=None, session_id=None, claim_size=None, username=None, user_id=None):
        return self.rpc('UpdateResourceClaim', id=id,
                                                    resource_id=resource_id,
                                                    task_id=task_id,
                                                    starttime=starttime,
                                                    endtime=endtime,
                                                    status=status,
                                                    session_id=session_id,
                                                    claim_size=claim_size,
                                                    username=username,
                                                    user_id=user_id)

    def updateTaskAndResourceClaims(self, task_id, starttime=None, endtime=None, task_status=None, claim_status=None, session_id=None, username=None, user_id=None):
        return self.rpc('UpdateTaskAndResourceClaims', task_id=task_id,
                                                        starttime=starttime,
                                                        endtime=endtime,
                                                        task_status=task_status,
                                                        claim_status=claim_status,
                                                        session_id=session_id,
                                                        username=username,
                                                        user_id=user_id)

    def getResourceUsages(self, claim_ids=None, lower_bound=None, upper_bound=None, resource_ids=None, task_ids=None, status=None, resource_type=None):
        all_usages = self.rpc('GetResourceUsages',
                              lower_bound=lower_bound,
                              upper_bound=upper_bound,
                              resource_ids=resource_ids,
                              task_ids=task_ids,
                              status=status,
                              resource_type=resource_type)

        for resource_usages in all_usages:
            for status, usages in resource_usages['usages'].items():
                for usage in usages:
                    usage['timestamp'] = usage['timestamp'].datetime()

        return all_usages

    def getResourceGroupTypes(self):
        return self.rpc('GetResourceGroupTypes')

    def getResourceGroups(self):
        return self.rpc('GetResourceGroups')

    def getResourceGroupMemberships(self):
        rg_memberships = self.rpc('GetResourceGroupMemberships')
        rg_memberships = convertStringDigitKeysToInt(rg_memberships)
        return rg_memberships

    def getResourceTypes(self):
        return self.rpc('GetResourceTypes')

    def getResources(self, resource_ids=None, resource_types=None, include_availability=False):
        return self.rpc('GetResources', resource_ids=resource_ids, resource_types=resource_types, include_availability=include_availability)

    def updateResourceAvailability(self, resource_id, active=None, available_capacity=None, total_capacity=None):
        return self.rpc('UpdateResourceAvailability',
                        resource_id=resource_id,
                        active=active,
                        available_capacity=available_capacity,
                        total_capacity=total_capacity)

    def getTask(self, id=None, mom_id=None, otdb_id=None):
        '''get a task for either the given (task)id, or for the given mom_id, or for the given otdb_id'''
        task = self.rpc('GetTask', id=id, mom_id=mom_id, otdb_id=otdb_id)
        if task:
            task['starttime'] = task['starttime'].datetime()
            task['endtime'] = task['endtime'].datetime()
        return task

    def insertTask(self, mom_id, otdb_id, status, task_type, specification_id):
        return self.rpc('InsertTask', mom_id=mom_id,
                                           otdb_id=otdb_id,
                                           status=status,
                                           type=task_type,
                                           specification_id=specification_id)

    def deleteTask(self, id):
        return self.rpc('DeleteTask', id=id)

    def updateTask(self, task_id, mom_id=None, otdb_id=None, status=None, task_type=None, specification_id=None):
        return self.rpc('UpdateTask',
                         id=task_id,
                         mom_id=mom_id,
                         otdb_id=otdb_id,
                         status=status,
                         task_type=task_type,
                         specification_id=specification_id)

    def updateTaskStatusForOtdbId(self, otdb_id, status):
        return self.rpc('UpdateTaskStatusForOtdbId',
                         otdb_id=otdb_id,
                         status=status)

    def getTasks(self, lower_bound=None, upper_bound=None):
        tasks = self.rpc('GetTasks', lower_bound=lower_bound, upper_bound=upper_bound)
        for task in tasks:
            task['starttime'] = task['starttime'].datetime()
            task['endtime'] = task['endtime'].datetime()
        return tasks

    def insertTaskPredecessor(self, task_id, predecessor_id):
        return self.rpc('InsertTaskPredecessor', task_id=task_id, predecessor_id=predecessor_id)

    def insertTaskPredecessors(self, task_id, predecessor_ids):
        return self.rpc('InsertTaskPredecessors', task_id=task_id, predecessor_ids=predecessor_ids)

    def getTaskTypes(self):
        return self.rpc('GetTaskTypes')

    def getTaskStatuses(self):
        return self.rpc('GetTaskStatuses')

    def getSpecification(self, id):
        specification = self.rpc('GetSpecification', id=id)
        if specification:
            specification['starttime'] = specification['starttime'].datetime()
            specification['endtime'] = specification['endtime'].datetime()
        return specification

    def insertSpecificationAndTask(self, mom_id, otdb_id, task_status, task_type, starttime, endtime, content):
        return self.rpc('InsertSpecificationAndTask',
                        mom_id=mom_id,
                        otdb_id=otdb_id,
                        task_status=task_status,
                        task_type=task_type,
                        starttime=starttime,
                        endtime=endtime,
                        content=content)

    def insertSpecification(self, starttime, endtime, content):
        return self.rpc('InsertSpecification',
                        starttime=starttime,
                        endtime=endtime,
                        content=content)

    def deleteSpecification(self, id):
        return self.rpc('DeleteSpecification', id=id)

    def updateSpecification(self, id, starttime=None, endtime=None, content=None):
        return self.rpc('UpdateSpecification',
                         id=id,
                         starttime=starttime,
                         endtime=endtime,
                         content=content)

    def getSpecifications(self):
        specifications = self.rpc('GetSpecifications')
        for specification in specifications:
            specification['starttime'] = specification['starttime'].datetime()
            specification['endtime'] = specification['endtime'].datetime()
        return specifications

    def getUnits(self):
        return self.rpc('GetUnits')

def do_tests(busname=DEFAULT_BUSNAME, servicename=DEFAULT_SERVICENAME):
    with RARPC(busname=busname, servicename=servicename) as rpc:
        #for i in range(0, 10):
            #taskId = rpc.insertTask(1234, 5678, 'active', 'OBSERVATION', 1)['id']
            #rcId = rpc.insertResourceClaim(1, taskId, datetime.datetime.utcnow(), datetime.datetime.utcnow() + datetime.timedelta(hours=1), 'CLAIMED', 1, 10, 'einstein', -1)['id']
            #print rpc.getResourceClaim(rcId)
            #rpc.updateResourceClaim(rcId, starttime=datetime.datetime.utcnow(), endtime=datetime.datetime.utcnow() + datetime.timedelta(hours=2), status='ALLOCATED')
            #print rpc.getResourceClaim(rcId)
            #print

        #tasks = rpc.getTasks()
        #for t in tasks:
            #print rpc.getTask(t['id'])
            #for i in range(4,9):
                #rcId = rpc.insertResourceClaim(i, t['id'], datetime.datetime.utcnow(), datetime.datetime.utcnow() + datetime.timedelta(hours=1), 'CLAIMED', 1, 10, 'einstein', -1)['id']
            ##print rpc.deleteTask(t['id'])
            ##print rpc.getTasks()
            ##print rpc.getResourceClaims()

        #print
        #taskId = tasks[0]['id']
        #print 'taskId=', taskId
        #print rpc.getResourceClaimsForTask(taskId)
        #print rpc.updateResourceClaimsForTask(taskId, starttime=datetime.datetime.utcnow(), endtime=datetime.datetime.utcnow() + datetime.timedelta(hours=3))
        #print rpc.getResourceClaimsForTask(taskId)

        #print rpc.getTasks()
        #print rpc.getResourceClaims()
        #print rpc.getResources()
        #print rpc.getResourceGroups()
        #print rpc.getResourceGroupMemberships()

        for rc in rpc.getResourceClaims():
            print rc
            rpc.insertResourceClaimProperty(rc['id'], 'nr_of_CS_files', 42)
            print rpc.getResourceClaimProperties(rc['id'])

        print
        print rpc.getResourceClaimProperties(task_id=493)

        #rpc.deleteTask(taskId)

        #print rpc.getTasks()
        #print rpc.getResourceClaims()



if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
    do_tests()
