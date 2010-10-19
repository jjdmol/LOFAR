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
				long double r, i;
				CalculateFitValue(*_originalData->GetRealPart(), *_originalData->GetImaginaryPart(), x, y, 1, r, i);
	
				_realBackground->SetValue(x, y, r);
				_imaginaryBackground->SetValue(x, y, i);
			}
		}
		else
		{
			long double r, i;
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
			long double r, i;
			CalculateFitValue(*_originalData->GetRealPart(), *_originalData->GetImaginaryPart(), x, y, 1, r, i);

			_realBackground->SetValue(x, y, r);
			_imaginaryBackground->SetValue(x, y, i);
		}
	}
	else
	{
		for(size_t x=0;x<_originalData->ImageWidth();++x)
		{
		long double r, i;
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

	long double *rotations = new long double[real->Height()];
	for(size_t y=0;y<real->Height();++y)
		rotations[y] = 0.0L;
	for(size_t x=0;x<real->Width();++x)
	{
		for(size_t y=0;y<real->Height();++y)
		{
			long double r = real->Value(x, y);
			long double i = imaginary->Value(x, y);
			long double freq = -rotations[y];
			rotations[y] +=
				GetFringeFrequency(x, y) * 2.0L * M_PIl;
			long double intRotations =  
				deltaTime * UVImager::GetFringeCount(0, x, y, _metaData);
			freq -= intRotations;

			long double cosfreq = cosl(freq*2.0L*M_PIl), sinfreq = sinl(freq*2.0L*M_PIl);
			
			long double newR = r * cosfreq - i * sinfreq;
			i = r * sinfreq + i * cosfreq;
			
			_realBackground->SetValue(x, y, newR);
			_imaginaryBackground->SetValue(x, y, i);
		}
	}
	delete[] rotations;
}

long double FringeStoppingFitter::CalculateMaskedAverage(const Image2D &image, size_t x, size_t yFrom, size_t yLength)
{
	long double average = 0.0L;
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

long double FringeStoppingFitter::CalculateUnmaskedAverage(const Image2D &image, size_t x, size_t yFrom, size_t yLength)
{
	long double average = 0.0L;
	size_t count = 0;
	for(size_t i = yFrom; i < yFrom+yLength; ++i)
	{
		if(std::isfinite(image.Value(x, i)))
		{
			average += image.Value(x, i);
			++count;
		}
	}
	return average / (long double) count;
}

void FringeStoppingFitter::CalculateFitValue(const Image2D &real, const Image2D &imaginary, size_t x, size_t yFrom, size_t yLength, long double &rValue, long double &iValue)
{
	size_t windowWidth;
	size_t yMid = yFrom + yLength/2;
	long double estimatedFrequency = GetIntFringeFrequency(x, yMid);
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

	long double fringeFrequency =  GetFringeFrequency(x, yMid) * 2.0L * M_PIl;
	long double *dataT = new long double[xRight - xLeft];
	long double *dataR = new long double[xRight - xLeft];
	long double *dataI = new long double[xRight - xLeft];
	
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
	long double
		phaseR, phaseI, amplitude, meanR, meanI;

	fitter.FindPhaseAndAmplitudeComplex(phaseR, amplitude, dataR, dataI, dataT, index, fringeFrequency);
	phaseI = fmodl(phaseR+M_PIl*0.5L, 2.0L*M_PIl);
	meanR = fitter.FindMean(phaseR, amplitude, dataR, dataT, index, fringeFrequency);
	meanI = fitter.FindMean(phaseI, amplitude, dataI, dataT, index, fringeFrequency);
	long double fitValueR =
		SinusFitter::Value(phaseR, amplitude, x, fringeFrequency, meanR);
	long double fitValueI =
		SinusFitter::Value(phaseI, amplitude, x, fringeFrequency, meanI);
	long double actualValueR =
		CalculateMaskedAverage(real, x, yFrom, yLength);
	long double actualValueI =
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

long double FringeStoppingFitter::GetIntFringeFrequency(size_t x, size_t y)
{
	return GetIntFringeFrequency(x, x+1, y);

}

long double FringeStoppingFitter::GetIntFringeFrequency(size_t xStart, size_t xEnd, size_t y)
{
	double deltaTime;
	if(_observationTimes->size()>1)
		deltaTime = (*_observationTimes)[1] - (*_observationTimes)[0];
	else
		deltaTime = 1.0;
	long double observeFreq = _bandInfo->channels[y].frequencyHz;
	Baseline baseline(*_antenna1Info, *_antenna2Info);
	long double delayRA = _fieldInfo->delayDirectionRA;
	long double delayDec = _fieldInfo->delayDirectionDec;
	return deltaTime *
		UVImager::GetIntegratedFringeStopFrequency((*_observationTimes)[xStart], (*_observationTimes)[xEnd-1]+deltaTime, baseline, delayRA, delayDec, observeFreq, xEnd-xStart);
}


long double FringeStoppingFitter::GetFringeFrequency(size_t x, size_t y)
{
	double deltaTime;
	if(_observationTimes->size()>1)
		deltaTime = (*_observationTimes)[1] - (*_observationTimes)[0];
	else
		deltaTime = 1.0;
	long double observeFreq = _bandInfo->channels[y].frequencyHz;
	Baseline baseline(*_antenna1Info, *_antenna2Info);
	long double delayRA = _fieldInfo->delayDirectionRA;
	long double delayDec = _fieldInfo->delayDirectionDec;
	return deltaTime *
		UVImager::GetFringeStopFrequency((*_observationTimes)[x], baseline, delayRA, delayDec, observeFreq);
}

void FringeStoppingFitter::GetRFIValue(long double &r, long double &i, int x, int y, const Baseline &, long double rfiPhase, long double rfiStrength)
{
	long double rotations =  
		UVImager::GetFringeCount(0, x, y, _metaData);
	r = cosl(rotations * 2.0L * M_PIl + rfiPhase) * rfiStrength;
	i = sinl(rotations * 2.0L * M_PIl + rfiPhase) * rfiStrength;
}

long double FringeStoppingFitter::GetRFIFitError(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd, int y, long double rfiPhase, long double rfiStrength)
{
	Baseline baseline(*_antenna1Info, *_antenna2Info);
	long double error = 0.0L;

	for(int x=xStart;x<xEnd;++x)
	{
		long double r,i;
		GetRFIValue(r,i,x,y,baseline,rfiPhase,rfiStrength);
		long double er = real->Value(x) - r;
		long double ei = imaginary->Value(x) - i;
		error += sqrtl(er*er + ei*ei);
	}

	return error;
}

void FringeStoppingFitter::MinimizeRFIFitError(long double &phase, long double &amplitude, SampleRowCPtr real, SampleRowCPtr imaginary, unsigned xStart, unsigned xEnd, unsigned y) const throw()
{
	// calculate 1/N * \sum_x v(t) e^{2 i \pi \tau_g(t)}, where \tau_g(t) is the number of phase rotations
	// because of the geometric delay as function of time x.

	const Baseline baseline(*_antenna1Info, *_antenna2Info);

	long double sumR = 0.0L, sumI = 0.0L;
	size_t n = 0;
	for(unsigned t=xStart;t<xEnd;++t)
	{
		const long double vR = real->Value(t);
		const long double vI = imaginary->Value(t);
		
		if(std::isfinite(vR) && std::isfinite(vI))
		{
			const long double tauge = UVImager::GetFringeCount(0, t, y, _metaData);
	
			sumR += vR * cosl(2.0L * M_PIl * tauge);
			sumR += vI * sinl(2.0L * M_PIl * tauge);
	
			sumI += vR * sinl(2.0L * M_PIl * tauge);
			sumI -= vI * cosl(2.0L * M_PIl * tauge);
			++n;
		}
	}

	sumR /= (long double) n;
	sumI /= (long double) n;

	phase = SinusFitter::Phase(sumR, sumI);
	amplitude = sqrtl(sumR*sumR + sumI*sumI);
}

long double FringeStoppingFitter::GetRowVariance(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd)
{
	long double avgSum = 0.0L, varSum = 0.0L;
	for(int x=xStart;x<xEnd;++x)
	{
		long double r = real->Value(x), i = imaginary->Value(x);
		avgSum += sqrtl(r*r + i*i);
	}
	avgSum /= (xEnd - xStart);
	for(int x=xStart;x<xEnd;++x)
	{
		long double r = real->Value(x), i = imaginary->Value(x);
		long double a = sqrtl(r*r + i*i);
		varSum += (a - avgSum) * (a - avgSum);
	}
	return sqrtl(varSum / (xEnd - xStart));
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
	long double phase, strength;
	MinimizeRFIFitError(phase, strength, real, imaginary, 0, _originalData->ImageWidth(), y);
	for(size_t x=0;x<_originalData->ImageWidth();++x)
	{
		Baseline baseline(*_antenna1Info, *_antenna2Info);
		long double rfiR, rfiI;
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
		long double windowPhase, windowStrength;
		 MinimizeRFIFitError(windowPhase, windowStrength, real, imaginary, windowStart, windowEnd, y);

		Baseline baseline(*_antenna1Info, *_antenna2Info);
		long double rfiR, rfiI;
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

long double FringeStoppingFitter::GetAmplitude(unsigned yStart, unsigned yEnd)
{
	unsigned y = (yStart + yEnd) / 2;
	SampleRowPtr
		real = SampleRow::CreateFromRowSum(_originalData->GetRealPart(), yStart, yEnd),
		imaginary = SampleRow::CreateFromRowSum(_originalData->GetImaginaryPart(), yStart, yEnd);
	long double phase, amplitude;
	MinimizeRFIFitError(phase, amplitude, real, imaginary, 0, _originalData->ImageWidth(), y);
	return amplitude;
}
