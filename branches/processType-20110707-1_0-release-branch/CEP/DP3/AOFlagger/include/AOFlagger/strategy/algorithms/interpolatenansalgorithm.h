#ifndef INTERPOLATENANSALGORITHM_H
#define INTERPOLATENANSALGORITHM_H

#include <AOFlagger/util/aologger.h>

#include <AOFlagger/msio/image2d.h>

class InterpolateNansAlgorithms
{
	public:
		static void InterpolateNans(Image2DCPtr image)
		{
			size_t count = 0;
			for(unsigned y=0;y<image->Height();++y)
			{
				for(unsigned x=0;x<image->Width();++x)
				{
					if(!std::isfinite(image->Value(x, y)))
					{
						++count;
					}
				}
			}
			if(count > 0)
			{
				AOLogger::Debug << "Number of nans: " << count << '\n';
			}
		}
};

#endif // INTERPOLATENANSALGORITHM_H
