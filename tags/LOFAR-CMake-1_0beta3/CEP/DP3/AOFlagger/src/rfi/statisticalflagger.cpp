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
#include <AOFlagger/rfi/statisticalflagger.h>

StatisticalFlagger::StatisticalFlagger()
{
}


StatisticalFlagger::~StatisticalFlagger()
{
}

bool StatisticalFlagger::SquareContainsFlag(Mask2DCPtr mask, size_t xLeft, size_t yTop, size_t xRight, size_t yBottom)
{
	for(size_t y=yTop;y<=yBottom;++y)
	{
		for(size_t x=xLeft;x<=xRight;++x)
		{
			if(mask->Value(x, y))
				return true;
		}
	}
	return false;
}

void StatisticalFlagger::EnlargeFlags(Mask2DPtr mask, size_t timeSize, size_t frequencySize)
{
	Mask2DCPtr old = Mask2D::CreateCopy(mask);
	for(size_t y=0;y<mask->Height();++y)
	{
		size_t top, bottom;
		if(y > frequencySize)
			top = y - frequencySize;
		else
			top = 0;
		if(y + frequencySize < mask->Height() - 1)
			bottom = y + frequencySize;
		else
			bottom = mask->Height() - 1;
		
		for(size_t x=0;x<mask->Width();++x)
		{
			size_t left, right;
			if(x > timeSize)
				left = x - timeSize;
			else
				left = 0;
			if(x + timeSize < mask->Width() - 1)
				right = x + timeSize;
			else
				right = mask->Width() - 1;
			
			if(SquareContainsFlag(old, left, top, right, bottom))
				mask->SetValue(x, y, true);
		}
	}
}

void StatisticalFlagger::LineRemover(Mask2DPtr mask, size_t maxTimeContamination, size_t maxFreqContamination)
{
	for(size_t x=0;x<mask->Width();++x)
	{
		size_t count = 0;
		for(size_t y=0;y<mask->Height();++y)
		{
			if(mask->Value(x,y))
				++count;
		}
		if(count > maxFreqContamination)
			FlagTime(mask, x);
	}

	for(size_t y=0;y<mask->Height();++y)
	{
		size_t count = 0;
		for(size_t x=0;x<mask->Width();++x)
		{
			if(mask->Value(x,y))
				++count;
		}
		if(count > maxTimeContamination)
			FlagFrequency(mask, y);
	}
}

void StatisticalFlagger::FlagTime(Mask2DPtr mask, size_t x)
{
	for(size_t y=0;y<mask->Height();++y)
	{
		mask->SetValue(x, y, true);
	}
}

void StatisticalFlagger::FlagFrequency(Mask2DPtr mask, size_t y)
{
	for(size_t x=0;x<mask->Width();++x)
	{
		mask->SetValue(x, y, true);
	}
}

void StatisticalFlagger::MaskToInts(Mask2DCPtr mask, int **maskAsInt)
{
	for(size_t y=0;y<mask->Height();++y)
	{
		int *column = maskAsInt[y];
		for(size_t x=0;x<mask->Width();++x)
		{
			column[x] = mask->Value(x, y) ? 1 : 0;
		}
	}
}

void StatisticalFlagger::SumToLeft(Mask2DCPtr mask, int **sums, size_t width, size_t step, bool reverse)
{
	if(reverse)
	{
		for(size_t y=0;y<mask->Height();++y)
		{
			int *column = sums[y];
			for(size_t x=width;x<mask->Width();++x)
			{
				if(mask->Value(x - width + 1, y))
					column[x] += step;
			}
		}
	} else {
		for(size_t y=0;y<mask->Height();++y)
		{
			int *column = sums[y];
			for(size_t x=0;x<mask->Width() - width;++x)
			{
				if(mask->Value(x + width - 1, y))
					column[x] += step;
			}
		}
	}
}

