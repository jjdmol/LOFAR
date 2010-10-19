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

#ifndef IMAGE_H
#define IMAGE_H

#include "../msio/image2d.h"
#include "../msio/timefrequencydata.h"

#include "../util/ffttools.h"

/*struct Image {
	enum Type { SinglePolarisationReal, SinglePolarisationComplex, StokesI };
	virtual ~Image() { };
	virtual unsigned ImageCount() const = 0;
	virtual const class Image2D &Image2D(unsigned index) const = 0;
	virtual void Visualise(class Image2D &destination) const = 0;
};

struct SinglePolarisationRealImage : public Image {
	SinglePolarisationRealImage() throw() : image(0) { }
	SinglePolarisationRealImage(const class Image2D *initialImage) throw() : image(initialImage) { }
	const class Image2D *image;
	virtual unsigned ImageCount() const { return 1; };
	virtual const class Image2D &Image2D(unsigned index) const { return *image; }
	virtual void Visualise(class Image2D &destination) const
	{
		destination.SetValues(*image);
	}
};

struct SinglePolarisationComplexImage : public Image {
	SinglePolarisationComplexImage() : real(0), imaginary(0) { }
	const class Image2D *real, *imaginary;
	virtual unsigned ImageCount() const { return 2; };
	virtual const class Image2D &Image2D(unsigned index) const { if(index==0) return *real; else return *imaginary; }
	virtual void Visualise(class Image2D &destination) const
	{
		class Image2D *i = FFTTools::CreateAbsoluteImage(*real, *imaginary);
		destination.SetValues(*i);
		delete i;
	}
};*/

class SurfaceFitMethod {
	public:
		virtual void Initialize(const TimeFrequencyData &input) = 0;
		virtual unsigned TaskCount() = 0;
		virtual void PerformFit(unsigned taskNumber) = 0;
		virtual ~SurfaceFitMethod() { }
		virtual TimeFrequencyData Background() = 0;
		virtual enum TimeFrequencyData::PhaseRepresentation PhaseRepresentation() const = 0;
};

#endif
