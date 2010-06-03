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
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/image2d.h>

#include <iostream>

Mask2D::Mask2D(size_t width, size_t height) :
	_width(width),
	_height(height),
	_values(new bool*[height])
{
	//std::cout << "Requesting " << (sizeof(bool*[height]) + sizeof(bool[width])*height) << " bytes of memory for a " << width << " x " << height << " mask." << std::endl;

	for(size_t i=0;i<height;++i)
		_values[i] = new bool[width];
}

Mask2D::~Mask2D()
{
	//std::cout << "Freed "  << (sizeof(bool*[_height]) + sizeof(bool[_width])*_height) << " bytes of memory for a " << _width << " x " << _height << " mask." << std::endl;
	for(size_t i=0;i<_height;++i)
		delete[] _values[i];
	delete[] _values;
}

Mask2D *Mask2D::CreateUnsetMask(const Image2D &templateImage)
{
	return new Mask2D(templateImage.Width(), templateImage.Height());
}

template <bool InitValue>
Mask2D *Mask2D::CreateSetMask(const class Image2D &templateImage)
{
	size_t
		width = templateImage.Width(),
		height = templateImage.Height();

	Mask2D *newMask = new Mask2D(width, height);
	for(size_t y=0;y<height;++y)
	{
		for(size_t x=0;x<width;++x)
			newMask->_values[y][x] = InitValue;
	}
	return newMask;
}

template Mask2D *Mask2D::CreateSetMask<false>(const class Image2D &templateImage);
template Mask2D *Mask2D::CreateSetMask<true>(const class Image2D &templateImage);

Mask2D *Mask2D::CreateCopy(const Mask2D &source)
{
	size_t
		width = source.Width(),
		height = source.Height();

	Mask2D *newMask = new Mask2D(width, height);
	for(size_t y=0;y<height;++y)
	{
		for(size_t x=0;x<width;++x)
			newMask->_values[y][x] = source._values[y][x];
	}
	return newMask;
}

Mask2DPtr Mask2D::ShrinkHorizontally(int factor) const
{
	size_t newWidth = (_width + factor - 1) / factor;

	Mask2D *newMask= new Mask2D(newWidth, _height);

	for(size_t x=0;x<newWidth;++x)
	{
		size_t binSize = factor;
		if(binSize + x*factor > _width)
			binSize = _width - x*factor;

		for(size_t y=0;y<_height;++y)
		{
			bool value = false;
			for(size_t binX=0;binX<binSize;++binX)
			{
				size_t curX = x*factor + binX;
				value = value | Value(curX, y);
			}
			newMask->SetValue(x, y, value);
		}
	}
	return Mask2DPtr(newMask);
}

void Mask2D::EnlargeHorizontallyAndSet(Mask2DCPtr smallMask, int factor)
{
	for(size_t x=0;x<smallMask->Width();++x)
	{
		size_t binSize = factor;
		if(binSize + x*factor > _width)
			binSize = _width - x*factor;

		for(size_t y=0;y<_height;++y)
		{
			for(size_t binX=0;binX<binSize;++binX)
			{
				size_t curX = x*factor + binX;
				SetValue(curX, y, smallMask->Value(x, y));
			}
		}
	}
}

