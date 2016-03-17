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
import logging
from optparse import OptionParser
from lofar.messaging import Service
from lofar.messaging import setQpidLogLevel
from lofar.messaging.Service import MessageHandlerInterface
from lofar.common.util import waitForInterrupt
from lofar.sas.resourceassignment.database import radb
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME
from lofar.common import dbcredentials
from lofar.common.util import convertIntKeysToString

logger = logging.getLogger(__name__)

class RADBHandler(MessageHandlerInterface):
    def __init__(self, **kwargs):
        super(RADBHandler, self).__init__(**kwargs)
        self.dbcreds = kwargs.pop("dbcreds", None)
        self.log_queries = kwargs.pop("log_queries", False)

        self.service2MethodMap = {
            'GetResourceClaimStatuses': self._getResourceClaimStatuses,
            'GetResourceClaimProperties': self._getResourceClaimProperties,
            'InsertResourceClaimProperty': self._insertResourceClaimProperty,
            'GetResourceClaimPropertyTypes': self._getResourceClaimPropertyTypes,
            'GetResourceClaims': self._getResourceClaims,
            'GetResourceClaim': self._getResourceClaim,
            'InsertResourceClaim': self._insertResourceClaim,
            'DeleteResourceClaim': self._deleteResourceClaim,
            'UpdateResourceClaim': self._updateResourceClaim,
            'UpdateTaskAndResourceClaims': self._updateTaskAndResourceClaims,
            'GetResourceGroupTypes': self._getResourceGroupTypes,
            'GetResourceGroups': self._getResourceGroups,
            'GetResourceGroupMemberships': self._getResourceGroupMemberships,
            'GetResourceTypes': self._getResourceTypes,
            'GetResources': self._getResources,
            'GetTasks': self._getTasks,
            'GetTask': self._getTask,
            'InsertTask': self._insertTask,
            'DeleteTask': self._deleteTask,
            'UpdateTask': self._updateTask,
            'InsertTaskPredecessor': self._insertTaskPredecessor,
            'insertTaskPredecessors': self._insertTaskPredecessors,
            'GetTaskStatuses': self._getTaskStatuses,
            'GetTaskTypes': self._getTaskTypes,
            'GetSpecifications': self._getSpecifications,
            'GetSpecification': self._getSpecification,
            'InsertSpecification': self._insertSpecification,
            'DeleteSpecification': self._deleteSpecification,
            'UpdateSpecification': self._updateSpecification,
            'GetUnits': self._getUnits}

    def prepare_loop(self):
        self.radb = radb.RADatabase(dbcreds=self.dbcreds, log_queries=self.log_queries)

    def _getTaskStatuses(self):
        return self.radb.getTaskStatuses()

    def _getTaskTypes(self):
        return self.radb.getTaskTypes()

    def _getResourceClaimStatuses(self):
        return self.radb.getResourceClaimStatuses()

    def _getResourceClaimPropertyTypes(self):
        return self.radb.getResourceClaimPropertyTypes()

    def _getResourceClaimProperties(self, **kwargs):
        return self.radb.getResourceClaimProperties(claim_id=kwargs.get('claim_id'), task_id=kwargs.get('task_id'))

    def _insertResourceClaimProperty(self, **kwargs):
        id = self.radb.insertResourceClaimProperty(kwargs.get('claim_id'), kwargs.get('property_type'), kwargs.get('value'))
        return {'id':id}

    def _getResourceClaims(self, **kwargs):
        return self.radb.getResourceClaims(lower_bound=kwargs.get('lower_bound'),
                                           upper_bound=kwargs.get('upper_bound'),
                                           task_id=kwargs.get('task_id'),
                                           status=kwargs.get('status'),
                                           resource_type=kwargs.get('resource_type'),
                                           extended=kwargs.get('extended', False))

    def _getResourceClaim(self, **kwargs):
        claim = self.radb.getResourceClaim(kwargs['id'])
        return claim

    def _insertResourceClaim(self, **kwargs):
        logger.info('InsertResourceClaim: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        id = self.radb.insertResourceClaim(kwargs['resource_id'],
                                           kwargs['task_id'],
                                           kwargs['starttime'].datetime(),
                                           kwargs['endtime'].datetime(),
                                           kwargs.get('status_id', kwargs.get('status')),
                                           kwargs['session_id'],
                                           kwargs['claim_size'],
                                           kwargs['username'],
                                           kwargs['user_id'],
                                           kwargs.get('properties'))
        return {'id':id}

    def _deleteResourceClaim(self, **kwargs):
        logger.info('DeleteResourceClaim: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        id = kwargs['id']
        deleted = self.radb.deleteResourceClaim(id)
        return {'id': id, 'deleted': deleted}

    def _updateResourceClaim(self, **kwargs):
        logger.info('UpdateResourceClaim: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        id = kwargs['id']
        updated = self.radb.updateResourceClaim(id,
                                                resource_id=kwargs.get('resource_id'),
                                                task_id=kwargs.get('task_id'),
                                                starttime=kwargs['starttime'].datetime() if 'starttime' in kwargs else None,
                                                endtime=kwargs['endtime'].datetime() if 'endtime' in kwargs else None,
                                                status=kwargs.get('status_id', kwargs.get('status')),
                                                session_id=kwargs.get('session_id'),
                                                claim_size=kwargs.get('claim_size'),
                                                nr_of_parts=kwargs.get('nr_of_parts'),
                                                username=kwargs.get('username'),
                                                user_id=kwargs.get('user_id'))
        return {'id': id, 'updated': updated}

    def _updateTaskAndResourceClaims(self, **kwargs):
        logger.info('UpdateTaskAndResourceClaims: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        task_id = kwargs['task_id']
        starttime = kwargs.get('starttime')
        if starttime:
            starttime = starttime.datetime();

        endtime = kwargs.get('endtime')
        if endtime:
            endtime = endtime.datetime();

        updated = self.radb.updateTaskAndResourceClaims(task_id,
                                                        starttime=starttime,
                                                        endtime=endtime,
                                                        task_status=kwargs.get('task_status_id', kwargs.get('task_status')),
                                                        claim_status=kwargs.get('claim_status_id', kwargs.get('claim_status')),
                                                        session_id=kwargs.get('session_id'),
                                                        username=kwargs.get('username'),
                                                        user_id=kwargs.get('user_id'))
        return {'task_id': task_id, 'updated': updated}

    def _getResourceGroupTypes(self):
        return self.radb.getResourceGroupTypes()

    def _getResourceGroups(self):
        return self.radb.getResourceGroups()

    def _getResourceGroupMemberships(self):
        rg_memberships = self.radb.getResourceGroupMemberships()
        rg_memberships = convertIntKeysToString(rg_memberships)
        return rg_memberships

    def _getResourceTypes(self):
        return self.radb.getResourceTypes()

    def _getResources(self):
        return self.radb.getResources()

    def _getTasks(self):
        return self.radb.getTasks()

    def _getTask(self, **kwargs):
        logger.info('GetTask: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        task = self.radb.getTask(id=kwargs.get('id'), mom_id=kwargs.get('mom_id'), otdb_id=kwargs.get('otdb_id'))
        return task

    def _insertTask(self, **kwargs):
        logger.info('InsertTask: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        task_id = self.radb.insertTask(kwargs['mom_id'],
                                       kwargs['otdb_id'],
                                       kwargs.get('status_id', kwargs.get('status', 'prepared')),
                                       kwargs.get('type_id', kwargs.get('type')),
                                       kwargs['specification_id'])
        return {'id':task_id }

    def _deleteTask(self, **kwargs):
        logger.info('DeleteTask: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        id = kwargs['id']
        deleted = self.radb.deleteTask(id)
        return {'id': id, 'deleted': deleted}

    def _updateTask(self, **kwargs):
        logger.info('UpdateTask: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        id = kwargs['id']
        updated = self.radb.updateTask(id,
                                       mom_id=kwargs.get('mom_id'),
                                       otdb_id=kwargs.get('otdb_id'),
                                       task_status=kwargs.get('status_id', kwargs.get('status', 'prepared')),
                                       task_type=kwargs.get('type_id', kwargs.get('type')),
                                       specification_id=kwargs.get('specification_id'))
        return {'id': id, 'updated': updated}

    def _insertTaskPredecessor(self, **kwargs):
        id = self.radb.insertTaskPredecessor(kwargs['task_id'],
                                             kwargs['predecessor_id'])
        return {'id':id}

    def _insertTaskPredecessors(self, **kwargs):
        ids = self.radb.insertTaskPredecessors(kwargs['task_id'],
                                               kwargs['predecessor_ids'])
        return {'ids':ids}

    def _getSpecifications(self):
        return self.radb.getSpecifications()

    def _getSpecification(self, **kwargs):
        logger.info('GetSpecification: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        specification = self.radb.getSpecification(kwargs['id'])
        return specification

    def _insertSpecification(self, **kwargs):
        logger.info('InsertSpecification: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        specification_id = self.radb.insertSpecification(kwargs['starttime'].datetime(),
                                                         kwargs['endtime'].datetime(),
                                                         kwargs['content'])
        return {'id':specification_id}

    def _deleteSpecification(self, **kwargs):
        logger.info('DeleteSpecification: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        id = kwargs['id']
        deleted = self.radb.deleteSpecification(id)
        return {'id': id, 'deleted': deleted}

    def _updateSpecification(self, **kwargs):
        logger.info('UpdateSpecification: %s' % dict({k:v for k,v in kwargs.items() if v != None}))
        id = kwargs['id']
        updated = self.radb.updateSpecification(id,
                                                starttime=kwargs['starttime'].datetime() if 'starttime' in kwargs else None,
                                                endtime=kwargs['endtime'].datetime() if 'endtime' in kwargs else None,
                                                content=kwargs.get('content'))
        return {'id': id, 'updated': updated}

    def _getUnits(self):
        return self.radb.getUnits()

def createService(busname=DEFAULT_BUSNAME, servicename=DEFAULT_SERVICENAME, broker=None, dbcreds=None, log_queries=False, verbose=False):
    return Service(servicename,
                   RADBHandler,
                   busname=busname,
                   broker=broker,
                   use_service_methods=True,
                   numthreads=4,
                   handler_args={'dbcreds': dbcreds, 'log_queries': log_queries},
                   verbose=verbose)

def main():
    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the resourceassignment database service')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option("-b", "--busname", dest="busname", type="string", default=DEFAULT_BUSNAME, help="Name of the bus exchange on the qpid broker, default: %s" % DEFAULT_BUSNAME)
    parser.add_option("-s", "--servicename", dest="servicename", type="string", default=DEFAULT_SERVICENAME, help="Name for this service, default: %s" % DEFAULT_SERVICENAME)
    parser.add_option('-Q', '--log-queries', dest='log_queries', action='store_true', help='log all pqsl queries')
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    parser.add_option_group(dbcredentials.options_group(parser))
    parser.set_defaults(dbcredentials="RADB")
    (options, args) = parser.parse_args()

    dbcreds = dbcredentials.parse_options(options)

    setQpidLogLevel(logging.INFO)
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if options.verbose else logging.INFO)

    with createService(busname=options.busname,
                       servicename=options.servicename,
                       broker=options.broker,
                       verbose=options.verbose,
                       log_queries=options.log_queries,
                       dbcreds=dbcreds):
        waitForInterrupt()

if __name__ == '__main__':
    main()
