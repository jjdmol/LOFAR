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
#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/imaging/observatorium.h>
#include <AOFlagger/util/rng.h>

#include <iostream>

Model::Model() : _noiseSigma(1), _sourceSigma(0)
{
}

Model::~Model()
{
}

template<typename T>
void Model::SimulateObservation(struct OutputReceiver<T> &receiver, Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency)
{
	size_t channelCount = observatorium.BandInfo().channelCount;

	for(size_t f=0;f<channelCount;++f)
	{
		double channelFrequency = frequency + observatorium.ChannelWidthHz() * f;
		receiver.SetY(f);
		for(size_t i=0;i<observatorium.AntennaCount();++i)
		{
			for(size_t j=i+1;j<observatorium.AntennaCount();++j)
			{
				const AntennaInfo
					&antenna1 = observatorium.GetAntenna(i),
					&antenna2 = observatorium.GetAntenna(j);
				double
					dx = antenna1.position.x - antenna2.position.x,
					dy = antenna1.position.y - antenna2.position.y,
					dz = antenna1.position.z - antenna2.position.z;

				SimulateCorrelation(receiver, delayDirectionDEC, delayDirectionRA, dx, dy, dz, channelFrequency, 12*60*60, 10.0);
			}
		}
	}
}

template void Model::SimulateObservation(struct OutputReceiver<UVImager> &receiver, Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);
template void Model::SimulateObservation(struct OutputReceiver<TimeFrequencyData> &receiver, Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);

std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> Model::SimulateObservation(class Observatorium &observatorium, num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency)
{
	size_t channelCount = observatorium.BandInfo().channelCount;
	OutputReceiver<TimeFrequencyData> tfOutputter;
	tfOutputter._real = Image2D::CreateZeroImagePtr(12*60*60/10, channelCount);
	tfOutputter._imaginary = Image2D::CreateZeroImagePtr(12*60*60/10, channelCount);
	SimulateObservation(tfOutputter, observatorium, delayDirectionDEC, delayDirectionRA, frequency);

	TimeFrequencyMetaData *metaData = new TimeFrequencyMetaData();
	metaData->SetAntenna1(observatorium.GetAntenna(0));
	metaData->SetAntenna2(observatorium.GetAntenna(1));
	metaData->SetBand(observatorium.BandInfo());
	std::vector<double> times;
	std::vector<UVW> uvws;
	num_t wavelength = 1.0L / frequency;
	double
		dx = metaData->Antenna1().position.x - metaData->Antenna2().position.x,
		dy = metaData->Antenna1().position.y - metaData->Antenna2().position.y,
		dz = metaData->Antenna1().position.z - metaData->Antenna2().position.z;
	for(num_t t=0.0;t<12*60*60;t+=10)
	{
		times.push_back(t);
		num_t earthLattitudeApprox = t*(M_PIn/12.0/60.0/60.0);
		UVW uvw;
		GetUVPosition(uvw.u, uvw.v, earthLattitudeApprox, delayDirectionDEC, delayDirectionRA, dx, dy, dz, wavelength);
		uvw.w = 0.0;
		uvws.push_back(uvw);
	}
	metaData->SetUVW(uvws);
	metaData->SetObservationTimes(times);

	return std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr>(TimeFrequencyData(StokesIPolarisation, tfOutputter._real, tfOutputter._imaginary), TimeFrequencyMetaDataPtr(metaData));
}

template<typename T>
void Model::SimulateCorrelation(struct OutputReceiver<T> &receiver, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t frequency, double totalTime, double integrationTime)
{
	num_t wavelength = 1.0L / frequency;
	size_t index = 0;
	for(num_t t=0.0;t<totalTime;t+=integrationTime)
	{
		num_t earthLattitudeApprox = t*(M_PIn/12.0/60.0/60.0);
		num_t u, v, r1, i1, r2, i2;
		GetUVPosition(u, v, earthLattitudeApprox, delayDirectionDEC, delayDirectionRA, dx, dy, dz, wavelength);
		SimulateAntenna(delayDirectionDEC, delayDirectionRA, 0, 0, frequency, earthLattitudeApprox, r1, i1);
		SimulateAntenna(delayDirectionDEC, delayDirectionRA, dx, dy, frequency, earthLattitudeApprox, r2, i2);
		num_t
			r = r1 * r2 - (i1 * -i2),
			i = r1 * -i2 + r2 * i1;
		receiver.SetUVValue(index, u, v, r, i, 1.0);
		++index;
	}
}

