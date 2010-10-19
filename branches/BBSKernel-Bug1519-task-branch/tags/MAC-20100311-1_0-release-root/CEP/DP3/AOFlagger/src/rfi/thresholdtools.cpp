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
#include <deque>

#include <AOFlagger/util/rng.h>

#include <AOFlagger/rfi/thresholdtools.h>

void ThresholdTools::MeanAndStdDev(Image2DCPtr image, Mask2DCPtr mask, long double &mean, long double &stddev)
{
	// Calculate mean
	mean = 0.0;
	unsigned count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width(); ++x) {
			if(!mask->Value(x, y) && isfinite(image->Value(x, y))) {
				num_t value = image->Value(x, y);
				mean += value;
				count++; 
			}
		}
	}
	mean /= (num_t) count;
	// Calculate variance
	stddev = 0.0;
	count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width(); ++x) {
			if(!mask->Value(x, y) && isfinite(image->Value(x, y))) {
				num_t value = image->Value(x, y);
				stddev += (value-mean)*(value-mean);
				count++; 
			}
		}
	}
	stddev = sqrt(stddev / (num_t) count);
}

void ThresholdTools::WinsorizedMeanAndStdDev(Image2DCPtr image, long double &mean, long double &stddev)
{
	size_t size = image->Width() * image->Height();
	num_t *data = new num_t[size];
	image->CopyData(data);
	std::sort(data, data + size);
	size_t lowIndex = (size_t) floor(0.1 * size);
	size_t highIndex = (size_t) ceil(0.9 * size)-1;
	num_t lowValue = data[lowIndex];
	num_t highValue = data[highIndex];
	delete[] data;

	// Calculate mean
	mean = 0.0;
	unsigned count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x = 0;x<image->Width(); ++x) {
			if(isfinite(image->Value(x, y))) {
				num_t value = image->Value(x, y);
				if(value < lowValue)
					mean += lowValue;
				else if(value > highValue)
					mean += highValue;
				else
					mean += value;
				count++; 
			}
		}
	}
	if(count > 0)
		mean /= (num_t) count;
	// Calculate variance
	stddev = 0.0;
	count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width(); ++x) {
			if(isfinite(image->Value(x, y))) {
				num_t value = image->Value(x, y);
				if(value < lowValue)
					stddev += (lowValue-mean)*(lowValue-mean);
				else if(value > highValue)
					stddev += (highValue-mean)*(highValue-mean);
				else
					stddev += (value-mean)*(value-mean);
				count++; 
			}
		}
	}
	if(count > 0)
		stddev = sqrt(1.54 * stddev / (num_t) count);
	else
		stddev = 0.0;
}

/** TODO : not completely done in the right way! */
void ThresholdTools::WinsorizedMeanAndStdDev(Image2DCPtr image, Mask2DCPtr mask, long double &mean, long double &stddev)
{
	size_t size = image->Width() * image->Height();
	num_t *data = new num_t[size];
	image->CopyData(data);
	std::sort(data, data + size);
	size_t lowIndex = (size_t) floor(0.1 * size);
	size_t highIndex = (size_t) ceil(0.9 * size)-1;
	num_t lowValue = data[lowIndex];
	num_t highValue = data[highIndex];
	delete[] data;

	// Calculate mean
	mean = 0.0;
	unsigned count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x = 0;x<image->Width(); ++x) {
			if(!mask->Value(x, y) && isfinite(image->Value(x, y))) {
				num_t value = image->Value(x, y);
				if(value < lowValue)
					mean += lowValue;
				else if(value > highValue)
					mean += highValue;
				else
					mean += value;
				count++; 
			}
		}
	}
	if(count > 0)
		mean /= (num_t) count;
	// Calculate variance
	stddev = 0.0;
	count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width(); ++x) {
			if(!mask->Value(x, y) && isfinite(image->Value(x, y))) {
				num_t value = image->Value(x, y);
				if(value < lowValue)
					stddev += (lowValue-mean)*(lowValue-mean);
				else if(value > highValue)
					stddev += (highValue-mean)*(highValue-mean);
				else
					stddev += (value-mean)*(value-mean);
				count++; 
			}
		}
	}
	if(count > 0)
		stddev = sqrt(1.54 * stddev / (num_t) count);
	else
		stddev = 0.0;
}

long double ThresholdTools::MinValue(Image2DCPtr image, Mask2DCPtr mask)
{
	long double minValue = 1e100;
	for(size_t y=0;y<image->Height();++y) {
		for(size_t x=0;x<image->Width();++x) {
			if(!mask->Value(x, y) && isfinite(image->Value(x, y)) && image->Value(x, y) < minValue)
				minValue = image->Value(x, y);
		}
	}
	return minValue;
}

long double ThresholdTools::MaxValue(Image2DCPtr image, Mask2DCPtr mask)
{
	long double maxValue = -1e100;
	for(size_t y=0;y<image->Height();++y) {
		for(size_t x=0;x<image->Width();++x) {
			if(!mask->Value(x, y) && isfinite(image->Value(x, y)) && image->Value(x, y) > maxValue)
				maxValue = image->Value(x, y);
		}
	}
	return maxValue;
}

