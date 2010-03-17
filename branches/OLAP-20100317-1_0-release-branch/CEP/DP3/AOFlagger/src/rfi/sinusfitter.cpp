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
#include <AOFlagger/rfi/sinusfitter.h>

#include <cmath>

SinusFitter::SinusFitter()
{
}

SinusFitter::~SinusFitter()
{
}

void SinusFitter::FindPhaseAndAmplitude(long double &phase, long double &amplitude, const long double *dataX, const long double *dataT, const size_t dataSize, const long double frequency) const throw()
{
	// calculate 1/N * \sum_t x(t) e^{-i * frequency * t}
	long double sumR = 0.0L, sumI = 0.0L;
	for(unsigned i=0;i<dataSize;++i)
	{
		const long double t = dataT[i];
		const long double x = dataX[i];
		
		sumR += x * cosl(-t * frequency);
		sumI += x * sinl(-t * frequency);
	}

	sumR /= (long double) dataSize;
	sumI /= (long double) dataSize;

	phase = Phase(sumR, sumI);
	amplitude = 2.0L * sqrtl(sumR*sumR + sumI*sumI);
}

void SinusFitter::FindPhaseAndAmplitudeComplex(long double &phase, long double &amplitude, const long double *dataR, const long double *dataI, const long double *dataT, const size_t dataSize, const long double frequency) const throw()
{
	// calculate 1/N * \sum_t x(t) e^{-i * frequency * t}
	long double sumR = 0.0L, sumI = 0.0L;
	for(unsigned i=0;i<dataSize;++i)
	{
		const long double t = dataT[i];
		const long double xR = dataR[i];
		const long double xI = dataI[i];
		
		sumR += xR * cosl(-t * frequency);
		sumR += xI * sinl(-t * frequency);

		sumI += xR * sinl(-t * frequency);
		sumI -= xI * cosl(-t * frequency);
	}

	sumR /= (long double) dataSize;
	sumI /= (long double) dataSize;

	phase = Phase(sumR, sumI);
	amplitude = sqrtl(sumR*sumR + sumI*sumI);
}

long double SinusFitter::FindMean(const long double phase, const long double amplitude, const long double *dataX, const long double *dataT, const size_t dataSize, const long double frequency)
{
	long double sum = 0.0L;
	for(unsigned i=0;i<dataSize;++i)
	{
		sum += dataX[i] - Value(phase, amplitude, dataT[i], frequency, 0.0L);
	}
	return sum / dataSize;
}
