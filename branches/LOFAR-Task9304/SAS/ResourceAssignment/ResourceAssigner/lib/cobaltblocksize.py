from fractions import gcd
from math import ceil

def lcm(a, b):
    """ Return the Least Common Multiple of a and b. """
    return abs(a * b) / gcd(a,b) if a and b else 0

def time2samples(t, clockMHz):
    return int(round(t * clockMHz * 1e6 / 1024))

def samples2time(s, clockMHz):
    return s / (clockMHz * 1e6 / 1024)

# Block size below which the overhead per block becomes unwieldy
MINBLOCKSIZE = int(round(time2samples(0.6, 200)))

# Block size above which the data does not fit on the GPU
MAXBLOCKSIZE = int(round(time2samples(1.3, 200)))

def nrSubblocks(integrationSamples):
    if integrationSamples < MINBLOCKSIZE:
        return max(1, int(round((MAXBLOCKSIZE + MINBLOCKSIZE) / 2.0 / integrationSamples)))

    return 1

def factor(correlator, coherentStokes, incoherentStokes):
    """
      Determine common factors needed for the block Size.

      The Cobalt GPU kernels require the Cobalt.blockSize to be a multiple
      of several values in order to:
         1) divide the work evenly over threads and blocks.
         2) prevent integration of samples from crossing blockSize boundaries.
    """

    factor = 1

    # Process correlator settings
    if correlator:
        # FIR_Filter.cu (16 = nr of PPF taps)
        factor = lcm(factor, 16 * correlator["nrChannelsPerSubband"])

        # Correlator.cu (minimum of 16 samples per channel)
        factor = lcm(factor, 16 * correlator["nrChannelsPerSubband"] * nrSubblocks(time2samples(correlator["integrationTime"], 200)))

    if coherentStokes:
        # DelayAndBandPass.cu
        factor = lcm(factor, 16 * 64)

        # CoherentStokesKernel.cc (1024 = maxNrThreadsPerBlock)
        factor = lcm(factor, 1024 * coherentStokes["timeIntegrationFactor"])

        # CoherentStokes.cu (integration should fit)
        factor = lcm(factor, 1024 * coherentStokes["timeIntegrationFactor"] * coherentStokes["nrChannelsPerSubband"])

    if incoherentStokes:
        # DelayAndBandPass.cu
        factor = lcm(factor, 16 * 64)

        # IncoherentStokes.cu (integration should fit)
        factor = lcm(factor, 1024 * incoherentStokes["timeIntegrationFactor"] * incoherentStokes["nrChannelsPerSubband"])

    return factor

class BlockSize(object):
    def __init__(self, factor, integrationSamples):
        self.nrSubblocks = nrSubblocks(integrationSamples)
        self.blockSize   = self._blockSize(self.nrSubblocks * integrationSamples, factor)
        self.nrBlocks    = self._nrBlocks(integrationSamples, self.blockSize)

        if self.nrSubblocks > 1:
            self.integrationSamples = self.blockSize / self.nrSubblocks
        else:
            self.integrationSamples = self.blockSize * self.nrBlocks

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
            if blockSize < MINBLOCKSIZE:
                continue

            if blockSize > MAXBLOCKSIZE:
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

if __name__ == "__main__":
    def inttime_generator():
        i = 0.05
        while i < 10.0:
            yield i
            i += 0.05
        while i < 30.0:
            yield i
            i += 0.5

    for i in inttime_generator():
        correlator = { "nrChannelsPerSubband": 64, "integrationTime": i }
        f = factor( correlator, None, None )

        bs = BlockSize(f, time2samples(i, 200))

        print "A request of",i," s becomes",samples2time(bs.integrationSamples, 200),"s (block size",bs.blockSize,", nr blocks",bs.nrBlocks,", nr subblocks",bs.nrSubblocks,"factor",f,")"


