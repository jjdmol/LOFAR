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
#include <AOFlagger/msio/antennainfo.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/util/rng.h>

#include <AOFlagger/rfi/fringestoppingfitter.h>
#include <AOFlagger/rfi/sinusfitter.h>

FringeStoppingFitter::FringeStoppingFitter() :
	_originalData(0), _fringesToConsider(1.0), _minWindowSize(32), _maxWindowSize(128), _returnFittedValue(false), _returnMeanValue(false), _fringeFit(true)
{
}

FringeStoppingFitter::~FringeStoppingFitter()
{
}

void FringeStoppingFitter::PerformFit(unsigned taskNumber)
{
	if(_fringeFit)
		PerformRFIFitOnOneChannel(taskNumber, _maxWindowSize);
	else
	{
		size_t x = taskNumber;
	
		if(_fitChannelsIndividually)
		{
			for(size_t y=0;y<_originalData->ImageHeight();++y)
			{
				num_t r, i;
				CalculateFitValue(*_originalData->GetRealPart(), *_originalData->GetImaginaryPart(), x, y, 1, r, i);
	
				_realBackground->SetValue(x, y, r);
				_imaginaryBackground->SetValue(x, y, i);
			}
		}
		else
		{
			num_t r, i;
			CalculateFitValue(*_originalData->GetRealPart(), *_originalData->GetImaginaryPart(), x, 0, _originalData->ImageHeight(), r, i);
		
			for(size_t y=0;y<_originalData->ImageHeight();++y)
			{
				_realBackground->SetValue(x, y, r);
				_imaginaryBackground->SetValue(x, y, i);
			}
		}
	}
}

void FringeStoppingFitter::PerformFitOnOneChannel(unsigned y)
{
	if(_fitChannelsIndividually)
	{
		for(size_t x=0;x<_originalData->ImageWidth();++x)
		{
			num_t r, i;
			CalculateFitValue(*_originalData->GetRealPart(), *_originalData->GetImaginaryPart(), x, y, 1, r, i);

			_realBackground->SetValue(x, y, r);
			_imaginaryBackground->SetValue(x, y, i);
		}
	}
	else
	{
		for(size_t x=0;x<_originalData->ImageWidth();++x)
		{
		num_t r, i;
		CalculateFitValue(*_originalData->GetRealPart(), *_originalData->GetImaginaryPart(), x, 0, _originalData->ImageHeight(), r, i);
		
			_realBackground->SetValue(x, y, r);
			_imaginaryBackground->SetValue(x, y, i);
		}
	}
}

void FringeStoppingFitter::PerformFringeStop()
{
	Image2DCPtr
		real = _originalData->GetRealPart(),
		imaginary = _originalData->GetImaginaryPart();

	double deltaTime;
	if(_observationTimes->size()>1)
		deltaTime = (*_observationTimes)[1] - (*_observationTimes)[0];
	else
		deltaTime = 1.0;
	Baseline baseline(*_antenna1Info, *_antenna2Info);

	num_t *rotations = new num_t[real->Height()];
	for(size_t y=0;y<real->Height();++y)
		rotations[y] = 0.0L;
	for(size_t x=0;x<real->Width();++x)
	{
		for(size_t y=0;y<real->Height();++y)
		{
			num_t r = real->Value(x, y);
			num_t i = imaginary->Value(x, y);
			num_t freq = -rotations[y];
			rotations[y] +=
				GetFringeFrequency(x, y) * 2.0L * M_PIn;
			num_t intRotations =  
				deltaTime * UVImager::GetFringeCount(0, x, y, _metaData);
			freq -= intRotations;

			num_t cosfreq = cosn(freq*2.0L*M_PIn), sinfreq = sinn(freq*2.0L*M_PIn);
			
			num_t newR = r * cosfreq - i * sinfreq;
			i = r * sinfreq + i * cosfreq;
			
			_realBackground->SetValue(x, y, newR);
			_imaginaryBackground->SetValue(x, y, i);
		}
	}
	delete[] rotations;
}

num_t FringeStoppingFitter::CalculateMaskedAverage(const Image2D &image, size_t x, size_t yFrom, size_t yLength)
{
	num_t average = 0.0L;
	size_t count = 0;
	for(size_t i = yFrom; i < yFrom+yLength; ++i)
	{
		if(!_originalMask->Value(x, i) && std::isfinite(image.Value(x, i)))
		{
			average += image.Value(x, i);
			++count;
		}
	}
	return average / (long double) count;
}

