#!/usr/bin/env python

import unittest
import uuid
import datetime
import logging
from lofar.messaging import Service
from lofar.sas.resourceassignment.resourceassignmentservice.service import createService
from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC, RARPCException
from qpid.messaging.exceptions import *

try:
    from qpid.messaging import Connection
    from qpidtoollibs import BrokerAgent
except ImportError:
    print 'Cannot run test without qpid tools'
    print 'Please source qpid profile'
    exit(3)

try:
    from mock import MagicMock
    from mock import patch
except ImportError:
    print 'Cannot run test without python MagicMock'
    print 'Please install MagicMock: pip install mock'
    exit(3)

connection = None
broker = None

try:
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
    logger = logging.getLogger(__name__)

    # setup broker connection
    connection = Connection.establish('127.0.0.1')
    broker = BrokerAgent(connection)

    # add test service busname
    busname = 'test-lofarbus-%s' % (uuid.uuid1())
    broker.addExchange('topic', busname)

    # the system under test is the service and the rpc, not the RADatabase
    # so, patch (mock) the RADatabase class during these tests.
    # when the service instantiates an RADatabase it will get the mocked class.
    with patch('lofar.sas.resourceassignment.database.radb.RADatabase', autospec=True) as MockRADatabase:
        mock = MockRADatabase.return_value
        # modify the return values of the various RADatabase methods with pre-cooked answers
        mock.getTaskStatuses.return_value = [{'id': 1, 'name': 'opened'}, {'id': 2, 'name': 'scheduled'}]
        mock.getTaskTypes.return_value = [{'id': 0, 'name': 'OBSERVATION'}, {'id': 1, 'name': 'PIPELINE'}]
        mock.getResourceClaimStatuses.return_value = [{'id': 0, 'name': 'CLAIMED'},{'id': 1, 'name': 'ALLOCATED'},{'id': 2, 'name': 'CONFLICT'}]
        mock.getUnits.return_value = [{'units': 'rsp_channel_bit', 'id': 0},{'units': 'bytes', 'id': 1},{'units': 'rcu_board', 'id': 2},{'units': 'bytes/second', 'id': 3},{'units': 'cores', 'id': 4}]
        mock.getResourceTypes.return_value = [{'unit_id': 0, 'id': 0, 'unit': 'rsp_channel_bit', 'name': 'rsp'},{'unit_id': 1, 'id': 1, 'unit': 'bytes', 'name': 'tbb'},{'unit_id': 2, 'id': 2, 'unit': 'rcu_board', 'name': 'rcu'},{'unit_id': 3, 'id': 3, 'unit': 'bytes/second', 'name': 'bandwidth'},{'unit_id': 4, 'id': 4, 'unit': 'cores', 'name': 'processor'},{'unit_id': 1, 'id': 5, 'unit': 'bytes', 'name': 'storage'}]
        mock.getResourceGroupTypes.return_value = [{'id': 0, 'name': 'instrument'},{'id': 1, 'name': 'cluster'},{'id': 2, 'name': 'station_group'},{'id': 3, 'name': 'station'},{'id': 4, 'name': 'node_group'},{'id': 5, 'name': 'node'}]
        mock.getResources.return_value = [{'type_id': 0, 'type': 'rsp', 'id': 0, 'unit': 'rsp_channel_bit', 'name': 'rsp'},{'type_id': 1, 'type': 'tbb', 'id': 1, 'unit': 'bytes', 'name': 'tbb'},{'type_id': 2, 'type': 'rcu', 'id': 2, 'unit': 'rcu_board', 'name': 'rcu'},{'type_id': 3, 'type': 'bandwidth', 'id': 3, 'unit': 'bytes/second', 'name': 'bandwidth'}]
        mock.getResourceGroups.return_value = [{'type_id': 0, 'type': 'instrument', 'id': 0, 'name': 'LOFAR'},{'type_id': 1, 'type': 'cluster', 'id': 1, 'name': 'CEP2'}]
        mock.getTasks.return_value = [{'status': 'prepared', 'type_id': 1, 'status_id': 200, 'specification_id': 1, 'starttime': datetime.datetime(2015, 11, 30, 12, 0), 'mom_id': -1, 'endtime': datetime.datetime(2015, 11, 30, 15, 0), 'type': 'PIPELINE', 'id': 5, 'otdb_id': -1}]
        mock.getTask.return_value = mock.getTasks.return_value[0]
        mock.getTask.side_effect = lambda x: mock.getTasks.return_value[0] if x == 5 else None
        mock.getResourceClaims.return_value = [{'username': 'paulus', 'status': 'ALLOCATED', 'user_id': 1, 'task_id': 5, 'status_id': 1, 'resource_id': 1, 'session_id': 1, 'claim_size': 10L, 'starttime': datetime.datetime(2015, 11, 30, 12, 0), 'endtime': datetime.datetime(2015, 11, 30, 12, 0), 'id': 5}]

        class Test1(unittest.TestCase):
            '''Test'''

            def test(self):
                '''basic test '''
                rpc = RARPC(busname=busname)
                self.assertEqual(mock.getTaskStatuses.return_value, rpc.getTaskStatuses())
                self.assertEqual(mock.getTaskTypes.return_value, rpc.getTaskTypes())
                self.assertEqual(mock.getResourceClaimStatuses.return_value, rpc.getResourceClaimStatuses())
                self.assertEqual(mock.getUnits.return_value, rpc.getUnits())
                self.assertEqual(mock.getResourceTypes.return_value, rpc.getResourceTypes())
                self.assertEqual(mock.getResourceGroupTypes.return_value, rpc.getResourceGroupTypes())
                self.assertEqual(mock.getResources.return_value, rpc.getResources())
                self.assertEqual(mock.getResourceGroups.return_value, rpc.getResourceGroups())
                self.assertEqual(mock.getTasks.return_value, rpc.getTasks())
                self.assertEqual(mock.getResourceClaims.return_value, rpc.getResourceClaims())

                #TODO: fix this test
                #self.assertEqual(None, rpc.getTask(1))
                #self.assertEqual(mock.getTask.return_value, rpc.getTask(5))

                # test non existing service method, should timeout
                with self.assertRaises(ValueError) as cm:
                    rpc.rpc('foo', timeout=1)
                    self.assertEqual(cm.exception.message, "{'backtrace': '', 'state': 'TIMEOUT', 'errmsg': 'RPC Timed out'}")

                ## test method with wrong args
                #with self.assertRaises(TypeError) as cm:
                    #rpc.rpc('GetTasks', timeout=1, fooarg='bar')
                    #self.assertTrue('got an unexpected keyword argument \'fooarg\'' in cm.exception.message)

        # create and run the service
        with createService(busname=busname):

            # and run all tests
            unittest.main(verbosity=2)

except ConnectError as ce:
    logger.error(ce)
    exit(3)
finally:
    # cleanup test bus and exit
    if broker:
        broker.delExchange(busname)
    if connection:
        connection.close()
