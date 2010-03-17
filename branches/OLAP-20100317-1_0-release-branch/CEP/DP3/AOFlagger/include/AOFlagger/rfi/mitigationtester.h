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
#ifndef MITIGATIONTESTER_H
#define MITIGATIONTESTER_H

#include <cstddef>
#include <vector>
#include <fstream>

#include "../msio/image2d.h"
#include "../msio/mask2d.h"

#include "../util/rng.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class MitigationTester{
	public:
		enum NoiseType { Gaussian, GaussianProduct, GaussianPartialProduct, Rayleigh };
	
		MitigationTester();

		~MitigationTester();

		void GenerateNoise(size_t scanCount, size_t frequencyCount, bool independentComplex = false, double sigma=1.0, enum NoiseType noiseType=GaussianPartialProduct);

		static class Image2D *CreateRayleighData(unsigned width, unsigned height);
		static class Image2D *CreateGaussianData(unsigned width, unsigned height);
		static class Image2D *CreateNoise(unsigned width, unsigned height, bool gaussian)
		{
			if(gaussian)
				return CreateGaussianData(width, height);
			else
				return CreateRayleighData(width, height);
		}

		void AddBroadbandLine(double lineStrength, size_t &rfiCount)
		{
			AddBroadbandLine(lineStrength, rfiCount, 1.0);
		}
		void AddBroadbandLine(double lineStrength, size_t &rfiCount, double frequencyRatio);

		static void AddBroadbandLine(Image2DPtr data, Mask2DPtr rfi, double lineStrength, size_t startTime, size_t duration)
		{
			AddBroadbandLine(data, rfi, lineStrength, startTime, duration, 1.0);
		}
		static void AddBroadbandLine(Image2DPtr data, Mask2DPtr rfi, double lineStrength, size_t startTime, size_t duration, double frequencyRatio)
		{
			AddBroadbandLine(data, rfi, lineStrength, startTime, duration, frequencyRatio, 0.5L - frequencyRatio/2.0L);
		}
		static void AddBroadbandLine(Image2DPtr data, Mask2DPtr rfi, double lineStrength, size_t startTime, size_t duration, double frequencyRatio, double frequencyOffsetRatio);

		static void AddRandomBroadbandLine(Image2DPtr data, Mask2DPtr rfi, double lineStrength, size_t startTime, size_t duration);

		void AddRFI(size_t &rfiCount);

		static void CountResults(Mask2DCPtr thresholdedMask, Mask2DCPtr originalRFI, size_t &correct, size_t &notfound, size_t &error);
		void CountCorrectRFI(Image2DCPtr tresholdedReal, Image2DCPtr tresholdedImaginary, size_t &correct, size_t &notfound, size_t &error);

		void SetZero();

		template<typename T1,typename T2>
		static void SaveGraph(const std::string &filename, const std::vector<T1> &x, const std::vector<T2> &y) throw();

		Image2DCPtr Real() const throw() { return _real; }

		Image2DCPtr Imaginary() const throw() { return _imaginary; }

		static std::string GetTestSetDescription(int number);
		static Image2DPtr CreateTestSet(int number, Mask2DPtr rfi, unsigned width, unsigned height, bool gaussianNoise = false);

	private:
		static void AddBroadbandToTestSet(Image2DPtr image, Mask2DPtr rfi, long double length);
		static void AddVarBroadbandToTestSet(Image2DPtr image, Mask2DPtr rfi);
		static void AddModelData(Image2DPtr image, unsigned sources);
		static void SubtractBackground(Image2DPtr image);

		static double Rand(enum NoiseType type) {
			switch(type) {
				case Gaussian: return RNG::Guassian();
				case GaussianProduct: return RNG::GuassianProduct();
				case GaussianPartialProduct: return RNG::GuassianPartialProduct();
				case Rayleigh: return RNG::Rayleigh();
			}
			throw std::exception();
		}

		Image2DPtr _real, _imaginary;
		void Clear();
};

template<typename T1,typename T2>
void MitigationTester::SaveGraph(const std::string &filename, const std::vector<T1> &x, const std::vector<T2> &y) throw()
{
	std::ofstream file(filename.c_str());
	for(size_t i=0;i<x.size();++i) {
		file << i << "\t" << x[i] << "\t" << y[i] << std::endl;
	}
	file.close();
}

#endif
