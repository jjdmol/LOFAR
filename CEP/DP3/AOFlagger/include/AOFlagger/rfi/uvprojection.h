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

class UVProjection
{
	static void ProjectThreeTimes(Image2DCPtr image, TimeFrequencyMetaDataCPtr metaData, size_t y, numl_t *rowValues, numl_t *rowPosition, numl_t directionRad, bool isImaginary)
	{
		numl_t bottomSign;
		if(isImaginary)
			bottomSign = -1.0;
		else
			bottomSign = 1.0;

		const numl_t cosRotate = cosnl(_directionRad);
		const numl_t sinRotate = sinnl(_directionRad);
		const size_t width = image->Width();

		// Find length of the major axis of the ellipse
		numl_t maxU = -1e20, minU = 1e20;
		for(size_t x=0;x<width;++x)
		{
			const UVW &uvw = metaData->UVW()[x];
			const numl_t uProjectUpper = uvw.u * cosRotate - uvw.v * sinRotate;
			if(uProjectUpper > maxU) maxU = uProjectUpper;
			if(uProjectUpper < minU) minU = uProjectUpper;
			const numl_t uProjectBottom = -uvw.u * cosRotate + uvw.v * sinRotate;
			if(uProjectBottom > maxU) maxU = uProjectBottom;
			if(uProjectBottom < minU) minU = uProjectBottom;
		}
		
		// Calculate the projected positions and change sign if necessary
		numl_t
			*rowValues = new numl_t[width*3],
			*rowPositions = new numl_t[width*3];
		for(size_t t=0;t<width;++t)
		{
			const UVW &uvw = metaData->UVW()[t];
			const numl_t vProject = uvw.u * sinRotate + uvw.v * cosRotate;
			numl_t uProject, currentSign;
			if(vProject >= 0.0) {
				uProject = uvw.u * cosRotate - uvw.v * sinRotate;
				currentSign = 1.0;
			} else {
				uProject = -uvw.u * cosRotate + uvw.v * sinRotate;
				currentSign = bottomSign;
			}
			numl_t centerValue = currentSign * image->Value(t, y);
			rowValues[t] = bottomSign * centerValue;
			rowValues[t + width] = centerValue;
			rowValues[t + width*2] = bottomSign * centerValue;
			rowPositions[t] = uProject - (maxU - minU);
			rowPositions[t + width] = uProject;
			rowPositions[t + width*2] = uProject + (maxU - minU);
		}
		
	}
};

#endif // RFI_UV_PROJECTION_H
