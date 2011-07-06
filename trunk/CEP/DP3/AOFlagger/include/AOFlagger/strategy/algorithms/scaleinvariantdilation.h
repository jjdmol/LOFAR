
#ifndef SCALEINVARIANTDILATION_H
#define SCALEINVARIANTDILATION_H

#include <AOFlagger/msio/types.h>

class ScaleInvariantDilation
{
	public:
		static void Dilate(bool *flags, const unsigned flagsSize, num_t eta)
		{
			// The test for a sample to become flagged is \sum_{y=Y1}^{Y2-1} ( \eta - \omega_d(y) ) >= 0.
			//               /  flags[y] : 0
			// \omega_d(y) = \ !flags[y] : 1
			
			// Make an array in which flagged samples are \eta and unflagged samples are \eta-1,
			// such that we can test for \sum_{y=Y1}^{Y2-1} values[y] >= 0
			num_t *values = new num_t[flagsSize];
			for(unsigned i=0 ; i<flagsSize ; ++i)
			{
				if(flags[i])
					values[i] = eta;
				else
					values[i] = eta - 1.0;
			}
			
			// For each x, we will now search for the largest sum of sequantial values that contains x.
			// If this sum is larger then 0, this value is part of a sequence that exceeds the test.
			
			// Define W(x) = \sum_{y=0}^{x} values[y], such that the maximum sequence containing x starts
			// at the element after W(y) is minimal in the range 0 <= y < x, and ends when
			// W(y) is maximum in the range x <= y < N.
			
			// Calculate these W's and minimum prefixes
			const unsigned wSize = flagsSize+1;
			num_t *w = new num_t[wSize];
			w[0] = 0.0;
			unsigned currentMinIndex = 0;
			unsigned *minIndices = new unsigned[wSize];
			minIndices[0] = 0;
			for(unsigned i=1 ; i!=wSize ; ++i)
			{
				w[i] = w[i-1] + values[i-1];

				if(w[i] < w[currentMinIndex])
				{
					currentMinIndex = i;
				}
				minIndices[i] = currentMinIndex;
			}
			
			// Calculate the maximum suffixes
			unsigned currentMaxIndex = wSize-1;
			unsigned *maxIndices = new unsigned[wSize];
			for(unsigned i=flagsSize-1 ; i!=0 ; --i)
			{
				maxIndices[i] = currentMaxIndex;
				if(w[i] > w[currentMaxIndex])
				{
					currentMaxIndex = i;
				}
			}
			maxIndices[0] = currentMaxIndex;
			
			// See if max sequence exceeds limit.
			for(unsigned i=0 ; i!=flagsSize ; ++i )
			{
				const num_t maxW = w[maxIndices[i]] - w[minIndices[i]];
				flags[i] = (maxW >= 0.0);
			}
			
			// Free our temporaries
			delete[] maxIndices;
			delete[] minIndices;
			delete[] w;
			delete[] values;
		}
		
	private:
		ScaleInvariantDilation() { }
};

#endif

