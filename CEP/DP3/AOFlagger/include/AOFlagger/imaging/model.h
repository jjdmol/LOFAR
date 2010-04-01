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
#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <cmath>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/types.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Model {
	struct PointSource {
		long double dec, ra, fluxIntensity, sqrtFluxIntensity;
	};
	public:
		Model();
		~Model();
		void SimulateBaseline(long double delayDirectionDEC, long double delayDirectionRA, long double dx, long double dy, long double dz, long double frequencyStart, long double frequencyEnd, long double seconds, class Image2D &destR, class Image2D &destI);
		void AddSource(long double dec, long double ra, long double fluxIntensity)
		{
			PointSource *source = new PointSource();
			source->dec = dec;
			source->ra = ra;
			source->fluxIntensity = fluxIntensity;
			source->sqrtFluxIntensity = sqrt(fluxIntensity);
			_sources.push_back(source);
		}
		void SimulateAntenna(num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t frequency, num_t earthLattitude, num_t &r, num_t &i);
		void SimulateUncoherentAntenna(num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t frequency, num_t earthLattitude, num_t &r, num_t &i, size_t index);
		void SimulateCorrelation(class UVImager &imager, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t frequency, double totalTime, double integrationTime);
		void SimulateObservation(class UVImager &imager, class Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);

		static void GetUVPosition(num_t &u, num_t &v, num_t earthLattitudeAngle, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t waveLength);
		static num_t GetWPosition(num_t delayDirectionDec, num_t delayDirectionRA, num_t frequency, num_t earthLattitudeAngleStart, num_t earthLattitudeAngleEnd, num_t dx, num_t dy);
	private:
		std::vector<PointSource *> _sources;
		double _noiseSigma;
		
		void AddFTOfSources(num_t u, num_t v, num_t &r, num_t &i);
		void AddFTOfSource(num_t u, num_t v, num_t &r, num_t &i, const PointSource *source);
};

#endif