num_t FringeStoppingFitter::CalculateUnmaskedAverage(const Image2D &image, size_t x, size_t yFrom, size_t yLength)
{
	num_t average = 0.0L;
	size_t count = 0;
	for(size_t i = yFrom; i < yFrom+yLength; ++i)
	{
		if(std::isfinite(image.Value(x, i)))
		{
			average += image.Value(x, i);
			++count;
		}
	}
	return average / (num_t) count;
}

void FringeStoppingFitter::CalculateFitValue(const Image2D &real, const Image2D &imaginary, size_t x, size_t yFrom, size_t yLength, num_t &rValue, num_t &iValue)
{
	size_t windowWidth;
	size_t yMid = yFrom + yLength/2;
	num_t estimatedFrequency = GetIntFringeFrequency(x, yMid);
	windowWidth = (size_t) ceil(_fringesToConsider/estimatedFrequency);
	if(windowWidth > _maxWindowSize)
		windowWidth = _maxWindowSize;
	if(windowWidth < _minWindowSize)
		windowWidth = _minWindowSize;

	size_t xLeft, xRight;
	xRight = x + windowWidth/2;
	if(x >= (windowWidth/2))
		xLeft = x - (windowWidth/2);
	else {
		xLeft = 0;
		xRight += (windowWidth/2) - x;
	}
	if(xRight > real.Width())
	{
		if(xLeft > xRight - real.Width())
			xLeft -= xRight - real.Width();
		else
			xLeft = 0;
		xRight = real.Width();
	}

	num_t fringeFrequency =  GetFringeFrequency(x, yMid) * 2.0 * M_PIn;
	num_t *dataT = new num_t[xRight - xLeft];
	num_t *dataR = new num_t[xRight - xLeft];
	num_t *dataI = new num_t[xRight - xLeft];
	
	size_t index = 0;
	for(size_t i=xLeft;i<xRight;++i)
	{
		dataR[index] = CalculateMaskedAverage(real, i, yFrom, yLength);
		dataI[index] = CalculateMaskedAverage(imaginary, i, yFrom, yLength);
		if(std::isfinite(dataR[index]) && std::isfinite(dataI[index]))
		{
			dataT[index] = i;
			++index;
		}
	}
	if(index == 0)
	{
		for(size_t i=xLeft;i<xRight;++i)
		{
			dataR[index] = CalculateUnmaskedAverage(real, i, yFrom, yLength);
			dataI[index] = CalculateUnmaskedAverage(imaginary, i, yFrom, yLength);
			if(std::isfinite(dataR[index]) && std::isfinite(dataI[index]))
			{
				dataT[index] = i;
				++index;
			}
		}
	}

	SinusFitter fitter;
	num_t
		phaseR, phaseI, amplitude, meanR, meanI;

	fitter.FindPhaseAndAmplitudeComplex(phaseR, amplitude, dataR, dataI, dataT, index, fringeFrequency);
	phaseI = fmodn(phaseR+M_PIn*0.5L, 2.0L*M_PIn);
	meanR = fitter.FindMean(phaseR, amplitude, dataR, dataT, index, fringeFrequency);
	meanI = fitter.FindMean(phaseI, amplitude, dataI, dataT, index, fringeFrequency);
	num_t fitValueR =
		SinusFitter::Value(phaseR, amplitude, x, fringeFrequency, meanR);
	num_t fitValueI =
		SinusFitter::Value(phaseI, amplitude, x, fringeFrequency, meanI);
	num_t actualValueR =
		CalculateMaskedAverage(real, x, yFrom, yLength);
	num_t actualValueI =
		CalculateMaskedAverage(imaginary, x, yFrom, yLength);
	delete[] dataR;
	delete[] dataI;
	delete[] dataT;

	if(_returnFittedValue)
	{
		rValue = fitValueR;
		iValue = fitValueI;
	}
	else if(_returnMeanValue)
	{
		rValue = meanR;
		iValue = meanI;
	}
	else
	{
		rValue = meanR + actualValueR - fitValueR;
		iValue = meanI + actualValueI - fitValueI;
	}
}

num_t FringeStoppingFitter::GetIntFringeFrequency(size_t x, size_t y)
{
	return GetIntFringeFrequency(x, x+1, y);

}

