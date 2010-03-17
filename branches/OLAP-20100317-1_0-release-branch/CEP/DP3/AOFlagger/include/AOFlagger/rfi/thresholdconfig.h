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
#ifndef THRESHOLDCONFIG_H
#define THRESHOLDCONFIG_H

#include <vector>

#include "../msio/image2d.h"
#include "../msio/mask2d.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ThresholdConfig {
	public:
		enum Method { SumThreshold, VarThreshold };
		enum Distribution { Uniform, Gaussian, Rayleigh };
		ThresholdConfig();
		~ThresholdConfig();
		void InitializeLengthsDefault(unsigned count=0);
		void InitializeLengthsFrom(unsigned count=0, unsigned startLength=0);
		void InitializeLengthsSingleSample();
		void InitializeThresholdsFromFirstThreshold(long double firstThreshold, enum Distribution noiseDistribution);
		void InitializeThresholdsWithFalseRate(size_t resolution, long double falseAlarmRate, enum Distribution noiseDistribution);
		long double CalculateFalseAlarmRate(size_t resolution, enum Distribution noiseDistribution);
		void Execute(Image2DCPtr image, Mask2DPtr mask, bool additive, long double sensitivity) const;
		void SetVerbose(bool verbose) throw() { _verbose = verbose; }
		void SetMethod(Method method) throw() { _method = method; }
		void SetExpThresholdFactor(long double expFactor) throw() { _expFactor=expFactor; }
		void SetFitBackground(class SurfaceFitMethod *method) throw() { _fitMethod = method; }
		long double GetThreshold(unsigned index) const throw() { return _operations[index].threshold; }
		void SetThreshold(unsigned index, long double threshold) throw() { _operations[index].threshold = threshold; }
		size_t GetLength(unsigned index) const throw() { return _operations[index].length; }
		size_t GetOperations() const throw() { return _operations.size(); }
		void SetMinimumConnectedSamples(size_t val) throw() { _minConnectedSamples = val; }
	private:
		void BinarySearch(long double probability, long double accuracy, size_t resolution);
		struct ThresholdOperation {
			size_t length;
			long double threshold;
			long double rate;
		};
		std::vector<ThresholdOperation> _operations;
		Method _method;
		Distribution _distribution;
		bool _verbose;
		long double _expFactor;
		class SurfaceFitMethod *_fitMethod;
		size_t _minConnectedSamples;
};

#endif
