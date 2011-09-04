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
#ifndef THRESHOLDMITIGATER_H
#define THRESHOLDMITIGATER_H

#include <cstddef>
#include <cstring>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/mask2d.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ThresholdMitigater{
	public:
		//static void Threshold(class Image2D &image, num_t threshold);

		template<size_t Length>
		static void HorizontalSumThreshold(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
		
		template<size_t Length>
		static void VerticalSumThreshold(Image2DCPtr input, Mask2DPtr mask, num_t threshold);
		
		/*template<size_t Length>
		static void SumThreshold(Image2DCPtr input, Mask2DPtr mask, num_t threshold)
		{
			HorizontalSumThreshold<Length>(input, mask, threshold);
			VerticalSumThreshold<Length>(input, mask, threshold);
		}*/

		template<size_t Length>
		static void HorizontalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, num_t threshold);

		template<size_t Length>
		static void VerticalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, num_t threshold);

		template<size_t Length>
		static void SumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, num_t hThreshold, num_t vThreshold)
		{
			HorizontalSumThresholdLarge<Length>(input, mask, hThreshold);
			VerticalSumThresholdLarge<Length>(input, mask, vThreshold);
		}
		
		static void VerticalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold);
		
		static void HorizontalSumThresholdLarge(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold);

		static void VarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold);
		
		static void HorizontalVarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold);
		
		static void VerticalVarThreshold(Image2DCPtr input, Mask2DPtr mask, size_t length, num_t threshold);

		//static void OptimalThreshold(Image2DCPtr input, Mask2DPtr mask, bool additive, num_t sensitivity = 1.0);
	private:
		ThresholdMitigater() { }
};

#endif
