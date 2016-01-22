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

if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

logger = logging.getLogger(__name__)


class RADBHandler(MessageHandlerInterface):
    def __init__(self, **kwargs):
        super(RADBHandler, self).__init__(**kwargs)
        self.username = kwargs.pop("username", 'resourceassignment')
        self.password = kwargs.pop("password", '')

        self.service2MethodMap = {
            'RAS.GetResourceClaimStatuses': self._getResourceClaimStatuses,
            'RAS.GetResourceClaims': self._getResourceClaims,
            'RAS.GetResourceClaim': self._getResourceClaim,
            'RAS.InsertResourceClaim': self._insertResourceClaim,
            'RAS.DeleteResourceClaim': self._deleteResourceClaim,
            'RAS.UpdateResourceClaim': self._updateResourceClaim,
            'RAS.GetResourceGroupTypes': self._getResourceGroupTypes,
            'RAS.GetResourceGroups': self._getResourceGroups,
            'RAS.GetResourceTypes': self._getResourceTypes,
            'RAS.GetResources': self._getResources,
            'RAS.GetTask': self._getTask,
            'RAS.InsertTask': self._insertTask,
            'RAS.DeleteTask': self._deleteTask,
            'RAS.UpdateTask': self._updateTask,
            'RAS.GetTaskStatuses': self._getTaskStatuses,
            'RAS.GetTaskTypes': self._getTaskTypes,
            'RAS.GetTaskTypes': self._getTaskTypes,
            'RAS.GetTasks': self._getTasks,
            'RAS.GetUnits': self._getUnits}

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

def createService(busname='lofarbus', radb_password=''):
    return Service('RAS.*',
                   RADBHandler,
                   busname=busname,
                   handler_args={'password': radb_password},
                   verbose=True)

def main():
    # make sure config.py is mode 600 to hide passwords
    if oct(stat('config.py').st_mode & 0777) != '0600':
        print 'Please change permissions of config.py to 600'
        exit(-1)

    # safely import radb_password
    from lofar.sas.resourceassignment.resourceassignmentservice.config import radb_password

    with createService(busname='lofarbus', radb_password=radb_password):
        waitForInterrupt()

if __name__ == '__main__':
    main()
