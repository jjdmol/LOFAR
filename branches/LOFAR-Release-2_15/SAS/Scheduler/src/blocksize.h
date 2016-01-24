/*
 * blocksize.h
 *
 * Author         : Jan David Mol
 * e-mail         : mol@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 29 July 2013
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/blocksize.h $
 *
 */

#ifndef COBALT_BLOCKSIZE_H_
#define COBALT_BLOCKSIZE_H_

#include <algorithm>
#include <iostream>
#include <cmath>
#include <math.h>

using namespace std;

#ifdef _WIN32
  double round(double value);

#endif

// Return the greatest common divider (GCD) of two numbers
// Return the greatest common divider (GCD) of two numbers
inline unsigned gcd(unsigned a, unsigned b)
{
  unsigned tmp;

  while (b != 0) {
    tmp = a;
    a = b;
    b = tmp % a;
  }

  return a;
}

// Return the least common multiple (LCM) of two numbers
inline unsigned lcm(unsigned a, unsigned b)
{
  if (a == 0) return 0;
  if (b == 0) return 0;

  return a / gcd(a, b) * b;
}


class BlockSize {
public:
        // The number of integrations to combine into a single block
        const size_t nrSubblocks;

private:
        // The block size must be a multiple of `factor' 
        const size_t factor;

        // The clock speed, to convert between samples and time
        const size_t clockMHz;
public:
        // The calculated block size
        const size_t blockSize;

        // The number of blocks
        const size_t nrBlocks;

        // The resulting integration time
        const double integrationTime;

	// Calculate the required factor based on observation parameters
	static size_t calcFactor(
	  bool correlatedEnabled,
	  unsigned correlatorChannelsPerSubband,
          double requestedIntegrationTime,

	  bool coherentStokesEnabled,
	  unsigned coherentChannelsPerSubband,
	  unsigned coherentTimeIntegrationFactor,

	  bool incoherentStokesEnabled,
	  unsigned incoherentChannelsPerSubband,
	  unsigned incoherentTimeIntegrationFactor
	)
	{
          /*
           * Determine common factors needed for the block Size.
           *
           * The Cobalt GPU kernels require the Cobalt.blockSize to be a multiple
           * of several values in order to:
           *   1) divide the work evenly over threads and blocks.
           *   2) prevent integration of samples from crossing blockSize boundaries.
           */

          unsigned factor = 1;

          if (correlatedEnabled) {
            // Add required multiples for the Correlator

            // 16 is number of PPF taps (FIR_Filter.cu)
            factor = lcm(factor, 16 * correlatorChannelsPerSubband);

            // each subblock needs at least 16 samples per channel (Correlator.cu)
            factor = lcm(factor, 16 * correlatorChannelsPerSubband * _nrSubblocks(requestedIntegrationTime));
          }

          if (coherentStokesEnabled) {
            // Add required multiples for the CS Beamformer

            // 16 * 64 (DelayAndBandPass.cu)
            factor = lcm(factor, 16 * 64);

            // 1024 is the maxNrThreadsPerBlock (CoherentStokesKernel.cc)
            // Note that coherenthannelsPerSubband is always a power of 2 < 1024
            factor = lcm(factor, 1024 * coherentTimeIntegrationFactor);

	    (void)coherentChannelsPerSubband;
          }

          if (incoherentStokesEnabled) {
            // Add required multiples for the IS Beamformer

            // 16 * 64 (DelayAndBandPass.cu)
            factor = lcm(factor, 16 * 64);

            // integration should fit (IncoherentStokes.cu)
            factor = lcm(factor, incoherentTimeIntegrationFactor * incoherentChannelsPerSubband);
          }

          return factor;
	}

	BlockSize(double requestedIntegrationTime, size_t factor = 12288, size_t clockMHz = 200)
        :
	  // For integration time < MINBLOCKSIZE, combine subblocks into a bigger block
	  nrSubblocks(_nrSubblocks(requestedIntegrationTime)),

          factor(factor),

          clockMHz(clockMHz),

		  // Calculate the block size (adjusted for combining subblocks
          blockSize(_blockSize(nrSubblocks * requestedIntegrationTime)),

