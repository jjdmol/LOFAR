#include <AOFlagger/strategy/actions/highpassfilteraction.h>

#include <AOFlagger/strategy/algorithms/highpassfilter.h>

namespace rfiStrategy {

void HighPassFilterAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
{
	TimeFrequencyData &data = artifacts.ContaminatedData();
	if(data.PolarisationCount() != 1)
		throw std::runtime_error("High-pass filtering needs single polarization");
	HighPassFilter filter;
	filter.SetHKernelSigma(_hKernelSigma);
	filter.SetHWindowSize(_windowWidth);
	filter.SetVKernelSigma(_vKernelSigma);
	filter.SetVWindowSize(_windowHeight);
	Mask2DCPtr mask = data.GetSingleMask();
	size_t imageCount = data.ImageCount();
	for(size_t i=0;i<imageCount;++i)
	{
		Image2DCPtr image = data.GetImage(i);
		data.SetImage(i, filter.Apply(image, mask));
	}
}

}
