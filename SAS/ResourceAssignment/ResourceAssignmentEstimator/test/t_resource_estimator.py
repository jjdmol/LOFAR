#!/usr/bin/env python

import unittest
import uuid
import datetime
from lofar.messaging import RPC, RPCException
from lofar.sas.resourceassignment.resourceassignmentestimator.service import createService
from lofar.sas.resourceassignment.resourceassignmentestimator.test.testset import TestSet

# add test service busname
busname = 'test-lofarbus-%s' % (uuid.uuid1())

class Test1(unittest.TestCase):
    '''Test'''

    def test(self):
        '''basic test '''
        ts = TestSet()

        # test observation
        ts.add_observation()
        with RPC('ResourceEstimation', busname=busname, timeout=3) as rpc:
            result = rpc( ts.test_dict() )
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
