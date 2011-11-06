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

#include <AOFlagger/strategy/algorithms/cnoisestatistics.h>
#include <AOFlagger/strategy/algorithms/noisestatistics.h>

#include <map>

#include <AOFlagger/quality/qualitydata.h>
#include <AOFlagger/quality/statisticalvalue.h>

class StatisticsCollector
{
	public:
		StatisticsCollector(unsigned polarizationCount) : _baselineStatistics(polarizationCount), _polarizationCount(polarizationCount)
		{
		}
		
		class DefaultStatistics
		{
			public:
				DefaultStatistics(unsigned _polarizationCount) :
					polarizationCount(_polarizationCount)
				{
					rfiCount = new unsigned long[polarizationCount];
					count = new unsigned long[polarizationCount];
					mean = new std::complex<float>[polarizationCount];
					sumP2 = new std::complex<float>[polarizationCount];
					dCount = new unsigned long[polarizationCount];
					dMean = new std::complex<float>[polarizationCount];
					dSumP2 = new std::complex<float>[polarizationCount];
				}
				
				~DefaultStatistics()
				{
					delete[] rfiCount;
					delete[] count;
					delete[] mean;
					delete[] sumP2;
					delete[] dCount;
					delete[] dMean;
					delete[] dSumP2;
				}
				
				unsigned long *rfiCount;
				unsigned long *count;
				std::complex<float> *mean;
				std::complex<float> *sumP2;
				unsigned long *dCount;
				std::complex<float> *dMean;
				std::complex<float> *dSumP2;
				
				unsigned polarizationCount;
			private:
				DefaultStatistics(const DefaultStatistics &other) { }
				void operator=(const DefaultStatistics &other) { }
		};
		
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
		
		void Add(unsigned antenna1, unsigned antenna2, double time, unsigned band, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI)
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
		
		void Save(QualityData &qualityData)
		{
			saveTime(qualityData);
			saveFrequency(qualityData);
			saveBaseline(qualityData);
		}
		
