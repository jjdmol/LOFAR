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
#ifndef SINUSFITTER_H
#define SINUSFITTER_H

#include <cstring>
#include "math.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class SinusFitter {
	public:
		SinusFitter();
		~SinusFitter();
		void FindPhaseAndAmplitude(long double &phase, long double &amplitude, const long double *dataX, const long double *dataT, const size_t dataSize, const long double frequency) const throw();
		void FindPhaseAndAmplitudeComplex(long double &phase, long double &amplitude, const long double *dataR, const long double *dataI, const long double *dataT, const size_t dataSize, const long double frequency) const throw();


		long double FindMean(const long double phase, const long double amplitude, const long double *dataX, const long double *dataT, const size_t dataSize, const long double frequency);

		static long double Value(const long double phase, const long double amplitude, const long double t, const long double frequency, long double mean)
		{
			return cosl(phase + t * frequency) * amplitude + mean;
		}

		template<typename T> static T Phase(T real, T imaginary)
		{
			if(real==0.0L)
			{
				if(imaginary==0.0L)
					return 0.0L;
				else if(imaginary > 0.0L)
					return M_PIl*0.5L;
				else
					return -M_PIl*0.5L;
			}
			else if(real>0.0L)
			{
				if(imaginary>=0.0L) // first 
					return atanl(imaginary/real);
				else // fourth
					return atanl(imaginary/real)+2.0L*M_PIl;
			}
			else
			{
				if(imaginary>=0.0L) // second
					return atanl(imaginary/real) + 1.0L*M_PIl;
				else // third
					return atanl(imaginary/real) + 1.0L*M_PIl;
			}
		}
	private:
		
};

#endif
