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
		: _sum(0.0), _sum2(0.0), _sum3(0.0), _sum4(0.0), _count(0)
		{
		}
		
		NoiseStatistics(const Array &samples)
		: _sum(0.0), _sum2(0.0), _sum3(0.0), _sum4(0.0), _count(0)
		{
			Add(samples);
		}
		
		void Set(const Array &samples)
		{
			_sum = 0.0;
			_sum2 = 0.0;
			_sum3 = 0.0;
			_sum4 = 0.0;
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
				_sum2 += (v * v);
				_sum3 += (v * v * v);
				_sum4 += (v * v * v * v);
			}
			_count += samples.size();
		}
		
		void Add(const NoiseStatistics &statistics)
		{
			_count += statistics._count;
			_sum += statistics._sum;
			_sum2 += statistics._sum2;
			_sum3 += statistics._sum3;
			_sum4 += statistics._sum4;
		}
		
		unsigned long Count() const
		{
			return _count;
		}
		
		stat_t Sum() const
		{
			return _sum;
		}
		
		stat_t Sum2() const
		{
			return _sum2;
		}
		
		stat_t Sum3() const
		{
			return _sum3;
		}
		
		stat_t Sum4() const
		{
			return _sum4;
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
				return (_sum2 + sumMeanSquared - (_sum2 * 2.0 / n)) / (n-1.0);
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
				return (_sum2 + sumMeanSquared - (_sum2 * 2.0 / n)) / n;
			}
		}
		
		stat_t FourthMoment() const
		{
			if(_count == 0)
				return 0.0;
			else
			{
				const stat_t
					n = _count,
					mean = _sum / n,
					mean2 = mean * mean;
				return (_sum4
					- 4.0 * (_sum3 * mean + _sum * mean2 * mean)
					+ 6.0 * _sum2 * mean2 +
					mean2 * mean2) / n;
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
		
		static unsigned WriteColumnCount() { return 8; }
		static unsigned VarianceColumn() { return 6; }
		
		static void WriteHeaders(const std::string &headerPrefix, std::ostream &stream)
		{
			stream <<
				headerPrefix << "Count\t" << headerPrefix << "Sum\t" << headerPrefix << "Sum2\t" <<
				headerPrefix << "Sum3\t" << headerPrefix << "Sum4\t" <<
				headerPrefix << "Mean\t" << headerPrefix << "Variance\t" <<
				headerPrefix << "VarianceOfVariance";
		}
		
		void WriteValues(std::ostream &stream) const
		{
			stream
				<< _count << '\t'
				<< _sum << '\t'
				<< _sum2 << '\t'
				<< _sum3 << '\t'
				<< _sum4 << '\t'
				<< Mean() << '\t'
				<< VarianceEstimator() << '\t'
				<< VarianceOfVarianceEstimator();
		}
		
		void ReadValues(std::istream &stream)
		{
			stat_t tmp;
			stream >> _count >> _sum >> _sum2 >> _sum3 >> _sum4 >> tmp >> tmp >> tmp;
		}
	private:
		stat_t _sum;
		stat_t _sum2;
		stat_t _sum3;
		stat_t _sum4;
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
		typedef std::pair<double, unsigned> TAIndex;
		typedef std::pair<double, std::pair<unsigned, unsigned> > TBIndex;
		typedef std::map<TFIndex, CNoiseStatistics> StatTFMap;
		typedef std::map<TBIndex, CNoiseStatistics> StatTBMap;
		
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
		
		bool Empty() const
		{
			return _valuesTB.empty() && _valuesTF.empty();
		}
		
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
				<< "CentralTime\tCentralFrequency\t";
			NoiseStatistics::WriteHeaders("Real", file);
			file << '\t';
			NoiseStatistics::WriteHeaders("Imag", file);
			file << '\n' << std::setprecision(14);
			
			for(StatTFMap::const_iterator i=_valuesTF.begin();i!=_valuesTF.end();++i)
			{
				const double
					centralTime = i->first.first,
					centralFrequency = i->first.second;
				const NoiseStatistics
					&realStat = i->second.real,
					&imaginaryStat = i->second.imaginary;
				file << centralTime << '\t' << centralFrequency << '\t';
				realStat.WriteValues(file);
				file << '\t';
				imaginaryStat.WriteValues(file);
				file << '\n';
			}
		}
		
		void SaveTA(const std::string &filename)
		{
			std::ofstream file(filename.c_str());
			file <<
				"CentralTime\tAntenna1\tAntenna2\t";
			NoiseStatistics::WriteHeaders("Real", file);
			file << '\t';
			NoiseStatistics::WriteHeaders("Imag", file);
			file << '\n' << std::setprecision(14);
			
			for(StatTBMap::const_iterator i=_valuesTB.begin();i!=_valuesTB.end();++i)
			{
				const double
					centralTime = i->first.first;
				const unsigned
					antenna1 = i->first.second.first,
					antenna2 = i->first.second.second;
				const NoiseStatistics
					&realStat = i->second.real,
					&imaginaryStat = i->second.imaginary;
				file << centralTime << '\t' << antenna1 << '\t' << antenna2 << '\t';
				realStat.WriteValues(file);
				file << '\t';
				imaginaryStat.WriteValues(file);
				file << '\n';
			}
		}
		
		void ReadTF(const std::string &filename)
		{
			std::ifstream file(filename.c_str());
			std::string headers;
			std::getline(file, headers);
			
			while(file.good())
			{
				double centralTime, centralFrequency;
				CNoiseStatistics statistics;
				file >> centralTime;
				if(file.eof()) break;
				file >> centralFrequency;
				statistics.real.ReadValues(file);
				statistics.imaginary.ReadValues(file);
				
				TFIndex index = TFIndex(centralTime, centralFrequency);
				add(_valuesTF, index, statistics);
			}
		}
		
		void ReadTA(const std::string &filename)
		{
			std::ifstream file(filename.c_str());
			std::string headers;
			std::getline(file, headers);
			
			while(file.good())
			{
				double centralTime;
				std::pair<unsigned, unsigned> baseline;
				CNoiseStatistics statistics;
				file >> centralTime;
				if(file.eof()) break;
				file >> baseline.first >> baseline.second;
				statistics.real.ReadValues(file);
				statistics.imaginary.ReadValues(file);
				
				TBIndex index = TBIndex(centralTime, baseline);
				add(_valuesTB, index, statistics);
			}
		}
		
		void SaveTimeAntennaPlot(const std::string &dataName, const std::string &plotName)
		{
			std::map<TAIndex, CNoiseStatistics> taValues;
			unsigned antennaCount = 0;
			for(StatTBMap::const_iterator i=_valuesTB.begin();i!=_valuesTB.end();++i)
			{
				double time = i->first.first;
				unsigned antenna1 = i->first.second.first;
				unsigned antenna2 = i->first.second.second;
				TAIndex index;
				
				index = TAIndex(time, antenna1);
				add(taValues, index, i->second);
				
				index = TAIndex(time, antenna2);
				add(taValues, index, i->second);
				
				if(antenna1 >= antennaCount)
					antennaCount = antenna1 + 1;
				if(antenna2 >= antennaCount)
					antennaCount = antenna2 + 1;
			}

			std::ofstream dataFile(dataName.c_str());

			// Write the headers
			dataFile <<
				"CentralTime";
			for(unsigned i=0;i<antennaCount;++i)
			{
				std::stringstream realStr, imagStr;
				dataFile << '\t';
				realStr << "Real" << i;
				NoiseStatistics::WriteHeaders(realStr.str(), dataFile);
				dataFile << '\t';
				imagStr << "Imag" << i;
				NoiseStatistics::WriteHeaders(imagStr.str(), dataFile);
			}
			dataFile << std::setprecision(14);
			
			double centralTime = 0.0;
			for(std::map<TAIndex, CNoiseStatistics>::const_iterator i=taValues.begin();
				i!=taValues.end();++i)
			{
				if(centralTime != i->first.first)
				{
					centralTime = i->first.first;
					dataFile << '\n' << centralTime;
				}
				const NoiseStatistics
					&realStat = i->second.real,
					&imaginaryStat = i->second.imaginary;
				if(realStat.Count() > 0 && imaginaryStat.Count() > 0)
				{
					dataFile << '\t' ;
					realStat.WriteValues(dataFile);
					dataFile << '\t';
					imaginaryStat.WriteValues(dataFile);
				} else {
					for(unsigned i=0;i<2 * NoiseStatistics::WriteColumnCount();++i)
					{
						dataFile << "\t?";
					}
				}
			}
			dataFile << '\n';
			const unsigned
				columnsPerAntenna = NoiseStatistics::WriteColumnCount(),
				varianceColumn = NoiseStatistics::VarianceColumn();
			
			std::ofstream stationTimePlot(plotName.c_str());
			double
				startTime = taValues.begin()->first.first,
				endTime = taValues.rbegin()->first.first;
			stationTimePlot << std::setprecision(14) <<
				"set term postscript enhanced color font \"Helvetica,12\"\n"
				"set title \"Noise statistics over time and station\"\n"
				"set xlabel \"Time (hrs)\"\n"
				"set ylabel \"Variance\"\n"
				"set output \"StationsTime-Var.ps\"\n"
				"set key inside top\n"
				"set log y\n"
				"set xrange [" << 0 << ":" << ((endTime-startTime)/(60.0*60.0)) << "]\n"
				"plot \\\n";
			std::stringstream timeAxisStr;
			timeAxisStr << std::setprecision(14) << "((column(1)-" << startTime << ")/(60.0*60.0))";
			const std::string timeAxis = timeAxisStr.str();
			for(unsigned x=0;x<antennaCount;++x)
			{
				if(x != 0)
					stationTimePlot << ", \\\n";
				stationTimePlot
				<< "\"" << dataName << "\" using "
				<< timeAxis
				<< ":((column(" << ((2*x)*columnsPerAntenna+varianceColumn + 2)
				<< ") + column(" << ((2*x+1)*columnsPerAntenna+varianceColumn + 2)
				<< "))/2) title \"Station " << x << "\" with lines lw 2";
			}
			stationTimePlot << '\n';
		}
		
		const StatTFMap &TBMap() const { return _valuesTF; }
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
			
			const TBIndex tbIndex = TBIndex(centralTime, std::pair<unsigned, unsigned>(metaData->Antenna1().id, metaData->Antenna2().id));
			add(_valuesTB, tbIndex, statistics);
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
		StatTBMap _valuesTB;
		unsigned _channelDistance;
		unsigned _tileWidth, _tileHeight;
};

#endif
