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

#ifndef NOISESTATIMAGESET_H
#define NOISESTATIMAGESET_H

#include <string>
#include <fstream>
#include <set>
#include <map>
#include <cmath>

#include <AOFlagger/msio/types.h>

#include <AOFlagger/strategy/algorithms/noisestatistics.h>
#include <AOFlagger/strategy/algorithms/noisestatisticscollector.h>

#include <AOFlagger/strategy/imagesets/singleimageset.h>

#include <AOFlagger/util/aologger.h>

namespace rfiStrategy {

	class NoiseStatImageSet : public SingleImageSet {
		public:
			enum Mode { Mean, StdDev, Variance, VarianceOfVariance };
			
			NoiseStatImageSet(const std::string &path) : SingleImageSet(), _path(path), _mode(StdDev)
			{
			}

			virtual ImageSet *Copy()
			{
				return new NoiseStatImageSet(_path);
			}

			virtual void Initialize()
			{
			}

			virtual std::string Name()
			{
				return "Noise statistics";
			}
			
			virtual std::string File()
			{
				return _path;
			}
			
			void SetMode(enum Mode mode)
			{
				_mode = mode;
			}
			
			enum Mode Mode() const
			{
				return _mode;
			}
			
			virtual BaselineData *Read()
			{
				NoiseStatisticsCollector collector;
				collector.ReadTF(_path);
				const NoiseStatisticsCollector::StatTFMap &map = collector.TBMap();
				
				// Scan map for width (time steps) and height (frequency channels)
				std::set<double> times, frequencies;
				for(NoiseStatisticsCollector::StatTFMap::const_iterator i=map.begin();
				i!=map.end();++i)
				{
					times.insert(i->first.first);
					frequencies.insert(i->first.second);
				}
				
				std::map<double, unsigned> timeIndices, frequencyIndices;
				std::vector<double> observationTimes;
				BandInfo bandInfo;
				
				if(times.size() > 0)
				{
					// Calculate average time step
					double avgTimeDistance = 0.0;
					double lastValue = *times.begin();
					std::set<double>::const_iterator i=times.begin();
					++i;
					while(i!=times.end())
					{
						avgTimeDistance += (*i) - lastValue;
						lastValue = *i;
						++i;
					}
					avgTimeDistance /= times.size()-1;
						AOLogger::Debug << "Average time step size: " << avgTimeDistance << '\n';

					// Create the times map
					unsigned index = 0, skippedIndices = 0;
					
					i=times.begin();
					lastValue = *i - avgTimeDistance;
					++i;
					while(i!=times.end())
					{
						if(fabs((*i) - lastValue - avgTimeDistance) < avgTimeDistance*9.0/10.0)
						{
							timeIndices.insert(std::pair<double, unsigned>(*i, index));
							observationTimes.push_back(*i);
							++index;
						} else {
							++skippedIndices;
							AOLogger::Debug << "Skipped index " << index << ", deviation=" << fabs((*i) - lastValue - avgTimeDistance) << '\n';
						}
						lastValue = *i;
						++i;
					}
					AOLogger::Debug << "Number of time indices skipped: " << skippedIndices << '\n';
				}
				
				unsigned width = timeIndices.size(), height = frequencies.size();
				AOLogger::Debug << "Image size: " << width << 'x' << height << '\n';
				Image2DPtr
					imageReal = Image2D::CreateEmptyImagePtr(width, height),
					imageImag = Image2D::CreateEmptyImagePtr(width, height);
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<true>(width, height);
				unsigned index = 0;
				
				for(std::set<double>::const_iterator i=frequencies.begin();i!=frequencies.end();++i)
				{
					frequencyIndices.insert(std::pair<double, unsigned>(*i, index));
					ChannelInfo channel;
					channel.frequencyHz = *i;
					channel.frequencyIndex = index;
					bandInfo.channels.push_back(channel);
					++index;
				}
				bandInfo.channelCount = bandInfo.channels.size();
				bandInfo.windowIndex = 0;
				
				// Rescan map and fill image
				for(NoiseStatisticsCollector::StatTFMap::const_iterator i=map.begin();
				i!=map.end();++i)
				{
					const double
						time = i->first.first,
						centralFrequency = i->first.second;
					const CNoiseStatistics
						&stats = i->second;
					std::map<double, unsigned>::iterator timeElement = timeIndices.upper_bound(time);
					if(timeElement != timeIndices.begin())
						--timeElement;
					const unsigned
						x = timeElement->second,
						y = frequencyIndices.find(centralFrequency)->second;
					
					if(stats.real.Count() > 25 && stats.imaginary.Count() > 25)
					{
						mask->SetValue(x, y, false);
					}
					switch(_mode)
					{
						case Mean:
							imageReal->SetValue(x, y, stats.real.Mean());
							imageImag->SetValue(x, y, stats.imaginary.Mean());
							break;
						case StdDev:
							imageReal->SetValue(x, y, stats.real.StdDevEstimator());
							imageImag->SetValue(x, y, stats.imaginary.StdDevEstimator());
							break;
						case Variance:
							imageReal->SetValue(x, y, stats.real.VarianceEstimator());
							imageImag->SetValue(x, y, stats.imaginary.VarianceEstimator());
							break;
						case VarianceOfVariance:
							imageReal->SetValue(x, y, stats.real.VarianceOfVarianceEstimator());
							imageImag->SetValue(x, y, stats.imaginary.VarianceOfVarianceEstimator());
							break;
					}
				}
				
				// Fill metadata
				TimeFrequencyMetaDataPtr metaData = TimeFrequencyMetaDataPtr(new TimeFrequencyMetaData());
				metaData->SetObservationTimes(observationTimes);
				metaData->SetBand(bandInfo);
				switch(_mode)
				{
					case Mean:
						metaData->SetDataDescription("Mean visibility difference");
						metaData->SetDataUnits("Jy");
						break;
					case StdDev:
						metaData->SetDataDescription("Stddev of visibility difference");
						metaData->SetDataUnits("Jy");
						break;
					case Variance:
						metaData->SetDataDescription("Variance of visibility difference");
						metaData->SetDataUnits("Jy^2");
						break;
					case VarianceOfVariance:
						metaData->SetDataDescription("Variance of visibility difference");
						metaData->SetDataUnits("Jy^4");
						break;
				}
				
				// Return it structured.
				TimeFrequencyData data(TimeFrequencyData::ComplexRepresentation, StokesIPolarisation, imageReal, imageImag);
				data.SetGlobalMask(mask);
				BaselineData *baselineData = new BaselineData(data, metaData);
				return baselineData;
			}
		private:
			std::string _path;
			enum Mode _mode;
	};

}

#endif
