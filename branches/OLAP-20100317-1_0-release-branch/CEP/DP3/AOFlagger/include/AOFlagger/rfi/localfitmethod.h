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
#ifndef LocalFitMethod_H
#define LocalFitMethod_H

#include <string>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlin.h>

#include <boost/thread/mutex.hpp>

#include "image.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class LocalFitMethod : public SurfaceFitMethod {
	public:
		enum Method { None, LeastSquare, LeastAbs, Average, GaussianWeightedAverage, FastGaussianWeightedAverage, Median, Minimum };
		LocalFitMethod();
		~LocalFitMethod();
		void SetToAverage(unsigned hSquareSize, unsigned vSquareSize)
		{
			ClearWeights();
			_hSquareSize = hSquareSize;
			_vSquareSize = vSquareSize;
			_precision = 0.0;
			_method = Average;
		}
		void SetToWeightedAverage(unsigned hSquareSize, unsigned vSquareSize, long double hKernelSize, long double vKernelSize)
		{
			ClearWeights();
			_hSquareSize = hSquareSize;
			_vSquareSize = vSquareSize;
			_precision = 0.0;
			_method = FastGaussianWeightedAverage;
			_hKernelSize = hKernelSize;
			_vKernelSize = vKernelSize;
		}
		void SetToMedianFilter(unsigned hSquareSize, unsigned vSquareSize)
		{
			ClearWeights();
			_hSquareSize = hSquareSize;
			_vSquareSize = vSquareSize;
			_method = Median;
			_precision = 0.0;
		}
		void SetToMinimumFilter(unsigned hSquareSize, unsigned vSquareSize)
		{
			ClearWeights();
			_hSquareSize = hSquareSize;
			_vSquareSize = vSquareSize;
			_method = Minimum;
			_precision = 0.0;
		}
		void SetToNone()
		{
			ClearWeights();
			_method = None;
		}
		void SetParameters(unsigned hSquareSize, unsigned vSquareSize, long double precision, enum Method method)
		{
			ClearWeights();
			_hSquareSize = hSquareSize;
			_vSquareSize = vSquareSize;
			_precision = precision;
			_method = method;
		}
		virtual void Initialize(const TimeFrequencyData &input);
		virtual unsigned TaskCount();
		virtual void PerformFit(unsigned taskNumber);
		virtual TimeFrequencyData Background() { return *_background; }
		virtual enum TimeFrequencyData::PhaseRepresentation PhaseRepresentation() const
		{
			return TimeFrequencyData::AmplitudePart;
		}
	private:
		struct ThreadLocal {
			LocalFitMethod *image;
			unsigned currentX, currentY;
			unsigned startX, startY, endX, endY;
			size_t emptyWindows;
		};
		long double Evaluate(unsigned x, unsigned y, long double *coefficients);
		static int SquareError(const gsl_vector * coefs, void *data, gsl_vector * f);
		static int SquareErrorDiff(const gsl_vector * x, void *data, gsl_matrix * J);
		static int SquareErrorComb(const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)
		{
			SquareError(x, data, f);
			SquareErrorDiff(x, data, J);
			return GSL_SUCCESS;
		}
		static int LinError(const gsl_vector * coefs, void *data, gsl_vector * f);
		static int LinErrorDiff(const gsl_vector * x, void *data, gsl_matrix * J);
		static int LinErrorComb(const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)
		{
			LinError(x, data, f);
			LinErrorDiff(x, data, J);
			return GSL_SUCCESS;
		}
		long double CalculateBackgroundValue(unsigned x, unsigned y);
		long double FitBackground(unsigned x, unsigned y, ThreadLocal &local);
		long double CalculateAverage(unsigned x, unsigned y, ThreadLocal &local);
		long double CalculateMedian(unsigned x, unsigned y, ThreadLocal &local);
		long double CalculateMinimum(unsigned x, unsigned y, ThreadLocal &local);
		long double CalculateWeightedAverage(unsigned x, unsigned y, ThreadLocal &local);
		void ClearWeights();
		void InitializeGaussianWeights();
		void PerformGaussianConvolution(Image2DPtr input);
		void CalculateWeightedAverageFast();
		Image2DPtr CreateFlagWeightsMatrix();
		void ElementWiseDivide(Image2DPtr leftHand, Image2DCPtr rightHand);

		Image2DCPtr _original;
		class TimeFrequencyData *_background;
		Image2DPtr _background2D;
		Mask2DCPtr _mask;
		unsigned _hSquareSize, _vSquareSize;
		long double _precision;
		long double *_previousCoefficients;
		num_t **_weights;
		long double _hKernelSize, _vKernelSize;
		boost::mutex _mutex;
		enum Method _method;
};

#endif
