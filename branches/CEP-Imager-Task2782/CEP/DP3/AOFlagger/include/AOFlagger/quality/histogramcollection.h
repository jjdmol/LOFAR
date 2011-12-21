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
#include <map>
#include <vector>

class HistogramCollection
{
	public:
		typedef std::pair<unsigned, unsigned> AntennaPair;
		
		HistogramCollection(unsigned polarizationCount) : _polarizationCount(polarizationCount)
		{
			_histograms = new std::map<AntennaPair, LogHistogram*>[polarizationCount];
		}
		
		~HistogramCollection()
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				for(std::map<AntennaPair, LogHistogram*>::iterator i=_histograms[p].begin(); i!=_histograms[p].end(); ++i)
				{
					delete i->second;
				}
			}
			delete[] _histograms;
		}
		
		void Add(const unsigned antenna1, const unsigned antenna2, const unsigned polarization, const std::complex<float> *values, const bool *isRFI, size_t sampleCount)
		{
			LogHistogram &histogram = GetHistogram(antenna1, antenna2, polarization);
			for(size_t i=0;i<sampleCount;++i)
			{
				const double amplitude = sqrt(values[i].real()*values[i].real() + values[i].imag()*values[i].imag());
				histogram.Add(amplitude, isRFI[i]);
			}
		}
		
		LogHistogram &GetHistogram(const unsigned a1, const unsigned a2, const unsigned polarization)
		{
			const AntennaPair antennae(a1, a2);
			std::map<AntennaPair, LogHistogram*>::iterator i = _histograms[polarization].find(antennae);
			if(i == _histograms[polarization].end())
			{
				i = _histograms[polarization].insert(std::pair<AntennaPair, LogHistogram*>(antennae, new LogHistogram())).first;
			}
			return *i->second;
		}
		
		void GetHistogramForCrossCorrelations(const unsigned polarization, LogHistogram &target)
		{
			for(unsigned p=0;p<_polarizationCount;++p)
			{
				for(std::map<AntennaPair, LogHistogram*>::iterator i=_histograms[p].begin(); i!=_histograms[p].end(); ++i)
				{
					if(i->first.first != i->first.second)
						target.Add(*i->second);
				}
			}
		}
		
		const std::map<AntennaPair, LogHistogram*> GetHistograms(const unsigned polarization) const
		{
			return _histograms[polarization];
		}
		
	private:
		unsigned _polarizationCount;
		std::map<AntennaPair, LogHistogram*> *_histograms;
};

#endif
