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

#ifndef TIMEFREQUENCYSTATIMAGESET_H
#define TIMEFREQUENCYSTATIMAGESET_H

#include <string>
#include <fstream>
#include <set>
#include <map>

#include <AOFlagger/msio/types.h>

#include <AOFlagger/strategy/imagesets/singleimageset.h>
#include <AOFlagger/util/aologger.h>

namespace rfiStrategy {

	class TimeFrequencyStatImageSet : public SingleImageSet {
		public:
			TimeFrequencyStatImageSet(const std::string &path) : SingleImageSet(), _path(path)
			{
			}

			virtual ImageSet *Copy()
			{
				return new TimeFrequencyStatImageSet(_path);
			}

			virtual void Initialize()
			{
			}

			virtual std::string Name()
			{
				return "RFI time/frequency statistics";
			}
			
			virtual std::string File()
			{
				return _path;
			}
			
			virtual BaselineData *Read()
			{
				// Scan file for width (time steps) and height (frequency bands)
				std::ifstream f(_path.c_str());
				std::string headers;
				std::getline(f, headers);
				std::set<double> times, frequencies;
				while(!f.eof())
				{
					double time, centralFrequency;
					unsigned long totalCount, rfiCount;
					f >> time;
					if(f.eof()) break;
					f
					>> centralFrequency
					>> totalCount
					>> rfiCount;
					times.insert(time);
					frequencies.insert(centralFrequency);
				}
				unsigned width = times.size(), height = frequencies.size();
				AOLogger::Debug << "Image size: " << width << 'x' << height << '\n';
				Image2DPtr image = Image2D::CreateEmptyImagePtr(width, height);
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<true>(width, height);
				
				std::map<double, unsigned> timeIndices, frequencyIndices;
				std::vector<double> observationTimes;
				BandInfo bandInfo;
				
				unsigned index = 0;
				for(std::set<double>::const_iterator i=times.begin();i!=times.end();++i)
				{
					timeIndices.insert(std::pair<double, unsigned>(*i, index));
					observationTimes.push_back(*i);
					++index;
				}
				
				index = 0;
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
				
				// Rescan file, now actually store data
				f.close();
				f.open(_path.c_str());
				std::getline(f, headers);
				while(!f.eof())
				{
					double time, centralFrequency;
					unsigned long totalCount, rfiCount;
					f >> time;
					if(f.eof()) break;
					f
					>> centralFrequency
					>> totalCount
					>> rfiCount;
					
					//AOLogger::Debug << "Time: " << time << " freq: " << centralFrequency << '\n';
					unsigned
						x = timeIndices.find(time)->second,
						y = frequencyIndices.find(centralFrequency)->second;
					//AOLogger::Debug << '(' << x << ',' << y << ")\n";
					
					if(totalCount > 0)
					{
						image->SetValue(x, y, 100.0L * (long double) rfiCount / (long double) totalCount);
						mask->SetValue(x, y, false);
					}
				}
				
				// Fill metadata
				TimeFrequencyMetaDataPtr metaData = TimeFrequencyMetaDataPtr(new TimeFrequencyMetaData());
				metaData->SetObservationTimes(observationTimes);
				metaData->SetBand(bandInfo);
				
				// Return it structured.
				TimeFrequencyData data(TimeFrequencyData::AmplitudePart, StokesIPolarisation, image);
				data.SetGlobalMask(mask);
				BaselineData *baselineData = new BaselineData(data, metaData);
				return baselineData;
			}
		private:
			std::string _path;
	};

}

#endif
