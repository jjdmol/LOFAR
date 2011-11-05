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
#ifndef STATISTICS_COLLECTOR_H
#define STATISTICS_COLLECTOR_H

#include "cnoisestatistics.h"
#include "noisestatistics.h"

#include <map>

#include <AOFlagger/msio/qualitydata.h>
#include <AOFlagger/msio/statisticalvalue.h>

class StatisticsCollector
{
	public:
		StatisticsCollector(unsigned polarizationCount) : _baselineStatistics(polarizationCount), _polarizationCount(polarizationCount)
		{
		}
		
		void Add(unsigned antenna1, unsigned antenna2, double time, const double *frequencies, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI)
		{
			if(samples.empty()) return;
			
			addTimeAndBaseline(antenna1, antenna2, time, polarization, samples, isRFI, false);
			addFrequency(frequencies, polarization, samples, isRFI, false);
			
			std::vector<std::complex<float> > diffSamples;
			
			for(std::vector<std::complex<float> >::const_iterator i=samples.begin();i+1!=samples.end();++i)
			{
				diffSamples.push_back(*(i+1) - *i);
			}
			bool *diffRFIFlags = new bool[samples.size()];
			for(unsigned i=0;i<samples.size()-1;++i)
			{
				diffRFIFlags[i] = isRFI[i] | isRFI[i+1];
			}
			addTimeAndBaseline(antenna1, antenna2, time, polarization, diffSamples, diffRFIFlags, true);
			addFrequency(frequencies, polarization, diffSamples, diffRFIFlags, true);
			addFrequency(frequencies+1, polarization, diffSamples, diffRFIFlags, true);
			delete[] diffRFIFlags;
		}
		
		void Save(QualityData &qualityData)
		{
			saveTime(qualityData);
		}
		
	private:
		struct StatisticSaver
		{
			QualityData::StatisticDimension dimension;
			double time;
			double frequency;
			unsigned antenna1;
			unsigned antenna2;
			QualityData *qualityData;
			
			void Save(StatisticalValue &value, unsigned kindIndex)
			{
				value.SetKindIndex(kindIndex);
				switch(dimension)
				{
					case QualityData::TimeDimension:
						qualityData->StoreTimeValue(time, frequency, value);
						break;
					case QualityData::FrequencyDimension:
						qualityData->StoreFrequencyValue(frequency, value);
						break;
					case QualityData::BaselineDimension:
						qualityData->StoreBaselineValue(antenna1, antenna2, frequency, value);
						break;
					case QualityData::BaselineTimeDimension:
						qualityData->StoreBaselineTimeValue(antenna1, antenna2, time, frequency, value);
						break;
				}
			}
		};
		
		struct Statistics
		{
			public:
				Statistics(unsigned _polarizationCount)
					: polarizationCount(_polarizationCount)
				{
					statistics = new CNoiseStatistics[polarizationCount];
					differentialStatistics = new CNoiseStatistics[polarizationCount];
					rfiCount = new unsigned long[polarizationCount];
				}
				Statistics(const Statistics &source)
				{
					polarizationCount = source.polarizationCount;
					statistics = new CNoiseStatistics[polarizationCount];
					differentialStatistics = new CNoiseStatistics[polarizationCount];
					rfiCount = new unsigned long[polarizationCount];
					for(unsigned p=0;p<polarizationCount;++p)
					{
						statistics[p] = source.statistics[p];
						differentialStatistics[p] = source.differentialStatistics[p];
						rfiCount[p] = source.rfiCount[p];
					}
				}
				
				~Statistics()
				{
					delete[] statistics;
					delete[] differentialStatistics;
					delete[] rfiCount;
				}
				
				CNoiseStatistics *statistics;
				CNoiseStatistics *differentialStatistics;
				unsigned long *rfiCount;
				
				unsigned polarizationCount;
			private:
				Statistics &operator=(const Statistics &source) { return *this; }
		};

		class BaselineStatisticsMap
		{
			public:
				BaselineStatisticsMap(unsigned polarizationCount) : _polarizationCount(polarizationCount)
				{
				}
				
