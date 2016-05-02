"""
  Code to derive the following parset input parameters for Cobalt:

    Cobalt.blockSize

        The number of samples in each unit of work. Needs to be a multiple of the working size
        of each individual step, for example, an 64-channel FFT requires blockSize to be a multiple
        of 64.

    Cobalt.Correlator.nrBlocksPerIntegration

        The number of correlator integration periods that fit in one block.

    Cobalt.Correlator.nrIntegrationsPerBlock

        The number of blocks that together form one integration period.

  Note that either nrBlocksPerIntegration or nrIntegrationsPerBlock has to be equal to 1.
"""

from fractions import gcd
from math import ceil

def lcm(a, b):
    """ Return the Least Common Multiple of a and b. """
    return abs(a * b) / gcd(a,b) if a and b else 0

class CorrelatorSettings(object):
    """ Settings for the Correlator. """

    def __init__(self):
        self.nrChannelsPerSubband = 64
        self.integrationTime      = 1.0

class StokesSettings(object):
    """ Settings for the Beamformer. """

    def __init__(self):
        self.nrChannelsPerSubband  = 1
        self.timeIntegrationFactor = 1

class BlockConstraints(object):
    """ Provide the constraints for the block size, as derived
        from the correlator and beamformer settings. """

    def __init__(self, correlatorSettings=None, coherentStokesSettings=None, incoherentStokesSettings=None, clockMHz=200):
        self.correlator       = correlatorSettings
        self.coherentStokes   = coherentStokesSettings
        self.incoherentStokes = incoherentStokesSettings

        self.clockMHz = clockMHz

    def minBlockSize(self):
        """ Block size below which the overhead per block becomes unwieldy. """
        return int(round(self._time2samples(0.6)))

    def maxBlockSize(self):
        """ Block size above which the data does not fit on the GPU. """
        return int(round(self._time2samples(1.3)))

    def nrSubblocks(self):
        if self.correlator:
            integrationSamples = self._time2samples(self.correlator.integrationTime)
            if integrationSamples < self.minBlockSize():
                return max(1, int(round((self.maxBlockSize() + self.minBlockSize()) / 2.0 / integrationSamples)))

        return 1

    def idealBlockSize(self):
        integrationTime = self.correlator.integrationTime if self.correlator else 1.0
        return self.nrSubblocks() * self._time2samples(integrationTime)

    def factor(self):
        """
          Determine common factors needed for the block Size.

          The Cobalt GPU kernels require the Cobalt.blockSize to be a multiple
          of several values in order to:
             1) divide the work evenly over threads and blocks.
             2) prevent integration of samples from crossing blockSize boundaries.
        """

        factor = 1

        # Process correlator settings
        if self.correlator:
            # FIR_Filter.cu (16 = nr of PPF taps)
            factor = lcm(factor, 16 * self.correlator.nrChannelsPerSubband)

            # Correlator.cu (minimum of 16 samples per channel)
            factor = lcm(factor, 16 * self.correlator.nrChannelsPerSubband * self.nrSubblocks())

        if self.coherentStokes:
            # DelayAndBandPass.cu
            factor = lcm(factor, 16 * 64)

            # CoherentStokesKernel.cc (1024 = maxNrThreadsPerBlock)
            factor = lcm(factor, 1024 * self.coherentStokes.timeIntegrationFactor)

            # CoherentStokes.cu (integration should fit)
            factor = lcm(factor, 1024 * self.coherentStokes.timeIntegrationFactor * self.coherentStokes.nrChannelsPerSubband)

        if self.incoherentStokes:
            # DelayAndBandPass.cu
            factor = lcm(factor, 16 * 64)

            # IncoherentStokes.cu (integration should fit)
            factor = lcm(factor, 1024 * self.incoherentStokes.timeIntegrationFactor * self.incoherentStokes.nrChannelsPerSubband)

        return factor

    def _time2samples(self, t):
        """ Convert a time `t' (seconds) into a number of station samples. """
        return int(round(t * self.clockMHz * 1e6 / 1024))

    def _samples2time(self, samples):
        """ Return the duration of a number of station samples. """
        return samples / (self.clockMHz * 1e6 / 1024)

class BlockSize(object):
    """ Derive Cobalt specifications given BlockConstraints. Output:

        BlockSize member | Cobalt parset key
        ---------------------------------------
        blockSize        | Cobalt.blockSize
        nrSubblocks      | Cobalt.Correlator.nrIntegrationsPerBlock
        nrBlocks         | Cobalt.Correlator.nrBlocksPerIntegration
        integrationTime  | Cobalt.Correlator.integrationTime
    """
    def __init__(self, constraints):
        self.constraints = constraints
        self.nrSubblocks = constraints.nrSubblocks()
        self.blockSize   = self._blockSize(constraints.idealBlockSize(), constraints.factor())
        self.nrBlocks    = self._nrBlocks(constraints.idealBlockSize(), self.blockSize)

        if self.nrSubblocks > 1:
            self.integrationSamples = self.blockSize / self.nrSubblocks
        else:
            self.integrationSamples = self.blockSize * self.nrBlocks

        self.integrationTime = constraints._samples2time(self.integrationSamples)

    def _nrBlocks(self, integrationSamples, blockSize):
        return max(1, int(round(integrationSamples / blockSize)))

    def _blockSize(self, integrationSamples, factor):
        bestBlockSize = None
        bestNrBlocks = None
        bestError = None

        # Create a comfortable range to search in for possible fits.
        maxFactorPerBlock = int(ceil(integrationSamples / factor)) * 2

        for factorsPerBlock in xrange(1, maxFactorPerBlock):
            blockSize = factorsPerBlock * factor;

            # Discard invalid block sizes
            if blockSize < self.constraints.minBlockSize():
                continue

            if blockSize > self.constraints.maxBlockSize():
                continue

            # Calculate the number of blocks we'd use
            nrBlocks = self._nrBlocks(integrationSamples, blockSize)

            # Calculate error for this solution
            diff = lambda a,b: max(a,b) - min(a,b)
            error = diff(integrationSamples, nrBlocks * blockSize)

            # Accept this candidate if best so far. Prefer
            # fewer blocks if candidates are (nearly) equal in their error.
            if not bestBlockSize \
            or error < bestError \
            or (error < 0.01 * integrationSamples and nrBlocks < bestNrBlocks):
                bestBlockSize = blockSize
                bestNrBlocks  = nrBlocks
                bestError     = error

        return bestBlockSize
