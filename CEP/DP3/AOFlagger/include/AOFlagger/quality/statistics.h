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
#ifndef QUALITY__STATISTICS_H
#define QUALITY__STATISTICS_H

class Statistics
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
		Statistics &operator=(const Statistics &source)
		{
			for(unsigned p=0;p<polarizationCount;++p)
			{
				statistics[p] = source.statistics[p];
				differentialStatistics[p] = source.differentialStatistics[p];
				rfiCount[p] = source.rfiCount[p];
			}
			return *this;
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
};

#endif