				Statistics &GetStatistics(unsigned antenna1, unsigned antenna2)
				{
					OuterMap::iterator antenna1Map = _map.insert(OuterPair(antenna1, InnerMap())).first;
					InnerMap &innerMap = antenna1Map->second;
					InnerMap::iterator antenna2Value = innerMap.find(antenna2);
					Statistics *statistics;
					if(antenna2Value == innerMap.end())
					{
						// The baseline does not exist yet, create empty statistics.
						statistics = &(innerMap.insert(InnerPair(antenna2, Statistics(_polarizationCount))).first->second);
					} else {
						statistics = &antenna2Value->second;
					}
					return *statistics;
				}
				
				std::vector<std::pair<unsigned, unsigned> > BaselineList()
				{
					std::vector<std::pair<unsigned, unsigned> > list;
					for(OuterMap::const_iterator outerIter = _map.begin(); outerIter!=_map.end(); ++outerIter)
					{
						const unsigned antenna1 = outerIter->first;
						const InnerMap &innerMap = outerIter->second;
						
						for(InnerMap::const_iterator innerIter = innerMap.begin(); innerIter!=innerMap.end(); ++innerIter)
						{
							const unsigned antenna2 = innerIter->first;
							list.push_back(std::pair<unsigned, unsigned>(antenna1, antenna2));
						}
					}
					return list;
				}
			private:
				typedef std::map<unsigned, Statistics> InnerMap;
				typedef std::pair<unsigned, Statistics> InnerPair;
				typedef std::map<unsigned, InnerMap > OuterMap;
				typedef std::pair<unsigned, InnerMap > OuterPair;
				
				OuterMap _map;
				unsigned _polarizationCount;
		};

		struct Indices
		{
			unsigned kindRFIRatio;
			unsigned kindCount;
			unsigned kindMean;
			unsigned kindSumP2;
			unsigned kindDCount;
			unsigned kindDMean;
			unsigned kindDSumP2;
			
			void fill(QualityData &qd)
			{
				kindRFIRatio = qd.StoreOrQueryKindIndex(QualityData::RFIRatioStatistic),
				kindCount = qd.StoreOrQueryKindIndex(QualityData::CountStatistic),
				kindMean = qd.StoreOrQueryKindIndex(QualityData::MeanStatistic),
				kindSumP2 = qd.StoreOrQueryKindIndex(QualityData::SumP2Statistic),
				kindDCount = qd.StoreOrQueryKindIndex(QualityData::DCountStatistic),
				kindDMean = qd.StoreOrQueryKindIndex(QualityData::DMeanStatistic),
				kindDSumP2 = qd.StoreOrQueryKindIndex(QualityData::DSumP2Statistic);
			}
		
		};

		void addTimeAndBaseline(unsigned antenna1, unsigned antenna2, double time, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI, bool isDiff)
		{
			NoiseStatistics::Array realArray, imagArray;
			unsigned long rfiCount = 0;
			for(unsigned i=0;i<samples.size();++i)
			{
				if(isRFI[i])
				{
					++rfiCount;
				} else {
					realArray.push_back(samples[i].real());
					imagArray.push_back(samples[i].imag());
				}
			}
			CNoiseStatistics cnoise(realArray, imagArray);
			
			DoubleStatMap::iterator i = _timeStatistics.insert(std::pair<double, Statistics>(time, Statistics(_polarizationCount))).first;
			Statistics &timeStat = i->second;
			Statistics &baselineStat = _baselineStatistics.GetStatistics(antenna1, antenna2);
			if(isDiff)
			{
				timeStat.differentialStatistics[polarization] += cnoise;
				baselineStat.differentialStatistics[polarization] += cnoise;
			} else {
				timeStat.statistics[polarization] += cnoise;
				timeStat.rfiCount[polarization] += rfiCount;
				baselineStat.statistics[polarization] += cnoise;
				baselineStat.rfiCount[polarization] += rfiCount;
			}
		}
		
		void addFrequency(const double *frequencies, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI, bool isDiff)
		{
			for(unsigned f=0;f<samples.size();++f)
			{
				double frequency = frequencies[f];
				
				unsigned long rfiCount;
				NoiseStatistics::Array realArray, imagArray;
				if(isRFI[f])
				{
					rfiCount = 1;
				} else {
					realArray.push_back(samples[f].real());
					imagArray.push_back(samples[f].imag());
					rfiCount = 0;
				}
				CNoiseStatistics cnoise(realArray, imagArray);
			
				DoubleStatMap::iterator i = _frequencyStatistics.insert(std::pair<double, Statistics>(frequency, Statistics(_polarizationCount))).first;
				Statistics &freqStat = i->second;
				if(isDiff)
				{
					freqStat.differentialStatistics[polarization] += cnoise;
				} else {
					freqStat.statistics[polarization] += cnoise;
					freqStat.rfiCount[polarization] += rfiCount;
				}
			}
		}
		
