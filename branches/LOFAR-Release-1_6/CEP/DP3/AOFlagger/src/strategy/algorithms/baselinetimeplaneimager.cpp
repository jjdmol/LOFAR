#include <AOFlagger/strategy/algorithms/baselinetimeplaneimager.h>
#include <AOFlagger/util/aologger.h>

#include <cmath>

#include <fftw3.h>

template<typename NumType>
void BaselineTimePlaneImager<NumType>::Image(NumType uTimesLambda, NumType vTimesLambda, NumType wTimesLambda, NumType lowestFrequency, NumType frequencyStep, size_t channelCount, const std::complex<NumType> *data, Image2D &output)
{
	AOLogger::Debug << "BTImager...\n";
	NumType phi = atan2(vTimesLambda, uTimesLambda);
	size_t imgSize = output.Width();
	NumType minLambda = frequencyToWavelength(lowestFrequency + frequencyStep*(NumType) channelCount);
	NumType scale = 1.0; // scale down from all sky to ...
	
	// Steps to be taken:
	// 1. Create a 1D array with the data in it (in 'u' dir) and otherwise zerod.
	//    This represents a cut through the uvw plane. The highest non-zero samples
	//    have uv-distance 2*|uvw|. Channels are not regridded. Therefore, the distance in
	//    samples is 2 * (lowestFrequency / frequencyStep + channelCount)
	//    (Two times larger than necessary to prevent border issues).
	//    
	// 2. Fourier transform this (FFT 1D)
	//    The 1D FFT is of size max(imgSize * 2, sampleDist * 2).
	
	// 3. Stretch, rotate and make it fill the plane
	//    - Stretch with 1/|uvw|
	//    - Rotate with phi
	// 4. Add to output
	
	size_t sampleDist = 2*((size_t) round(lowestFrequency/frequencyStep) + channelCount);
	size_t fftSize = std::max(imgSize*2, 2*sampleDist) * 4;
	fftw_complex
		*fftInp = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize),
		*fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize);
	fftw_plan plan = fftw_plan_dft_1d(fftSize, fftInp, fftOut, FFTW_FORWARD, FFTW_ESTIMATE);
	size_t fftCentre = fftSize / 2;
	size_t startChannel = (lowestFrequency/frequencyStep);
	for(size_t i=0;i!=fftSize;++i) {
		fftInp[i][0] = 0.0;
		fftInp[i][1] = 0.0;
	}
	for(size_t ch=0;ch!=channelCount;++ch)
	{
		fftInp[fftSize - (startChannel + ch)][0] = data->real();
		fftInp[fftSize - (startChannel + ch)][1] = data->imag();
		fftInp[(startChannel + ch)][0] = data->real();
		fftInp[(startChannel + ch)][1] = -data->imag();
		++data;
	}
	
	AOLogger::Debug << "FFT...\n";
	fftw_execute_dft(plan, fftInp, fftOut);
	//std::swap(fftInp, fftOut);
	fftw_free(fftInp);
	
	NumType uvwDist = sqrt(uTimesLambda*uTimesLambda + vTimesLambda*vTimesLambda + wTimesLambda*wTimesLambda) / minLambda;
	AOLogger::Debug << "phi=" << phi << ",imgSize=" << imgSize << ",minLambda=" << minLambda << ",fftSize=" << fftSize << ",uvwDist=" << uvwDist << ",sampleDist=" << sampleDist << '\n';
	
	NumType cosPhi = cos(phi), sinPhi = sin(phi);
	NumType mid = (NumType) imgSize / 2.0;
	
	NumType transformGen = scale * (2.0*uvwDist / sampleDist) * (fftSize / imgSize);
	NumType transformX = cosPhi * transformGen;
	// Negative rotation (thus positive sin sign)
	NumType transformY = sinPhi * transformGen;
	for(size_t y=0;y!=imgSize;++y)
	{
		num_t *destPtr = output.ValuePtr(0, y);
		NumType yr = (NumType) y - mid;
		NumType yrTransformed = yr * transformY;
		for(size_t x=0;x!=imgSize;++x)
		{
			NumType xr = (NumType) x - mid;
			NumType srcX = xr * transformX + yrTransformed;
			size_t srcXIndex = (size_t) round(srcX) + fftCentre;
			if(srcXIndex < fftSize) {
				if(srcXIndex < fftCentre)
					*destPtr += fftOut[srcXIndex+fftCentre][0];
				else
					*destPtr += fftOut[srcXIndex-fftCentre][0];
			}
			++destPtr;
			//else
			//	output.SetValue(x, y, 0.0);
			//if(x==0 && y==0) AOLogger::Debug << "srcX=" << srcX << '\n';
		}
	}
	
	fftw_destroy_plan(plan);
	fftw_free(fftOut);
}

template class BaselineTimePlaneImager<float>;
template class BaselineTimePlaneImager<double>;
