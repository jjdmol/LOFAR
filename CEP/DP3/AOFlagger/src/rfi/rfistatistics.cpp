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
	
	boost::mutex::scoped_lock lock(_mutex);
	if(metaData->Antenna1().id == metaData->Antenna2().id)
	{
		addChannels(_autoChannels, image, mask, metaData, segmentedMask);
		addTimesteps(_autoTimesteps, image, mask, metaData, segmentedMask);
		addAmplitudes(_autoAmplitudes, image, mask, metaData, segmentedMask);
		saveChannels(_autoChannels, "counts-channels-auto.txt");
		saveTimesteps(_autoTimesteps, "counts-timesteps-auto.txt");
		saveAmplitudes(_autoAmplitudes, "counts-amplitudes-auto.txt");
	} else {
		addChannels(_crossChannels, image, mask, metaData, segmentedMask);
		addTimesteps(_crossTimesteps, image, mask, metaData, segmentedMask);
		addAmplitudes(_crossAmplitudes, image, mask, metaData, segmentedMask);
		saveChannels(_crossChannels, "counts-channels-cross.txt");
		saveTimesteps(_crossTimesteps, "counts-timesteps-cross.txt");
		saveAmplitudes(_crossAmplitudes, "counts-amplitudes-cross.txt");
	}
	addBaselines(image, mask, metaData, segmentedMask);
	saveBaselines("counts-baselines.txt");
}

void RFIStatistics::addChannels(std::map<double, class ChannelInfo> &channels, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	for(size_t y=1;y<image->Height();++y)
	{
		long unsigned rfiCount = 0;
		long double rfiSummedAmplitude = 0.0;
		long unsigned broadbandRfiCount = 0;
		long unsigned lineRfiCount = 0;
		long double broadbandRfiAmplitude = 0.0;
		long double lineRfiAmplitude = 0.0;
		
		for(size_t x=0;x<image->Width();++x)
		{
			if(mask->Value(x, y) && std::isfinite(image->Value(x, y)))
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
		if(channels.count(metaData->Band().channels[y].frequencyHz) == 0)
		{
			ChannelInfo channel(metaData->Band().channels[y].frequencyHz);
			channel.totalCount = image->Width();
			channel.rfiCount = rfiCount;
			channel.rfiSummedAmplitude = rfiSummedAmplitude;
			channel.broadbandRfiCount = broadbandRfiCount;
			channel.lineRfiCount = lineRfiCount;
			channel.broadbandRfiAmplitude = broadbandRfiAmplitude;
			channel.lineRfiAmplitude = lineRfiAmplitude;
			channels.insert(std::pair<double, ChannelInfo>(channel.frequencyHz, channel));
		} else {
			ChannelInfo &channel = channels.find(metaData->Band().channels[y].frequencyHz)->second;
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

void RFIStatistics::addTimesteps(std::map<double, class TimestepInfo> &timesteps, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	for(size_t x=0;x<image->Width();++x)
	{
		long unsigned rfiCount = 0;
		long double rfiSummedAmplitude = 0.0;
		long unsigned broadbandRfiCount = 0;
		long unsigned lineRfiCount = 0;
		long double broadbandRfiAmplitude = 0.0;
		long double lineRfiAmplitude = 0.0;
		
		for(size_t y=1;y<image->Height();++y)
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
		if(timesteps.count(metaData->ObservationTimes()[x]) == 0)
		{
			TimestepInfo timestep(metaData->ObservationTimes()[x]);
			timestep.totalCount = image->Width();
			timestep.rfiCount = rfiCount;
			timestep.rfiSummedAmplitude = rfiSummedAmplitude;
			timestep.broadbandRfiCount = broadbandRfiCount;
			timestep.lineRfiCount = lineRfiCount;
			timestep.broadbandRfiAmplitude = broadbandRfiAmplitude;
			timestep.lineRfiAmplitude = lineRfiAmplitude;
			timesteps.insert(std::pair<double, TimestepInfo>(timestep.time, timestep));
		} else {
			TimestepInfo &timestep = timesteps.find(metaData->ObservationTimes()[x])->second;
			timestep.totalCount += image->Width();
			timestep.rfiCount += rfiCount;
			timestep.rfiSummedAmplitude += rfiSummedAmplitude;
			timestep.broadbandRfiCount += broadbandRfiCount;
			timestep.lineRfiCount += lineRfiCount;
			timestep.broadbandRfiAmplitude += broadbandRfiAmplitude;
			timestep.lineRfiAmplitude += lineRfiAmplitude;
		}
	}
}

void RFIStatistics::addAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	for(size_t y=1;y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			double amp = image->Value(x, y);
			if(std::isfinite(amp))
			{
				double centralAmp = getCentralAmplitude(amp);
				std::map<double, class AmplitudeBin>::iterator element =
					amplitudes.find(centralAmp);
				
				AmplitudeBin bin;
				if(element == amplitudes.end())
				{
					bin.centralAmplitude = centralAmp;
					bin.centralLogAmplitude = log10(centralAmp);
				} else {
					bin = element->second;
				}
				++bin.count;
				if(mask->Value(x, y))
				{
					++bin.rfiCount;
					if(segmentedImage->Value(x, y) == Morphology::BROADBAND_SEGMENT)
					{
						++bin.broadbandRfiCount;
					} else if(segmentedImage->Value(x, y) == Morphology::LINE_SEGMENT)
					{
						++bin.lineRfiCount;
					}
				}
				if(element == amplitudes.end())
					amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmp, bin));
				else
					amplitudes.find(centralAmp)->second = bin;
			}
		}
	}
}