void StatisticalFlagger::SumToTop(Mask2DCPtr mask, int **sums, size_t width, size_t step, bool reverse)
{
	if(reverse)
	{
		for(size_t y=width;y<mask->Height();++y)
		{
			int *column = sums[y];
			for(size_t x=0;x<mask->Width();++x)
			{
				if(mask->Value(x, y - width + 1))
					column[x] += step;
			}
		}
	} else {
		for(size_t y=0;y<mask->Height() - width;++y)
		{
			int *column = sums[y];
			for(size_t x=0;x<mask->Width();++x)
			{
				if(mask->Value(x, y + width - 1))
					column[x] += step;
			}
		}
	}
}

void StatisticalFlagger::ThresholdTime(Mask2DPtr mask, int **sums, int thresholdLevel, int width)
{
	int halfWidthL = width / 2;
	int halfWidthR = width / 2 + width % 2;
	for(size_t y=0;y<mask->Height();++y)
	{
		int *column = sums[y];
		for(size_t x=halfWidthL;x<mask->Width() - halfWidthR;++x)
		{
			if(column[x] >= thresholdLevel)
			{
				for(int i=-halfWidthL;i<halfWidthR;++i)
					mask->SetValue(x + i, y, true);
			}
		}
	}
}

void StatisticalFlagger::ThresholdFrequency(Mask2DPtr mask, int **sums, int thresholdLevel, int width)
{
	int halfWidthT = width / 2;
	int halfWidthB = width / 2 + width % 2;
	for(size_t y=halfWidthT;y<mask->Height() - halfWidthB;++y)
	{
		int *column = sums[y];
		for(size_t x=0;x<mask->Width();++x)
		{
			if(column[x] >= thresholdLevel)
			{
				for(int i=-halfWidthT;i<halfWidthB;++i)
					mask->SetValue(x, y + i, true);
			}
		}
	}
}

void StatisticalFlagger::DensityTimeFlagger(Mask2DPtr mask, num_t minimumGoodDataRatio)
{
	num_t width = 2.0;
	size_t iterations = 0, step = 1;
	bool reverse = false;
	
	Mask2DPtr newMask = Mask2D::CreateCopy(mask);
	
	int **sums = new int*[mask->Height()];
	for(size_t y=0;y<mask->Height();++y)
		sums[y] = new int[mask->Width()];
	
	MaskToInts(mask, sums);
	
	while(width < mask->Width())
	{
		++iterations;
		SumToLeft(mask, sums, (size_t) width, step, reverse);
		ThresholdTime(newMask, sums, (int) ceil(minimumGoodDataRatio*(num_t)(width)), (size_t) width);
	
		num_t newWidth = width * 1.05;
		if((size_t) newWidth == (size_t) width)
			newWidth = width + 1.0;
		step = (size_t) (newWidth - width);
		width = newWidth;
		reverse = !reverse;
		//++width;
}

	for(size_t y=0;y<mask->Height();++y)
		delete[] sums[y];
	delete[] sums;
	
	(*mask) = newMask;
}

void StatisticalFlagger::DensityFrequencyFlagger(Mask2DPtr mask, num_t minimumGoodDataRatio)
{
	num_t width = 2.0;
	size_t iterations = 0, step = 1;
	bool reverse = false;
	
	Mask2DPtr newMask = Mask2D::CreateCopy(mask);
	
	int **sums = new int*[mask->Height()];
	for(size_t y=0;y<mask->Height();++y)
		sums[y] = new int[mask->Width()];
	
	MaskToInts(mask, sums);
	
	while(width < mask->Height())
	{
		++iterations;
		SumToTop(mask, sums, (size_t) width, step, reverse);
		ThresholdFrequency(newMask, sums, (int) ceil(minimumGoodDataRatio*(num_t)(width)), (size_t) width);
	
		num_t newWidth = width * 1.05;
		if((size_t) newWidth == (size_t) width)
			newWidth = width + 1.0;
		step = (size_t) (newWidth - width);
		width = newWidth;
		reverse = !reverse;
		//++width;
}

	for(size_t y=0;y<mask->Height();++y)
		delete[] sums[y];
	delete[] sums;
	
	(*mask) = newMask;
}

