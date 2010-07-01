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
#include <fstream>
#include <cmath>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/util/plot.h>
#include <AOFlagger/rfi/morphology.h>

RFIStatistics::RFIStatistics()
{
}

RFIStatistics::~RFIStatistics()
{
}

void RFIStatistics::Add(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData)
{
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);
	SegmentedImagePtr segmentedMask = SegmentedImage::CreatePtr(mask->Width(), mask->Height());
	
	Morphology morphology;
	morphology.SegmentByLengthRatio(mask, segmentedMask);
	morphology.Classify(segmentedMask);
	
	addChannels(image, mask, metaData, segmentedMask);
	saveChannels();
}

void RFIStatistics::addChannels(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	for(size_t y=0;y<image->Height();++y)
	{
		long unsigned rfiCount = 0;
		long double rfiSummedAmplitude = 0.0;
		long unsigned broadbandRfiCount = 0;
		long unsigned lineRfiCount = 0;
		long double broadbandRfiAmplitude = 0.0;
		long double lineRfiAmplitude = 0.0;
		
		for(size_t x=0;x<image->Width();++x)
		{
			if(mask->Value(x, y))
			{
				++rfiCount;
				rfiSummedAmplitude += image->Value(x, y);
				if(segmentedImage->Value(x, y) == Morphology::BROADBAND_SEGMENT)
				{
					++broadbandRfiCount;
					broadbandRfiAmplitude += image->Value(x, y);
				} else if(segmentedImage->Value(x, y) == Morphology::LINE_SEGMENT)
				{
					++lineRfiCount;
					lineRfiAmplitude += image->Value(x, y);
				}
			}
		}
		if(_channels.count(metaData->Band().channels[y].frequencyHz) == 0)
		{
			ChannelInfo channel(metaData->Band().channels[y].frequencyHz);
			channel.totalCount = image->Width();
			channel.rfiCount = rfiCount;
			channel.rfiSummedAmplitude = rfiSummedAmplitude;
			channel.broadbandRfiCount = broadbandRfiCount;
			channel.lineRfiCount = lineRfiCount;
			channel.broadbandRfiAmplitude = broadbandRfiAmplitude;
			channel.lineRfiAmplitude = lineRfiAmplitude;
			_channels.insert(std::pair<double, ChannelInfo>(channel.frequencyHz, channel));
		} else {
			ChannelInfo &channel = _channels.find(metaData->Band().channels[y].frequencyHz)->second;
			channel.totalCount += image->Width();
			channel.rfiCount += rfiCount;
			channel.rfiSummedAmplitude += rfiSummedAmplitude;
			channel.broadbandRfiCount += broadbandRfiCount;
			channel.lineRfiCount += lineRfiCount;
			channel.broadbandRfiAmplitude += broadbandRfiAmplitude;
			channel.lineRfiAmplitude += lineRfiAmplitude;
		}
	}
}

void RFIStatistics::saveChannels()
{
	std::ofstream file("counts-channels.txt");
	file << "frequency\ttotalCount\trfiCount\trfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\n";
	for(std::map<double, class ChannelInfo>::const_iterator i=_channels.begin();i!=_channels.end();++i)
	{
		const ChannelInfo &c = i->second;
		file
			<< c.frequencyHz << "\t"
			<< c.totalCount << "\t"
			<< c.rfiCount << "\t"
			<< c.rfiSummedAmplitude << "\t"
			<< c.broadbandRfiCount << "\t"
			<< c.lineRfiCount << "\t"
			<< c.broadbandRfiAmplitude << "\t"
			<< c.lineRfiAmplitude << "\n";
	}
	file.close();
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
