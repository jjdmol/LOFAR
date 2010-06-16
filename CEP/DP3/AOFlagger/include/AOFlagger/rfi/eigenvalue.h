#ifndef RFI_EIGENVALUE_H
#define RFI_EIGENVALUE_H

#include <AOFlagger/msio/image2d.h>

class Eigenvalue
{
	public:
		static double Compute(Image2DCPtr real, Image2DCPtr imaginary);
};

#endif // RFI_EIGENVALUE_H