void ThresholdTools::SetFlaggedValuesToZero(Image2DPtr dest, Mask2DCPtr mask)
{
	for(unsigned y=0;y<dest->Height();++y) {
		for(unsigned x=0;x<dest->Width();++x) {
			if(mask->Value(x, y)) dest->SetValue(x, y, 0.0);
		}
	}
}

void ThresholdTools::CountMaskLengths(Mask2DCPtr mask, int *lengths, size_t lengthsSize)
{
	for(unsigned i=0;i<lengthsSize;++i)
		lengths[i] = 0;
	int *horizontal, *vertical;
	horizontal = new int[mask->Width()*mask->Height()];
	vertical = new int[mask->Width()*mask->Height()];
	unsigned y=0, index=0;
	// Count horizontally lengths
	while(y < mask->Height()) {
		unsigned x = 0;
		while(x < mask->Width()) {
			if(mask->Value(x, y)) {
				unsigned xStart = x;
				do
				{
					++x;
					++index;
				} while(x < mask->Width() && mask->Value(x, y));
				for(unsigned i=0;i<x-xStart;++i)
					horizontal[index-(x-xStart)+i] = x - xStart;
			} else {
				horizontal[index] = 0;
				++x;
				++index;
			}
		}
		++y;
	}
	// Count vertically lengths
	unsigned x = 0;
	while(x < mask->Width()) {
		unsigned y = 0;
		while(y < mask->Height()) {
			if(mask->Value(x, y)) {
				unsigned yStart = y;
				while(y < mask->Height() && mask->Value(x, y))
				{
					++y;
				}
				for(unsigned i=yStart;i<y;++i)
					vertical[i*mask->Width()+x] = y - yStart;
			} else {
				vertical[y*mask->Width()+x] = 0;
				++y;
			}
		}
		++x;
	}
	// Count the horizontal distribution
	index = 0;
	for(unsigned y=0;y<mask->Height();++y) {
		unsigned x=0;
		while(x<mask->Width()) {
			if(horizontal[index] != 0) {
				int count = horizontal[index];
				bool dominant = false;
				for(int i=0;i<count;++i) {
					if(count >= vertical[index+i]) {
						dominant = true;
						break;
					}
				}
				if(dominant && (unsigned) count-1 < lengthsSize)
					++lengths[count-1];
				x += count;
				index += count;
			} else {
				++index;
				++x;
			}
		}
	}
	// Count the vertical distribution
	for(unsigned x=0;x<mask->Width();++x) {
		unsigned y = 0;
		while(y<mask->Height()) {
			if(vertical[y*mask->Width() + x] != 0) {
				int count = vertical[y*mask->Width() + x];
				bool dominant = false;
				for(int i=0;i<count;++i) {
					if(count >= horizontal[(y+i)*mask->Width() + x]) {
						dominant = true;
						break;
					}
				}
				if(dominant && (unsigned) count-1 < lengthsSize)
					++lengths[count-1];
				y += count;
			} else {
				++y;
			}
		}
	}
	delete[] vertical;
	delete[] horizontal;
}

void ThresholdTools::OneDimensionalConvolution(long double *data, unsigned dataSize, const long double *kernel, unsigned kernelSize)
{
	long double *tmp = new long double[dataSize]; 
	for(unsigned i=0;i<dataSize;++i)
	{
		unsigned kStart = 0;
		if(i < kernelSize / 2)
			kStart = kernelSize/2 - i;
		unsigned kEnd = kernelSize;
		if(i + kernelSize/2 > dataSize)
			kEnd = dataSize - i + kernelSize/2;
		long double sum = 0.0;
		long double weight = 0.0;
		for(unsigned k=kStart;k<kEnd;++k)
		{
			sum += data[i+k-kernelSize/2]*kernel[k];
			weight += kernel[k];
		}
		tmp[i] = sum / weight;
	}
	for(unsigned i=0;i<dataSize;++i)
		data[i] = tmp[i];
	delete[] tmp;
}

void ThresholdTools::OneDimensionalGausConvolution(long double *data, unsigned dataSize, long double variance)
{
	unsigned kernelSize = (unsigned) round(variance*3.0L);
	if(kernelSize > dataSize) kernelSize = dataSize;
	long double *kernel = new long double[kernelSize];
	for(unsigned i=0;i<kernelSize;++i)
	{
		long double x = ((long double) i-(long double) kernelSize/2.0L);
		kernel[i] = RNG::EvaluateGaussian(x, variance);
	}
	OneDimensionalConvolution(data, dataSize, kernel, kernelSize);
	delete[] kernel;
}

long double ThresholdTools::Mode(Image2DCPtr image, Mask2DCPtr mask)
{
	long double mode = 0.0;
	unsigned count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width(); ++x) {
			long double value = image->Value(x, y);
			if(!mask->Value(x, y) && isfinite(value)) {
				mode += value*value;
				count++; 
			}
		}
	}
	return sqrtl(mode / (2.0L * (long double) count));
}

