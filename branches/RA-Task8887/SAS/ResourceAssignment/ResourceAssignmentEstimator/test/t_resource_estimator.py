#!/usr/bin/env python

import unittest
import uuid
import datetime
import logging
from lofar.messaging import RPC, RPCException
from lofar.sas.resourceassignment.resourceassignmentestimator.service import createService
from lofar.sas.resourceassignment.resourceassignmentestimator.test.testset import TestSet

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)

try:
    from qpid.messaging import Connection
    from qpidtoollibs import BrokerAgent
except ImportError:
    print 'Cannot run test without qpid tools'
    print 'Please source qpid profile'
    exit(3)

try:
    # setup broker connection
    connection = Connection.establish('127.0.0.1')
    broker = BrokerAgent(connection)

    # add test service busname
    busname = 'test-lofarbus-raestimator-%s' % (uuid.uuid1())
    broker.addExchange('topic', busname)

    class Test1(unittest.TestCase):
        '''Test'''

        def test(self):
            '''basic test '''
            self.maxDiff = None
            ts = TestSet()

            # test observation
            ts.add_observation()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc(ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add beams
            ts.add_observation_beams()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add flys_eye
            ts.enabble_flys_eye()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add coherent_stokes
            ts.enable_observations_coherent_stokes()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add incoherent_stokes
            ts.enable_observations_incoherent_stokes()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add calibration_pipeline
            ts.enable_calibration_pipeline()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add longbaseline_pipeline
            ts.enable_longbaseline_pipeline()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add pulsar_pipeline
            ts.enable_pulsar_pipeline()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

            # test add image_pipeline
            ts.enable_image_pipeline()
            with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
                result = rpc( ts.test_dict() )
            self.assertEqual(result[0], ts.valid_dict())

    # create and run the service
    with createService(busname=busname):
        # and run all tests
        unittest.main(verbosity=2)

finally:
    # cleanup test bus and exit
    broker.delExchange(busname)
    connection.close()
