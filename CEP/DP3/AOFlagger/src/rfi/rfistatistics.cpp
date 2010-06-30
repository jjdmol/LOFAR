/***************************************************************************
 *   Copyright (C) 2008-2010 by A.R. Offringa   *
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
#include <AOFlagger/rfi/rfistatistics.h>

#include <deque>
#include <iostream>
#include <cmath>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/util/plot.h>

RFIStatistics::RFIStatistics()
{
}

RFIStatistics::~RFIStatistics()
{
}

void RFIStatistics::Add(Image2DCPtr image, Mask2DCPtr mask)
{
	size_t rfiCount = 0;
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);
}

long double RFIStatistics::FitScore(Image2DCPtr image, Image2DCPtr fit, Mask2DCPtr mask)
{
	long double summedError = 0.0L;
	unsigned count = 0;

	for(unsigned y=0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width();++x) {
			if(!mask->Value(x, y) && std::isfinite(image->Value(x, y)))
			{
				long double error = image->Value(x, y) - fit->Value(x, y);
				summedError += error * error;
				++count;
			} else {
			}
		}
	}
	long double procentData = (long double) count / (image->Width() * image->Height());
	long double averageError = summedError / (image->Width() * image->Height());
	//long double avgError = summedError / (image->Width()*image->Height());
	//return 1.0L/(summedError + avgError * 2.0L * (long double) count);
	return procentData/averageError;
}

num_t RFIStatistics::DataQuality(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned startX, unsigned endX)
{
	unsigned count = 0;
	double sum = 0;
	for(unsigned y=0;y<image->Height();++y)
	{
		for(unsigned x=startX;x<endX;++x)
		{
			if(!mask->Value(x, y) && std::isfinite(image->Value(x, y)) && std::isfinite(model->Value(x,y)))
			{
				num_t noise = fabsn(image->Value(x, y) - model->Value(x, y));
				num_t signal = fabsn(model->Value(x, y));
				if(signal != 0.0)
				{
					if(noise <= 1e-50) noise = 1e-50;
					num_t snr = logn(signal / noise);
					sum += snr;

					++count;
				}
			}
		}
	}
	if(count == 0)
		return 0;
	else
		return sum / (sqrtn(count) * sqrtn((endX-startX) * image->Height()));
}
num_t RFIStatistics::FrequencySNR(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned channel)
{
	num_t sum = 0.0;
	size_t count = 0;
	for(unsigned x=0;x<image->Width();++x)
	{
		if(!mask->Value(x, channel))
		{
			num_t noise = fabsn(image->Value(x, channel) - model->Value(x, channel));
			num_t signal = fabsn(model->Value(x, channel));
			if(std::isfinite(signal) && std::isfinite(noise))
			{
				if(noise <= 1e-50) noise = 1e-50;
				num_t snr = logn(signal / noise);
				sum += snr;
	
				++count;
			}
		}
	}
	return expn(sum / count);
}