num_t FringeStoppingFitter::GetIntFringeFrequency(size_t xStart, size_t xEnd, size_t y)
{
	double deltaTime;
	if(_observationTimes->size()>1)
		deltaTime = (*_observationTimes)[1] - (*_observationTimes)[0];
	else
		deltaTime = 1.0;
	num_t observeFreq = _bandInfo->channels[y].frequencyHz;
	Baseline baseline(*_antenna1Info, *_antenna2Info);
	num_t delayRA = _fieldInfo->delayDirectionRA;
	num_t delayDec = _fieldInfo->delayDirectionDec;
	return deltaTime *
		UVImager::GetIntegratedFringeStopFrequency((*_observationTimes)[xStart], (*_observationTimes)[xEnd-1]+deltaTime, baseline, delayRA, delayDec, observeFreq, xEnd-xStart);
}


num_t FringeStoppingFitter::GetFringeFrequency(size_t x, size_t y)
{
	double deltaTime;
	if(_observationTimes->size()>1)
		deltaTime = (*_observationTimes)[1] - (*_observationTimes)[0];
	else
		deltaTime = 1.0;
	num_t observeFreq = _bandInfo->channels[y].frequencyHz;
	Baseline baseline(*_antenna1Info, *_antenna2Info);
	num_t delayRA = _fieldInfo->delayDirectionRA;
	num_t delayDec = _fieldInfo->delayDirectionDec;
	return deltaTime *
		UVImager::GetFringeStopFrequency((*_observationTimes)[x], baseline, delayRA, delayDec, observeFreq);
}

void FringeStoppingFitter::GetRFIValue(num_t &r, num_t &i, int x, int y, const Baseline &, num_t rfiPhase, num_t rfiStrength)
{
	num_t rotations =  
		UVImager::GetFringeCount(0, x, y, _metaData);
	r = cosn(rotations * 2.0L * M_PIn + rfiPhase) * rfiStrength;
	i = sinn(rotations * 2.0L * M_PIn + rfiPhase) * rfiStrength;
}

num_t FringeStoppingFitter::GetRFIFitError(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd, int y, num_t rfiPhase, num_t rfiStrength)
{
	Baseline baseline(*_antenna1Info, *_antenna2Info);
	num_t error = 0.0L;

	for(int x=xStart;x<xEnd;++x)
	{
		num_t r,i;
		GetRFIValue(r,i,x,y,baseline,rfiPhase,rfiStrength);
		num_t er = real->Value(x) - r;
		num_t ei = imaginary->Value(x) - i;
		error += sqrtn(er*er + ei*ei);
	}

	return error;
}

void FringeStoppingFitter::MinimizeRFIFitError(num_t &phase, num_t &amplitude, SampleRowCPtr real, SampleRowCPtr imaginary, unsigned xStart, unsigned xEnd, unsigned y) const throw()
{
	// calculate 1/N * \sum_x v(t) e^{2 i \pi \tau_g(t)}, where \tau_g(t) is the number of phase rotations
	// because of the geometric delay as function of time x.

	const Baseline baseline(*_antenna1Info, *_antenna2Info);

	num_t sumR = 0.0L, sumI = 0.0L;
	size_t n = 0;
	for(unsigned t=xStart;t<xEnd;++t)
	{
		const num_t vR = real->Value(t);
		const num_t vI = imaginary->Value(t);
		
		if(std::isfinite(vR) && std::isfinite(vI))
		{
			const num_t tauge = UVImager::GetFringeCount(0, t, y, _metaData);
	
			sumR += vR * cosn(2.0L * M_PIn * tauge);
			sumR += vI * sinn(2.0L * M_PIn * tauge);
	
			sumI += vR * sinn(2.0L * M_PIn * tauge);
			sumI -= vI * cosn(2.0L * M_PIn * tauge);
			++n;
		}
	}

	sumR /= (num_t) n;
	sumI /= (num_t) n;

	phase = SinusFitter::Phase(sumR, sumI);
	amplitude = sqrtn(sumR*sumR + sumI*sumI);
}

num_t FringeStoppingFitter::GetRowVariance(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd)
{
	num_t avgSum = 0.0L, varSum = 0.0L;
	for(int x=xStart;x<xEnd;++x)
	{
		num_t r = real->Value(x), i = imaginary->Value(x);
		avgSum += sqrtn(r*r + i*i);
	}
	avgSum /= (xEnd - xStart);
	for(int x=xStart;x<xEnd;++x)
	{
		num_t r = real->Value(x), i = imaginary->Value(x);
		num_t a = sqrtn(r*r + i*i);
		varSum += (a - avgSum) * (a - avgSum);
	}
	return sqrtn(varSum / (xEnd - xStart));
}

