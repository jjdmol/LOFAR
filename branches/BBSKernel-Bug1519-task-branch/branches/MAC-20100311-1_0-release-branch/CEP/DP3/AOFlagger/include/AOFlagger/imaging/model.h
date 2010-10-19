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

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Model {
	struct PointSource {
		long double dec, ra, fluxIntensity;
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
			_sources.push_back(source);
		}

	private:
		std::vector<PointSource *> _sources;
		
		void AddFTOfSources(long double u, long double v, long double &r, long double &i);
		void AddFTOfSource(long double u, long double v, long double &r, long double &i, const PointSource *source);
		static void GetUVPosition(long double &u, long double &v, long double earthLattitudeAngle, long double delayDirectionDEC, long double delayDirectionRA, long double dx, long double dy, long double dz, long double waveLength);
};

#endif
