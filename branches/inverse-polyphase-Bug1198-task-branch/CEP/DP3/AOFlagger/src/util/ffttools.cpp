/***************************************************************************
 *   Copyright (C) 2007 by Andre Offringa   *
 *   offringa@gmail.com   *
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
#include <AOFlagger/util/ffttools.h>

#include <fftw3.h>

#include <AOFlagger/rfi/sinusfitter.h>

Image2D *FFTTools::CreateFFTImage(const Image2D &original, FFTOutputMethod method) throw()
{
	Image2D *image;
	if(method == Both)
	 image = Image2D::CreateEmptyImage(original.Width()+2, original.Height());
	else
	 image = Image2D::CreateEmptyImage(original.Width()/2+1, original.Height());
	unsigned long n_in = original.Width() * original.Height();
	unsigned long n_out = original.Width() * (original.Height()/2+1);
	
	double *in = (double*) fftw_malloc(sizeof(double) * n_in);
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n_out);
	
	// According to the specification of fftw, the execute function might
	// destroy the input array ("in"), wherefore we need to copy it.
	unsigned long ptr = 0;
	for(unsigned long y=0;y<original.Height();y++) {
		for(unsigned long x=0;x<original.Width();x++) {
			if(original.IsSet(x, y))
				in[ptr] = original.Value(x, y);
			else
				in[ptr] = 0.0;
			ptr++;
		}
	}
	
	fftw_plan plan = fftw_plan_dft_r2c_2d(original.Width(), original.Height(),
		in, out, FFTW_ESTIMATE);
	
	fftw_execute(plan);
	
	// Copy data to new image
	if(method != Both) {
		ptr = 0;
		unsigned long halfwidth = original.Width()/2+1;
		for(unsigned long y=0;y<image->Height();y++) {
			for(unsigned long x=0;x<halfwidth;x++) {
				switch(method) {
					case Real: image->SetValue(x, y, out[ptr][0]); break;
					case Imaginary: image->SetValue(x ,y, out[ptr][1]); break;
					case Absolute:
					default:
						image->SetValue(x, y, sqrtl(out[ptr][0]*out[ptr][0] + out[ptr][1]*out[ptr][1]));
						break;
				}
				ptr++;
			}
		}
	} else {
		unsigned out_ptr = 0;
		unsigned long halfwidth = original.Width()/2+1;
		for(unsigned long y=0;y<image->Height();y++) {
			for(unsigned long x=0;x<halfwidth;x++) {
				image->SetValue(x ,y, out[out_ptr][0]);
				out_ptr++;
			}
			out_ptr -= halfwidth;
			for(unsigned long x=0;x<halfwidth;x++) {
				image->SetValue(x, y, out[out_ptr][1]);
				out_ptr++;
			}
		}
	}
	
	fftw_destroy_plan(plan);
	fftw_free(in);
	fftw_free(out);
	
	return image;
}

void FFTTools::CreateFFTImage(const Image2D &real, const Image2D &imaginary, Image2D &realOut, Image2D &imaginaryOut, bool centerAfter, bool negate) throw()
{
	unsigned long n_in = real.Width() * real.Height();
	fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n_in);
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n_in);
	
	bool centerBefore = true;
	if(centerBefore) {
		Image2D *tmp = CreateShiftedImageFromFFT(real);
		realOut.SetValues(*tmp);
		delete tmp;
		tmp = CreateShiftedImageFromFFT(imaginary);
		imaginaryOut.SetValues(*tmp);
		delete tmp;
	} else {
		realOut.SetValues(real);
		imaginaryOut.SetValues(imaginary);
	}

	unsigned long ptr = 0;
	for(unsigned long y=0;y<real.Height();y++) {
		for(unsigned long x=0;x<real.Width();x++) {
			in[ptr][0] = realOut.Value(x, y);
			in[ptr][1] = imaginaryOut.Value(x, y);
			ptr++;
		}
	}

	int sign = 1;
	if(negate)
		sign = -1;
	fftw_plan plan = fftw_plan_dft_2d(real.Width(), real.Height(),
		in, out, sign, FFTW_ESTIMATE);
	fftw_execute(plan);
	
	ptr = 0;
	for(unsigned long y=0;y<real.Height();y++) {
		for(unsigned long x=0;x<real.Width();x++) {
			realOut.SetValue(x, y, out[ptr][0]);
			imaginaryOut.SetValue(x, y, out[ptr][1]);
			ptr++;
		}
	}
	fftw_free(in);
	fftw_free(out);
	if(centerAfter) {
		Image2D *tmp = CreateShiftedImageFromFFT(realOut);
		realOut.SetValues(*tmp);
		delete tmp;
		tmp = CreateShiftedImageFromFFT(imaginaryOut);
		imaginaryOut.SetValues(*tmp);
		delete tmp;
	}
}

Image2D *FFTTools::CreateFullImageFromFFT(const Image2D &fft)
{
	int width = fft.Width()*2;
	Image2D *image = Image2D::CreateEmptyImage(width, fft.Height());
	for(unsigned y=0;y<fft.Height();++y) {
		for(unsigned x=0;x<fft.Width();++x) {
			image->SetValue(x, y, fft.Value(fft.Width()-x-1, (y+fft.Height()/2)%fft.Height()));
			image->SetValue(fft.Width()*2 - x - 1, fft.Height() - y - 1, fft.Value(fft.Width()-x-1, (y+fft.Height()/2)%fft.Height()));
		}
	}
	return image;
}

Image2D *FFTTools::CreateShiftedImageFromFFT(const Image2D &fft)
{
	Image2D *image = Image2D::CreateEmptyImage(fft.Width(), fft.Height());
	for(unsigned y=0;y<fft.Height();++y) {
		for(unsigned x=0;x<fft.Width();++x) {
			image->SetValue(x, y, fft.Value((x+fft.Width()/2)%fft.Width(), (y+fft.Height()/2)%fft.Height()));
		}
	}
	return image;
}

Image2D *FFTTools::CreateAbsoluteImage(const Image2D &real, const Image2D &imaginary)
{
	Image2D *image = Image2D::CreateEmptyImage(real.Width(), real.Height());
	for(unsigned y=0;y<real.Height();++y) {
		for(unsigned x=0;x<real.Width();++x)
			image->SetValue(x, y, sqrtl(real.Value(x,y)*real.Value(x,y) + imaginary.Value(x,y)*imaginary.Value(x,y)));
	}
	return image;
} 

Image2DPtr FFTTools::CreatePhaseImage(Image2DCPtr real, Image2DCPtr imaginary)
{
	Image2DPtr image = Image2D::CreateEmptyImagePtr(real->Width(), real->Height());
	for(unsigned y=0;y<real->Height();++y) {
		for(unsigned x=0;x<real->Width();++x)
			image->SetValue(x, y, SinusFitter::Phase(real->Value(x,y), imaginary->Value(x,y)));
	}
	return image;
} 

void FFTTools::FFTConvolve(const Image2D &realIn, const Image2D &imaginaryIn, const Image2D &realKernel, const Image2D &imaginaryKernel, Image2D &outReal, Image2D &outImaginary)
{
	Image2D
		*realFFTIn = Image2D::CreateEmptyImage(realIn.Width(), realIn.Height()),
		*imaginaryFFTIn = Image2D::CreateEmptyImage(imaginaryIn.Width(), imaginaryIn.Height());
	CreateFFTImage(realIn, imaginaryIn, *realFFTIn, *imaginaryFFTIn); 
	Image2D
		*realFFTKernel = Image2D::CreateEmptyImage(realKernel.Width(), realKernel.Height()),
		*imaginaryFFTKernel = Image2D::CreateEmptyImage(imaginaryKernel.Width(), imaginaryKernel.Height());
	CreateFFTImage(realKernel, imaginaryKernel, *realFFTKernel, *imaginaryFFTKernel);

	Multiply(*realFFTIn, *imaginaryFFTIn, *realFFTKernel, *imaginaryFFTKernel);

	CreateFFTImage(*realFFTIn, *imaginaryFFTIn, outReal, outImaginary, true, true);
	
	delete imaginaryFFTKernel;
	delete realFFTKernel;
	delete imaginaryFFTIn;
	delete realFFTIn;
}

void FFTTools::FFTConvolveFFTKernel(const Image2D &realIn, const Image2D &imaginaryIn, const Image2D &realFFTKernel, const Image2D &imaginaryFFTKernel, Image2D &outReal, Image2D &outImaginary)
{
	Image2D
		*realFFTIn = Image2D::CreateEmptyImage(realIn.Width(), realIn.Height()),
		*imaginaryFFTIn = Image2D::CreateEmptyImage(imaginaryIn.Width(), imaginaryIn.Height());
	CreateFFTImage(realIn, imaginaryIn, *realFFTIn, *imaginaryFFTIn);

	Multiply(*realFFTIn, *imaginaryFFTIn, realFFTKernel, imaginaryFFTKernel);

	CreateFFTImage(*realFFTIn, *imaginaryFFTIn, outReal, outImaginary, true, true);
	
	delete imaginaryFFTIn;
	delete realFFTIn;
}

void FFTTools::Multiply(Image2D &left, const Image2D &right)
{
	for(unsigned y=0;y<left.Height();++y)
	{
		for(unsigned x=0;x<left.Width();++x)
			left.SetValue(x, y, left.Value(x,y)*right.Value(x,y));
	}
}

void FFTTools::Divide(Image2D &left, const Image2D &right)
{
	for(unsigned y=0;y<left.Height();++y)
	{
		for(unsigned x=0;x<left.Width();++x)
			left.SetValue(x, y, left.Value(x,y)/right.Value(x,y));
	}
}
 
void FFTTools::Multiply(Image2D &leftReal, Image2D &leftImaginary, const Image2D &rightReal, const Image2D &rightImaginary)
{
	for(unsigned y=0;y<leftReal.Height();++y)
	{
		for(unsigned x=0;x<leftReal.Width();++x) {
			long double r1 = leftReal.Value(x,y);
			long double i1 = leftImaginary.Value(x,y);
			long double r2 = rightReal.Value(x,y);
			long double i2 = rightImaginary.Value(x,y);
			leftReal.SetValue(x, y, r1*r2 - i1*i2);
			leftImaginary.SetValue(x, y, r1*i2 + r2*i1);
		}
	}
}

void FFTTools::Sqrt(Image2D &image)
{
	for(unsigned y=0;y<image.Height();++y) {
		for(unsigned x=0;x<image.Width();++x) {
			image.SetValue(x, y, sqrtl(fabs(image.Value(x, y))));
		}
	}
}

void FFTTools::CreateHorizontalFFTImage(Image2D &real, Image2D &imaginary, bool negate) throw()
{
	if(real.Height() == 0) return;
	unsigned long n_in = real.Width();
	fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n_in);
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n_in);

	for(unsigned long x=0;x<real.Width();++x) {
		in[x][0] = real.Value(x, 0);
		in[x][1] = imaginary.Value(x, 0);
	}

	int sign = 1;
	if(negate)
		sign = -1;

	for(unsigned long y=0;y<real.Height();++y) {
		for(unsigned long x=0;x<real.Width();++x) {
			in[x][0] = real.Value(x, y);
			in[x][1] = imaginary.Value(x, y);
		}
		fftw_plan plan = fftw_plan_dft_1d(real.Width(), in, out, sign, FFTW_ESTIMATE);
		fftw_execute(plan);
		for(unsigned long x=0;x<real.Width();++x) {
			real.SetValue(x, y, out[x][0]);
			imaginary.SetValue(x, y, out[x][1]);
		}
	}
}
