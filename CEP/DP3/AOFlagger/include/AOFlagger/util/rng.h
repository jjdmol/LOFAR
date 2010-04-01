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
#ifndef RNG_H
#define RNG_H

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class RNG{
	public:
		static double Guassian() throw();
		static double GuassianProduct() throw();
		static double GuassianPartialProduct() throw();
		static double GuassianComplex() throw();
		static double Rayleigh() throw();
		static double Uniform() throw();
		static double EvaluateRayleigh(double x, double sigma) throw();
		static long double EvaluateGaussian(long double x, long double sigma) throw();
		static double EvaluateGaussian2D(long double x1, long double x2, long double sigmaX1, long double sigmaX2) throw();
		static double IntegrateGaussian(long double upperLimit) throw();
		static void DoubleGaussian(long double &a, long double &b) throw();
	private: 
		RNG();
};

#endif