template void Model::SimulateCorrelation(struct OutputReceiver<UVImager> &receiver, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t frequency, double totalTime, double integrationTime);

void Model::SimulateAntenna(num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t frequency, num_t earthLattitude, num_t &r, num_t &i)
{
	r = 0.0;
	i = 0.0;
	num_t delayW = GetWPosition(delayDirectionDEC, delayDirectionRA, frequency, earthLattitude, dx, dy);
	for(std::vector<PointSource *>::const_iterator iter=_sources.begin();iter!=_sources.end();++iter)
	{
		PointSource &source = **iter;
		num_t w = GetWPosition(source.dec, source.ra, frequency, earthLattitude, dx, dy);
		num_t fieldStrength = source.sqrtFluxIntensity + RNG::Gaussian() * _sourceSigma;
		num_t noiser, noisei;
		RNG::ComplexGaussianAmplitude(noiser, noisei);
		r += fieldStrength * cosn((w - delayW) * M_PIn * 2.0) + noiser * _noiseSigma;
		i += fieldStrength * sinn((w - delayW) * M_PIn * 2.0) + noisei * _noiseSigma;
	}
}

void Model::SimulateUncoherentAntenna(num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t frequency, num_t earthLattitude, num_t &r, num_t &i, size_t index)
{
	num_t delayW = GetWPosition(delayDirectionDEC, delayDirectionRA, frequency, earthLattitude, dx, dy);

	//if(index%(_sources.size()+1) == _sources.size())
	//{
	num_t noiser, noisei;
	RNG::ComplexGaussianAmplitude(noiser, noisei);
	noiser *= _noiseSigma;
	noisei *= _noiseSigma;
	//}
	//else {
		PointSource &source = *_sources[index%_sources.size()];
		num_t w = GetWPosition(source.dec, source.ra, frequency, earthLattitude, dx, dy);
		num_t fieldStrength = source.sqrtFluxIntensity + RNG::Gaussian() * _sourceSigma;
		r = fieldStrength * cosn((w - delayW) * M_PIn * 2.0) + noiser;
		i = fieldStrength * sinn((w - delayW) * M_PIn * 2.0) + noisei;
	//}
}

void Model::SimulateBaseline(long double delayDirectionDEC, long double delayDirectionRA, long double dx, long double dy, long double dz, long double frequencyStart, long double frequencyEnd, long double seconds, class Image2D &destR, class Image2D &destI)
{
	for(unsigned fIndex=0;fIndex<destR.Height();++fIndex)
	{
		long double frequency = (frequencyEnd - frequencyStart) * (long double) fIndex/destR.Height() + frequencyStart;

		for(unsigned tIndex=0;tIndex<destR.Width();++tIndex)
		{
			long double timeRotation =
				(long double) tIndex * M_PIn * seconds /
				(12.0L*60.0L*60.0L * destR.Width());

			num_t u,v;
			GetUVPosition(u, v, timeRotation, delayDirectionDEC, delayDirectionRA, dx, dy, dz, 1.0L/frequency);
			num_t r, i;
			AddFTOfSources(u, v, r, i);
			destR.AddValue(tIndex, fIndex, r);
			destI.AddValue(tIndex, fIndex, i);
		}
	}
}

