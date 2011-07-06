
#ifndef SCALEINVARIANTDILATION_H
#define SCALEINVARIANTDILATION_H

#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/types.h>

/**
 * This class contains functions that implement an algorithm to dilate
 * a flag mask. The amount of dilation is relative to the size of the flagged
 * areas in the input, hence it is scale invariant. This behaviour is very
 * effective for application after amplitude based RFI detection and is part
 * of the default LOFAR flagging pipeline.
 * 
 * The rule for this scale invariant dilation is as follows:
 * Consider the sequence w(y) of size N, where w(y) = 0 if sample y is flagged and w(y) = 1
 * otherwise. If there exists a subsequence within w that
 * includes y and that has a flagged ratio of η or more, y will be flagged.
 * 
 * Thus:
 * if Y1 and Y2 exists, such that \sum_{y=Y1}^{Y2-1} w(y)  <  η (Y2 - Y1), flag y.
 * 
 * The algorithm will be applied both in time and in frequency direction, thus w(y) can
 * contain a slice through the time-frequency image in either directions.
 * 
 * If you want to understand the algorithm better, read Offringa et al. 2010 (PoS RFI2010),
 * or the Dilate(bool *, const unsigned, num_t) function, which is the proof of concept
 * O(N) algorithm and includes a lot of comments.
 * 
 * Thanks to Jasper van de Gronde for the idea of an O(N) algorithm.
 * 
 * @author A.R. Offringa
 */
class ScaleInvariantDilation
{
	public:
		/**
		 */
		static void Dilate(bool *flags, const unsigned flagsSize, num_t eta)
		{
			// The test for a sample to become flagged can be rewritten as
			//         \sum_{y=Y1}^{Y2-1} ( η - w(y) ) >= 0.
			// With w(y) =      flags[y] : 0
			//                 !flags[y] : 1
			
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
		
		static void DilateHorizontally(Mask2DPtr mask, num_t eta)
		{
			const unsigned
				width = mask->Width(),
				wSize = width+1;
			num_t
				*values = new num_t[width],
				*w = new num_t[wSize];
			unsigned
				*minIndices = new unsigned[wSize],
				*maxIndices = new unsigned[wSize];
			
			for(unsigned row=0;row<mask->Height();++row)
			{
				for(unsigned i=0 ; i<width ; ++i)
				{
					if(mask->Value(i, row))
						values[i] = eta;
					else
						values[i] = eta - 1.0;
				}
				
				w[0] = 0.0;
				unsigned currentMinIndex = 0;
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
				for(unsigned i=width-1 ; i!=0 ; --i)
				{
					maxIndices[i] = currentMaxIndex;
					if(w[i] > w[currentMaxIndex])
					{
						currentMaxIndex = i;
					}
				}
				maxIndices[0] = currentMaxIndex;
				
				// See if max sequence exceeds limit.
				for(unsigned i=0 ; i!=width ; ++i )
				{
					const num_t maxW = w[maxIndices[i]] - w[minIndices[i]];
					mask->SetValue(i, row, (maxW >= 0.0));
				}
			}
			// Free our temporaries
			delete[] maxIndices;
			delete[] minIndices;
			delete[] w;
			delete[] values;
		}
		
		static void DilateVertically(Mask2DPtr mask, num_t eta)
		{
			const unsigned
				height = mask->Height(),
				wSize = height+1;
			num_t
				*values = new num_t[height],
				*w = new num_t[wSize];
			unsigned
				*minIndices = new unsigned[wSize],
				*maxIndices = new unsigned[wSize];
			
			for(unsigned column=0;column<mask->Width();++column)
			{
				for(unsigned i=0 ; i<height ; ++i)
				{
					if(mask->Value(column, i))
						values[i] = eta;
					else
						values[i] = eta - 1.0;
				}
				
				w[0] = 0.0;
				unsigned currentMinIndex = 0;
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
				for(unsigned i=height-1 ; i!=0 ; --i)
				{
					maxIndices[i] = currentMaxIndex;
					if(w[i] > w[currentMaxIndex])
					{
						currentMaxIndex = i;
					}
				}
				maxIndices[0] = currentMaxIndex;
				
				// See if max sequence exceeds limit.
				for(unsigned i=0 ; i!=height ; ++i )
				{
					const num_t maxW = w[maxIndices[i]] - w[minIndices[i]];
					mask->SetValue(column, i, (maxW >= 0.0));
				}
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