long double ThresholdTools::WinsorizedMode(Image2DCPtr image, Mask2DCPtr mask)
{
	size_t size = image->Width() * image->Height();
	num_t *data = new num_t[size];
	image->CopyData(data);
	std::sort(data, data + size);
	size_t highIndex = (size_t) ceil(0.9 * size)-1;
	long double highValue = data[highIndex];
	delete[] data;

	num_t mode = 0.0;
	unsigned count = 0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x = 0;x<image->Width(); ++x) {
			num_t value = image->Value(x, y);
			if(!mask->Value(x, y) && isfinite(value)) {
				if(value > highValue)
					mode += highValue * highValue;
				else
					mode += value * value;
				count++; 
			}
		}
	}
	// The correction factor 1.0541 was found by running simulations
	// It corresponds with the correction factor needed when winsorizing 10% of the 
	// data, meaning that the highest 10% is set to the value exactly at the
	// 90%/10% limit.
	if(count > 0)
		return sqrtl(mode / (2.0L * (num_t) count)) * 1.0541L;
	else
		return 0.0;
}

long double ThresholdTools::WinsorizedMode(Image2DCPtr image)
{
	size_t size = image->Width() * image->Height();
	num_t *data = new num_t[size];
	image->CopyData(data);
	std::sort(data, data + size);
	size_t highIndex = (size_t) ceil(0.9 * size)-1;
	num_t highValue = data[highIndex];
	delete[] data;

	num_t mode = 0.0;
	for(unsigned y = 0;y<image->Height();++y) {
		for(unsigned x = 0;x<image->Width(); ++x) {
			num_t value = image->Value(x, y);
			if(value > highValue || !isfinite(value))
				mode += highValue * highValue;
			else
				mode += value * value;
		}
	}
	// The correction factor 1.0541 was found by running simulations
	// It corresponds with the correction factor needed when winsorizing 10% of the 
	// data, meaning that the highest 10% is set to the value exactly at the
	// 90%/10% limit.
	if(size > 0)
		return sqrtl(mode / (2.0L * (num_t) size)) * 1.0541L;
	else
		return 0.0;
}

void ThresholdTools::FilterConnectedSamples(Mask2DPtr mask, size_t minConnectedSampleArea, bool eightConnected)
{
	for(unsigned y=0;y<mask->Height();++y) {
		for(unsigned x=0;x<mask->Width();++x)
			if(mask->Value(x, y))
				FilterConnectedSample(mask, x, y, minConnectedSampleArea, eightConnected);
	}
}

struct ConnectedAreaCoord
{
	ConnectedAreaCoord(unsigned _x, unsigned _y) throw() { x=_x; y=_y; } 
	unsigned x, y;
};

void ThresholdTools::FilterConnectedSample(Mask2DPtr mask, unsigned x, unsigned y, size_t minConnectedSampleArea, bool eightConnected)
{
	std::deque<ConnectedAreaCoord> tosearch, changed;
	tosearch.push_back(ConnectedAreaCoord(x, y));
	size_t count = 0;

	do
	{
		ConnectedAreaCoord c = tosearch.front();
		tosearch.pop_front();
		if(mask->Value(c.x, c.y))
		{
			mask->SetValue(c.x, c.y, false);
			changed.push_back(ConnectedAreaCoord(c.x, c.y));
			if(c.x>0)
				tosearch.push_back(ConnectedAreaCoord(c.x-1, c.y));
			if(c.x<mask->Width()-1)
				tosearch.push_back(ConnectedAreaCoord(c.x+1, c.y));
			if(c.y>0)
				tosearch.push_back(ConnectedAreaCoord(c.x, c.y-1));
			if(c.y<mask->Height()-1)
				tosearch.push_back(ConnectedAreaCoord(c.x, c.y+1));
			if(eightConnected) {
			if(c.x>0 && c.y>0)
				tosearch.push_back(ConnectedAreaCoord(c.x-1, c.y-1));
			if(c.x<mask->Width()-1 && c.y<mask->Height()-1)
				tosearch.push_back(ConnectedAreaCoord(c.x+1, c.y+1));
			if(c.x<mask->Width()-1 && c.y>0)
				tosearch.push_back(ConnectedAreaCoord(c.x+1, c.y-1));
			if(c.x>0 && c.y<mask->Height()-1)
				tosearch.push_back(ConnectedAreaCoord(c.x-1, c.y+1));
			}
			++count;
		}
	}
	while(tosearch.size() != 0 && count < minConnectedSampleArea);

	if(count >= minConnectedSampleArea) {
		while(changed.size() != 0)
		{
			ConnectedAreaCoord c = changed.front();
			changed.pop_front();
			mask->SetValue(c.x, c.y, true);
		}
	}
}

void ThresholdTools::UnrollPhase(Image2DPtr image)
{
	for(unsigned y=0;y<image->Height();++y)
	{
		num_t prev = image->Value(0, y);
		for(unsigned x=1;x<image->Width();++x)
		{
			num_t val = image->Value(x, y);
			while(val - prev > M_PIl) val -= 2.0L * M_PIl;
			while(prev - val > M_PIl) val += 2.0L * M_PIl;
			image->SetValue(x, y, val);
			prev = val;
		}
	}
}
