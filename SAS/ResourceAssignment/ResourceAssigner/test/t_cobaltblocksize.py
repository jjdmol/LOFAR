#!/usr/bin/env python

import unittest
from lofar.sas.resourceassignment.resourceassigner.cobaltblocksize import *
import logging

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)

class TestBlockConstraints(unittest.TestCase):
    def test_blockSizes(self):
        """ Test whether minBlockSize <= idealBlockSize <= maxBlockSize. """

        for clockMHz in [160, 200]:
            c = BlockConstraints(clockMHz=clockMHz)

            self.assertLessEqual(c.minBlockSize(), c.idealBlockSize())
            self.assertLessEqual(c.idealBlockSize(), c.maxBlockSize())

    def testCorrelator(self):
        """ Test basic constraints for the Correlator. """

        corr = CorrelatorSettings()
        corr.nrChannelsPerSubband = 64
        corr.integrationTime = 1.0

        c = BlockConstraints(correlatorSettings=corr)

        self.assertEqual(c.nrSubblocks(), 1)
        self.assertGreaterEqual(c.factor(), 1)

    def testCorrelatorSubblocks(self):
        """ Test basic constraints for the Correlator if there are subblocks. """

        corr = CorrelatorSettings()
        corr.nrChannelsPerSubband = 64
        corr.integrationTime = 0.1

        c = BlockConstraints(correlatorSettings=corr)

        self.assertGreater(c.nrSubblocks(), 1)
        self.assertGreaterEqual(c.factor(), 1)

        # Set of subblocks needs to be valid
        self.assertGreater(c.nrSubblocks() * c._time2samples(corr.integrationTime), c.minBlockSize())
        self.assertLess(c.nrSubblocks() * c._time2samples(corr.integrationTime), c.maxBlockSize())

    def testCoherentStokes(self):
        """ Test basic constraints for the Coherent beamformer. """

        coh = StokesSettings()
        coh.nrChannelsPerSubband = 16
        coh.timeIntegrationFactor = 4

        c = BlockConstraints(coherentStokesSettings=coh)

        self.assertEqual(c.nrSubblocks(), 1)
        self.assertGreaterEqual(c.factor(), 1)

    def testIncoherentStokes(self):
        """ Test basic constraints for the Incoherent beamformer. """

        incoh = StokesSettings()
        incoh.nrChannelsPerSubband = 16
        incoh.timeIntegrationFactor = 4

        c = BlockConstraints(incoherentStokesSettings=incoh)

        self.assertEqual(c.nrSubblocks(), 1)
        self.assertGreaterEqual(c.factor(), 1)

class TestBlockSize(unittest.TestCase):
    def testCorrelatorIntegrationTime(self):
        """ Test whether specified integration times 0.05 - 30.0 result in integration times with an error <5%. """

        def integrationTimes():
            i = 0.05
            while i < 10.0:
                yield i
                i += 0.05
            while i < 30.0:
                yield i
                i += 0.5

        for integrationTime in integrationTimes():
            correlator = CorrelatorSettings()
            correlator.nrChannelsPerSubband = 64
            correlator.integrationTime = integrationTime

            c = BlockConstraints( correlator, None, None )
            bs = BlockSize(c)

            self.assertAlmostEquals(c._samples2time(bs.integrationSamples), integrationTime, delta = integrationTime * 0.05)

if __name__ == "__main__":
    unittest.main(verbosity=2)

