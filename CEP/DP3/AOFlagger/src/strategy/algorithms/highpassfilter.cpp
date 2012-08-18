
#include <AOFlagger/strategy/algorithms/highpassfilter.h>
#include <AOFlagger/util/rng.h>

HighPassFilter::~HighPassFilter()
{
	delete[] _hKernel;
	delete[] _vKernel;
}

void HighPassFilter::applyLowPass(const Image2DPtr &image)
{
	// Guassian convolution can be separated in two 1D convolution
	// because of properties of the 2D Gaussian function.
	Image2DPtr temp = Image2D::CreateZeroImagePtr(image->Width(), image->Height());
	size_t hKernelMid = _hWindowSize/2;
	for(size_t i=0; i<=_hWindowSize; ++i) {
		const num_t kernelValue = _hKernel[i];
		for(unsigned y=0;y<image->Height();++y) {
			const size_t
				xStart = (i >= hKernelMid) ? 0 : (hKernelMid-i),
				xEnd = (i <= hKernelMid) ? image->Width() : image->Width()-i+hKernelMid;
			for(unsigned x=xStart;x<xEnd;++x)	
				temp->AddValue(x, y, image->Value(x+i-hKernelMid, y)*kernelValue);
		}
	}
	
	image->SetAll(0.0);
	size_t vKernelMid = _vWindowSize/2;
	for(size_t i=0; i<=_vWindowSize; ++i) {
		const num_t kernelValue = _vKernel[i];
		for(unsigned x=0;x<image->Width();++x) {
			const size_t
				yStart = (i >= vKernelMid) ? 0 : (vKernelMid-i),
				yEnd = (i <= vKernelMid) ? image->Height() : image->Height()-i+vKernelMid;
			for(unsigned y=yStart;y<yEnd;++y)
				image->AddValue(x, y, temp->Value(x, y+i-vKernelMid)*kernelValue);
		}
	}
}

Image2DPtr HighPassFilter::Apply(const Image2DCPtr &image, const Mask2DCPtr &mask)
{
	initializeKernel();
	Image2DPtr
		outputImage = Image2D::CreateUnsetImagePtr(image->Width(), image->Height()),
		weights = Image2D::CreateUnsetImagePtr(image->Width(), image->Height());
	setFlaggedValuesToZeroAndMakeWeights(image, outputImage, mask, weights);
	applyLowPass(outputImage);
	applyLowPass(weights);
	elementWiseDivide(outputImage, weights);
	weights.reset();
	return Image2D::CreateFromDiff(image, outputImage);
}

void HighPassFilter::initializeKernel()
{
	if(_hKernel == 0)
	{
		_hKernel = new num_t[_hWindowSize];
		const int midPointX = _hWindowSize/2;
		for(int x = 0 ; x < (int) _hWindowSize ; ++x)
			_hKernel[x] = RNG::EvaluateGaussian(x-midPointX, _hKernelSigma);
	}
	
	if(_vKernel == 0)
	{
		_vKernel = new num_t[_vWindowSize];
		const	int midPointY = _vWindowSize/2;
		for(int y = 0 ; y < (int) _vWindowSize ; ++y)
			_vKernel[y] = RNG::EvaluateGaussian(y-midPointY, _vKernelSigma);
	}
}

void HighPassFilter::setFlaggedValuesToZeroAndMakeWeights(const Image2DCPtr &inputImage, const Image2DPtr &outputImage, const Mask2DCPtr &inputMask, const Image2DPtr &weightsOutput)
{
	const size_t width = inputImage->Width();
	for(size_t y=0;y<inputImage->Height();++y)
	{
		for(size_t x=0;x<width;++x)
		{
			if(inputMask->Value(x, y))
			{
				outputImage->SetValue(x, y, 0.0);
				weightsOutput->SetValue(x, y, 0.0);
			} else {
				outputImage->SetValue(x, y, inputImage->Value(x, y));
				weightsOutput->SetValue(x, y, 1.0);
			}
		}
	}
}

void HighPassFilter::elementWiseDivide(const Image2DPtr &leftHand, const Image2DCPtr &rightHand)
{
	for(unsigned y=0;y<leftHand->Height();++y) {
		for(unsigned x=0;x<leftHand->Width();++x) {
			if(rightHand->Value(x, y) == 0.0)
				leftHand->SetValue(x, y, 0.0);
			else
				leftHand->SetValue(x, y, leftHand->Value(x, y) / rightHand->Value(x, y));
		}
	}
}