		void Load(const QualityData &qualityData)
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
					for(unsigned p=0;p<polarizationCount;++p)
						rfiCount[p] = 0;
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
				Statistics &operator+=(const Statistics other)
				{
					for(unsigned p=0;p<polarizationCount;++p)
					{
						statistics[p] += other.statistics[p];
						differentialStatistics[p] += other.differentialStatistics[p];
						rfiCount[p] += other.rfiCount[p];
					}
					return *this;
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
				
				void Clear()
				{
					_map.clear();
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
		
			void fill(const QualityData &qd)
			{
				kindRFIRatio = qd.QueryKindIndex(QualityData::RFIRatioStatistic),
				kindCount = qd.QueryKindIndex(QualityData::CountStatistic),
				kindMean = qd.QueryKindIndex(QualityData::MeanStatistic),
				kindSumP2 = qd.QueryKindIndex(QualityData::SumP2Statistic),
				kindDCount = qd.QueryKindIndex(QualityData::DCountStatistic),
				kindDMean = qd.QueryKindIndex(QualityData::DMeanStatistic),
				kindDSumP2 = qd.QueryKindIndex(QualityData::DSumP2Statistic);
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
		
		void initializeEmptyStatistics(QualityData &qualityData, QualityData::StatisticDimension dimension)
		{
			qualityData.InitializeEmptyStatistic(dimension, QualityData::RFIRatioStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityData::CountStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityData::MeanStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityData::SumP2Statistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityData::DCountStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityData::DMeanStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityData::DSumP2Statistic);
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
			initializeEmptyStatistics(qd, QualityData::TimeDimension);
			
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
			initializeEmptyStatistics(qd, QualityData::FrequencyDimension);
			
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
			initializeEmptyStatistics(qd, QualityData::BaselineDimension);
			
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
		
		void assignStatistic(Statistics &destination, const StatisticalValue &source, QualityData::StatisticKind kind)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				switch(kind)
				{
					case QualityData::RFIRatioStatistic:
						destination.rfiCount[p] = round((double) destination.statistics[p].Count() / ((1.0/source.Value(p).real())-1.0));
						break;
					case QualityData::CountStatistic:
						destination.statistics[p].SetCount((long unsigned) source.Value(p).real());
						break;
					case QualityData::MeanStatistic:
						destination.statistics[p].SetSum(source.Value(p) * (float) destination.statistics[p].Count());
						break;
					case QualityData::SumP2Statistic:
						destination.statistics[p].SetSum2(source.Value(p));
						break;
					case QualityData::DCountStatistic:
						destination.differentialStatistics[p].SetCount((long unsigned) source.Value(p).real());
						break;
					case QualityData::DMeanStatistic:
						destination.differentialStatistics[p].SetSum(source.Value(p) * (float) destination.differentialStatistics[p].Count());
						break;
					case QualityData::DSumP2Statistic:
						destination.differentialStatistics[p].SetSum2(source.Value(p));
						break;
					default:
						break;
				}
			}
		}
		
		void loadSingleTimeStatistic(const QualityData &qd, QualityData::StatisticKind kind)
		{
			std::vector<std::pair<QualityData::TimePosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryTimeStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityData::TimePosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityData::TimePosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				Statistics &stat = getTimeStatistic(position.time);
				assignStatistic(stat, statValue, kind);
			}
		}
		
		void loadTime(const QualityData &qd)
		{
			loadSingleTimeStatistic(qd, QualityData::CountStatistic);
			loadSingleTimeStatistic(qd, QualityData::MeanStatistic);
			loadSingleTimeStatistic(qd, QualityData::SumP2Statistic);
			loadSingleTimeStatistic(qd, QualityData::DCountStatistic);
			loadSingleTimeStatistic(qd, QualityData::DMeanStatistic);
			loadSingleTimeStatistic(qd, QualityData::DSumP2Statistic);
			loadSingleTimeStatistic(qd, QualityData::RFIRatioStatistic);
		}
		
		void loadSingleFrequencyStatistic(const QualityData &qd, QualityData::StatisticKind kind)
		{
			std::vector<std::pair<QualityData::FrequencyPosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryFrequencyStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityData::FrequencyPosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityData::FrequencyPosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				Statistics &stat = getFrequencyStatistic(position.frequency);
				assignStatistic(stat, statValue, kind);
			}
		}
		
		void loadFrequency(const QualityData &qd)
		{
			loadSingleFrequencyStatistic(qd, QualityData::CountStatistic);
			loadSingleFrequencyStatistic(qd, QualityData::MeanStatistic);
			loadSingleFrequencyStatistic(qd, QualityData::SumP2Statistic);
			loadSingleFrequencyStatistic(qd, QualityData::DCountStatistic);
			loadSingleFrequencyStatistic(qd, QualityData::DMeanStatistic);
			loadSingleFrequencyStatistic(qd, QualityData::DSumP2Statistic);
			loadSingleFrequencyStatistic(qd, QualityData::RFIRatioStatistic);
		}
		
		void loadSingleBaselineStatistic(const QualityData &qd, QualityData::StatisticKind kind)
		{
			std::vector<std::pair<QualityData::BaselinePosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryBaselineStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityData::BaselinePosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityData::BaselinePosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				Statistics &stat = _baselineStatistics.GetStatistics(position.antenna1, position.antenna2);
				assignStatistic(stat, statValue, kind);
			}
		}
		
		void loadBaseline(const QualityData &qd)
		{
			loadSingleBaselineStatistic(qd, QualityData::CountStatistic);
			loadSingleBaselineStatistic(qd, QualityData::MeanStatistic);
			loadSingleBaselineStatistic(qd, QualityData::SumP2Statistic);
			loadSingleBaselineStatistic(qd, QualityData::DCountStatistic);
			loadSingleBaselineStatistic(qd, QualityData::DMeanStatistic);
			loadSingleBaselineStatistic(qd, QualityData::DSumP2Statistic);
			loadSingleBaselineStatistic(qd, QualityData::RFIRatioStatistic);
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
