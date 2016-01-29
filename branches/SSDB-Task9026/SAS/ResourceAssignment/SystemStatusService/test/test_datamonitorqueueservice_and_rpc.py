#!/usr/bin/env python

import unittest
import uuid
import datetime
from lofar.messaging import Service
from lofar.sas.resourceassignment.systemstatus.ssdbservice import runservice
from lofar.sas.resourceassignment.systemstatus.ssdbrpc import SSDBPC

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

try:
    # setup broker connection
    connection = Connection.establish('127.0.0.1')
    broker = BrokerAgent(connection)

    # add test service busname
    busname = 'test-lofarbus-%s' % (uuid.uuid1())
    broker.addExchange('topic', busname)

    # the system under test is the service and the rpc, not the RADatabase
    # so, patch (mock) the RADatabase class during these tests.
    # when the service instantiates an RADatabase it will get the mocked class.
    with patch('lofar.sas.resourceassignment.resourceassignmentservice.ssdb', autospec=True) as MockSSDBDatabase:
        mock = MockSSDBDatabase.return_value
        # modify the return values of the various RADatabase methods with pre-cooked answers
            def ensure_connected(self):
        self.DBconnected = (self.conn and self.conn.status==1)
        if not self.DBconnected:
            try:
                self.conn= pg.connect("dbname=%s user=%s password=%s" % (DATABASE,USER,PASSWORD))
                self.DBconnected = (self.conn and self.conn.status==1)
            except Exception as e:
                logger.error("DB connection could not be restored.")
        return self.DBconnected

        mock.getstatenames.return_value=
        mock.getactivegroupnames.return_value=
        mock.gethostsforgid.return_value=
        mock.gethostsforgroups.return_value=
        mock.listall.return_value=
        mock.getIngestJobs.return_value=
        mock.getIngestMain.return_value=
        
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
                self.assertEqual(None, rpc.getTask(1))
                self.assertEqual(mock.getTask.return_value, rpc.getTask(5))

                # test non existing service method, should timeout
                with self.assertRaises(RARPCException) as cm:
                    rpc._rpc('foo', timeout=1)
                self.assertEqual(cm.exception.message, "{'backtrace': '', 'state': 'TIMEOUT', 'errmsg': 'RPC Timed out'}")

                # test method with wrong args
                with self.assertRaises(RARPCException) as cm:
                    rpc._rpc('RAS.GetTasks', timeout=1, fooarg='bar')
                #self.assertEqual(cm.exception.message, '''{'backtrace': u'', 'state': u'ERROR', 'errmsg': u"TypeError: _getTasks() got an unexpected keyword argument 'fooarg'\n"}''')

        # create and run the service
        with createService(busname=busname):

            # and run all tests
            unittest.main(verbosity=2)

finally:
    # cleanup test bus and exit
    broker.delExchange(busname)
    connection.close()