void RFIStatistics::addBaselines(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	long unsigned rfiCount = 0;
	long double rfiSummedAmplitude = 0.0;
	long unsigned broadbandRfiCount = 0;
	long unsigned lineRfiCount = 0;
	long double broadbandRfiAmplitude = 0.0;
	long double lineRfiAmplitude = 0.0;

	for(size_t y=1;y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			if(mask->Value(x, y) && std::isfinite(image->Value(x, y)))
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
	}
	int
		a1 = metaData->Antenna1().id,
		a2 = metaData->Antenna2().id;
	if(_baselines.count(a1) == 0)
		_baselines.insert(BaselineMatrix::value_type(a1, std::map<int, BaselineInfo>() ));
	BaselineMatrix::mapped_type &row = _baselines.find(a1)->second;
	
	if(row.count(a2) == 0)
	{
		BaselineInfo baseline;
		baseline.antenna1 = a1;
		baseline.antenna2 = a2;
		baseline.antenna1Name = metaData->Antenna1().name;
		baseline.antenna2Name = metaData->Antenna2().name;
		baseline.count = image->Width() * image->Height();
		baseline.rfiCount = rfiCount;
		baseline.rfiSummedAmplitude = rfiSummedAmplitude;
		baseline.broadbandRfiCount = broadbandRfiCount;
		baseline.lineRfiCount = lineRfiCount;
		baseline.broadbandRfiAmplitude = broadbandRfiAmplitude;
		baseline.lineRfiAmplitude = lineRfiAmplitude;
		row.insert(std::pair<int, BaselineInfo>(a2, baseline));
	} else {
		BaselineInfo &baseline = row.find(a2)->second;
		baseline.count += image->Width() * image->Height();
		baseline.rfiCount += rfiCount;
		baseline.rfiSummedAmplitude += rfiSummedAmplitude;
		baseline.broadbandRfiCount += broadbandRfiCount;
		baseline.lineRfiCount += lineRfiCount;
		baseline.broadbandRfiAmplitude += broadbandRfiAmplitude;
		baseline.lineRfiAmplitude += lineRfiAmplitude;
	}
}

void RFIStatistics::saveChannels(std::map<double, class ChannelInfo> &channels, const char *filename)
{
	std::ofstream file(filename);
	file << "frequency\ttotalCount\trfiCount\trfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\n";
	for(std::map<double, class ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
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

void RFIStatistics::saveTimesteps(std::map<double, class TimestepInfo> &timesteps, const char *filename)
{
	std::ofstream file(filename);
	file << "timestep\ttotalCount\trfiCount\trfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\n";
	for(std::map<double, class TimestepInfo>::const_iterator i=timesteps.begin();i!=timesteps.end();++i)
	{
		const TimestepInfo &c = i->second;
		file
			<< c.time << "\t"
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

void RFIStatistics::saveAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, const char *filename)
{
	std::ofstream file(filename);
	file << "centr-amplitude\tlog-centr-amplitude\tcount\trfiCount\tbroadbandRfiCount\tlineRfiCount\n";
	for(std::map<double, class AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
	{
		const AmplitudeBin &a = i->second;
		file
			<< a.centralAmplitude << "\t"
			<< a.centralLogAmplitude << "\t"
			<< a.count << "\t"
			<< a.rfiCount << "\t"
			<< a.broadbandRfiCount << "\t"
			<< a.lineRfiCount << "\n";
	}
	file.close();
}

void RFIStatistics::saveBaselines(const char *filename)
{
	std::ofstream file(filename);
	file << "a1\ta2\ta1name\ta2name\tcount\trfiCount\tbroadbandRfiCount\tlineRfiCount\n";
	for(BaselineMatrix::const_iterator i=_baselines.begin();i!=_baselines.end();++i)
	{
		const std::map<int, BaselineInfo> &row = i->second;
		for(std::map<int, BaselineInfo>::const_iterator j=row.begin();j!=row.end();++j)
		{
			const BaselineInfo &b = j->second;
			file
				<< b.antenna1 << "\t"
				<< b.antenna2 << "\t"
				<< b.antenna1Name << "\t"
				<< b.antenna2Name << "\t"
				<< b.count << "\t"
				<< b.rfiCount << "\t"
				<< b.broadbandRfiCount << "\t"
				<< b.lineRfiCount << "\n";
		}
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
