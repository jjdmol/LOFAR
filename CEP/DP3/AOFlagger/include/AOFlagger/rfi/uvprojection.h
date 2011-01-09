/***************************************************************************
 *   Copyright (C) 2008-2010 by A.R. Offringa   *
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

#ifndef RFI_UV_PROJECTION_H
#define RFI_UV_PROJECTION_H

#include <AOFlagger/msio/types.h>
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/timefrequencymetadata.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/util/aologger.h>

class UVProjection
{
	public:
		/**
		* This function will project a uv-track onto a straight line that has an angle of directionRad.
		*/
		static void Project(Image2DCPtr image, TimeFrequencyMetaDataCPtr metaData, size_t y, numl_t *rowValues, numl_t *rowUPositions, numl_t *rowVPositions, bool *rowNegatedSigns, numl_t directionRad, const bool isImaginary)
		{
			numl_t bottomSign;
			if(isImaginary)
				bottomSign = -1.0;
			else
				bottomSign = 1.0;

			const numl_t cosRotate = cosnl(directionRad);
			const numl_t sinRotate = sinnl(directionRad);
			const size_t width = image->Width();
			const numl_t frequency = metaData->Band().channels[y].frequencyHz;

			// Find length of the major axis of the ellipse
			/*numl_t maxU = -1e20, minU = 1e20;
			for(size_t x=0;x<width;++x)
			{
				const UVW &uvw = metaData->UVW()[x];
				const numl_t uProjectUpper = uvw.u * cosRotate - uvw.v * sinRotate;
				if(uProjectUpper > maxU) maxU = uProjectUpper;
				if(uProjectUpper < minU) minU = uProjectUpper;
				const numl_t uProjectBottom = -uvw.u * cosRotate + uvw.v * sinRotate;
				if(uProjectBottom > maxU) maxU = uProjectBottom;
				if(uProjectBottom < minU) minU = uProjectBottom;
			}*/
			
			// Calculate the projected positions and change sign if necessary
			for(size_t t=0;t<width;++t)
			{
				const UVW &uvw = metaData->UVW()[t];
				const numl_t vProject = uvw.u * sinRotate + uvw.v * cosRotate;
				numl_t uProject, currentSign;
				bool currentSignIsNegatated;
				if(vProject >= 0.0) {
					uProject = uvw.u * cosRotate - uvw.v * sinRotate;
					currentSign = 1.0;
					currentSignIsNegatated = false;
					rowVPositions[t] = vProject * frequency / UVImager::SpeedOfLight();
				} else {
					uProject = -uvw.u * cosRotate + uvw.v * sinRotate;
					currentSign = bottomSign;
					currentSignIsNegatated = isImaginary;
					rowVPositions[t] = -vProject * frequency / UVImager::SpeedOfLight();
				}
				rowValues[t] = currentSign * image->Value(t, y);
				rowUPositions[t] = uProject * frequency / UVImager::SpeedOfLight();
				rowNegatedSigns[t] = currentSignIsNegatated;
			}
		}
		
		static void MaximalUPositions(size_t width, numl_t *uPositions, numl_t &leftPosition, numl_t &rightPosition)
		{
			leftPosition = 0.0,
			rightPosition = 0.0;
			for(unsigned i=0;i<width;++i)
			{
				if(uPositions[i] < leftPosition)
					leftPosition = uPositions[i];
				if(uPositions[i] > rightPosition)
					rightPosition = uPositions[i];
			}
		}
	
		static void ProjectImage(Image2DCPtr source, Image2DPtr destination, Image2DPtr weights, TimeFrequencyMetaDataCPtr metaData, numl_t directionRad, numl_t etaParameter, bool sourceIsImaginary)
		{
			const size_t
				inputWidth = source->Width();
			numl_t
				*rowValues = new numl_t[inputWidth],
				*rowUPositions = new numl_t[inputWidth],
				*rowVPositions = new numl_t[inputWidth];
			bool
				*rowSignsNegated = new bool[inputWidth];

			for(unsigned y=0;y<source->Height();++y)
			{
				UVProjection::Project(source, metaData, y, rowValues, rowUPositions, rowVPositions, rowSignsNegated, directionRad, sourceIsImaginary);

				numl_t leftDist, rightDist;
				MaximalUPositions(inputWidth, rowUPositions, leftDist, rightDist);
				numl_t maxDist = rightDist > -leftDist ? rightDist : -leftDist;

				for(unsigned t=0;t<inputWidth-1;++t)
				{
					unsigned
						t1 = t,
						t2 = t+1;
					double
						u1 = rowUPositions[t],
						u2 = rowUPositions[t+1];
					if(u1 > u2)
					{
						u1 = rowUPositions[t+1];
						u2 = rowUPositions[t];
						t1 = t+1;
						t2 = t;
					}
					if(u2 - u1 >= maxDist)
					{
						numl_t midValue = (rowValues[t1]+rowValues[t2])/2.0;
						Interpolate(destination, weights, leftDist, rightDist, leftDist, u1, midValue, rowValues[t2], y);
						Interpolate(destination, weights, leftDist, rightDist, u2, rightDist, rowValues[t1], midValue, y);
					} else {
						Interpolate(destination, weights, leftDist, rightDist, u1, u2, rowValues[t1], rowValues[t2], y);
					}
				}
				unsigned
					rangeStart = (unsigned) roundnl(etaParameter * (num_t) inputWidth / 2.0),
					rangeEnd = inputWidth - rangeStart;
				for(unsigned x=0;x<inputWidth;++x)
				{
					if(x > rangeStart && x < rangeEnd)
					{
						if(weights->Value(x, y) != 0.0)
							destination->SetValue(x, y, destination->Value(x, y) / weights->Value(x, y));
						else
							AOLogger::Warn << "UV projection did not fill entire range\n";
					} else {
						destination->SetValue(x, y, 0.0);
					}
				}
			}
		}
		
		/**
		 * This function converts a distance from phase centre in radians of the sky towards an
		 * index inside an image created by ProjectImage() that has been FFT'ed (hence it represents
		 * an index into a frequency).
		 */
		void GetIndicesInProjectedImage(numl_t distance, numl_t minU, numl_t maxU, unsigned sourceWidth, unsigned destWidth, unsigned &lowestIndex, unsigned &highestIndex)
		{
			numl_t sincScale = 1.0/distance;
			numl_t clippingFrequency = 1.0/(sincScale * sourceWidth / (0.5*(maxU-minU)));
			numl_t fourierClippingIndex = (numl_t) destWidth * 0.5 * clippingFrequency;
			if(fourierClippingIndex*2.0 > destWidth)
				fourierClippingIndex = (numl_t) destWidth/2.0;
			if(fourierClippingIndex < 0.0)
				fourierClippingIndex = 0.0;
			lowestIndex = (unsigned) ceil((numl_t) destWidth * 0.5 - fourierClippingIndex),
			highestIndex = (unsigned) floor((numl_t) destWidth * 0.5 + fourierClippingIndex);
		}
		
	private:
		
		static void Interpolate(Image2DPtr destination, Image2DPtr weights, numl_t leftDist, numl_t rightDist, numl_t u1, numl_t u2, numl_t v1, numl_t v2, unsigned y)
		{
			int
				width = destination->Width(),
				left = (int) ((u1 - leftDist) * (numl_t) width / (rightDist-leftDist)),
				right = (int) ((u2 - leftDist) * (numl_t) width / (rightDist-leftDist));
			if(left < 0) left = 0;
			if(right >= width) right = width;
			if(right - left < 1 && left+1 < width) right = left+1;
			int count = right-left;
			for(int x=left;x<right;++x)
			{
				numl_t value = v1 + ((v2-v1)*(numl_t) (x-left)/(numl_t) count);
				destination->SetValue(x, y, destination->Value(x, y) + value);
				weights->SetValue(x, y, weights->Value(x, y) + 1.0);
			}
		}

};

#endif // RFI_UV_PROJECTION_H
