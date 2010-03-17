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
#include <AOFlagger/rfi/thresholdconfig.h>

#include <iostream>
#include <math.h>

#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/util/rng.h>

#include <AOFlagger/rfi/localfitmethod.h>
#include <AOFlagger/rfi/thresholdmitigater.h>
#include <AOFlagger/rfi/thresholdtools.h>

ThresholdConfig::ThresholdConfig() :
	_method(SumThreshold), _distribution(Gaussian), _verbose(false), _expFactor(0.0L), _fitMethod(0), _minConnectedSamples(1)
{
}

ThresholdConfig::~ThresholdConfig()
{
}

void ThresholdConfig::InitializeLengthsFrom(unsigned count, unsigned startLength)
{
	if(count > 6 || count == 0)
		count = 6;
	if(startLength == 0)
		startLength = 1;
	struct ThresholdOperation o[6];
	o[0].length = startLength;
	o[1].length = startLength + 1;
	o[2].length = startLength + 3;
	o[3].length = startLength + 7;
	o[4].length = startLength + 15;
	o[5].length = startLength + 63;
	_operations.clear();
	for(unsigned i=0;i<count;++i)
		_operations.push_back(o[i]);
}

void ThresholdConfig::InitializeLengthsDefault(unsigned count)
{
	if(count > 9 || count == 0)
		count = 9;
	struct ThresholdOperation o[9];
	o[0].length = 1;
	o[1].length = 2;
	o[2].length = 4;
	o[3].length = 8;
	o[4].length = 16;
	o[5].length = 32;
	o[6].length = 64;
	o[7].length = 128;
	o[8].length = 256;
	_operations.clear();
	for(unsigned i=0;i<count;++i)
		_operations.push_back(o[i]);
}

void ThresholdConfig::InitializeLengthsSingleSample()
{
	ThresholdOperation operation;
	operation.length = 1;
	_operations.push_back(operation);
}

void ThresholdConfig::InitializeThresholdsFromFirstThreshold(long double firstThreshold, enum Distribution noiseDistribution)
{
	// Previously:
		//_operations[i].threshold = firstThreshold * powl(sqrtl(sqrtl(2.0))*sqrtl(2.0), logl(_operations[i].length)/logl(2.0)) / (long double) _operations[i].length;
	long double expFactor = _expFactor;
	if(expFactor == 0.0L) {
		switch(_method) {
		default:
		case SumThreshold:
			expFactor = 1.5;
			break;
		case VarThreshold:
			expFactor = 1.2;
			break;
		}
	}
	for(unsigned i=0;i<_operations.size();++i)
	{
		_operations[i].threshold = firstThreshold * powl(expFactor, logl(_operations[i].length)/logl(2.0)) / (long double) _operations[i].length;
	}
	_distribution = noiseDistribution;
}

void ThresholdConfig::InitializeThresholdsWithFalseRate(size_t resolution, long double falseAlarmRate, enum Distribution noiseDistribution)
{
	InitializeThresholdsFromFirstThreshold(1.0, noiseDistribution);
	BinarySearch(falseAlarmRate, falseAlarmRate / 25.0, resolution);
	for(unsigned i=0;i<_operations.size();++i)
		std::cout << "/ " << _operations[i].threshold;
	std::cout << std::endl;
}

void ThresholdConfig::BinarySearch(long double probability, long double accuracy, size_t resolution)
{
	long double leftLimit = 0.0;
	long double rightLimit = 1.0;
	for(unsigned i=0;i<_operations.size();++i)
		_operations[i].threshold *= rightLimit;
	long double p = CalculateFalseAlarmRate(resolution, _distribution);
	for(unsigned i=0;i<_operations.size();++i)
		_operations[i].threshold /= rightLimit;
	//cout << "P(" << rightLimit << "," << samples << ")=" << p << endl;
	while(p > probability) {
		rightLimit *= 2;

		for(unsigned i=0;i<_operations.size();++i)
			_operations[i].threshold *= rightLimit;
		p =  CalculateFalseAlarmRate(resolution, _distribution);
		for(unsigned i=0;i<_operations.size();++i)
			_operations[i].threshold /= rightLimit;
		//cout << "P(" << rightLimit << "," << samples << ")=" << p << endl;
	}
	long double m = (rightLimit - leftLimit) / 2.0;

	for(unsigned i=0;i<_operations.size();++i)
		_operations[i].threshold *= m;
	p =  CalculateFalseAlarmRate(resolution, _distribution);
	for(unsigned i=0;i<_operations.size();++i)
		_operations[i].threshold /= m;

	//cout << "P(" << m << "," << samples << ")=" << p << endl;
	int iterations = 0;
	while(fabsf(p - probability) > accuracy && iterations < 15)
	{
		if(p > probability)
			leftLimit = m;
		else
			rightLimit = m;
		m = (rightLimit + leftLimit) / 2.0;

		for(unsigned i=0;i<_operations.size();++i)
			_operations[i].threshold *= m;
		p =  CalculateFalseAlarmRate(resolution, _distribution);
		for(unsigned i=0;i<_operations.size();++i)
			_operations[i].threshold /= m;

		std::cout << "P(" << m << ")=" << p << std::endl;
		++iterations;
	}
	for(unsigned i=0;i<_operations.size();++i)
		_operations[i].threshold *= m;
}

