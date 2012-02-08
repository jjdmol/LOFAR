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
#ifndef LOGHISTOGRAM_H
#define LOGHISTOGRAM_H

#include <map>
#include <cmath>
#include <stdexcept>

class LogHistogram
{
	public:
		enum HistogramType
		{
			TotalAmplitudeHistogram,
			RFIAmplitudeHistogram,
			DataAmplitudeHistogram
		};
	private:
		struct AmplitudeBin
		{
			AmplitudeBin() :
				count(0)
			{
			}
			long unsigned count;
			
			long unsigned GetCount() const
			{
				return count;
			}
			
			AmplitudeBin &operator+=(const AmplitudeBin &other)
			{
				count += other.count;
				return *this;
			}
		};
		
	public:
		void Add(const double amplitude)
		{
			if(std::isfinite(amplitude))
			{
				const double centralAmp = getCentralAmplitude(amplitude);
				AmplitudeBin &bin = getBin(centralAmp);
				++bin.count;
			}
		}
		
		void Add(const LogHistogram &histogram)
		{
			for(std::map<double, AmplitudeBin>::const_iterator i=histogram._amplitudes.begin(); i!=histogram._amplitudes.end();++i)
			{
				AmplitudeBin &bin = getBin(i->first);
				bin += i->second;
			}
		}
		
		double NormalizedSlope(double startAmplitude, double endAmplitude) const
		{
			unsigned long n = 0;
			long double sumX = 0.0, sumXY = 0.0, sumY = 0.0, sumXSquare = 0.0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=_amplitudes.begin();i!=_amplitudes.end();++i)
			{
				if(i->first >= startAmplitude && i->first < endAmplitude)
				{
					long unsigned count = i->second.GetCount();
					double x = log10(i->first);
					double y = log10((double) count / i->first);
					++n;
					sumX += x;
					sumXSquare += x * x;
					sumY += y;
					sumXY += x * y;
				}
			}
			return (sumXY - sumX*sumY/n)/(sumXSquare - (sumX*sumX/n));
		}
		
		double MaxAmplitude() const
		{
			if(_amplitudes.empty())
				return 0.0;
			return _amplitudes.rbegin()->first;
		}
		
		double MinPositiveAmplitude() const
		{
			std::map<double, AmplitudeBin>::const_iterator i = _amplitudes.begin();
			if(i == _amplitudes.end())
				return 0.0;
			while(i->first <= 0.0)
			{
				++i;
				if(i == _amplitudes.end())
					return 0.0;
			}
			return i->first;
		}
		
		double NormalizedCount(double startAmplitude, double endAmplitude) const
		{
			unsigned long count = 0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=_amplitudes.begin();i!=_amplitudes.end();++i)
			{
				if(i->first >= startAmplitude && i->first < endAmplitude)
					count += i->second.GetCount();
			}
			return (double) count / (endAmplitude - startAmplitude);
		}
		
		double NormalizedCount(double centreAmplitude) const
		{
			const double key = getCentralAmplitude(centreAmplitude);
			std::map<double, AmplitudeBin>::const_iterator i = _amplitudes.find(key);
			if(i == _amplitudes.end()) return 0.0;
			return (double) i->second.GetCount() / key;
		}
		
		class iterator
		{
			public:
				iterator(LogHistogram &histogram, std::map<double, AmplitudeBin>::iterator iter) :
					_iterator(iter)
				{ }
				iterator(const iterator &source) :
					_iterator(source._iterator)
				{ }
				iterator &operator=(const iterator &source)
				{
					_iterator = source._iterator;
					return *this;
				}
				bool operator==(const iterator &other) const { return other._iterator == _iterator; }
				bool operator!=(const iterator &other) const { return other._iterator != _iterator; }
				iterator &operator++() { ++_iterator; return *this; }
				double value() const { return _iterator->first; }
				double normalizedCount() const { return _iterator->second.GetCount() / value(); }
				double unnormalizedCount() const { return _iterator->second.GetCount(); }
				double binStart() const { return pow10(log10(_iterator->first)-0.005); }
				double binEnd() const { return pow10(log10(_iterator->first)+0.005); }
			private:
				std::map<double, AmplitudeBin>::iterator _iterator;
		};
		
		iterator begin()
		{
			return iterator(*this, _amplitudes.begin());
		}
		
		iterator end()
		{
			return iterator(*this, _amplitudes.end());
		}
		
	private:
		AmplitudeBin &getBin(double centralAmplitude)
		{
			std::map<double, class AmplitudeBin>::iterator element =
				_amplitudes.find(centralAmplitude);
			
			if(element == _amplitudes.end())
			{
				element = _amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmplitude, AmplitudeBin())).first;
			}
			return element->second;
		}
		
		std::map<double, AmplitudeBin> _amplitudes;
		
		static double getCentralAmplitude(const double amplitude)
		{
			if(amplitude>=0.0)
				return pow10(round(100.0*log10(amplitude))/100.0);
			else
				return -pow10(round(100.0*log10(-amplitude))/100.0);
		}
};

#endif
