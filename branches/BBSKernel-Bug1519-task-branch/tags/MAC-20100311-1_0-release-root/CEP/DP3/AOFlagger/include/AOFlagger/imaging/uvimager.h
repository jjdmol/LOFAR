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
#ifndef UVIMAGER_H
#define UVIMAGER_H

#include "../msio/timefrequencymetadata.h"
#include "../msio/measurementset.h"
#include "../msio/date.h"

struct SingleFrequencySingleBaselineData {
	casa::Complex data;
	bool flag;
	bool available;
	double time;
	unsigned field;
};

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class UVImager {
	public:
		enum ImageKind { Homogeneous, Flagging };
		UVImager(unsigned long xRes, unsigned long yRes, ImageKind imageKind=Homogeneous);
		~UVImager();
		void Image(class MeasurementSet &measurementSet, unsigned band);
		void Image(class MeasurementSet &measurementSet, unsigned band, const class IntegerDomain &frequencies);
		void Image(const class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, unsigned frequencyIndex);
		void InverseImage(class MeasurementSet &prototype, unsigned band, const class Image2D &uvReal, const class Image2D &uvImaginary, unsigned antenna1, unsigned antenna2);
		const class Image2D &WeightImage() const throw() { return *_uvWeights; }
		const class Image2D &RealUVImage() const throw() { return *_uvReal; }
		const class Image2D &ImaginaryUVImage() const throw() { return *_uvImaginary; }
		void SetInvertFlagging(bool newValue) throw() { _invertFlagging = newValue; }
		void SetDirectFT(bool directFT) throw() { _directFT = directFT; }

		/**
		 * This function calculates the uv position, but it's not optimized for speed, so it's not to be used in an imager.
		 * @param u the u position (in the uv-plane domain)
		 * @param v the v position (in the uv-plane domain)
		 * @param baseline information about the baseline
		 * @param time the time to calculate the u,v position for
		 */
		static void GetUVPosition(long double &u, long double &v, size_t timeIndex, size_t frequencyIndex, TimeFrequencyMetaDataCPtr metaData);

		static double GetFringeStopFrequency(long double time, const Baseline &baseline, long double delayDirectionRA, long double delayDirectionDec, long double frequency);
		//static double GetFringeCount(long double timeStart, long double timeEnd, const Baseline &baseline, long double delayDirectionRA, long double delayDirectionDec, long double frequency);
		static double GetFringeCount(size_t timeIndexStart, size_t timeIndexEnd, unsigned channelIndex, TimeFrequencyMetaDataCPtr metaData);
		static double GetIntegratedFringeStopFrequency(long double timeStart, long double timeEnd, const Baseline &baseline, long double delayDirectionRA, long double delayDirectionDec, long double frequency, size_t steps);
		void Empty();
		void PerformFFT();
		bool HasUV() const throw() { return _uvReal != 0; }
		bool HasFFT() const throw() { return _uvFTReal != 0; }
		const class Image2D &FTReal() const throw() { return *_uvFTReal; }
		const class Image2D &FTImaginary() const throw() { return *_uvFTImaginary; }
		void SetUVScaling(long double newScale) throw()
		{
			_uvScaling = newScale;
		}
		void ApplyWeightsToUV();
	private:
		void Clear();
		struct AntennaCache {
			long double wavelength;
			long double dx, dy, dz;
		};
		void Image(const class IntegerDomain &frequencies);
		void Image(const IntegerDomain &frequencies, const IntegerDomain &antenna1Domain, const IntegerDomain &antenna2Domain);
		void Image(unsigned frequencyIndex, class AntennaInfo &antenna1, class AntennaInfo &antenna2, SingleFrequencySingleBaselineData *data);

		// This is the fast variant.
		void GetUVPosition(long double &u, long double &v, const SingleFrequencySingleBaselineData &data, const AntennaCache &cache);
		void SetUVValue(long double u, long double v, long double r, long double i, long double weight);
		void SetUVFTValue(long double u, long double v, long double r, long double i, long double weight);


		unsigned long _xRes, _yRes;
		unsigned long _xResFT, _yResFT;
		long double _uvScaling;
		class Image2D *_uvReal, *_uvImaginary, *_uvWeights;
		class Image2D *_uvFTReal, *_uvFTImaginary;
		class Image2D *_timeFreq;
		MeasurementSet *_measurementSet;
		unsigned _antennaCount, _fieldCount;
		AntennaInfo *_antennas;
		BandInfo _band;
		FieldInfo *_fields;
		size_t _scanCount;
		ImageKind _imageKind;
		bool _invertFlagging, _directFT;
		bool _ignoreBoundWarnings;
};

#endif
