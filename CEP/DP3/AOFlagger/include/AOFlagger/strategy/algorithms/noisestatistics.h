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
#ifndef NOISESTATISTICS_H
#define NOISESTATISTICS_H

#include <cstring>
#include <map>
#include <set>
#include <complex>
#include <fstream>
#include <iomanip>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/timefrequencymetadata.h>
#include <AOFlagger/msio/mask2d.h>

struct stat;
class NoiseStatistics {
	public:

		typedef numl_t stat_t;
		typedef std::vector<stat_t> Array;
	
		NoiseStatistics()
		: _sum(0.0), _sumSecond(0.0), _sumThird(0.0), _sumFourth(0.0), _count(0)
		{
		}
		
		NoiseStatistics(const Array &samples)
		: _sum(0.0), _sumSecond(0.0), _sumThird(0.0), _sumFourth(0.0), _count(0)
		{
			Add(samples);
		}
		
		void Set(const Array &samples)
		{
			_sum = 0.0;
			_sumSecond = 0.0;
			_sumThird = 0.0;
			_sumFourth = 0.0;
			_count = 0;
			Add(samples);
		}
		
		void Add(const Array &samples)
		{
			// Calculate sum & mean
			for(Array::const_iterator i = samples.begin(); i != samples.end(); ++i)
			{
				const stat_t v = *i;
				_sum += v;
				_sumSecond += (v * v);
				_sumThird += (v * v * v);
				_sumFourth += (v * v * v * v);
			}
			_count += samples.size();
		}
		
		void Add(const NoiseStatistics &statistics)
		{
			_count += statistics._count;
			_sum += statistics._sum;
			_sumSecond += statistics._sum;
			_sumThird += statistics._sumThird;
			_sumFourth += statistics._sumFourth;
		}
		
		unsigned long Count() const
		{
			return _count;
		}
		
		stat_t Sum() const
		{
			return _sum;
		}
		
		stat_t Mean() const
		{
			if(_count == 0)
				return 0.0;
			else
				return _sum / (numl_t) _count;
		}
		
		stat_t VarianceEstimator() const
		{
			if(_count <= 1)
				return 0.0;
			else
			{
				const stat_t n = _count;
				const stat_t sumMeanSquared = (_sum * _sum) / n;
				return (_sumSecond + sumMeanSquared - (_sumSecond * 2.0 / n)) / (n-1.0);
			}
		}
		
		stat_t SecondMoment() const
		{
			if(_count == 0)
				return 0.0;
			else
			{
				const stat_t n = _count;
				const stat_t sumMeanSquared = (_sum * _sum) / n;
				return (_sumSecond + sumMeanSquared - (_sumSecond * 2.0 / n)) / n;
			}
		}
		
		stat_t FourthMoment() const
		{
			if(_count == 0)
				return 0.0;
			else
			{
				const stat_t n = _count;
				const stat_t mean = _sum / n;
				return (_sumFourth
					- 4.0 * (_sumThird * mean + _sum * mean * mean * mean)
					+ 6.0 * _sumSecond * mean * mean) / n;
			}
		}
		
		stat_t VarianceOfVarianceEstimator() const
		{
			const long double n = _count;
			if(n <= 1)
				return 0.0;
			else
			{
				const long double moment2 = SecondMoment();
				return ( FourthMoment() - moment2 * moment2 * (n-3.0)/(n-1.0) ) / n;
			}
		}
		
	private:
		stat_t _sum;
		stat_t _sumSecond;
		stat_t _sumThird;
		stat_t _sumFourth;
		unsigned long _count;
};

class CNoiseStatistics
{
	public:
		CNoiseStatistics() : real(), imaginary()
		{
		}
		
		CNoiseStatistics(const NoiseStatistics::Array &realValues, const NoiseStatistics::Array &imaginaryValues)
		: real(realValues), imaginary(imaginaryValues)
		{
		}
		
		CNoiseStatistics(const CNoiseStatistics &source) : real(source.real), imaginary(source.imaginary)
		{
		}
		