void ThresholdConfig::Execute(Image2DCPtr image, Mask2DPtr mask, bool additive, long double sensitivity) const
{
	if(!additive)
		mask->SetAll<false>();

	long double factor;
	
	switch(_distribution) {
		case Gaussian: {
		long double mean, stddev;
		ThresholdTools::WinsorizedMeanAndStdDev(image, mask, mean, stddev);
		if(stddev == 0.0L)
			factor = sensitivity;
		else
			factor = stddev * sensitivity;
		if(_verbose)
			std::cout << "Stddev=" << stddev << " first threshold=" << _operations[0].threshold << std::endl; 
		} break;
		case Rayleigh: {
		long double mode = ThresholdTools::WinsorizedMode(image, mask);
		if(mode == 0.0L)
			factor = sensitivity;
		else
			factor = sensitivity * mode;
		if(_verbose) {
			long double mean, stddev;
			ThresholdTools::WinsorizedMeanAndStdDev(image, mask, mean, stddev);
			std::cout << "Mode=" << mode << " first threshold=" << _operations[0].threshold*factor << std::endl;
			std::cout << "Stddev=" << stddev << std::endl; 
		} 
		} break;
		default:
		factor = sensitivity;
		break;
	}

	for(unsigned i=0;i<_operations.size();++i) {
		switch(_method) {
			case SumThreshold:
			if(_verbose)
				std::cout << "Performing SumThreshold with length " << _operations[i].length 
					<< ", threshold " << _operations[i].threshold*factor << "..." << std::endl;
			ThresholdMitigater::SumThresholdLarge(image, mask, _operations[i].length, _operations[i].threshold*factor);
			break;
			case VarThreshold:
			if(_verbose)
				std::cout << "Performing VarThreshold with length " << _operations[i].length 
					<< ", threshold " << _operations[i].threshold*factor << "..." << std::endl;
			ThresholdMitigater::VarThreshold(image, mask, _operations[i].length, _operations[i].threshold*factor);
			break;
		}
	}

	if(_minConnectedSamples > 1)
		ThresholdTools::FilterConnectedSamples(mask, _minConnectedSamples);
} 

long double ThresholdConfig::CalculateFalseAlarmRate(size_t resolution, enum Distribution noiseDistribution)
{
	Image2DPtr image = Image2D::CreateZeroImagePtr(resolution, resolution);
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(resolution, resolution);
	for(unsigned y=0;y<resolution;++y) {
		for(unsigned x=0;x<resolution;++x) {
			switch(noiseDistribution) {
				case Uniform:
				image->SetValue(x, y, RNG::Uniform());
				break;
				case Gaussian:
				image->SetValue(x, y, RNG::Guassian());
				break;
				case Rayleigh:
				image->SetValue(x, y, RNG::Rayleigh());
				break;
			}
		}
	}
	if(_fitMethod != 0)
	{
		TimeFrequencyData *data;
		if(_fitMethod->PhaseRepresentation() == TimeFrequencyData::ComplexRepresentation) { 
			data = new TimeFrequencyData(TimeFrequencyData::SinglePolarisation, image, image);
		} else
			data = new TimeFrequencyData(TimeFrequencyData::AmplitudePart, TimeFrequencyData::SinglePolarisation, image);
		data->SetGlobalMask(mask);
		_fitMethod->Initialize(*data);
		for(unsigned i=0;i<_fitMethod->TaskCount();++i)
			_fitMethod->PerformFit(i);
		Image2D *diff = Image2D::CreateFromDiff(*image, *_fitMethod->Background().GetSingleImage());
		image->SetValues(*diff);
		delete diff;
	}
	Execute(image, mask, true, 1.0L);
	long double prob = (long double) mask->GetCount<true>() / (resolution*resolution);
	int lengths[32];
	ThresholdTools::CountMaskLengths(mask, lengths, 32);
	for(unsigned j=1;j<33;++j) {
		if(_verbose)
			std::cout << "," << (long double)  lengths[j-1]*j / (resolution*resolution);
		for(unsigned i=0;i<_operations.size();++i) {
			if(j == _operations[i].length) {
				_operations[i].rate = (long double) lengths[j-1]*j / (resolution*resolution*j);
			}
		}
	}
	if(_verbose)
		std::cout << std::endl;
	return prob;
}
