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
#ifndef HISTOGRAM_COLLECTION_H
#define HISTOGRAM_COLLECTION_H

#include "loghistogram.h"
#include <complex>

class HistogramCollection
{
	public:
		void Add(const unsigned antenna1, const unsigned antenna2, const std::complex<float> sample, const bool isRFI)
		{
			LogHistogram &histogram = getHistogram(antenna1, antenna2);
			const double amplitude = sample.real()*sample.real() + sample.imag()
			histogram
		}
	private:
		LogHistogram &getHistogram(const unsigned a1, const unsigned a2)
		{
			const AntennaPair antennae(a1, a2);
			std::map<AntennaPair, LogHistogram*>::iterator i = _histograms.find(antennae);
			if(i == _histograms.end())
			{
				i = _histograms.insert(std::pair<AntennaPair, LogHistogram*>(antennae, new LogHistogram())).first;
			}
			return *i->second;
		}
		
		typedef std::pair<unsigned, unsigned> AntennaPair;
		std::map<AntennaPair, LogHistogram*> _histograms;
};

#endif
