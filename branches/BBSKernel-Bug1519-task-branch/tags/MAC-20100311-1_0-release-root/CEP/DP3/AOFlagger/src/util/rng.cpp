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
#include <AOFlagger/util/rng.h>

#include <stdlib.h>
#include <math.h>

#include <AOFlagger/msio/image2d.h>

RNG::RNG()
{
}

double RNG::Uniform() throw()
{
	return (long double) rand() / (long double) RAND_MAX;
}

double RNG::Guassian() throw()
{
	long double a, b;
	DoubleGaussian(a, b);
	return a;
}

double RNG::GuassianProduct() throw()
{
	long double a, b;
	DoubleGaussian(a, b);
	return a * b;
}

double RNG::GuassianPartialProduct() throw()
{
	long double a, b;
	DoubleGaussian(a, b);
	if(a >= 0.0)
		a = pow(a, sqrt(2.0)/2.0);
	else
		a = -pow(-a, sqrt(2.0)/2.0);
	if(b >= 0.0)
		b = pow(b, sqrt(2.0)/2.0);
	else
		b = -pow(-b, sqrt(2.0)/2.0);
	return a*b;
}

void RNG::DoubleGaussian(long double &a, long double &b) throw()
{
	long double x1, x2, w;
	
	do {
		long double r1 = (long double) rand() / (long double) RAND_MAX; 
		long double r2 = (long double) rand() / (long double) RAND_MAX; 
		x1 = 2.0 * r1 - 1.0;
		x2 = 2.0 * r2 - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = sqrt( (-2.0 * logl( w ) ) / w );
	a = x1 * w;
	b = x2 * w;
}

double RNG::Rayleigh() throw()
{
	double x = Guassian(), y = Guassian();
	return sqrt(x*x + y*y);
}

double RNG::EvaluateRayleigh(double x, double sigma) throw()
{
	return x * exp(-x*x/(2.0*sigma*sigma)) / (sigma * sigma);
}

double RNG::IntegrateGaussian(long double upperLimit) throw()
{
	long double integral = 0.0L, term = 0.0L, stepSize = 1e-4L;
	upperLimit -= stepSize/2.0L;
	do {
		term = expl(-upperLimit * upperLimit / 2.0L);
		upperLimit -= stepSize;
		integral += term * stepSize;
	} while(term >= 1e-6 || upperLimit >= 0);
	return integral / sqrt(2.0L * M_PI);
}

long double RNG::EvaluateGaussian(long double x, long double sigma) throw()
{
	return 1.0L / (sigma * sqrt(2.0L*M_PI)) * exp(-0.5L*x*x/sigma);
}

double RNG::EvaluateGaussian2D(long double x1, long double x2, long double sigmaX1, long double sigmaX2) throw()
{
	return 1.0L / (2.0L*M_PI*sigmaX1*sigmaX2) * exp(-0.5L*(x1*(1.0L/sigmaX1)*x1 + x2*(1.0L/sigmaX2)*x2));
}
