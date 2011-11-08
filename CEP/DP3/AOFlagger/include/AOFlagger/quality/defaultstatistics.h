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

#endif
