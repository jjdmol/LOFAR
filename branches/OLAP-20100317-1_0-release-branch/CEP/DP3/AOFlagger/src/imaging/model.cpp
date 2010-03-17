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
#include <AOFlagger/imaging/model.h>

#include <AOFlagger/msio/image2d.h>

#include <iostream>


Model::Model()
{
}

Model::~Model()
{
}

void Model::SimulateBaseline(long double delayDirectionDEC, long double delayDirectionRA, long double dx, long double dy, long double dz, long double frequencyStart, long double frequencyEnd, long double seconds, class Image2D &destR, class Image2D &destI)
{
	for(unsigned fIndex=0;fIndex<destR.Height();++fIndex)
	{
		long double frequency = (frequencyEnd - frequencyStart) * (long double) fIndex/destR.Height() + frequencyStart;

		for(unsigned tIndex=0;tIndex<destR.Width();++tIndex)
		{
			long double timeRotation =
				(long double) tIndex * M_PI * 2.0L * seconds /
				(24.0L*60.0L*60.0L * destR.Width());

			long double u,v;
			GetUVPosition(u, v, timeRotation, delayDirectionDEC, delayDirectionRA, dx, dy, dz, 1.0L/frequency);
			long double r, i;
			AddFTOfSources(u, v, r, i);
			destR.AddValue(tIndex, fIndex, r);
			destI.AddValue(tIndex, fIndex, i);
		}
	}
}

void Model::GetUVPosition(long double &u, long double &v, long double earthLattitudeAngle, long double delayDirectionDEC, long double delayDirectionRA, long double dx, long double dy, long double dz, long double wavelength)
{
	long double pointingLattitude = delayDirectionRA;

	// Rotate baseline plane towards phase center, first rotate around z axis, then around x axis
	long double raRotation = earthLattitudeAngle - pointingLattitude + M_PI*0.5L;
	long double tmpCos = cosl(raRotation);
	long double tmpSin = sinl(raRotation);

	long double dxProjected = tmpCos*dx - tmpSin*dy;
	long double tmpdy = tmpSin*dx + tmpCos*dy;

	tmpCos = cosl(-delayDirectionDEC);
	tmpSin = sinl(-delayDirectionDEC);
	long double dyProjected = tmpCos*tmpdy - tmpSin*dz;

	// Now, the newly projected positive z axis of the baseline points to the phase center

	long double baselineLength = sqrtl(dxProjected*dxProjected + dyProjected*dyProjected);
	
	long double baselineAngle;
	if(baselineLength == 0.0)
		baselineAngle = 0.0;
	else {
		baselineLength /= 299792458.0L * wavelength;
		if(dxProjected > 0.0L)
			baselineAngle = atanl(dyProjected/dxProjected);
		else
			baselineAngle = M_PI - atanl(dyProjected/-dxProjected);
	}
		
	u = cosl(baselineAngle)*baselineLength;
	v = -sinl(baselineAngle)*baselineLength;
}

void Model::AddFTOfSources(long double u, long double v, long double &rVal, long double &iVal)
{
	rVal = 0.0;
	iVal = 0.0;

	for(std::vector<PointSource *>::const_iterator i=_sources.begin();i!=_sources.end();++i)
	{
		AddFTOfSource(u, v, rVal, iVal, (*i));
	}
}

void Model::AddFTOfSource(long double u, long double v, long double &r, long double &i, const PointSource *source)
{
	// Calculate F(X) = f(x) e ^ {i 2 pi (x1 u + x2 v) } 
	long double fftRotation = (u * source->dec/180.0L + v * source->ra/180.0L) * -2.0L * M_PI;
	r += cosl(fftRotation) * source->fluxIntensity;
	i += sinl(fftRotation) * source->fluxIntensity;
}
