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
#ifndef STATISTICS_COLLECTION_H
#define STATISTICS_COLLECTION_H

#include <AOFlagger/strategy/algorithms/cnoisestatistics.h>
#include <AOFlagger/strategy/algorithms/noisestatistics.h>

#include "baselinestatisticsmap.h"
#include "defaultstatistics.h"
#include "qualitytablesformatter.h"
#include "statistics.h"
#include "statisticalvalue.h"

class StatisticsCollection
{
	public:
		StatisticsCollection(unsigned polarizationCount) : _baselineStatistics(polarizationCount), _polarizationCount(polarizationCount)
		{
		}
		
		void Clear()
		{
			_timeStatistics.clear();
			_frequencyStatistics.clear();
			_baselineStatistics.Clear();
		}
		
		void InitializeBand(unsigned band, const double *frequencies, unsigned channelCount)
		{
			std::vector<Statistics *> pointers;
			for(unsigned i=0;i<channelCount;++i)
			{
				pointers.push_back(&getFrequencyStatistic(frequencies[i]));
			}
			_bands.insert(std::pair<unsigned, std::vector<Statistics *> >(band, pointers));
		}
		
		void Add(unsigned antenna1, unsigned antenna2, double time, unsigned band, int polarization, const std::vector<std::complex<float> > &samples, const bool *isRFI)
		{
			if(samples.empty()) return;
			
			addTimeAndBaseline(antenna1, antenna2, time, polarization, samples, isRFI, false);
			if(antenna1 != antenna2)
				addFrequency(band, polarization, samples, isRFI, false, false);
			
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
			if(antenna1 != antenna2)
			{
				addFrequency(band, polarization, diffSamples, diffRFIFlags, true, false);
				addFrequency(band, polarization, diffSamples, diffRFIFlags, true, true);
			}
			delete[] diffRFIFlags;
		}
		
		void Save(QualityTablesFormatter &qualityData)
		{
			saveTime(qualityData);
			saveFrequency(qualityData);
			saveBaseline(qualityData);
		}
		
		void Load(QualityTablesFormatter &qualityData)
		{
			loadTime(qualityData);
			loadFrequency(qualityData);
			loadBaseline(qualityData);
		}
		
		void GetGlobalTimeStatistics(DefaultStatistics &statistics)
		{
			Statistics global = getGlobalStatistics(_timeStatistics);
			setDefaultsFromStatistics(statistics, global);
		}
		
		void GetGlobalFrequencyStatistics(DefaultStatistics &statistics)
		{
			Statistics global = getGlobalStatistics(_frequencyStatistics);
			setDefaultsFromStatistics(statistics, global);
		}
		
		void GetGlobalAutoBaselineStatistics(DefaultStatistics &statistics)
		{
			Statistics global = getGlobalBaselineStatistics<true>();
			setDefaultsFromStatistics(statistics, global);
		}
		
		void GetGlobalCrossBaselineStatistics(DefaultStatistics &statistics)
		{
			Statistics global = getGlobalBaselineStatistics<false>();
			setDefaultsFromStatistics(statistics, global);
		}
		
		const BaselineStatisticsMap &BaselineStatistics() const
		{
			return _baselineStatistics;
		}
		
		const std::map<double, Statistics> &TimeStatistics() const
		{
			return _timeStatistics;
		}
		
		const std::map<double, Statistics> &FrequencyStatistics() const
		{
			return _frequencyStatistics;
		}
		
		unsigned PolarizationCount() const
		{
			return _polarizationCount;
		}
	private:
		struct StatisticSaver
		{
			QualityTablesFormatter::StatisticDimension dimension;
			double time;
			double frequency;
			unsigned antenna1;
			unsigned antenna2;
			QualityTablesFormatter *qualityData;
			
			void Save(StatisticalValue &value, unsigned kindIndex)
			{
				value.SetKindIndex(kindIndex);
				switch(dimension)
				{
					case QualityTablesFormatter::TimeDimension:
						qualityData->StoreTimeValue(time, frequency, value);
						break;
					case QualityTablesFormatter::FrequencyDimension:
						qualityData->StoreFrequencyValue(frequency, value);
						break;
					case QualityTablesFormatter::BaselineDimension:
						qualityData->StoreBaselineValue(antenna1, antenna2, frequency, value);
						break;
					case QualityTablesFormatter::BaselineTimeDimension:
						qualityData->StoreBaselineTimeValue(antenna1, antenna2, time, frequency, value);
						break;
				}
			}
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
			