void Model::GetUVPosition(num_t &u, num_t &v, num_t earthLattitudeAngle, num_t delayDirectionDEC, num_t delayDirectionRA, num_t dx, num_t dy, num_t dz, num_t wavelength)
{
	long double pointingLattitude = delayDirectionRA;

	// Rotate baseline plane towards phase center, first rotate around z axis, then around x axis
	long double raRotation = earthLattitudeAngle - pointingLattitude + M_PIn*0.5L;
	long double tmpCos = cosn(raRotation);
	long double tmpSin = sinn(raRotation);

	long double dxProjected = tmpCos*dx - tmpSin*dy;
	long double tmpdy = tmpSin*dx + tmpCos*dy;

	tmpCos = cosn(-delayDirectionDEC);
	tmpSin = sinn(-delayDirectionDEC);
	long double dyProjected = tmpCos*tmpdy - tmpSin*dz;

	// Now, the newly projected positive z axis of the baseline points to the phase center

	long double baselineLength = sqrtn(dxProjected*dxProjected + dyProjected*dyProjected);
	
	long double baselineAngle;
	if(baselineLength == 0.0)
		baselineAngle = 0.0;
	else {
		baselineLength /= 299792458.0L * wavelength;
		if(dxProjected > 0.0L)
			baselineAngle = atann(dyProjected/dxProjected);
		else
			baselineAngle = M_PIn - atann(dyProjected/-dxProjected);
	}
		
	u = cosn(baselineAngle)*baselineLength;
	v = -sinn(baselineAngle)*baselineLength;
}

num_t Model::GetWPosition(num_t delayDirectionDec, num_t delayDirectionRA, num_t frequency, num_t earthLattitudeAngle, num_t dx, num_t dy)
{
	num_t wavelength = 299792458.0L / frequency;
	num_t raSinEnd = sinn(-delayDirectionRA - earthLattitudeAngle);
	num_t raCosEnd = cosn(-delayDirectionRA - earthLattitudeAngle);
	num_t decCos = cosn(delayDirectionDec);
	// term "+ dz * decCos" is eliminated because of subtraction
	num_t wPosition =
		(dx*raCosEnd - dy*raSinEnd) * (-decCos) / wavelength;
	return wPosition;
}

void Model::AddFTOfSources(num_t u, num_t v, num_t &rVal, num_t &iVal)
{
	rVal = 0.0;
	iVal = 0.0;

	for(std::vector<PointSource *>::const_iterator i=_sources.begin();i!=_sources.end();++i)
	{
		AddFTOfSource(u, v, rVal, iVal, (*i));
	}
}

void Model::AddFTOfSource(num_t u, num_t v, num_t &r, num_t &i, const PointSource *source)
{
	// Calculate F(X) = f(x) e ^ {i 2 pi (x1 u + x2 v) } 
	long double fftRotation = (u * source->dec/180.0L + v * source->ra/180.0L) * -2.0L * M_PIn;
	r += cosn(fftRotation) * source->fluxIntensity;
	i += sinn(fftRotation) * source->fluxIntensity;
}

void Model::loadUrsaMajor()
{
	double
		s = 0.00005, //scale
		rs = 8.0; // stretch in dec
	double cd = -M_PIn - 0.05;
	double cr = 0.05;
	double fluxoffset = 0.0;

	AddSource(cd + s*rs*40, cr + s*72, 8.0/8.0 + fluxoffset); // Dubhe
	AddSource(cd + s*rs*-16, cr + s*81, 4.0/8.0 + fluxoffset); // Beta
	AddSource(cd + s*rs*-45, cr + s*2, 3.0/8.0 + fluxoffset); // Gamma
	AddSource(cd + s*rs*-6, cr + s*-27, 2.0/8.0 + fluxoffset); // Delta
	AddSource(cd + s*rs*-4, cr + s*-85, 6.0/8.0 + fluxoffset); // Alioth
	AddSource(cd + s*rs*2, cr + s*-131, 5.0/8.0 + fluxoffset); // Zeta
	AddSource(cd + s*rs*-36, cr + s*-192, 7.0/8.0 + fluxoffset); // Alkaid
}

void Model::loadUrsaMajorDistortingSource()
{
	double
		s = 0.00005, //scale
		rs = 8.0; // stretch in dec
	double cd = -M_PIn - 0.05;
	double cr = 0.05;
	double fluxoffset = 0.0;

	AddSource(cd + s*rs*320, cr + s*600, 4.0 + fluxoffset); // Dubhe
}
