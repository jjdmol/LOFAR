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

#ifndef AOREMOTE__OBSERVATION_TIMERANGE_H
#define AOREMOTE__OBSERVATION_TIMERANGE_H

#include <algorithm>
#include <stdexcept>
#include <map>
#include <vector>

#include <AOFlagger/msio/antennainfo.h>
#include <AOFlagger/msio/types.h>
#include <AOFlagger/msio/msrowdataext.h>

#include "clusteredobservation.h"

namespace aoRemote {
	
class ObservationTimerange
{
	public:
		ObservationTimerange(ClusteredObservation &observation) :
			_observation(observation),
			_bands(observation.Size()),
			_bandStartLookup(observation.Size()),
			_polarizationCount(0),
			_timestepCount(0),
			_gridFrequencySize(0),
			_realData(0), _imagData(0),
			_u(0), _v(0), _w(0)
		{
		}
		
		~ObservationTimerange()
		{
			deallocate();
		}
		
		void SetBandInfo(size_t nodeIndex, const BandInfo &bandInfo)
		{
			_bands[nodeIndex] = bandInfo;
		}
		
		void Initialize(size_t polarizationCount, size_t timestepCount)
		{
			deallocate();
			
			_polarizationCount = polarizationCount;
			_timestepCount = timestepCount;
			
			std::map<double, BandRangeInfo > ranges;
			
			if(_bands.empty())
				throw std::runtime_error("InitializeChannels was called, but no bands are available");
			
			size_t indx = 0;
			for(std::vector<BandInfo>::const_iterator i = _bands.begin();i != _bands.end();++i)
			{
				const BandInfo &band = *i;
				double startFreq = band.channels.begin()->frequencyHz;
				BandRangeInfo range;
				range.endFrequency = band.channels.rbegin()->frequencyHz;
				range.nodeIndex = indx;
				ranges.insert(std::pair<double, BandRangeInfo>(startFreq, range));
				++indx;
			}
			// Check for overlap
			if(!ranges.empty())
			{
				std::map<double, BandRangeInfo>::const_iterator nextPtr = ranges.begin();
					++nextPtr;
				for(std::map<double, BandRangeInfo>::const_iterator i=ranges.begin();nextPtr!=ranges.end();++i)
				{
					const double
						endFirst = i->second.endFrequency,
						beginSecond = nextPtr->first;
					if(endFirst >= beginSecond)
						throw std::runtime_error("Observation has measurement sets whose bands overlap in frequency");
					++nextPtr;
				}
			}
			// Enumerate channels
			std::vector<double> channels;
			for(std::map<double, BandRangeInfo>::const_iterator i=ranges.begin();i!=ranges.end();++i)
			{
				const BandInfo &band = _bands[i->second.nodeIndex];
				for(std::vector<ChannelInfo>::const_iterator c=band.channels.begin();c!=band.channels.end();++c)
					channels.push_back(c->frequencyHz);
			}
			// Find the median distance between channels
			std::vector<double> distances;
			for(std::vector<double>::const_iterator i=channels.begin();i+1!=channels.end();++i)
			{
				const double curDistance = *(i+1)-*i;
				if(curDistance > 0.0)
					distances.push_back(curDistance);
				else if(curDistance == 0.0)
					throw std::runtime_error("The full set contains two channels with the same frequency");
				else
					throw std::runtime_error("Channels were not ordered correctly in one of the sets");
			}
			double gridDistance = *std::min_element(distances.begin(), distances.end());
			std::cout << "Frequency resolution: " << gridDistance << "\n";
			
			// Create band start index lookup table
			size_t channelCount = 0;
			for(std::map<double, BandRangeInfo>::const_iterator i=ranges.begin();i!=ranges.end();++i)
			{
				size_t nodeIndex = i->second.nodeIndex;
				_bandStartLookup[nodeIndex] = channelCount;
				channelCount += _bands[nodeIndex].channels.size();
			}
			std::cout << "Channel count: " << channelCount << "\n";
			
			// Create grid
			_gridIndexLookup.resize(channelCount);
			size_t lookupIndex = 0;
			size_t gridIndex = 0;
			double gridPos = channels[0];
			while(lookupIndex < channels.size())
			{
				// we will round each channel to its nearest point on the grid with resolution "gridDistance"
				size_t gridDist = (size_t) round((channels[lookupIndex] - gridPos) / gridDistance);
				gridPos = channels[lookupIndex];
				gridIndex += gridDist;
				_gridIndexLookup[lookupIndex] = gridIndex;
				++lookupIndex;
			}
			_gridFrequencySize = gridIndex+1;
			std::cout << "Grid points: " << _gridFrequencySize << "\n";
			
			// Allocate memory
			const size_t allocSize = _gridFrequencySize * _timestepCount * _polarizationCount;
			_realData = new num_t[allocSize * 2];
			_imagData = &_realData[allocSize];
			_u = new double[_timestepCount];
			_v = new double[_timestepCount];
			_w = new double[_timestepCount];
		}
		
		void SetTimestepData(size_t nodeIndex, const MSRowDataExt *rows, size_t rowCount)
		{
			size_t bandStart = _bandStartLookup[nodeIndex];
			//const MSRowData &firstRowData = rows[0].Data();
			const unsigned pCount = _polarizationCount;
			for(size_t r=0;r<rowCount;++r)
			{
				const MSRowData &row = rows[r].Data();
				const num_t *realPtr = row.RealPtr();
				const num_t *imagPtr = row.ImagPtr();
				std::vector<size_t>::const_iterator gridPtr = _gridIndexLookup.begin()+bandStart;
				for(size_t c=0;c<row.ChannelCount();++c)
				{
					const size_t gridIndex = *gridPtr;
					size_t fullIndex = (r * _gridFrequencySize + gridIndex) * pCount;
					for(unsigned p=0;p<pCount;++p)
					{
						_realData[fullIndex] = *realPtr;
						_imagData[fullIndex] = *imagPtr;
						++fullIndex;
						++realPtr;
						++imagPtr;
					}
					++gridPtr;
				}
			}
		}
		
	private:
		struct BandRangeInfo { double endFrequency; size_t nodeIndex; };
		ClusteredObservation &_observation;
		std::vector<BandInfo> _bands;
		std::vector<size_t> _bandStartLookup;
		std::vector<size_t> _gridIndexLookup;
		size_t _polarizationCount;
		size_t _timestepCount;
		size_t _gridFrequencySize;
		
		// First index is polarization, second is frequency, third is timestep
		num_t *_realData;
		num_t *_imagData;
		double *_u, *_v, *_w;
		
		void deallocate()
		{
			delete[] _realData;
			delete[] _u;
			delete[] _v;
			delete[] _w;
		}
};
	
}

#endif
