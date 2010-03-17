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
#ifndef RFISTATISTICS_H
#define RFISTATISTICS_H

#include <cstring>
#include <vector>

#include "../msio/image2d.h"
#include "../msio/mask2d.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class RFIStatistics {
	public:
		RFIStatistics();
		~RFIStatistics();
		void Add(Image2DCPtr image, Mask2DCPtr mask);

		size_t GetRFIConnectedSampleCount() { return _samplesConnectedAlgorithm.size(); }
		size_t GetInvalidValuesCount() { return _totalInvalidValues; }

		long double GetAverageConnectedRFISize() { return getAverageRFISize(_samplesConnectedAlgorithm); }
		long double GetAverageConnectedRFIFlux() { return getAverageRFIFlux(_samplesConnectedAlgorithm); }
		long double GetAverageConnectedRFIDuration() { return getAverageRFIDuration(_samplesConnectedAlgorithm); }
		long double GetAverageConnectedRFIFrequencyCoverage() { return getAverageRFIFrequencyCoverage(_samplesConnectedAlgorithm); }

		long double GetAveragePeeledRFISize() { return getAverageRFISize(_samplesPeeledAlgorithm); }
		long double GetAveragePeeledRFIFlux() { return getAverageRFIFlux(_samplesPeeledAlgorithm); }
		long double GetAveragePeeledRFIDuration() { return getAverageRFIDuration(_samplesPeeledAlgorithm); }
		long double GetAveragePeeledRFIFrequencyCoverage() { return getAverageRFIFrequencyCoverage(_samplesPeeledAlgorithm); }

		long double GetAverageNonRFIFlux() const;

		static long double FitScore(const Image2D &image, const Image2D &fit, Mask2DCPtr mask);
		static long double FitScore(Image2DCPtr image, Image2DCPtr fit, Mask2DCPtr mask);

		void MakePlot() const { plot(_samplesConnectedAlgorithm); plot(_samplesPeeledAlgorithm); }

		static num_t DataQuality(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned startX, unsigned endX);

		static num_t FrequencySNR(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned channel);

	private:
		struct RFISampleProperties
		{
			size_t size, duration, frequencyCoverage;
			long double flux;
		};
		struct SamplePosition
		{
			size_t x, y;
			SamplePosition(size_t _x, size_t _y) throw() { x=_x; y=_y; }
		};

		long double getAverageRFISize(std::vector<RFISampleProperties> &sampleProperties);
		long double getAverageRFIFlux(std::vector<RFISampleProperties> &sampleProperties);
		long double getAverageRFIDuration(std::vector<RFISampleProperties> &sampleProperties);
		long double getAverageRFIFrequencyCoverage(std::vector<RFISampleProperties> &sampleProperties);

		long double _totalNonRFIFlux;
		size_t _totalAddedImages;
		size_t _totalAddedSamples;
		size_t _totalAddedNonRFISamples;
		size_t _totalInvalidValues;

		void FindConnectedSamples(Mask2DPtr mask, std::vector<SamplePosition> &positions, SamplePosition start);
		void SetSamples(Mask2DPtr mask, const std::vector<SamplePosition> &positions, bool val);
		void GetSampleProperties(RFISampleProperties &properties, Image2DCPtr image, const std::vector<SamplePosition> &positions);
		long double AverageMasked(Image2DCPtr image, Mask2DCPtr mask, bool negate);
		size_t InvalidValues(Image2DCPtr image, Mask2DCPtr mask, bool negate) const;

		void calculateLengths(Image2DCPtr image, Mask2DCPtr mask);
		size_t GetFrequencyMax(Mask2DCPtr mask, size_t time, size_t &pos);
		size_t GetTimeMax(Mask2DCPtr mask, size_t channel, size_t &pos);
		bool isSurrounded(Mask2DCPtr mask, size_t x, size_t y)
		{
			if(x==0 || x==mask->Width()-1 || y==0 || y==mask->Height()-1)
				return false;
			return mask->Value(x-1, y) && mask->Value(x+1,y) && mask->Value(x, y-1) && mask->Value(x, y+1);
		}
		void countTimeLine(Image2DCPtr image, size_t xStart, size_t xLength, size_t y);
		void countFrequencyLine(Image2DCPtr image, size_t yStart, size_t yLength, size_t x);

		void plot(const std::vector<RFISampleProperties> &samples) const;

		std::vector<RFISampleProperties> _samplesConnectedAlgorithm;
		std::vector<RFISampleProperties> _samplesPeeledAlgorithm;
		bool _eightConnected;
		size_t _binCount;
};

#endif
