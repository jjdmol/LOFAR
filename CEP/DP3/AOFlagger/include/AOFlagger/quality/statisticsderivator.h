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

#include <AOFlagger/strategy/algorithms/cnoisestatistics.h>

class StatisticsDerivator
{
	public:
		StatisticsDerivator(const StatisticsCollection &collection) :
			_collection(collection)
		{
		}
		
		std::complex<float> GetComplexBaselineStatistic(QualityTablesFormatter::StatisticKind kind, unsigned antenna1, unsigned antenna2, unsigned polarization) const
		{
			const Statistics &statistics = _collection.BaselineStatistics().GetStatistics(antenna1, antenna2);
			return deriveComplex(kind, statistics, polarization);
		}
		
		std::complex<float> GetComplexTimeStatistic(QualityTablesFormatter::StatisticKind kind, double time, unsigned polarization) const
		{
			const Statistics &statistics = _collection.TimeStatistics().find(time)->second;
			return deriveComplex(kind, statistics, polarization);
		}
	
		std::complex<float> GetComplexFrequencyStatistic(QualityTablesFormatter::StatisticKind kind, double frequency, unsigned polarization) const
		{
			const Statistics &statistics = _collection.FrequencyStatistics().find(frequency)->second;
			return deriveComplex(kind, statistics, polarization);
		}
	
		std::complex<float> GetComplexStatistic(QualityTablesFormatter::StatisticKind kind, const Statistics &statistics, unsigned polarization) const
		{
			return deriveComplex(kind, statistics, polarization);
		}
		
		static std::complex<float> GetVariance(unsigned long n, std::complex<float> mean, std::complex<float> sumP2)
		{
			return deriveVariance(n, mean, sumP2);
		}
		
		static double GetVarianceAmplitude(unsigned long n, std::complex<float> mean, std::complex<float> sumP2)
		{
			const std::complex<float> variance = deriveVariance(n, mean, sumP2);
			return sqrt(variance.real()*variance.real() + variance.imag()*variance.imag());
		}
	private:
		std::complex<float> deriveComplex(QualityTablesFormatter::StatisticKind kind, const Statistics &statistics, unsigned polarization) const
		{
			switch(kind)
			{
				case QualityTablesFormatter::CountStatistic:
					return std::complex<float>(statistics.statistics[polarization].Count(), 0.0);
					break;
				case QualityTablesFormatter::MeanStatistic:
					return statistics.statistics[polarization].Mean();
					break;
				case QualityTablesFormatter::SumP2Statistic:
					return statistics.statistics[polarization].Sum2();
					break;
				case QualityTablesFormatter::VarianceStatistic:
					return deriveVariance(statistics.statistics[polarization].Count(),
																statistics.statistics[polarization].Mean(),
																statistics.statistics[polarization].Sum2());
					break;
				case QualityTablesFormatter::DCountStatistic:
					return std::complex<float>(statistics.differentialStatistics[polarization].Count(), 0.0f);
					break;
				case QualityTablesFormatter::DMeanStatistic:
					return statistics.differentialStatistics[polarization].Mean();
					break;
				case QualityTablesFormatter::DSumP2Statistic:
					return statistics.differentialStatistics[polarization].Sum2();
					break;
				case QualityTablesFormatter::DVarianceStatistic:
					return deriveVariance(statistics.differentialStatistics[polarization].Count(),
																statistics.differentialStatistics[polarization].Mean(),
																statistics.differentialStatistics[polarization].Sum2());
					break;
				case QualityTablesFormatter::RFIRatioStatistic:
					return std::complex<float>((double) statistics.rfiCount[polarization] / (statistics.statistics[polarization].Count() + statistics.rfiCount[polarization]), 0.0f);
					break;
				case QualityTablesFormatter::RFICountStatistic:
					return std::complex<float>(statistics.rfiCount[polarization], 0.0f);
					break;
				default:
					throw std::runtime_error("Can not derive requested statistic");
			}
		}
		
		static std::complex<float> deriveVariance(unsigned long n, std::complex<float> mean, std::complex<float> sumP2)
		{
			return std::complex<float>(deriveVariance(n, mean.real(), sumP2.real()),
																 deriveVariance(n, mean.imag(), sumP2.imag()));
		}
		
		static double deriveVariance(unsigned long n, double mean, double sumP2)
		{
			const double sumMeanSquared = mean * mean * n;
			return (sumP2 + sumMeanSquared - (mean * n * 2.0 * mean)) / (n-1.0);
		}
		
		const StatisticsCollection &_collection;
};

#endif
