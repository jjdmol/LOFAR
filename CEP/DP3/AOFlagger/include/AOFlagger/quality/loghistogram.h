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
		
		void Add(const double amplitude, bool isRfi)
		{
			if(std::isfinite(amplitude))
			{
				const double centralAmp = getCentralAmplitude(amplitude);
				std::map<double, class AmplitudeBin>::iterator element =
					_amplitudes.find(centralAmp);
				
				AmplitudeBin *bin;
				if(element == _amplitudes.end())
				{
					element = _amplitudes.insert(std::pair<double, AmplitudeBin>(centralAmp, AmplitudeBin())).first;
				}
				bin = &element->second;
				
				++bin->count;
				if(isRfi)
				{
					++bin->rfiCount;
				}
			}
		}
		
		double NormalizedSlope(double startAmplitude, double endAmplitude, enum HistogramType type) const
		{
			unsigned long n = 0;
			long double sumX = 0.0, sumXY = 0.0, sumY = 0.0, sumXSquare = 0.0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=_amplitudes.begin();i!=_amplitudes.end();++i)
			{
				if(i->first >= startAmplitude && i->first < endAmplitude)
				{
					long unsigned count;
					switch(type)
					{
						case TotalAmplitudeHistogram: count = i->second.count + i->second.rfiCount; break;
						case RFIAmplitudeHistogram: count = i->second.rfiCount; break;
						case DataAmplitudeHistogram: count = i->second.count; break;
					}
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
				throw std::runtime_error("Empty histogram");
			return _amplitudes.rbegin()->first;
		}
		
		double MinPositiveAmplitude() const
		{
			std::map<double, AmplitudeBin>::const_iterator i = _amplitudes.begin();
			if(i == _amplitudes.end())
				throw std::runtime_error("Empty histogram");
			while(i->first <= 0.0)
			{
				++i;
				if(i == _amplitudes.end())
					throw std::runtime_error("Histogram does not contain positive values");
			}
			return i->first;
		}
		
		double NormalizedCount(double startAmplitude, double endAmplitude, enum HistogramType type) const
		{
			unsigned long count = 0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=_amplitudes.begin();i!=_amplitudes.end();++i)
			{
				if(i->first >= startAmplitude && i->first < endAmplitude)
				{
					unsigned long thisCount = 0;
					switch(type)
					{
						case TotalAmplitudeHistogram: thisCount = i->second.count + i->second.rfiCount; break;
						case RFIAmplitudeHistogram: thisCount = i->second.rfiCount; break;
						case DataAmplitudeHistogram: thisCount = i->second.count; break;
					}
					count += thisCount;
				}
			}
			return (double) count / (endAmplitude - startAmplitude);
		}
	private:
		struct AmplitudeBin
		{
			AmplitudeBin() :
				count(0), rfiCount(0)
			{
			}
			long unsigned count;
			long unsigned rfiCount;
		};
		
		std::map<double, AmplitudeBin> _amplitudes;
		
		static double getCentralAmplitude(const double amplitude)
		{
			if(amplitude>=0.0)
				return pow(10.0, round(100.0*log10(amplitude))/100.0);
			else
				return -pow(10.0, round(100.0*log10(-amplitude))/100.0);
		}
};

#endif