		void initializeEmptyStatistics(QualityData &qualityData, QualityData::QualityTable table)
		{
			qualityData.InitializeEmptyStatistic(table, QualityData::RFIRatioStatistic);
			qualityData.InitializeEmptyStatistic(table, QualityData::CountStatistic);
			qualityData.InitializeEmptyStatistic(table, QualityData::MeanStatistic);
			qualityData.InitializeEmptyStatistic(table, QualityData::SumP2Statistic);
			qualityData.InitializeEmptyStatistic(table, QualityData::DCountStatistic);
			qualityData.InitializeEmptyStatistic(table, QualityData::DMeanStatistic);
			qualityData.InitializeEmptyStatistic(table, QualityData::DSumP2Statistic);
		}
		
		void saveEachStatistic(StatisticSaver &saver, const Statistics &stat, const Indices &indices)
		{
			StatisticalValue value(_polarizationCount);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, std::complex<float>((double) stat.rfiCount[p] / (double) (stat.rfiCount[p] + stat.statistics[p].Count()), 0.0f));
			saver.Save(value, indices.kindRFIRatio);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, std::complex<float>(stat.statistics[p].Count(), 0.0f));
			saver.Save(value, indices.kindCount);

			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.statistics[p].Mean());
			saver.Save(value, indices.kindMean);

			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.statistics[p].Sum2());
			saver.Save(value, indices.kindSumP2);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, std::complex<float>(stat.differentialStatistics[p].Count(), 0.0f));
			saver.Save(value, indices.kindDCount);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.differentialStatistics[p].Mean());
			saver.Save(value, indices.kindDMean);

			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.differentialStatistics[p].Sum2());
			saver.Save(value, indices.kindDSumP2);
		}
		
		void saveTime(QualityData &qd)
		{
			initializeEmptyStatistics(qd, QualityData::TimeStatisticTable);
			
			Indices indices;
			indices.fill(qd);
				
			StatisticSaver saver;
			saver.dimension = QualityData::TimeDimension;
			saver.frequency = centralFrequency();
			saver.qualityData = &qd;
			
			for(DoubleStatMap::const_iterator i=_timeStatistics.begin();i!=_timeStatistics.end();++i)
			{
				saver.time = i->first;
				const Statistics &stat = i->second;
				
				saveEachStatistic(saver, stat, indices);
			}
		}
		
		void saveFrequency(QualityData &qd)
		{
			initializeEmptyStatistics(qd, QualityData::FrequencyStatisticTable);
			
			Indices indices;
			indices.fill(qd);
				
			StatisticSaver saver;
			saver.dimension = QualityData::FrequencyDimension;
			saver.qualityData = &qd;
			
			for(DoubleStatMap::const_iterator i=_frequencyStatistics.begin();i!=_frequencyStatistics.end();++i)
			{
				saver.frequency = i->first;
				const Statistics &stat = i->second;
				
				saveEachStatistic(saver, stat, indices);
			}
		}
		
		void saveBaseline(QualityData &qd)
		{
			initializeEmptyStatistics(qd, QualityData::BaselineStatisticTable);
			
			Indices indices;
			indices.fill(qd);
			
			StatisticSaver saver;
			saver.dimension = QualityData::BaselineDimension;
			saver.frequency = centralFrequency();
			saver.qualityData = &qd;
			
			const std::vector<std::pair<unsigned, unsigned> > baselines = _baselineStatistics.BaselineList();
			
			for(std::vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
			{
				saver.antenna1 = i->first;
				saver.antenna2 =  i->second;
				
				const Statistics &stat = _baselineStatistics.GetStatistics(saver.antenna1, saver.antenna2);
				
				saveEachStatistic(saver, stat, indices);
			}
		}
		
		double centralFrequency() const
		{
			double min =_frequencyStatistics.begin()->first;
			double max = _frequencyStatistics.rbegin()->first;
			return (min + max) / 2.0;
		}
		
		typedef std::map<double, Statistics> DoubleStatMap;
		DoubleStatMap _timeStatistics;
		DoubleStatMap _frequencyStatistics;
		BaselineStatisticsMap _baselineStatistics;
		unsigned _polarizationCount;
};

#endif