		void operator=(const CNoiseStatistics &source)
		{
			real = source.real;
			imaginary = source.imaginary;
		}
		
		void operator+=(const CNoiseStatistics &rhs)
		{
			real.Add(rhs.real);
			imaginary.Add(rhs.imaginary);
		}
		
		NoiseStatistics real;
		NoiseStatistics imaginary;
};

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class NoiseStatisticsCollector {
	public:
		typedef std::pair<double, double> TFIndex;
		typedef std::pair<double, std::pair<unsigned, unsigned> > TAIndex;
		typedef std::map<TFIndex, CNoiseStatistics> StatTFMap;
		typedef std::map<TAIndex, CNoiseStatistics> StatTAMap;
		
		NoiseStatisticsCollector()
		: _channelDistance(1), _tileWidth(200), _tileHeight(16)
		{
		}
		
		unsigned ChannelDistance() const { return _channelDistance; }
		void SetChannelDistance(unsigned channelDistance) { _channelDistance = channelDistance; }
		
		unsigned TileWidth() const { return _tileWidth; }
		void SetTileWidth(unsigned tileWidth) { _tileWidth = tileWidth; }
		
		unsigned TileHeight() const { return _tileHeight; }
		void SetTileHeight(unsigned tileHeight) { _tileHeight = tileHeight; }
		
		void Add(Image2DCPtr real, Image2DCPtr imaginary, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData)
		{
			Image2DCPtr
				realDiff = subtractChannels(real, _channelDistance),
				imagDiff = subtractChannels(imaginary, _channelDistance);
			Mask2DCPtr
				maskDiff = createMaskForSubtracted(mask, _channelDistance);
			
			// The number of tiles per dimension is ceil(imageWidth / tileWidth):
			const unsigned
				tileXCount = (realDiff->Width() + _tileWidth - 1) / _tileWidth,
				tileYCount = (realDiff->Height() + _tileHeight - 1) / _tileHeight;
			
			for(unsigned y=0;y<tileYCount;++y)
			{
				for(unsigned x=0;x<tileXCount;++x)
				{
					const unsigned
						timeStart = realDiff->Width() * x / tileXCount,
						timeEnd = realDiff->Width() * (x + 1) / tileXCount,
						freqStart = realDiff->Height() * y / tileYCount,
						freqEnd = realDiff->Height() * (y + 1) / tileYCount;
					add(realDiff, imagDiff, mask, metaData, timeStart, timeEnd, freqStart, freqEnd);
				}
			}
		}
		
		void SaveTF(const std::string &filename)
		{
			std::ofstream file(filename.c_str());
			file
				<< "CentralTime\tCentralFrequency\tRealCount\tRealMean\tRealVariance\tRealVarianceOfVariance\tImaginaryCount\tImaginaryMean\tImaginaryVariance\tImaginaryVarianceOfVariance\n"
				<< std::setprecision(14);
			for(StatTFMap::const_iterator i=_valuesTF.begin();i!=_valuesTF.end();++i)
			{
				const double
					centralTime = i->first.first,
					centralFrequency = i->first.second;
				const NoiseStatistics
					&realStat = i->second.real,
					&imaginaryStat = i->second.imaginary;
				file
					<< centralTime << '\t'
					<< centralFrequency << '\t'
					<< realStat.Count() << '\t'
					<< realStat.Mean() << '\t'
					<< realStat.VarianceEstimator() << '\t'
					<< realStat.VarianceOfVarianceEstimator() << '\t'
					<< imaginaryStat.Count() << '\t'
					<< imaginaryStat.Mean() << '\t'
					<< imaginaryStat.VarianceEstimator() << '\t'
					<< imaginaryStat.VarianceOfVarianceEstimator() << '\n';
			}
		}
		
		void SaveTA(const std::string &filename)
		{
			std::ofstream file(filename.c_str());
			file
				<< "CentralTime\tAntenna1\tAntenna2\tRealCount\tRealMean\tRealVariance\tRealVarianceOfVariance\tImaginaryCount\tImaginaryMean\tImaginaryVariance\tImaginaryVarianceOfVariance\n"
				<< std::setprecision(14);
			for(StatTAMap::const_iterator i=_valuesTA.begin();i!=_valuesTA.end();++i)
			{
				const double
					centralTime = i->first.first;
				const unsigned
					antenna1 = i->first.second.first,
					antenna2 = i->first.second.second;
				const NoiseStatistics
					&realStat = i->second.real,
					&imaginaryStat = i->second.imaginary;
				file
					<< centralTime << '\t'
					<< antenna1 << '\t'
					<< antenna2 << '\t'
					<< realStat.Count() << '\t'
					<< realStat.Mean() << '\t'
					<< realStat.VarianceEstimator() << '\t'
					<< realStat.VarianceOfVarianceEstimator() << '\t'
					<< imaginaryStat.Count() << '\t'
					<< imaginaryStat.Mean() << '\t'
					<< imaginaryStat.VarianceEstimator() << '\t'
					<< imaginaryStat.VarianceOfVarianceEstimator() << '\n';
			}
		}
	private:
		void add(Image2DCPtr real, Image2DCPtr imaginary, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, unsigned timeStart, unsigned timeEnd, unsigned freqStart, unsigned freqEnd)
		{
			NoiseStatistics::Array realValues, imagValues;
			
			for(unsigned y=freqStart;y<freqEnd;++y)
			{
				for(unsigned x=timeStart;x<timeEnd;++x)
				{
					if(!mask->Value(x, y))
					{
						realValues.push_back(real->Value(x, y));
						imagValues.push_back(imaginary->Value(x, y));
					}
				}
			}
			const CNoiseStatistics statistics(realValues, imagValues);
			const std::vector<ChannelInfo> &channels = metaData->Band().channels;
			const double
				centralTime = (metaData->ObservationTimes()[timeStart] +
					metaData->ObservationTimes()[timeEnd-1]) * 0.5,
				centralFrequency = (channels[freqStart].frequencyHz + channels[freqEnd-1].frequencyHz) * 0.5;
				
			const TFIndex tfIndex = TFIndex(centralTime, centralFrequency);
			add(_valuesTF, tfIndex, statistics);
			
			const TAIndex taIndex = TAIndex(centralTime, std::pair<unsigned, unsigned>(metaData->Antenna1().id, metaData->Antenna2().id));
			add(_valuesTA, taIndex, statistics);
		}
		
		template<typename IndexType>
		void add(std::map<IndexType, CNoiseStatistics> &map, const IndexType &index, const CNoiseStatistics &statistics)
		{
			typename std::map<IndexType, CNoiseStatistics>::iterator i = map.find(index);
			if(i == map.end())
			{
				map.insert(std::pair<IndexType, CNoiseStatistics>(index, statistics));
			} else {
				i->second += statistics;
			}
		}
		
		Image2DPtr subtractChannels(Image2DCPtr image, unsigned channelDistance=1) const
		{
			Image2DPtr
				subtracted = Image2D::CreateEmptyImagePtr(image->Width(), image->Height() - channelDistance);
			
			for(unsigned y=0;y<subtracted->Height();++y)
			{
				for(unsigned x=0;x<subtracted->Width();++x)
				{
					subtracted->SetValue(x, y, image->Value(x, y) - image->Value(x, y + channelDistance));
				}
			}
			return subtracted;
		}
		
		Mask2DPtr createMaskForSubtracted(Mask2DCPtr mask, unsigned channelDistance=1) const
		{
			Mask2DPtr
				subMask = Mask2D::CreateUnsetMaskPtr(mask->Width(), mask->Height() - channelDistance);
			
			for(unsigned y=0;y<subMask->Height();++y)
			{
				for(unsigned x=0;x<subMask->Width();++x)
				{
					subMask->SetValue(x, y, mask->Value(x, y) || mask->Value(x, y + channelDistance));
				}
			}
			return subMask;
		}
		
		StatTFMap _valuesTF;
		StatTAMap _valuesTA;
		unsigned _channelDistance;
		unsigned _tileWidth, _tileHeight;
};

#endif