		  // Calculate the number of blocks to integrate over
          nrBlocks(_nrBlocks(requestedIntegrationTime, blockSize)),

          integrationTime(samples2time(blockSize / nrSubblocks * nrBlocks))
        {
        }

private:
    static size_t _nrSubblocks(double integrationTime)
    {
      if (integrationTime < MINBLOCKSIZE) {
        return std::max(size_t(1), static_cast<size_t>(round(((MAXBLOCKSIZE + MINBLOCKSIZE) / 2.0) / integrationTime)));
      }

      return 1UL;
    }

	size_t _nrBlocks(double integrationTime, size_t blockSize)
	{
	  // Convert the requested integration time to #samples
	  const size_t integrationSamples = time2samples(integrationTime);
      return max(size_t(1), static_cast<size_t>(
                     round(1.0 * integrationSamples / blockSize)));
	}

	size_t _blockSize(double integrationTime)
	{
	  const size_t integrationSamples = time2samples(integrationTime);
//	  const size_t minSamples = time2samples(MINBLOCKSIZE);

	  size_t bestBlockSize = 0;
	  size_t bestNrBlocks  = 0;
	  size_t bestDiff(0);

	  for (size_t factorsPerBlock = 1; true; ++factorsPerBlock) {
	    const size_t blockSize = factorsPerBlock * factor;

	    // Discard invalid block sizes
	    if (blockSize < time2samples(MINBLOCKSIZE))
	      continue;
	  
            // blockSize only increases, so we can stop once
            // blocks become too big  
	    if (blockSize > time2samples(MAXBLOCKSIZE))
	      break;

	    // Calculate the number of blocks we'd use
	    const size_t n = _nrBlocks(integrationTime, blockSize);

	    // Calculate the error w.r.t. the requested integration time
	    const size_t diff = udiff(integrationSamples, n * blockSize); 

            //cout << n << " * " << blockSize << " -> error = " << diff << endl;

	    /*
	     * We prefer this configuration, if either:
	     * 1. it is the first viable configuration
	     * 2. it results in a smaller total error in integration time
	     * 3. it results in a <1% error, and uses fewer blocks
	     */
	    if (bestBlockSize == 0
	     || diff <= bestDiff
	     || (diff < 0.01 * integrationSamples && n < bestNrBlocks)) {
	      // beaten our record
	      bestBlockSize = blockSize;
              bestNrBlocks = n;
	      bestDiff = diff;
	    }
	  }

	  // Return the best configuration, or default to the 
	  // minimal block size to cover very small integration
	  // times.
	  return bestBlockSize;
	}

	// The blocks have a minimum size, below which the overhead
	// per block becomes too unwieldy.
    static const double MINBLOCKSIZE; // = 0.6;

	// The maximum block size is given by the amount of data that
	// still fits into one GPU.
    static const double MAXBLOCKSIZE; //  = 1.3;

	size_t time2samples(double t) const {
	  return static_cast<size_t>(t * clockMHz * 1e6 / 1024);
	}

	double samples2time(size_t n) const {
	  return n / (clockMHz * 1e6 / 1024);
	}

	static size_t udiff(size_t a, size_t b) {
	  return a >= b ? a - b : b - a;
	}
};

/*
int main()
{
  // test various configurations
  for(double i = 0.0; i < 30.0; i += (i < 10 ? 0.05 : 0.5)) {
    size_t factor = BlockSize::calcFactor(
      true, 64, i,
      false, 0, 0,
      false, 0, 0);

    BlockSize bs(i, factor);

    cout << "A request of " << i << " s becomes " << bs.integrationTime << " s (block size " << bs.blockSize << ", nr blocks " << bs.nrBlocks << ", nr subblocks " << bs.nrSubblocks << ")" << endl;
  }

  // test stability
  double intTime = 1.5;
  for (size_t i = 0; i < 10; ++i) {
    BlockSize bs(intTime, 16 * 64);
    cout << intTime << " -> " << bs.integrationTime << " (" << bs.nrBlocks << " * " << bs.blockSize << ")" << endl;
    intTime = bs.integrationTime;
  }
}
*/


#endif // COBALT_BLOCKSIZE_H_