			void fill(QualityTablesFormatter &qd)
			{
				kindRFIRatio = qd.StoreOrQueryKindIndex(QualityTablesFormatter::RFIRatioStatistic),
				kindCount = qd.StoreOrQueryKindIndex(QualityTablesFormatter::CountStatistic),
				kindMean = qd.StoreOrQueryKindIndex(QualityTablesFormatter::MeanStatistic),
				kindSumP2 = qd.StoreOrQueryKindIndex(QualityTablesFormatter::SumP2Statistic),
				kindDCount = qd.StoreOrQueryKindIndex(QualityTablesFormatter::DCountStatistic),
				kindDMean = qd.StoreOrQueryKindIndex(QualityTablesFormatter::DMeanStatistic),
				kindDSumP2 = qd.StoreOrQueryKindIndex(QualityTablesFormatter::DSumP2Statistic);
			}
		};

		void addTimeAndBaseline(unsigned antenna1, unsigned antenna2, double time, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI, bool isDiff)
		{
			NoiseStatistics::Array realArray, imagArray;
			unsigned long rfiCount = 0;
			for(unsigned i=0;i<samples.size();++i)
			{
				if(std::isfinite(samples[i].real()) && std::isfinite(samples[i].imag()))
				{
					if(isRFI[i])
					{
						++rfiCount;
					} else {
						realArray.push_back(samples[i].real());
						imagArray.push_back(samples[i].imag());
					}
				}
			}
			CNoiseStatistics cnoise(realArray, imagArray);
			
			if(antenna1 != antenna2)
			{
				Statistics &timeStat = getTimeStatistic(time);
				if(isDiff)
				{
					timeStat.differentialStatistics[polarization] += cnoise;
				} else {
					timeStat.statistics[polarization] += cnoise;
					timeStat.rfiCount[polarization] += rfiCount;
				}
			}
			Statistics &baselineStat = _baselineStatistics.GetStatistics(antenna1, antenna2);
			if(isDiff)
			{
				baselineStat.differentialStatistics[polarization] += cnoise;
			} else {
				baselineStat.statistics[polarization] += cnoise;
				baselineStat.rfiCount[polarization] += rfiCount;
			}
		}
		
		void addFrequency(unsigned band, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI, bool isDiff, bool shiftOneUp)
		{
			std::vector<Statistics *> &bandStats = _bands.find(band)->second;
			const unsigned fAdd = shiftOneUp ? 1 : 0;
			for(unsigned f=0;f<samples.size();++f)
			{
				unsigned long rfiCount = 0;
				NoiseStatistics::Array realArray, imagArray;
				if(std::isfinite(samples[f].real()) && std::isfinite(samples[f].imag()))
				{
					if(isRFI[f])
					{
						rfiCount = 1;
					} else {
						realArray.push_back(samples[f].real());
						imagArray.push_back(samples[f].imag());
					}
				}
				CNoiseStatistics cnoise(realArray, imagArray);
			
				Statistics &freqStat = *bandStats[f + fAdd];
				if(isDiff)
				{
					freqStat.differentialStatistics[polarization] += cnoise;
				} else {
					freqStat.statistics[polarization] += cnoise;
					freqStat.rfiCount[polarization] += rfiCount;
				}
			}
		}
		
		void initializeEmptyStatistics(QualityTablesFormatter &qualityData, QualityTablesFormatter::StatisticDimension dimension)
		{
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::RFIRatioStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::CountStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::MeanStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::SumP2Statistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::DCountStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::DMeanStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::DSumP2Statistic);
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
		
		void saveTime(QualityTablesFormatter &qd)
		{
			initializeEmptyStatistics(qd, QualityTablesFormatter::TimeDimension);
			
			Indices indices;
			indices.fill(qd);
				
			StatisticSaver saver;
			saver.dimension = QualityTablesFormatter::TimeDimension;
			saver.frequency = centralFrequency();
			saver.qualityData = &qd;
			
			for(DoubleStatMap::const_iterator i=_timeStatistics.begin();i!=_timeStatistics.end();++i)
			{
				saver.time = i->first;
				const Statistics &stat = i->second;
				
				saveEachStatistic(saver, stat, indices);
			}
		}
		
