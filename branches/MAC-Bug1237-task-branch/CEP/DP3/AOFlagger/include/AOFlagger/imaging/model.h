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

#include <AOFlagger/imaging/uvimager.h>

template<typename T>
struct OutputReceiver
{
	void SetY(size_t) { }
};
template<>
struct OutputReceiver<class UVImager>
{
	UVImager *_imager;
	void SetUVValue(size_t, double u, double v, double r, double i, double w)
	{
		_imager->SetUVValue(u, v, r, i, w);
		_imager->SetUVValue(-u, -v, r, -i, w);
	}
	void SetY(size_t) { }
};
template<>
struct OutputReceiver<class TimeFrequencyData>
{
	Image2DPtr _real, _imaginary;
	size_t _y;
	void SetUVValue(size_t x, double, double, double r, double i, double)
	{
		_real->SetValue(x, _y, r);
		_imaginary->SetValue(x, _y, i);
	}
	void SetY(size_t y) { _y = y; }
};
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
		void SimulateAntenna(num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t frequency, num_t earthLattitude, num_t &r, num_t &i);
		void SimulateUncoherentAntenna(num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t frequency, num_t earthLattitude, num_t &r, num_t &i, size_t index);

		template<typename T>
		void SimulateCorrelation(struct OutputReceiver<T> &receiver, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t frequency, double totalTime, double integrationTime);

		void SimulateObservation(class UVImager &imager, class Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency)
		{
			OutputReceiver<UVImager> imagerOutputter;
			imagerOutputter._imager = &imager;
			SimulateObservation(imagerOutputter, observatorium, delayDirectionDEC, delayDirectionRA, frequency);
		}

		std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> SimulateObservation(class Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);

		template<typename T>
		void SimulateObservation(struct OutputReceiver<T> &receiver, class Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);

		static void GetUVPosition(num_t &u, num_t &v, num_t earthLattitudeAngle, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t waveLength);
		static num_t GetWPosition(num_t delayDirectionDec, num_t delayDirectionRA, num_t frequency, num_t earthLattitudeAngle, num_t dx, num_t dy);

		void loadUrsaMajor();
		void loadUrsaMajorDistortingSource();
	private:
		std::vector<PointSource *> _sources;
		double _noiseSigma, _sourceSigma;
		
		void AddFTOfSources(num_t u, num_t v, num_t &r, num_t &i);
		void AddFTOfSource(num_t u, num_t v, num_t &r, num_t &i, const PointSource *source);
};

#endif
