#!/usr/bin/python
# $Id$

'''
Simple Service listening on momqueryservice.GetProjectDetails
which gives the project details for each requested mom object id

Example usage:
service side: just run this service somewhere where it can access the momdb and
a qpid broker.
Make sure the bus exists: qpid-config add exchange topic <busname>

client side: do a RPC call to the <busname>.GetProjectDetails with a
comma seperated string of mom2object id's as argument.
You get a dict of mom2id to project-details-dict back.

with RPC(busname, 'GetProjectDetails') as getProjectDetails:
    res, status = getProjectDetails(ids_string)

'''
from os import stat
import logging
from lofar.messaging import Service
from lofar.messaging.Service import MessageHandlerInterface
from lofar.common.util import waitForInterrupt
from lofar.sas.resourceassignment.resourceassignmentservice import radb
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME

logger = logging.getLogger(__name__)

class RADBHandler(MessageHandlerInterface):
    def __init__(self, **kwargs):
        super(RADBHandler, self).__init__(**kwargs)
        self.username = kwargs.pop("username", 'resourceassignment')
        self.password = kwargs.pop("password", '')

        self.service2MethodMap = {
            self.servicename + '.GetResourceClaimStatuses': self._getResourceClaimStatuses,
            self.servicename + '.GetResourceClaims': self._getResourceClaims,
            self.servicename + '.GetResourceClaim': self._getResourceClaim,
            self.servicename + '.InsertResourceClaim': self._insertResourceClaim,
            self.servicename + '.DeleteResourceClaim': self._deleteResourceClaim,
            self.servicename + '.UpdateResourceClaim': self._updateResourceClaim,
            self.servicename + '.GetResourceGroupTypes': self._getResourceGroupTypes,
            self.servicename + '.GetResourceGroups': self._getResourceGroups,
            self.servicename + '.GetResourceTypes': self._getResourceTypes,
            self.servicename + '.GetResources': self._getResources,
            self.servicename + '.GetTask': self._getTask,
            self.servicename + '.InsertTask': self._insertTask,
            self.servicename + '.DeleteTask': self._deleteTask,
            self.servicename + '.UpdateTask': self._updateTask,
            self.servicename + '.GetTaskStatuses': self._getTaskStatuses,
            self.servicename + '.GetTaskTypes': self._getTaskTypes,
            self.servicename + '.GetTaskTypes': self._getTaskTypes,
            self.servicename + '.GetTasks': self._getTasks,
            self.servicename + '.GetUnits': self._getUnits}

    def prepare_loop(self):
        self.radb = radb.RADatabase(username=self.username,
                                    password=self.password)

    def _getTaskStatuses(self):
        return self.radb.getTaskStatuses()

    def _getTaskTypes(self):
        return self.radb.getTaskTypes()

    def _getResourceClaimStatuses(self):
        return self.radb.getResourceClaimStatuses()

    def _getResourceClaims(self):
        return self.radb.getResourceClaims()

    def _getResourceClaim(self, **kwargs):
        claim = self.radb.getResourceClaim(kwargs['id'])
        return claim

    def _insertResourceClaim(self, **kwargs):
        logger.info('InsertResourceClaim: %s' % kwargs)
        id = self.radb.insertResourceClaim(kwargs['resource_id'],
                                           kwargs['task_id'],
                                           kwargs['starttime'].datetime(),
                                           kwargs['endtime'].datetime(),
                                           kwargs.get('status_id', kwargs.get('status')),
                                           kwargs['session_id'],
                                           kwargs['claim_size'],
                                           kwargs['username'],
                                           kwargs['user_id'])
        return {'id':id}

    def _deleteResourceClaim(self, **kwargs):
        logger.info('DeleteResourceClaim: %s' % kwargs)
        id = kwargs['id']
        deleted = self.radb.deleteResourceClaim(id)
        return {'id': id, 'deleted': deleted}

    def _updateResourceClaim(self, **kwargs):
        logger.info('UpdateResourceClaim: %s' % kwargs)
        id = kwargs['id']
        updated = self.radb.updateResourceClaim(id,
                                                resource_id=kwargs.get('resource_id'),
                                                task_id=kwargs.get('task_id'),
                                                starttime=kwargs['starttime'].datetime() if 'starttime' in kwargs else None,
                                                endtime=kwargs['endtime'].datetime() if 'endtime' in kwargs else None,
                                                status=kwargs.get('status_id', kwargs.get('status')),
                                                session_id=kwargs.get('session_id'),
                                                claim_size=kwargs.get('claim_size'),
                                                username=kwargs.get('username'),
                                                user_id=kwargs.get('user_id'))
        return {'id': id, 'updated': updated}

    def _getResourceGroupTypes(self):
        return self.radb.getResourceGroupTypes()

    def _getResourceGroups(self):
        return self.radb.getResourceGroups()

    def _getResourceTypes(self):
        return self.radb.getResourceTypes()

    def _getResources(self):
        return self.radb.getResources()

    def _getTasks(self):
        return self.radb.getTasks()

    def _getTask(self, **kwargs):
        logger.info('GetTask: %s' % kwargs)
        task = self.radb.getTask(kwargs['id'])
        return task

    def _insertTask(self, **kwargs):
        logger.info('InsertTask: %s' % kwargs)
        task_id = self.radb.insertTask(kwargs['mom_id'],
                                       kwargs['otdb_id'],
                                       kwargs.get('status_id', kwargs.get('status', 'prepared')),
                                       kwargs.get('type_id', kwargs.get('type')),
                                       kwargs['specification_id'])
        return {'id':task_id }

    def _deleteTask(self, **kwargs):
        logger.info('DeleteTask: %s' % kwargs)
        id = kwargs['id']
        deleted = self.radb.deleteTask(id)
        return {'id': id, 'deleted': deleted}

    def _updateTask(self, **kwargs):
        logger.info('UpdateTask: %s' % kwargs)
        id = kwargs['id']
        updated = self.radb.updateTask(id,
                                       mom_id=kwargs.get('mom_id'),
                                       otdb_id=kwargs.get('otdb_id'),
                                       task_status=kwargs.get('status_id', kwargs.get('status', 'prepared')),
                                       task_type=kwargs.get('type_id', kwargs.get('type')),
                                       specification_id=kwargs.get('specification_id'))
        return {'id': id, 'updated': updated}

    def _getUnits(self):
        return self.radb.getUnits()

def createService(busname=DEFAULT_BUSNAME, servicename=DEFAULT_SERVICENAME, radb_password=''):
    return Service(servicename,
                   RADBHandler,
                   busname=busname,
                   handler_args={'password': radb_password},
                   verbose=True)

def main(busname=DEFAULT_BUSNAME, servicename=DEFAULT_SERVICENAME):
    # make sure config.py is mode 600 to hide passwords
    if oct(stat('config.py').st_mode & 0777) != '0600':
        print 'Please change permissions of config.py to 600'
        exit(-1)

    # safely import radb_password
    from lofar.sas.resourceassignment.resourceassignmentservice.config import radb_password

    with createService(busname=busname,
                       servicename=servicename,
                       radb_password=radb_password):
        waitForInterrupt()

if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
    main()