		void saveFrequency(QualityTablesFormatter &qd)
		{
			initializeEmptyStatistics(qd, QualityTablesFormatter::FrequencyDimension);
			
			Indices indices;
			indices.fill(qd);
				
			StatisticSaver saver;
			saver.dimension = QualityTablesFormatter::FrequencyDimension;
			saver.qualityData = &qd;
			
			for(DoubleStatMap::const_iterator i=_frequencyStatistics.begin();i!=_frequencyStatistics.end();++i)
			{
				saver.frequency = i->first;
				const Statistics &stat = i->second;
				
				saveEachStatistic(saver, stat, indices);
			}
		}
		
		void saveBaseline(QualityTablesFormatter &qd)
		{
			initializeEmptyStatistics(qd, QualityTablesFormatter::BaselineDimension);
			
			Indices indices;
			indices.fill(qd);
			
			StatisticSaver saver;
			saver.dimension = QualityTablesFormatter::BaselineDimension;
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
		
		Statistics &getTimeStatistic(double time)
		{
			// We use find() to see if the value exists, and only use insert() when it does not,
			// because insert is slow (because a "Statistic" needs to be created).
			DoubleStatMap::iterator i = _timeStatistics.find(time);
			if(i == _timeStatistics.end())
			{
				i = _timeStatistics.insert(std::pair<double, Statistics>(time, Statistics(_polarizationCount))).first;
			}
			return i->second;
		}
		
		Statistics &getFrequencyStatistic(double frequency)
		{
			// Use insert() only when not exist, as it is slower then find because a
			// Statistic is created.
			DoubleStatMap::iterator i = _frequencyStatistics.find(frequency);
			if(i == _frequencyStatistics.end())
			{
				i = _frequencyStatistics.insert(std::pair<double, Statistics>(frequency, Statistics(_polarizationCount))).first;
			}
			return i->second;
		}
		
		void assignStatistic(Statistics &destination, const StatisticalValue &source, QualityTablesFormatter::StatisticKind kind)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				switch(kind)
				{
					case QualityTablesFormatter::RFIRatioStatistic:
						destination.rfiCount[p] = round((double) destination.statistics[p].Count() / ((1.0/source.Value(p).real())-1.0));
						break;
					case QualityTablesFormatter::CountStatistic:
						destination.statistics[p].SetCount((long unsigned) source.Value(p).real());
						break;
					case QualityTablesFormatter::MeanStatistic:
						destination.statistics[p].SetSum(source.Value(p) * (float) destination.statistics[p].Count());
						break;
					case QualityTablesFormatter::SumP2Statistic:
						destination.statistics[p].SetSum2(source.Value(p));
						break;
					case QualityTablesFormatter::DCountStatistic:
						destination.differentialStatistics[p].SetCount((long unsigned) source.Value(p).real());
						break;
					case QualityTablesFormatter::DMeanStatistic:
						destination.differentialStatistics[p].SetSum(source.Value(p) * (float) destination.differentialStatistics[p].Count());
						break;
					case QualityTablesFormatter::DSumP2Statistic:
						destination.differentialStatistics[p].SetSum2(source.Value(p));
						break;
					default:
						break;
				}
			}
		}
		
		void loadSingleTimeStatistic(QualityTablesFormatter &qd, QualityTablesFormatter::StatisticKind kind)
		{
			std::vector<std::pair<QualityTablesFormatter::TimePosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryTimeStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityTablesFormatter::TimePosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityTablesFormatter::TimePosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				Statistics &stat = getTimeStatistic(position.time);
				assignStatistic(stat, statValue, kind);
			}
		}
		
		void loadTime(QualityTablesFormatter &qd)
		{
			loadSingleTimeStatistic(qd, QualityTablesFormatter::CountStatistic);
			loadSingleTimeStatistic(qd, QualityTablesFormatter::MeanStatistic);
			loadSingleTimeStatistic(qd, QualityTablesFormatter::SumP2Statistic);
			loadSingleTimeStatistic(qd, QualityTablesFormatter::DCountStatistic);
			loadSingleTimeStatistic(qd, QualityTablesFormatter::DMeanStatistic);
			loadSingleTimeStatistic(qd, QualityTablesFormatter::DSumP2Statistic);
			loadSingleTimeStatistic(qd, QualityTablesFormatter::RFIRatioStatistic);
		}
		
		void loadSingleFrequencyStatistic(QualityTablesFormatter &qd, QualityTablesFormatter::StatisticKind kind)
		{
			std::vector<std::pair<QualityTablesFormatter::FrequencyPosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryFrequencyStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityTablesFormatter::FrequencyPosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityTablesFormatter::FrequencyPosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				Statistics &stat = getFrequencyStatistic(position.frequency);
				assignStatistic(stat, statValue, kind);
			}
		}
		
		void loadFrequency(QualityTablesFormatter &qd)
		{
			loadSingleFrequencyStatistic(qd, QualityTablesFormatter::CountStatistic);
			loadSingleFrequencyStatistic(qd, QualityTablesFormatter::MeanStatistic);
			loadSingleFrequencyStatistic(qd, QualityTablesFormatter::SumP2Statistic);
			loadSingleFrequencyStatistic(qd, QualityTablesFormatter::DCountStatistic);
			loadSingleFrequencyStatistic(qd, QualityTablesFormatter::DMeanStatistic);
			loadSingleFrequencyStatistic(qd, QualityTablesFormatter::DSumP2Statistic);
			loadSingleFrequencyStatistic(qd, QualityTablesFormatter::RFIRatioStatistic);
		}
		
		void loadSingleBaselineStatistic(QualityTablesFormatter &qd, QualityTablesFormatter::StatisticKind kind)
		{
			std::vector<std::pair<QualityTablesFormatter::BaselinePosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryBaselineStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityTablesFormatter::BaselinePosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityTablesFormatter::BaselinePosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				Statistics &stat = _baselineStatistics.GetStatistics(position.antenna1, position.antenna2);
				assignStatistic(stat, statValue, kind);
			}
		}
		
		void loadBaseline(QualityTablesFormatter &qd)
		{
			loadSingleBaselineStatistic(qd, QualityTablesFormatter::CountStatistic);
			loadSingleBaselineStatistic(qd, QualityTablesFormatter::MeanStatistic);
			loadSingleBaselineStatistic(qd, QualityTablesFormatter::SumP2Statistic);
			loadSingleBaselineStatistic(qd, QualityTablesFormatter::DCountStatistic);
			loadSingleBaselineStatistic(qd, QualityTablesFormatter::DMeanStatistic);
			loadSingleBaselineStatistic(qd, QualityTablesFormatter::DSumP2Statistic);
			loadSingleBaselineStatistic(qd, QualityTablesFormatter::RFIRatioStatistic);
		}
		
		double centralFrequency() const
		{
			double min =_frequencyStatistics.begin()->first;
			double max = _frequencyStatistics.rbegin()->first;
			return (min + max) / 2.0;
		}
		
		typedef std::map<double, Statistics> DoubleStatMap;
		Statistics getGlobalStatistics(const DoubleStatMap &statMap) const
		{
			Statistics global(_polarizationCount);
			for(DoubleStatMap::const_iterator i=statMap.begin();i!=statMap.end();++i)
			{
				const Statistics &stat = i->second;
				global += stat;
			}
			return global;
		}
		
		template<bool AutoCorrelations>
		Statistics getGlobalBaselineStatistics()
		{
			Statistics global(_polarizationCount);
			const std::vector<std::pair<unsigned, unsigned> > baselines = _baselineStatistics.BaselineList();
			
			for(std::vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
			{
				const unsigned
					antenna1 = i->first,
					antenna2 =  i->second;
				if( ((antenna1 == antenna2) && AutoCorrelations) || ((antenna1 != antenna2) && (!AutoCorrelations)))
				{
					const Statistics &stat = _baselineStatistics.GetStatistics(antenna1, antenna2);
					global += stat;
				}
			}
			return global;
		}
		
		void setDefaultsFromStatistics(DefaultStatistics &defaults, const Statistics &stat)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				defaults.rfiCount[p] = stat.rfiCount[p];
				defaults.count[p] = stat.statistics[p].Count();
				defaults.mean[p] = stat.statistics[p].Mean();
				defaults.sumP2[p] = stat.statistics[p].Sum2();
				defaults.dCount[p] = stat.differentialStatistics[p].Count();
				defaults.dMean[p] = stat.differentialStatistics[p].Mean();
				defaults.dSumP2[p] = stat.differentialStatistics[p].Sum2();
			}
		}
		
		DoubleStatMap _timeStatistics;
		DoubleStatMap _frequencyStatistics;
		std::map<unsigned, std::vector< Statistics *> > _bands;
		BaselineStatisticsMap _baselineStatistics;
		unsigned _polarizationCount;
};

#endif
