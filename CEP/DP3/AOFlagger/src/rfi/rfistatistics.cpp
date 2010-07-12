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
#include <iomanip>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/util/plot.h>
#include <AOFlagger/rfi/morphology.h>

RFIStatistics::RFIStatistics()
{
}

RFIStatistics::~RFIStatistics()
{
}

void RFIStatistics::Add(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
{
	Mask2DCPtr mask = data.GetSingleMask();
	SegmentedImagePtr segmentedMask = SegmentedImage::CreatePtr(mask->Width(), mask->Height());
	Image2DCPtr image = data.GetSingleImage();
	
	Morphology morphology;
	morphology.SegmentByLengthRatio(mask, segmentedMask);
	SegmentedImagePtr classifiedMask = SegmentedImage::CreateCopy(segmentedMask);
	morphology.Classify(classifiedMask);
	
	boost::mutex::scoped_lock lock(_mutex);
	if(metaData->Antenna1().id == metaData->Antenna2().id)
	{
		addFeatures(_autoAmplitudes, image, mask, metaData, segmentedMask);
		segmentedMask.reset();
		addChannels(_autoChannels, image, mask, metaData, classifiedMask);
		addTimesteps(_autoTimesteps, image, mask, metaData, classifiedMask);
		addAmplitudes(_autoAmplitudes, image, mask, metaData, classifiedMask);
		if(data.Polarisation() == DipolePolarisation)
			addPolarisations(_autoAmplitudes, data, metaData);
		saveChannels(_autoChannels, "counts-channels-auto.txt");
		saveTimesteps(_autoTimesteps, "counts-timesteps-auto.txt");
		saveAmplitudes(_autoAmplitudes, "counts-amplitudes-auto.txt");
	} else {
		addFeatures(_crossAmplitudes, image, mask, metaData, segmentedMask);
		segmentedMask.reset();
		addChannels(_crossChannels, image, mask, metaData, classifiedMask);
		addTimesteps(_crossTimesteps, image, mask, metaData, classifiedMask);
		addAmplitudes(_crossAmplitudes, image, mask, metaData, classifiedMask);
		if(data.Polarisation() == DipolePolarisation)
			addPolarisations(_crossAmplitudes, data, metaData);
		saveChannels(_crossChannels, "counts-channels-cross.txt");
		saveTimesteps(_crossTimesteps, "counts-timesteps-cross.txt");
		saveAmplitudes(_crossAmplitudes, "counts-amplitudes-cross.txt");
	}
	addBaselines(image, mask, metaData, classifiedMask);
	saveBaselines("counts-baselines.txt");
}

void RFIStatistics::Add(const ChannelInfo &channel, bool autocorrelation)
{
	std::map<double, ChannelInfo> *channels;
	if(autocorrelation)
		channels = &_autoChannels;
	else
		channels = &_crossChannels;
	
	std::map<double, ChannelInfo>::iterator element = channels->find(channel.frequencyHz);
	if(element == channels->end())
	{
		channels->insert(std::pair<double, ChannelInfo>(channel.frequencyHz, channel));
	} else {
		ChannelInfo &c = element->second;
		c.totalCount += channel.totalCount;
		c.totalAmplitude += channel.totalAmplitude;
		c.rfiCount += channel.rfiCount;
		c.rfiAmplitude += channel.rfiAmplitude;
		c.broadbandRfiCount += channel.broadbandRfiCount;
		c.lineRfiCount += channel.lineRfiCount;
		c.broadbandRfiAmplitude += channel.broadbandRfiAmplitude;
		c.lineRfiAmplitude += channel.lineRfiAmplitude;
	}
}

void RFIStatistics::Add(const TimestepInfo &timestep, bool autocorrelation)
{
	std::map<double, TimestepInfo> *timesteps;
	if(autocorrelation)
		timesteps = &_autoTimesteps;
	else
		timesteps = &_crossTimesteps;
	
	std::map<double, TimestepInfo>::iterator element = timesteps->find(timestep.time);
	if(element == timesteps->end())
	{
		timesteps->insert(std::pair<double, TimestepInfo>(timestep.time, timestep));
	} else {
		TimestepInfo &t = element->second;
		t.totalCount += timestep.totalCount;
		t.totalAmplitude += timestep.totalAmplitude;
		t.rfiCount += timestep.rfiCount;
		t.rfiAmplitude += timestep.rfiAmplitude;
		t.broadbandRfiCount += timestep.broadbandRfiCount;
		t.lineRfiCount += timestep.lineRfiCount;
		t.broadbandRfiAmplitude += timestep.broadbandRfiAmplitude;
		t.lineRfiAmplitude += timestep.lineRfiAmplitude;
	}
}

void RFIStatistics::Add(const AmplitudeBin &amplitudeBin, bool autocorrelation)
{
	std::map<double, AmplitudeBin> *amplitudes;
	if(autocorrelation)
		amplitudes = &_autoAmplitudes;
	else
		amplitudes = &_crossAmplitudes;
	
	std::map<double, AmplitudeBin>::iterator element = amplitudes->find(amplitudeBin.centralAmplitude);
	if(element == amplitudes->end())
	{
		amplitudes->insert(std::pair<double, AmplitudeBin>(amplitudeBin.centralAmplitude, amplitudeBin));
	} else {
		AmplitudeBin &a = element->second;
		a.count += amplitudeBin.count;
		a.rfiCount += amplitudeBin.rfiCount;
		a.broadbandRfiCount += amplitudeBin.broadbandRfiCount;
		a.lineRfiCount += amplitudeBin.lineRfiCount;
		a.featureAvgCount += amplitudeBin.featureAvgCount;
		a.featureMaxCount += amplitudeBin.featureMaxCount;
		a.featureIntCount += amplitudeBin.featureIntCount;
	}
}

void RFIStatistics::Add(const BaselineInfo &baseline)
{
	BaselineMatrix::iterator rowElement = _baselines.find(baseline.antenna1);
	if(rowElement == _baselines.end())
	{
		_baselines.insert(BaselineMatrix::value_type(baseline.antenna1, std::map<int, BaselineInfo>()));
		rowElement = _baselines.find(baseline.antenna1);
	}
	
	std::map<int, BaselineInfo> &row = rowElement->second;
	std::map<int, BaselineInfo>::iterator element = row.find(baseline.antenna2);
	if(element == row.end())
	{
		row.insert(std::pair<int, BaselineInfo>(baseline.antenna2, baseline));
	} else {
		BaselineInfo &b = element->second;
		b.count += baseline.count;
		b.totalAmplitude += baseline.totalAmplitude;
		b.rfiCount += baseline.rfiCount;
		b.rfiAmplitude += baseline.rfiAmplitude;
		b.broadbandRfiCount += baseline.broadbandRfiCount;
		b.lineRfiCount += baseline.lineRfiCount;
		b.broadbandRfiAmplitude += baseline.broadbandRfiAmplitude;
		b.lineRfiAmplitude += baseline.lineRfiAmplitude;
	}
}

void RFIStatistics::addChannels(std::map<double, class ChannelInfo> &channels, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	for(size_t y=1;y<image->Height();++y)
	{
		long unsigned count = 0;
		long double totalAmplitude = 0.0;
		long unsigned rfiCount = 0;
		long double rfiAmplitude = 0.0;
		long unsigned broadbandRfiCount = 0;
		long unsigned lineRfiCount = 0;
		long double broadbandRfiAmplitude = 0.0;
		long double lineRfiAmplitude = 0.0;
		
		for(size_t x=0;x<image->Width();++x)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				totalAmplitude += image->Value(x, y);
				++count;
				if(mask->Value(x, y))
				{
					++rfiCount;
					rfiAmplitude += image->Value(x, y);
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
		if(channels.count(metaData->Band().channels[y].frequencyHz) == 0)
		{
			ChannelInfo channel(metaData->Band().channels[y].frequencyHz);
			channel.totalCount = count;
			channel.totalAmplitude = totalAmplitude;
			channel.rfiCount = rfiCount;
			channel.rfiAmplitude = rfiAmplitude;
			channel.broadbandRfiCount = broadbandRfiCount;
			channel.lineRfiCount = lineRfiCount;
			channel.broadbandRfiAmplitude = broadbandRfiAmplitude;
			channel.lineRfiAmplitude = lineRfiAmplitude;
			channels.insert(std::pair<double, ChannelInfo>(channel.frequencyHz, channel));
		} else {
			ChannelInfo &channel = channels.find(metaData->Band().channels[y].frequencyHz)->second;
			channel.totalCount += count;
			channel.totalAmplitude += totalAmplitude;
			channel.rfiCount += rfiCount;
			channel.rfiAmplitude += rfiAmplitude;
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
		long unsigned totalCount = 0;
		long double totalAmplitude = 0.0;
		long unsigned rfiCount = 0;
		long double rfiAmplitude = 0.0;
		long unsigned broadbandRfiCount = 0;
		long unsigned lineRfiCount = 0;
		long double broadbandRfiAmplitude = 0.0;
		long double lineRfiAmplitude = 0.0;
		
		for(size_t y=1;y<image->Height();++y)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				++totalCount;
				totalAmplitude += image->Value(x, y);

				if(mask->Value(x, y))
				{
					++rfiCount;
					rfiAmplitude += image->Value(x, y);
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
		if(timesteps.count(metaData->ObservationTimes()[x]) == 0)
		{
			TimestepInfo timestep(metaData->ObservationTimes()[x]);
			timestep.totalCount = totalCount;
			timestep.totalAmplitude = totalAmplitude;
			timestep.rfiCount = rfiCount;
			timestep.rfiAmplitude = rfiAmplitude;
			timestep.broadbandRfiCount = broadbandRfiCount;
			timestep.lineRfiCount = lineRfiCount;
			timestep.broadbandRfiAmplitude = broadbandRfiAmplitude;
			timestep.lineRfiAmplitude = lineRfiAmplitude;
			timesteps.insert(std::pair<double, TimestepInfo>(timestep.time, timestep));
		} else {
			TimestepInfo &timestep = timesteps.find(metaData->ObservationTimes()[x])->second;
			timestep.totalCount += totalCount;
			timestep.totalAmplitude += totalAmplitude;
			timestep.rfiCount += rfiCount;
			timestep.rfiAmplitude += rfiAmplitude;
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

void RFIStatistics::addPolarisations(std::map<double, class AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
{
	for(size_t polIndex=0;polIndex<data.PolarisationCount();++polIndex)
	{
		TimeFrequencyData *polData = data.CreateTFDataFromPolarisationIndex(polIndex);
		Image2DCPtr image = polData->GetSingleImage();
		Mask2DCPtr mask = polData->GetSingleMask();
		delete polData;
		
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
					} else {
						bin = element->second;
					}
					switch(polIndex)
					{
						case 0: ++bin.xxCount; break;
						case 1: ++bin.xyCount; break;
						case 2: ++bin.yxCount; break;
						case 3: ++bin.yyCount; break;
					}
					if(mask->Value(x, y))
					{
						switch(polIndex)
						{
							case 0: ++bin.xxRfiCount; break;
							case 1: ++bin.xyRfiCount; break;
							case 2: ++bin.yxRfiCount; break;
							case 3: ++bin.yyRfiCount; break;
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
}

void RFIStatistics::addBaselines(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	long unsigned count = 0;
	long unsigned rfiCount = 0;
	long double totalAmplitude = 0.0;
	long double rfiAmplitude = 0.0;
	long unsigned broadbandRfiCount = 0;
	long unsigned lineRfiCount = 0;
	long double broadbandRfiAmplitude = 0.0;
	long double lineRfiAmplitude = 0.0;

	for(size_t y=1;y<image->Height();++y)
	{
		for(size_t x=0;x<image->Width();++x)
		{
			if(std::isfinite(image->Value(x, y)))
			{
				++count;
				totalAmplitude += image->Value(x, y);
				if(mask->Value(x, y))
				{
					++rfiCount;
					rfiAmplitude += image->Value(x, y);
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
	}
	int
		a1 = metaData->Antenna1().id,
		a2 = metaData->Antenna2().id;
	if(_baselines.count(a1) == 0)
		_baselines.insert(BaselineMatrix::value_type(a1, std::map<int, BaselineInfo>() ));
	BaselineMatrix::mapped_type &row = _baselines.find(a1)->second;
	
	Baseline baselineMSData(metaData->Antenna1(), metaData->Antenna2());

	if(row.count(a2) == 0)
	{
		BaselineInfo baseline;
		baseline.antenna1 = a1;
		baseline.antenna2 = a2;
		baseline.antenna1Name = metaData->Antenna1().name;
		baseline.antenna2Name = metaData->Antenna2().name;
		baseline.baselineLength = baselineMSData.Distance();
		baseline.baselineAngle = baselineMSData.Angle();

		baseline.count = count;
		baseline.totalAmplitude = totalAmplitude;
		baseline.rfiCount = rfiCount;
		baseline.rfiAmplitude = rfiAmplitude;
		baseline.broadbandRfiCount = broadbandRfiCount;
		baseline.lineRfiCount = lineRfiCount;
		baseline.broadbandRfiAmplitude = broadbandRfiAmplitude;
		baseline.lineRfiAmplitude = lineRfiAmplitude;
		row.insert(std::pair<int, BaselineInfo>(a2, baseline));
	} else {
		BaselineInfo &baseline = row.find(a2)->second;
		baseline.count += count;
		baseline.totalAmplitude += totalAmplitude;
		baseline.rfiCount += rfiCount;
		baseline.rfiAmplitude += rfiAmplitude;
		baseline.broadbandRfiCount += broadbandRfiCount;
		baseline.lineRfiCount += lineRfiCount;
		baseline.broadbandRfiAmplitude += broadbandRfiAmplitude;
		baseline.lineRfiAmplitude += lineRfiAmplitude;
	}
}

void RFIStatistics::addFeatures(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage)
{
	FeatureMap features;
	
	for(size_t y=0;y<image->Height();++y) {
		for(size_t x=0;x<image->Width();++x) {
			if(mask->Value(x, y) && std::isfinite(image->Value(x, y)))
			{
				FeatureMap::iterator i = features.find(segmentedImage->Value(x, y));
				if(i == features.end()) {
					FeatureInfo newFeature;
					newFeature.amplitudeSum = image->Value(x, y);
					newFeature.amplitudeMax = image->Value(x, y);
					newFeature.sampleCount = 1;
					features.insert(FeatureMap::value_type(segmentedImage->Value(x, y), newFeature));
				} else {
					FeatureInfo &feature = i->second;
					num_t sampleValue = image->Value(x, y);
					feature.amplitudeSum += sampleValue;
					if(sampleValue > feature.amplitudeMax)
						feature.amplitudeMax = sampleValue;
					feature.sampleCount++;
				}
			}
		}
	}
	for(FeatureMap::const_iterator i=features.begin();i!=features.end();++i)
	{
		const FeatureInfo &feature = i->second;
		double intBin = getCentralAmplitude(feature.amplitudeSum);
		double avgBin = getCentralAmplitude(feature.amplitudeSum / feature.sampleCount);
		double maxBin = getCentralAmplitude(feature.amplitudeMax);
		if(amplitudes.count(intBin) == 0)
		{
			AmplitudeBin bin;
			bin.centralAmplitude = intBin;
			bin.featureIntCount=1;
			amplitudes.insert(std::pair<double, AmplitudeBin>(intBin, bin));
		} else {
			amplitudes[intBin].featureIntCount++;
		}
		if(amplitudes.count(avgBin) == 0)
		{
			AmplitudeBin bin;
			bin.centralAmplitude = avgBin;
			bin.featureAvgCount=1;
			amplitudes.insert(std::pair<double, AmplitudeBin>(avgBin, bin));
		} else {
			amplitudes[avgBin].featureAvgCount++;
		}
		if(amplitudes.count(maxBin) == 0)
		{
			AmplitudeBin bin;
			bin.centralAmplitude = maxBin;
			bin.featureMaxCount=1;
			amplitudes.insert(std::pair<double, AmplitudeBin>(maxBin, bin));
		} else {
			amplitudes[maxBin].featureMaxCount++;
		}
	}
}

void RFIStatistics::saveChannels(std::map<double, class ChannelInfo> &channels, const char *filename)
{
	std::ofstream file(filename);
	file << "frequency\ttotalCount\trfiCount\trfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\n" << std::setprecision(14);
	for(std::map<double, class ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
	{
		const ChannelInfo &c = i->second;
		file
			<< c.frequencyHz << "\t"
			<< c.totalCount << "\t"
			<< c.totalAmplitude << "\t"
			<< c.rfiCount << "\t"
			<< c.rfiAmplitude << "\t"
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
	file << "timestep\ttotalCount\ttotalAmplitude\trfiCount\trfiAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\n" << std::setprecision(14);
	for(std::map<double, class TimestepInfo>::const_iterator i=timesteps.begin();i!=timesteps.end();++i)
	{
		const TimestepInfo &c = i->second;
		file
			<< c.time << "\t"
			<< c.totalCount << "\t"
			<< c.totalAmplitude << "\t"
			<< c.rfiCount << "\t"
			<< c.rfiAmplitude << "\t"
			<< c.broadbandRfiCount << "\t"
			<< c.lineRfiCount << "\t"
			<< c.broadbandRfiAmplitude << "\t"
			<< c.lineRfiAmplitude << "\n";
	}
	file.close();
}

void RFIStatistics::saveSubbands(std::map<double, class ChannelInfo> &channels, const char *filename)
{
	std::ofstream file(filename);
	file << "subband\ts-frequency\te-frequency\ttotalCount\ttotalAmplitude\trfiCount\trfiSummedAmplitude\tbroadbandRfiCount\tlineRfiCount\tbroadbandRfiAmplitude\tlineRfiAmplitude\n" << std::setprecision(14);
	size_t index = 0;
	double 
		bandTotal = 0.0,
		bandAmp = 0.0,
		bandRFI = 0.0,
		bandRFIAmp = 0.0,
		bandBRFI = 0.0,
		bandLRFI = 0.0,
		bandBRFIAmp = 0.0,
		bandLRFIAmp = 0.0;
	for(std::map<double, class ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
	{
		const ChannelInfo &c = i->second;
		bandTotal += c.totalCount;
		bandAmp += c.totalAmplitude;
		bandRFI += c.rfiCount;
		bandRFIAmp += c.rfiAmplitude;
		bandBRFI += c.broadbandRfiCount;
		bandLRFI += c.lineRfiCount;
		bandBRFIAmp += c.broadbandRfiAmplitude;
		bandLRFIAmp += c.lineRfiAmplitude;
		if(index%255 == 0)
			file << index/255 << '\t' << c.frequencyHz << '\t';
		else if(index%255 == 254)
		{
			file
			<< c.frequencyHz << "\t"
			<< bandTotal << "\t"
			<< bandAmp << "\t"
			<< bandRFI << "\t"
			<< bandRFIAmp << "\t"
			<< bandBRFI << "\t"
			<< bandLRFI << "\t"
			<< bandBRFIAmp << "\t"
			<< bandLRFIAmp << "\n";
			bandTotal = 0.0;
			bandAmp = 0.0;
			bandRFI = 0.0;
			bandRFIAmp = 0.0;
			bandBRFI = 0.0;
			bandLRFI = 0.0;
			bandBRFIAmp = 0.0;
			bandLRFIAmp = 0.0;
		}
		++index;
	}
	file.close();
	if(index%255 != 0)
		std::cout << "Warning: " << (index%255) << " rows were not part of a sub-band (channels were not dividable by 256)" << std::endl;
}

void RFIStatistics::saveAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, const char *filename)
{
	std::ofstream file(filename);
	file << "centr-amplitude\tlog-centr-amplitude\tcount\trfiCount\tbroadbandRfiCount\tlineRfiCount\tfeatureAvgCount\tfeatureIntCount\tfeatureMaxCount\txx\txy\tyx\tyy\txxRfi\txyRfi\tyxRfi\tyyRfi\n" << std::setprecision(14);
	for(std::map<double, class AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
	{
		const AmplitudeBin &a = i->second;
		double logAmp = a.centralAmplitude > 0.0 ? log10(a.centralAmplitude) : 0.0;
		file
			<< a.centralAmplitude << "\t"
			<< logAmp << "\t"
			<< a.count << "\t"
			<< a.rfiCount << "\t"
			<< a.broadbandRfiCount << "\t"
			<< a.lineRfiCount << "\t"
			<< a.featureAvgCount<< "\t"
			<< a.featureIntCount << "\t"
			<< a.featureMaxCount << "\t"
			<< a.xxCount << "\t"
			<< a.xyCount << "\t"
			<< a.yxCount << "\t"
			<< a.yyCount << "\t"
			<< a.xxRfiCount << "\t"
			<< a.xyRfiCount << "\t"
			<< a.yxRfiCount << "\t"
			<< a.yyRfiCount << "\n";
	}
	file.close();
}

void RFIStatistics::saveBaselines(const char *filename)
{
	std::ofstream file(filename);
	file << "a1\ta2\ta1name\ta2name\tbaselineLength\tbaselineAngle\tcount\ttotalAmplitude\trfiCount\tbroadbandRfiCount\tlineRfiCount\n" << std::setprecision(14);
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
				<< b.baselineLength << "\t"
				<< b.baselineAngle << "\t"
				<< b.count << "\t"
				<< b.totalAmplitude << "\t"
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
