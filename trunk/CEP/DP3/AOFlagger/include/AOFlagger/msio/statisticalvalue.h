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
#ifndef MSIO_STATISTICAL_VALUE_H
#define MSIO_STATISTICAL_VALUE_H

#include <ms/MeasurementSets/MeasurementSet.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/

struct StatisticalValue {
	StatisticalValue(unsigned _polarizationCount) :
		polarizationCount(_polarizationCount),
		values(new std::complex<float>[_polarizationCount])
	{
	}
	
	StatisticalValue(const StatisticalValue &source) :
		polarizationCount(source.polarizationCount),
		values(new std::complex<float>[source.polarizationCount])
	{
		kindIndex = source.kindIndex;
		for(unsigned i=0;i<polarizationCount;++i)
			values[i] = source.values[i];
	}
	
	~StatisticalValue()
	{
		delete[] values;
	}
	
	StatisticalValue &operator=(const StatisticalValue &source)
	{
		if(polarizationCount != source.polarizationCount)
		{
			polarizationCount = source.polarizationCount;
			delete[] values;
			values = new std::complex<float>[polarizationCount];
		}
		kindIndex = source.kindIndex;
		for(unsigned i=0;i<polarizationCount;++i)
			values[i] = source.values[i];
		return *this;
	}
	
	unsigned polarizationCount;
	int kindIndex;
	std::complex<float> *values;
};

#endif