void FringeStoppingFitter::PerformRFIFitOnOneChannel(unsigned y)
{
	SampleRowPtr
		real = SampleRow::CreateFromRow(_originalData->GetRealPart(), y),
		imaginary = SampleRow::CreateFromRow(_originalData->GetImaginaryPart(), y);
	PerformRFIFitOnOneRow(real, imaginary, y);
}

void FringeStoppingFitter::PerformRFIFitOnOneRow(SampleRowCPtr real, SampleRowCPtr imaginary, unsigned y)
{
	num_t phase, strength;
	MinimizeRFIFitError(phase, strength, real, imaginary, 0, _originalData->ImageWidth(), y);
	for(size_t x=0;x<_originalData->ImageWidth();++x)
	{
		Baseline baseline(*_antenna1Info, *_antenna2Info);
		num_t rfiR, rfiI;
		GetRFIValue(rfiR, rfiI, x, y, baseline, phase, strength);
		_realBackground->SetValue(x, y, rfiR);
		_imaginaryBackground->SetValue(x, y, rfiI);
	}
}

void FringeStoppingFitter::PerformRFIFitOnOneChannel(unsigned y, unsigned windowSize)
{
	SampleRowPtr
		real = SampleRow::CreateFromRowWithMissings(_originalData->GetRealPart(), _originalMask, y),
		imaginary = SampleRow::CreateFromRowWithMissings(_originalData->GetImaginaryPart(), _originalMask, y);
	PerformRFIFitOnOneRow(real, imaginary, y, windowSize);
}

void FringeStoppingFitter::PerformRFIFitOnOneRow(SampleRowCPtr real, SampleRowCPtr imaginary, unsigned y, unsigned windowSize)
{
	//long double phase, strength;
	//MinimizeRFIFitError(phase, strength, real, imaginary, 0, _originalData->ImageWidth(), y);
	unsigned halfWindowSize = windowSize / 2;
	for(size_t x=0;x<real->Size();++x)
	{
		size_t windowStart, windowEnd;
		if(x > halfWindowSize)
			windowStart = x - halfWindowSize;
		else
			windowStart = 0;
		if(x + halfWindowSize < real->Size())
			windowEnd = x + halfWindowSize;
		else
			windowEnd =  real->Size();
		num_t windowPhase, windowStrength;
		 MinimizeRFIFitError(windowPhase, windowStrength, real, imaginary, windowStart, windowEnd, y);

		Baseline baseline(*_antenna1Info, *_antenna2Info);
		num_t rfiR, rfiI;
		GetRFIValue(rfiR, rfiI, x, y, baseline, windowPhase, windowStrength);
		_realBackground->SetValue(x, y, rfiR);
		_imaginaryBackground->SetValue(x, y, rfiI);
	}
}

void FringeStoppingFitter::PerformRFIFit()
{
	for(size_t y=0;y<_originalData->ImageHeight();++y)
	{
		PerformRFIFitOnOneChannel(y);
	}
}

void FringeStoppingFitter::PerformRFIFit(unsigned yStart, unsigned yEnd, unsigned windowSize)
{
	SampleRowPtr
		real = SampleRow::CreateFromRowSum(_originalData->GetRealPart(), yStart, yEnd),
		imaginary = SampleRow::CreateFromRowSum(_originalData->GetImaginaryPart(), yStart, yEnd);
	PerformRFIFitOnOneRow(real, imaginary, (yStart + yEnd) / 2, windowSize);
}

void FringeStoppingFitter::PerformRFIFit(unsigned yStart, unsigned yEnd)
{
	SampleRowPtr
		real = SampleRow::CreateFromRowSum(_originalData->GetRealPart(), yStart, yEnd),
		imaginary = SampleRow::CreateFromRowSum(_originalData->GetImaginaryPart(), yStart, yEnd);
	PerformRFIFitOnOneRow(real, imaginary, (yStart + yEnd) / 2);
}

num_t FringeStoppingFitter::GetAmplitude(unsigned yStart, unsigned yEnd)
{
	unsigned y = (yStart + yEnd) / 2;
	SampleRowPtr
		real = SampleRow::CreateFromRowSum(_originalData->GetRealPart(), yStart, yEnd),
		imaginary = SampleRow::CreateFromRowSum(_originalData->GetImaginaryPart(), yStart, yEnd);
	num_t phase, amplitude;
	MinimizeRFIFitError(phase, amplitude, real, imaginary, 0, _originalData->ImageWidth(), y);
	return amplitude;
}
