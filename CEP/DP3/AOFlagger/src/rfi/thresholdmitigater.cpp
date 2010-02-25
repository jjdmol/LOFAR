/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/rfi/thresholdmitigater.h>
#include <AOFlagger/rfi/thresholdtools.h>

void ThresholdMitigater::Threshold(Image2D &image, num_t threshold) throw()
{
	for(size_t y=0;y<image.Height();++y) {
		for(size_t x=0;x<image.Width();++x) {
			if(image.Value(x, y) > threshold)
				image.SetValue(x, y, 1.0);
			else
				image.SetValue(x, y, 0.0);
		}
	}
}

void ThresholdMitigater::HorizontalSumThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold) throw()
{
	if(length <= input->Width())
	{
		size_t width = input->Width()-length+1; 
		for(size_t y=0;y<input->Height();++y) {
			for(size_t x=0;x<width;++x) {
				num_t sum = 0.0;
				size_t count = 0;
				for(size_t i=0;i<length;++i) {
					if(!mask->Value(x+i, y)) {
						sum += input->Value(x+i, y);
						count++;
					}
				}
				if(count>0 && fabs(sum/count) > threshold) {
					for(size_t i=0;i<length;++i)
						mask->SetValue(x + i, y, true);
				}
			}
		}
	}
}

void ThresholdMitigater::VerticalSumThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold) throw()
{
	if(length <= input->Height())
	{
		size_t height = input->Height()-length+1; 
		for(size_t y=0;y<height;++y) {
			for(size_t x=0;x<input->Width();++x) {
				num_t sum = 0.0;
				size_t count = 0;
				for(size_t i=0;i<length;++i) {
					if(!mask->Value(x, y+i)) {
						sum += input->Value(x, y + i);
						count++;
					}
				}
				if(count>0 && fabs(sum/count) > threshold) {
					for(size_t i=0;i<length;++i)
					mask->SetValue(x, y + i, true);
				}
			}
		}
	}
}

template<size_t Length>
void ThresholdMitigater::HorizontalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, num_t threshold) throw()
{
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);
	const size_t width = mask->Width(), height = mask->Height();
	if(Length <= width)
	{
		for(size_t y=0;y<height;++y)
		{
			num_t sum = 0.0;
			size_t count = 0, xLeft, xRight;

			for(xRight=0;xRight<Length-1;++xRight)
			{
				if(!mask->Value(xRight, y))
				{
					sum += input->Value(xRight, y);
					count++;
				}
			}

			xLeft = 0;
			while(xRight < width)
			{
				// add the sample at the right
				if(!mask->Value(xRight, y))
				{
					sum += input->Value(xRight, y);
					++count;
				}
				// Check
				if(count>0 && fabs(sum/count) > threshold)
				{
					for(size_t i=0;i<Length;++i)
						maskCopy->SetValue(xLeft + i, y, true);
				}
				// subtract the sample at the left
				if(!mask->Value(xLeft, y))
				{
					sum -= input->Value(xLeft, y);
					--count;
				}
				++xLeft;
				++xRight;
			}
		}
	}
	(*mask) = maskCopy;
}

template<size_t Length>
void ThresholdMitigater::VerticalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, num_t threshold) throw()
{
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);
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
	(*mask) = maskCopy;
}

void ThresholdMitigater::SumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold) throw()
{
	switch(length)
	{
		case 1: SumThreshold(input, mask, 1, threshold); break;
		case 2: SumThreshold(input, mask, 2, threshold); break;
		case 4: SumThresholdLarge<4>(input, mask, threshold); break;
		case 8: SumThresholdLarge<8>(input, mask, threshold); break;
		case 16: SumThresholdLarge<16>(input, mask, threshold); break;
		case 32: SumThresholdLarge<32>(input, mask, threshold); break;
		case 64: SumThresholdLarge<64>(input, mask, threshold); break;
		case 128: SumThresholdLarge<128>(input, mask, threshold); break;
		case 256: SumThresholdLarge<256>(input, mask, threshold); break;
		default: throw BadUsageException("Invalid value for length");
	}	
}

void ThresholdMitigater::HorizontalVarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, double threshold) throw()
{
	unsigned width = input->Width()-length+1;
	for(size_t y=0;y<input->Height();++y) {
		for(size_t x=0;x<width;++x) {
			bool flag = true;
			for(size_t i=0;i<length;++i) {
				if(input->Value(x+i, y) < threshold && input->Value(x+i, y) > -threshold) {
					flag = false;
					break;
				}
			}
			if(flag) {
				for(size_t i=0;i<length;++i)
					mask->SetValue(x + i, y, true);
			}
		}
	}
}

void ThresholdMitigater::VerticalVarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, double threshold) throw()
{
	unsigned height = input->Height()-length+1; 
	for(size_t y=0;y<height;++y) {
		for(size_t x=0;x<input->Width();++x) {
			bool flag = true;
			for(size_t i=0;i<length;++i) {
				if(input->Value(x, y+i) <= threshold && input->Value(x, y+i) >= -threshold) {
					flag = false;
					break;
				}
			}
			if(flag) {
				for(size_t i=0;i<length;++i)
					mask->SetValue(x, y + i, true);
			}
		}
	}
}

void ThresholdMitigater::VarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold) throw()
{
	HorizontalVarThreshold(input, mask, length, threshold);
	VerticalVarThreshold(input, mask, length, threshold);
}

void ThresholdMitigater::OptimalThreshold(Image2DCPtr input, Mask2DPtr mask, bool additive, num_t sensitivity) {
	long double mean, stddev;
	ThresholdTools::WinsorizedMeanAndStdDev(input, mask, mean, stddev);
	if(!additive)
		mask->SetAll<false>();
	ThresholdMitigater::SumThreshold(input, mask, 1, sensitivity * stddev);
	ThresholdMitigater::SumThreshold(input, mask, 2, sensitivity * stddev * 1.4 * 1.2);
	ThresholdMitigater::SumThreshold(input, mask, 4, sensitivity * stddev * 2.0 * 1.4);
	ThresholdMitigater::SumThreshold(input, mask, 8, sensitivity * stddev * 2.8 * 2.0);
}
