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
#ifndef QUALITY__STATISTICS_DERIVATOR_H
#define QUALITY__STATISTICS_DERIVATOR_H

#include "statisticscollection.h"

#include <complex>
#include <set>

#include <AOFlagger/msio/timefrequencydata.h>

class StatisticsDerivator
{
	public:
		StatisticsDerivator(const StatisticsCollection &collection) :
			_collection(collection)
		{
		}
		
		std::complex<long double> GetComplexBaselineStatistic(QualityTablesFormatter::StatisticKind kind, unsigned antenna1, unsigned antenna2, unsigned polarization) const
		{
			const DefaultStatistics &statistics = _collection.BaselineStatistics().GetStatistics(antenna1, antenna2);
			return deriveComplex<long double>(kind, statistics, polarization);
		}
		
		std::complex<long double> GetComplexTimeStatistic(QualityTablesFormatter::StatisticKind kind, double time, unsigned polarization) const
		{
			const DefaultStatistics &statistics = _collection.TimeStatistics().find(time)->second;
			return deriveComplex<long double>(kind, statistics, polarization);
		}
	
		std::complex<long double> GetComplexFrequencyStatistic(QualityTablesFormatter::StatisticKind kind, double frequency, unsigned polarization) const
		{
			const DefaultStatistics &statistics = _collection.FrequencyStatistics().find(frequency)->second;
			return deriveComplex<long double>(kind, statistics, polarization);
		}
	
		std::complex<long double> GetComplexStatistic(QualityTablesFormatter::StatisticKind kind, const DefaultStatistics &statistics, unsigned polarization) const
		{
			return deriveComplex<long double>(kind, statistics, polarization);
		}
		
		static std::complex<long double> Variance(unsigned long n, std::complex<long double> sum, std::complex<long double> sumP2)
		{
			return deriveVariance<long double>(n, sum, sumP2);
		}
		
		static long double VarianceAmplitude(unsigned long n, std::complex<long double> sum, std::complex<long double> sumP2)
		{
			const std::complex<long double> variance = deriveVariance(n, sum, sumP2);
			return sqrt(variance.real()*variance.real() + variance.imag()*variance.imag());
		}
		
		TimeFrequencyData CreateTFData()
		{
			const std::map<double, std::map<double, DefaultStatistics> > &map = _collection.AllTimeStatistics();
			std::set<double> frequencies;
			std::set<double> timesteps;
			for(std::map<double, std::map<double, DefaultStatistics> >::const_iterator i=map.begin();i!=map.end();++i)
			{
				const double frequency = i->first;
				const std::map<double, DefaultStatistics> &innerMap = i->second;
				for(std::map<double, DefaultStatistics>::const_iterator j=innerMap.begin();j!=innerMap.end();++j)
				{
					const double time = j->first;
					
				}
			}
		}
	private:
		template<typename T>
		std::complex<T> deriveComplex(QualityTablesFormatter::StatisticKind kind, const DefaultStatistics &statistics, unsigned polarization) const
		{
			switch(kind)
			{
				case QualityTablesFormatter::CountStatistic:
					return std::complex<T>(statistics.count[polarization], 0.0);
					break;
				case QualityTablesFormatter::MeanStatistic:
					return statistics.Mean<T>(polarization);
					break;
				case QualityTablesFormatter::SumP2Statistic:
					return statistics.SumP2<T>(polarization);
					break;
				case QualityTablesFormatter::VarianceStatistic:
					return deriveVariance(statistics.count[polarization],
																statistics.sum[polarization],
																statistics.sumP2[polarization]);
					break;
				case QualityTablesFormatter::DCountStatistic:
					return std::complex<T>(statistics.dCount[polarization], 0.0f);
					break;
				case QualityTablesFormatter::DMeanStatistic:
					return statistics.DMean<T>(polarization);
					break;
				case QualityTablesFormatter::DSumP2Statistic:
					return statistics.DSumP2<T>(polarization);
					break;
				case QualityTablesFormatter::DVarianceStatistic:
					return deriveVariance(statistics.dCount[polarization],
																statistics.dSum[polarization],
																statistics.dSumP2[polarization]);
					break;
				case QualityTablesFormatter::RFIRatioStatistic:
					return std::complex<T>((double) statistics.rfiCount[polarization] / (statistics.count[polarization] + statistics.rfiCount[polarization]), 0.0f);
					break;
				case QualityTablesFormatter::RFICountStatistic:
					return std::complex<T>(statistics.rfiCount[polarization], 0.0f);
					break;
				case QualityTablesFormatter::SignalToNoiseStatistic:
				{
					const std::complex<T> variance = deriveComplex<T>(QualityTablesFormatter::DVarianceStatistic, statistics, polarization);
					return std::complex<T>(statistics.Mean<T>(polarization).real() / variance.real(), statistics.Mean<T>(polarization).imag() / variance.imag());
					break;
				}
				default:
					throw std::runtime_error("Can not derive requested statistic");
			}
		}
		
		template<typename T>
		static std::complex<T> deriveVariance(unsigned long n, std::complex<T> sum, std::complex<T> sumP2)
		{
			return std::complex<float>(deriveVariance(n, sum.real(), sumP2.real()),
																 deriveVariance(n, sum.imag(), sumP2.imag()));
		}
		
		template<typename T>
		static T deriveVariance(unsigned long n, T sum, T sumP2)
		{
			T sumMeanSquared = sum * sum / n;
			return (sumP2 - sumMeanSquared) / (n-1.0);
		}
		
		const StatisticsCollection &_collection;
};

#endif
