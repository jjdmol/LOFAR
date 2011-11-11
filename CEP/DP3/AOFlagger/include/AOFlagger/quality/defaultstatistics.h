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
#ifndef QUALITY__DEFAULT_STATISTICS_H
#define QUALITY__DEFAULT_STATISTICS_H

#include <complex>
#include <stdint.h>

class DefaultStatistics
{
	public:
		DefaultStatistics(unsigned polarizationCount) :
			_polarizationCount(polarizationCount)
		{
			initialize();
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] = 0;
				count[p] = 0;
				sum[p] = 0.0;
				sumP2[p] = 0.0;
				dCount[p] = 0;
				dSum[p] = 0.0;
				dSumP2[p] = 0.0;
			}
		}
		
		~DefaultStatistics()
		{
			destruct();
		}
		
		DefaultStatistics(const DefaultStatistics &other)
		: _polarizationCount(other._polarizationCount)
		{
			initialize();
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] = other.rfiCount[p];
				count[p] = other.count[p];
				sum[p] = other.sum[p];
				sumP2[p] = other.sumP2[p];
				dCount[p] = other.dCount[p];
				dSum[p] = other.dSum[p];
				dSumP2[p] = other.dSumP2[p];
			}
		}
		
		DefaultStatistics &operator=(const DefaultStatistics &other)
		{
			if(other._polarizationCount != _polarizationCount)
			{
				destruct();
				_polarizationCount = other._polarizationCount;
				initialize();
			}
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] = other.rfiCount[p];
				count[p] = other.count[p];
				sum[p] = other.sum[p];
				sumP2[p] = other.sumP2[p];
				dCount[p] = other.dCount[p];
				dSum[p] = other.dSum[p];
				dSumP2[p] = other.dSumP2[p];
			}
			return *this;
		}
		
		DefaultStatistics &operator+=(const DefaultStatistics &other)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				rfiCount[p] += other.rfiCount[p];
				count[p] += other.count[p];
				sum[p] += other.sum[p];
				sumP2[p] += other.sumP2[p];
				dCount[p] += other.dCount[p];
				dSum[p] += other.dSum[p];
				dSumP2[p] += other.dSumP2[p];
			}
			return *this;
		}
		
		void Serialize(std::ostream &stream) const
		{
			const uint64_t polarizationCount = _polarizationCount;
			stream.write(reinterpret_cast<const char*>(&polarizationCount), sizeof(polarizationCount));
			
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				const uint64_t rfiCount_ = rfiCount[p];
				stream.write(reinterpret_cast<const char*>(&rfiCount_), sizeof(rfiCount_));
				
				const uint64_t count_ = count[p];
				stream.write(reinterpret_cast<const char*>(&count_), sizeof(count_));
				
				stream.write(reinterpret_cast<const char*>(&sum[p]), sizeof(sum[p]));
				stream.write(reinterpret_cast<const char*>(&sumP2[p]), sizeof(sumP2[p]));
				
				const uint64_t dCount_ = count[p];
				stream.write(reinterpret_cast<const char*>(&dCount_), sizeof(dCount_));
				
				stream.write(reinterpret_cast<const char*>(&dSum[p]), sizeof(dSum[p]));
				stream.write(reinterpret_cast<const char*>(&dSumP2[p]), sizeof(dSumP2[p]));
			}
		}
		
		unsigned PolarizationCount() const
		{
			return _polarizationCount;
		}
		
		template<typename T>
		std::complex<T> Mean(unsigned polarization) const
		{
			return std::complex<T>(sum[polarization].real() / count[polarization], sum[polarization].imag() / count[polarization]);
		}
		
		template<typename T>
		std::complex<T> SumP2(unsigned polarization) const
		{
			return std::complex<T>(sumP2[polarization].real(), sumP2[polarization].imag());
		}
		
		template<typename T>
		std::complex<T> DMean(unsigned polarization) const
		{
			return std::complex<T>(dSum[polarization].real() / dCount[polarization], dSum[polarization].imag() / dCount[polarization]);
		}
		
		template<typename T>
		std::complex<T> DSumP2(unsigned polarization) const
		{
			return std::complex<T>(dSumP2[polarization].real(), dSumP2[polarization].imag());
		}
		
		unsigned long *rfiCount;
		unsigned long *count;
		std::complex<long double> *sum;
		std::complex<long double> *sumP2;
		unsigned long *dCount;
		std::complex<long double> *dSum;
		std::complex<long double> *dSumP2;
		
	private:
		void initialize()
		{
			rfiCount = new unsigned long[_polarizationCount];
			count = new unsigned long[_polarizationCount];
			sum = new std::complex<long double>[_polarizationCount];
			sumP2 = new std::complex<long double>[_polarizationCount];
			dCount = new unsigned long[_polarizationCount];
			dSum = new std::complex<long double>[_polarizationCount];
			dSumP2 = new std::complex<long double>[_polarizationCount];
		}
		
		void destruct()
		{
			delete[] rfiCount;
			delete[] count;
			delete[] sum;
			delete[] sumP2;
			delete[] dCount;
			delete[] dSum;
			delete[] dSumP2;
		}
		
		unsigned _polarizationCount;
};

#endif
