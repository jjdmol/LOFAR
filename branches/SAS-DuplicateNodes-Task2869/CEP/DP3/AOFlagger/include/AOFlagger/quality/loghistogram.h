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

class LogHistogram
{
	public:
		Add(const double amplitude, bool isRfi)
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
