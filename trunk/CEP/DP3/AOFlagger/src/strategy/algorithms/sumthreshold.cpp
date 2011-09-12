#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/strategy/algorithms/thresholdmitigater.h>
#include <AOFlagger/util/aologger.h>

template<size_t Length>
void ThresholdMitigater::VerticalSumThresholdLargeSSE(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	Mask2D *maskCopy = Mask2D::CreateCopy(*mask);
	const size_t width = mask->Width(), height = mask->Height();
	const __m128 zero4 = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
	const __m128i zero4i = _mm_set_epi32(0, 0, 0, 0);
	const __m128i ones4 = _mm_set_epi32(1, 1, 1, 1);
	const __m128 threshold4Pos = _mm_set1_ps(threshold);
	const __m128 threshold4Neg = _mm_set1_ps(-threshold); 
	if(Length <= height)
	{
		for(size_t x=0;x<width;x += 4)
		{
			__m128 sum4 = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
			__m128i count4 = _mm_set_epi32(0, 0, 0, 0);
			size_t yBottom;
			
			for(yBottom=0;yBottom<Length-1;++yBottom)
			{
				const bool *rowPtr = mask->ValuePtr(x, yBottom);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtr[3], rowPtr[2], rowPtr[1], rowPtr[0]),
													zero4i));
				
				// DEBUG statement
				/*if(x == 0)
				{
					unsigned f, cond;
					float sum;
					_mm_store_ss(reinterpret_cast<float*>(&f), _mm_castsi128_ps(count4));
					_mm_store_ss(reinterpret_cast<float*>(&cond), conditionMask);
					_mm_store_ss(&sum, sum4);
					std::cout << f << ' ' << sum << ' ' << cond << " .. ";
				}*/
				
				// Conditionally increment counters
				count4 = _mm_add_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Add values with conditional move
				__m128 m = _mm_and_ps(_mm_load_ps(input->ValuePtr(x, yBottom)), conditionMask);
				sum4 = _mm_add_ps(sum4, _mm_or_ps(m, _mm_andnot_ps(conditionMask, zero4)));
			}
			
			size_t yTop = 0;
			while(yBottom < height)
			{
				// ** Add the 4 sample at the bottom **
				
				// get a ptr
				const bool *rowPtr = mask->ValuePtr(x, yBottom);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtr[3], rowPtr[2], rowPtr[1], rowPtr[0]),
													_mm_set_epi32(0, 0, 0, 0)));
				
				// Conditionally increment counters
				count4 = _mm_add_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Add values with conditional move
				sum4 = _mm_add_ps(sum4,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(x, yBottom)), conditionMask),
										_mm_andnot_ps(conditionMask, zero4)));
				
				// DEBUG statement
				/*if(x == 0)
				{
					unsigned f;
					float sum;
					_mm_store_ss(reinterpret_cast<float*>(&f), _mm_castsi128_ps(count4));
					_mm_store_ss(&sum, sum4);
					std::cout << f << ' ' << sum << ' ';
				}*/

				// ** Check sum **
				
				// if sum/count > threshold || sum/count < -threshold
				__m128 count4AsSingle = _mm_cvtepi32_ps(count4);
				const unsigned flagConditions =
					_mm_movemask_ps(_mm_cmpgt_ps(_mm_div_ps(sum4, count4AsSingle), threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(_mm_div_ps(sum4, count4AsSingle), threshold4Neg));
				// | _mm_movemask_ps(_mm_cmplt_ps(count4, zero4i));
				
				union
				{
					bool theChars[4];
					unsigned theInt;
				} outputValues = { {
					(flagConditions&1)!=0,
					(flagConditions&2)!=0,
					(flagConditions&4)!=0,
					(flagConditions&8)!=0 } };
				
				// The assumption is that most of the values are actually not thresholded, hence, if
				// this is the case, we circumvent the whole loop at the cost of one extra comparison:
				if(outputValues.theInt != 0)
				{
					for(size_t i=0;i<Length;++i)
					{
						unsigned *outputPtr = reinterpret_cast<unsigned*>(maskCopy->ValuePtr(x, yTop + i));
						
						*outputPtr |= outputValues.theInt;
					}
				}
				
				// ** Subtract the sample at the top **
				
				// get a ptr
				const bool *tRowPtr = mask->ValuePtr(x, yTop);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(tRowPtr[3], tRowPtr[2], tRowPtr[1], tRowPtr[0]),
													zero4i));
				
				// Conditionally decrement counters
				count4 = _mm_sub_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Subtract values with conditional move
				sum4 = _mm_sub_ps(sum4,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(x, yTop)), conditionMask),
										_mm_andnot_ps(conditionMask, zero4)));
				
				// ** Next... **
				++yTop;
				++yBottom;
			}
		}
	}
	mask->Swap(*maskCopy);
	delete maskCopy;
}

template<size_t Length>
void ThresholdMitigater::VerticalSumThresholdLargeCompare(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	Mask2D *maskCopy = Mask2D::CreateCopy(*mask);
	const size_t width = mask->Width(), height = mask->Height();
	if(Length <= height)
	{
		for(size_t x=0;x<width;++x)
		{
			num_t sum = 0.0;
			size_t count = 0, yTop, yBottom;

			for(yBottom=0;yBottom<Length-1;++yBottom)
			{
				if(!mask->Value(x, yBottom))
				{
					sum += input->Value(x, yBottom);
					++count;
				}
			}

			yTop = 0;
			while(yBottom < height)
			{
				// add the sample at the bottom
				if(!mask->Value(x, yBottom))
				{
					sum += input->Value(x, yBottom);
					++count;
				}
				// Check
				if(count>0 && fabs(sum/count) > threshold)
				{
					for(size_t i=0;i<Length;++i)
						maskCopy->SetValue(x, yTop + i, true);
				}
				// subtract the sample at the top
				if(!mask->Value(x, yTop))
				{
					sum -= input->Value(x, yTop);
					--count;
				}
				++yTop;
				++yBottom;
			}
		}
	}
	mask->Swap(*maskCopy);
	delete maskCopy;
}

template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<1>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<2>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<4>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<8>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<16>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<32>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<64>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<128>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
template
void ThresholdMitigater::VerticalSumThresholdLargeSSE<256>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);

template
void ThresholdMitigater::VerticalSumThresholdLargeCompare<8>(Image2DCPtr input, Mask2DPtr mask, num_t threshold);