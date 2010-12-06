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
};

#endif // RFI_UV_PROJECTION_H
