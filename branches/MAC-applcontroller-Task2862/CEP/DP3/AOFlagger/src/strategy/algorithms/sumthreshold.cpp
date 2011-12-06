#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/strategy/algorithms/thresholdmitigater.h>
#include <AOFlagger/util/aologger.h>

/**
 * The SSE version of the Vertical SumThreshold algorithm using intrinsics.
 *
 * The SumThreshold algorithm is applied on 4 time steps at a time. Since the SSE has
 * instructions that operate on 4 floats at a time, this could in theory speed up the
 * processing with a factor of 4. However, a lot of time is lost shuffling the data in the
 * right order/registers, and since a timestep consists of 1 byte booleans and 4 byte
 * floats, there's a penalty. Finally, since the 4 timesteps have to be processed exactly
 * the same way, conditional branches had to be replaced by conditional moves.
 *
 * The average profit of SSE intrinsics vs no SSE seems to be about a factor of 1.5 to 2-3,
 * depending on the Length parameter, but also depending on the number of flags (With
 * Length=256, it can make a factor of 3 difference). It might also vary on different
 * processors; e.g. on my Desktop Xeon with older gcc, the profit was less pronounced,
 * while my Intel i5 at home showed an avg factor of over 2 difference.
 *
 * The algorithm works with Length=1, but since that is a normal thresholding operation,
 * computing a sumthreshold has a lot of overhead, hence is not optimal at that size.
 */
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
				
				// ** Check sum **
				
				// if sum/count > threshold || sum/count < -threshold
				__m128 count4AsSingle = _mm_cvtepi32_ps(count4);
				const unsigned flagConditions =
					_mm_movemask_ps(_mm_cmpgt_ps(_mm_div_ps(sum4, count4AsSingle), threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(_mm_div_ps(sum4, count4AsSingle), threshold4Neg));
				// | _mm_movemask_ps(_mm_cmplt_ps(count4, zero4i));
				
				// The assumption is that most of the values are actually not thresholded, hence, if
				// this is the case, we circumvent the whole loop at the cost of one extra comparison:
				if(flagConditions != 0)
				{
					union
					{
						bool theChars[4];
						unsigned theInt;
					} outputValues = { {
						(flagConditions&1)!=0,
						(flagConditions&2)!=0,
						(flagConditions&4)!=0,
						(flagConditions&8)!=0 } };

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

/*template<size_t Length>
void ThresholdMitigater::HorizontalSumThresholdLargeSSE(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
{
	Mask2D *maskCopy = Mask2D::CreateCopy(*mask);
	const size_t width = mask->Width(), height = mask->Height();
	const __m128 zero4 = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
	const __m128i zero4i = _mm_set_epi32(0, 0, 0, 0);
	const __m128i ones4 = _mm_set_epi32(1, 1, 1, 1);
	const __m128 threshold4Pos = _mm_set1_ps(threshold);
	const __m128 threshold4Neg = _mm_set1_ps(-threshold); 
	if(Length <= width)
	{
		for(size_t y=0;y<width;y += 4)
		{
			__m128
				sum4A = _mm_set_ps(0.0, 0.0, 0.0, 0.0),
				sum4B = _mm_set_ps(0.0, 0.0, 0.0, 0.0),
				sum4C = _mm_set_ps(0.0, 0.0, 0.0, 0.0),
				sum4D = _mm_set_ps(0.0, 0.0, 0.0, 0.0);
			__m128i
				count4A = _mm_set_epi32(0, 0, 0, 0),
				count4B = _mm_set_epi32(0, 0, 0, 0),
				count4C = _mm_set_epi32(0, 0, 0, 0),
				count4D = _mm_set_epi32(0, 0, 0, 0);
			size_t xRight;
			
			for(xRight=0;xRight<Length-1;++xRight)
			{
				const bool
					*rowPtrA = mask->ValuePtr(xRight, y),
					*rowPtrB = mask->ValuePtr(xRight, y+1),
					*rowPtrC = mask->ValuePtr(xRight, y+2),
					*rowPtrD = mask->ValuePtr(xRight, y+3);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMaskA = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[0], rowPtrC[0], rowPtrB[0], rowPtrA[0]),
													zero4i));
				__m128 conditionMaskB = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[1], rowPtrC[1], rowPtrB[1], rowPtrA[1]),
													zero4i));
				__m128 conditionMaskC = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[2], rowPtrC[2], rowPtrB[2], rowPtrA[2]),
													zero4i));
				__m128 conditionMaskD = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[3], rowPtrC[3], rowPtrB[3], rowPtrA[3]),
													zero4i));
				
				// Conditionally increment counters
				count4A = _mm_add_epi32(count4A, _mm_and_si128(_mm_castps_si128(conditionMaskA), ones4));
				count4B = _mm_add_epi32(count4B, _mm_and_si128(_mm_castps_si128(conditionMaskB), ones4));
				count4C = _mm_add_epi32(count4C, _mm_and_si128(_mm_castps_si128(conditionMaskC), ones4));
				count4D = _mm_add_epi32(count4D, _mm_and_si128(_mm_castps_si128(conditionMaskD), ones4));
				
				// Add values with conditional move
				__m128
					a = _mm_load_ps(input->ValuePtr(xRight, y)),
					b = _mm_load_ps(input->ValuePtr(xRight, y+1)),
					c = _mm_load_ps(input->ValuePtr(xRight, y+2)),
					d = _mm_load_ps(input->ValuePtr(xRight, y+3));
					
				_MM_TRANSPOSE4_PS(a,b,c,d);
				
				sum4A = _mm_add_ps(sum4A, _mm_or_ps(_mm_and_ps(a, conditionMaskA), _mm_andnot_ps(conditionMaskA, zero4)));
				sum4B = _mm_add_ps(sum4B, _mm_or_ps(_mm_and_ps(b, conditionMaskA), _mm_andnot_ps(conditionMaskA, zero4)));
				sum4C = _mm_add_ps(sum4C, _mm_or_ps(_mm_and_ps(c, conditionMaskA), _mm_andnot_ps(conditionMaskA, zero4)));
				sum4D = _mm_add_ps(sum4D, _mm_or_ps(_mm_and_ps(d, conditionMaskA), _mm_andnot_ps(conditionMaskA, zero4)));
			}
			
			size_t xLeft = 0;
			while(xRight< width)
			{
				// ** Add the 4 sample at the right **
				
				const bool
					*rowPtrA = mask->ValuePtr(xRight, y),
					*rowPtrB = mask->ValuePtr(xRight, y+1),
					*rowPtrC = mask->ValuePtr(xRight, y+2),
					*rowPtrD = mask->ValuePtr(xRight, y+3);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				__m128 conditionMaskA = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[0], rowPtrC[0], rowPtrB[0], rowPtrA[0]),
													zero4i));
				__m128 conditionMaskB = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[1], rowPtrC[1], rowPtrB[1], rowPtrA[1]),
													zero4i));
				__m128 conditionMaskC = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[2], rowPtrC[2], rowPtrB[2], rowPtrA[2]),
													zero4i));
				__m128 conditionMaskD = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(rowPtrD[3], rowPtrC[3], rowPtrB[3], rowPtrA[3]),
													zero4i));
				
				// Conditionally increment counters
				count4A = _mm_add_epi32(count4A, _mm_and_si128(_mm_castps_si128(conditionMaskA), ones4));
				count4B = _mm_add_epi32(count4B, _mm_and_si128(_mm_castps_si128(conditionMaskB), ones4));
				count4C = _mm_add_epi32(count4C, _mm_and_si128(_mm_castps_si128(conditionMaskC), ones4));
				count4D = _mm_add_epi32(count4D, _mm_and_si128(_mm_castps_si128(conditionMaskD), ones4));
				
				// Add values with conditional move
				sum4A = _mm_add_ps(sum4A,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(xRight, y)), conditionMaskA),
										_mm_andnot_ps(conditionMaskA, zero4)));
				sum4B = _mm_add_ps(sum4B,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(xRight, y)), conditionMaskB),
										_mm_andnot_ps(conditionMaskB, zero4)));
				sum4C = _mm_add_ps(sum4C,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(xRight, y)), conditionMaskC),
										_mm_andnot_ps(conditionMaskC, zero4)));
				sum4D = _mm_add_ps(sum4D,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(xRight, y)), conditionMaskD),
										_mm_andnot_ps(conditionMaskD, zero4)));
				
				// ** Check sum **
				
				// if sum/count > threshold || sum/count < -threshold
				__m128
					count4AsSingleA = _mm_cvtepi32_ps(count4A),
					count4AsSingleB = _mm_cvtepi32_ps(count4B),
					count4AsSingleC = _mm_cvtepi32_ps(count4C),
					count4AsSingleD = _mm_cvtepi32_ps(count4D);
				const unsigned
					flagConditionsA =
					_mm_movemask_ps(_mm_cmpgt_ps(_mm_div_ps(sum4A, count4AsSingleA), threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(_mm_div_ps(sum4A, count4AsSingleA), threshold4Neg)),
					flagConditionsB =
					_mm_movemask_ps(_mm_cmpgt_ps(_mm_div_ps(sum4B, count4AsSingleB), threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(_mm_div_ps(sum4B, count4AsSingleB), threshold4Neg)),
					flagConditionsC =
					_mm_movemask_ps(_mm_cmpgt_ps(_mm_div_ps(sum4C, count4AsSingleC), threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(_mm_div_ps(sum4C, count4AsSingleC), threshold4Neg)),
					flagConditionsD =
					_mm_movemask_ps(_mm_cmpgt_ps(_mm_div_ps(sum4D, count4AsSingleD), threshold4Pos)) |
					_mm_movemask_ps(_mm_cmplt_ps(_mm_div_ps(sum4D, count4AsSingleD), threshold4Neg));
				
				if(flagConditions & 1 != 0)
					maskCopy->SetHorizontalValues(xLeft, y, true, Length);
				if(flagConditions & 2 != 0)
					maskCopy->SetHorizontalValues(xLeft, y, true, Length);
				if(flagConditions & 4 != 0)
					maskCopy->SetHorizontalValues(xLeft, y, true, Length);
				if(flagConditions & 8 != 0)
					maskCopy->SetHorizontalValues(xLeft, y, true, Length);
				
				// ** Subtract the sample at the top **
				
				// get a ptr
				const bool *tRowPtr = mask->ValuePtr(xLeft, y);
				
				// Assign each integer to one bool in the mask
				// Convert true to 0xFFFFFFFF and false to 0
				conditionMask = _mm_castsi128_ps(
					_mm_cmpeq_epi32(_mm_set_epi32(tRowPtr[3], tRowPtr[2], tRowPtr[1], tRowPtr[0]),
													zero4i));
				
				// Conditionally decrement counters
				count4 = _mm_sub_epi32(count4, _mm_and_si128(_mm_castps_si128(conditionMask), ones4));
				
				// Subtract values with conditional move
				sum4 = _mm_sub_ps(sum4,
					_mm_or_ps(_mm_and_ps(_mm_load_ps(input->ValuePtr(xLeft, y)), conditionMask),
										_mm_andnot_ps(conditionMask, zero4)));
				
				// ** Next... **
				++xLeft;
				++xRight;
			}
		}
	}
	mask->Swap(*maskCopy);
	delete maskCopy;
}*/

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
