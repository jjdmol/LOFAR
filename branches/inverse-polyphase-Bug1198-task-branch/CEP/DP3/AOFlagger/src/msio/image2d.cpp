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
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/pngfile.h>
#include <AOFlagger/msio/fitsfile.h>

#include <algorithm>
#include <limits>

#include <iostream>

Image2D::Image2D(long width, long height) : _width(width), _height(height)
{
	//std::cout << "Requesting " << (sizeof(num_t[width*height]) + sizeof(bool[width*height])) << " bytes of memory for a " << width << " x " << height << " image." << std::endl;

	_data = new num_t[width * height];
	_isSet = new bool[width * height];
}

Image2D::~Image2D()
{
	delete[] _data;
	delete[] _isSet;

	//std::cout << "Freed "  << (sizeof(num_t[_width*_height]) + sizeof(bool[_width*_height])) << " bytes of memory for a " << _width << " x " << _height << " image." << std::endl;
}

Image2D *Image2D::CreateEmptyImage(long width, long height) 
{
	Image2D *image = new Image2D(width, height);
	image->Clear();
	return image;
}

Image2D *Image2D::CreateZeroImage(long width, long height) 
{
	Image2D *image = new Image2D(width, height);
	image->SetZero();
	return image;
}

void Image2D::SetZero() 
{
	for(long unsigned y=0;y<_height;y++) {
		for(long unsigned x=0;x<_width;x++) {
			_isSet[y*_width+x] = true;
			_data[y*_width+x] = 0.0;
		}
	}
}

Image2D *Image2D::CreateFromSum(const Image2D &imageA, const Image2D &imageB)
{
	if(imageA.Width() != imageB.Width() || imageA.Height() != imageB.Height())
		throw IOException("Images do not match in size");
	long width = imageA.Width(), height = imageA.Height();
	Image2D *image = new Image2D(width, height);
	for(long y=0;y<height;y++) {
		for(long x=0;x<width;x++) {
			image->_isSet[y*width+x] = imageA._isSet[y*width+x] && imageB._isSet[y*width+x];
			image->_data[y*width+x] = imageA._data[y*width+x] + imageB._data[y*width+x];
		}
	}
	return image;
}

Image2D *Image2D::CreateFromDiff(const Image2D &imageA, const Image2D &imageB)
{
	if(imageA.Width() != imageB.Width() || imageA.Height() != imageB.Height())
		throw IOException("Images do not match in size");
	long width = imageA.Width(), height = imageA.Height();
	Image2D *image = new Image2D(width, height);
	for(long y=0;y<height;y++) {
		for(long x=0;x<width;x++) {
			image->_isSet[y*width+x] = imageA._isSet[y*width+x] && imageB._isSet[y*width+x];
			image->_data[y*width+x] = imageA._data[y*width+x] - imageB._data[y*width+x];
		}
	}
	return image;
}

Image2D *Image2D::CreateCopy(const Image2D &image)
{
	long width = image.Width(), height = image.Height();
	Image2D *newImage = new Image2D(width, height);
	for(long i=0;i<width*height;i++) {
		newImage->_isSet[i] = image._isSet[i];
		newImage->_data[i] = image._data[i];
	}
	return newImage;
}

void Image2D::SetValues(const Image2D &source)
{
	for(unsigned i=0;i<_width*_height;i++) {
		_isSet[i] = source._isSet[i];
		_data[i] = source._data[i];
	}
}

void Image2D::Clear()
{
	for(unsigned y=0;y<_height;y++) {
		for(unsigned x=0;x<_width;x++) {
			_isSet[y*_width+x] = false;
			_data[y*_width+x] = 0.0;
		}
	}
}

num_t Image2D::GetAverage() const {
	long count = 0;
	num_t total = 0.0;
	for(unsigned long i=0;i<_width * _height;i++) {
		if(_isSet[i]) {
			total += _data[i];
			count++;
		}
	}
	return total/(num_t) count;
}

num_t Image2D::GetMaximum() const {
	num_t max = -1e100;
	for(unsigned long i=0;i<_width * _height;++i) {
		if(_isSet[i] && _data[i] > max) {
			max = _data[i];
		}
	}
	return max;
}

num_t Image2D::GetMinimum() const {
	num_t min = 1e100;
	for(unsigned long i=0;i<_width * _height;++i) {
		if(_isSet[i] && _data[i] < min) {
			min = _data[i];
		}
	}
	return min;
}

num_t Image2D::GetMaximumFinite() const {
	num_t max = -1e100;
	for(unsigned long i=0;i<_width * _height;++i) {
		if(_isSet[i] && isfinite(_data[i]) && _data[i] > max) {
			max = _data[i];
		}
	}
	return max;
}

num_t Image2D::GetMinimumFinite() const {
	num_t min = 1e100;
	for(unsigned long i=0;i<_width * _height;++i) {
		if(_isSet[i] && isfinite(_data[i]) && _data[i] < min) {
			min = _data[i];
		}
	}
	return min;
}

unsigned long Image2D::GetUnsetValueCount() const {
	unsigned long count = 0;
	for(unsigned long i=0;i<_width * _height;i++) {
		if(!_isSet[i])
			count++;
	}
	return count;
}

bool Image2D::ContainsOnlyZeros() const 
{
	for(unsigned long i=0;i<_width * _height;++i)
	{
		if(_isSet[i] && _data[i] != 0.0)
			return false;
	}
	return true;
}

num_t Image2D::GetMaxMinNormalizationFactor() const
{
	num_t max = GetMaximum(), min = GetMinimum();
	num_t range = (-min) > max ? (-min) : max;
	return 1.0/range;
}

num_t Image2D::GetStdDev() const
{
	num_t mean = GetAverage();
	unsigned long count = 0;
	num_t total = 0.0;
	for(unsigned long i=0;i<_width * _height;++i)
	{
		if(_isSet[i]) {
			total += (_data[i]-mean)*(_data[i]-mean);
			count++;
		}
	}
	return sqrt(total / (num_t) count);
}

num_t Image2D::GetRMS(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height) const
{
	unsigned long count = 0;
	num_t total = 0.0;
	for(unsigned long y=yOffset;y<height+yOffset;++y) {
		for(unsigned long x=xOffset;x<width+xOffset;++x)
		{
			if(IsSet(x,y)) {
				num_t v = Value(x, y);
				total += v * v;
				count++;
			}
		}
	}
	return sqrt(total / (num_t) count);
}

void Image2D::NormalizeVariance()
{
	num_t variance = GetStdDev();
	for(unsigned long i=0;i<_width * _height;++i)
		_data[i] /= variance;
}

Image2D *Image2D::CreateFromFits(FitsFile &file, int imageNumber)
{
      int dimensions = file.GetCurrentImageDimensionCount();
      if(dimensions >= 2) {
              Image2D *image = new Image2D(file.GetCurrentImageSize(1), file.GetCurrentImageSize(2));
              long bufferSize = image->_width * image->_height;
              file.ReadCurrentImageData(bufferSize*imageNumber, image->_data, bufferSize, -1e100);
              for(int i=0;i<bufferSize;i++) {
                      image->_isSet[i] = (image->_data[i] != -1e100);
											if(!image->_isSet[i])
												image->_data[i]=0.0;
							}
              return image;
      } else {
              throw FitsIOException("No 2D images in HUD");
      }
}

long Image2D::GetImageCountInHUD(FitsFile &file) {
      int dimensions = file.GetCurrentImageDimensionCount();
      long total2DImageCount = 0;
      if(dimensions>=2) {
              total2DImageCount = 1;
              for(int j=3;j<=dimensions;j++) {
                      total2DImageCount *= file.GetCurrentImageSize(j);
              }
      }
      return total2DImageCount;
}

void Image2D::SaveToFitsFile(const std::string &filename) const
{
      FitsFile file(filename);
      file.Create();
      file.AppendImageHUD(FitsFile::Double64ImageType, _width, _height);
      long bufferSize = _width * _height;
      double *buffer = new double[bufferSize];
      for(long i=0;i<bufferSize;i++) {
              if(_isSet[i])
                      buffer[i] = _data[i];
              else
                      buffer[i] = -1e100;
      }
      try {
              file.WriteImage(0, buffer, bufferSize, -1e100);
              file.Close();
      } catch(FitsIOException &exception) {
              delete[] buffer;
              throw;
      }
      delete[] buffer;
}

size_t Image2D::GetCountAbove(num_t value) const
{
	size_t count=0;
	for(size_t i=0;i<_width * _height;++i)
		if(_data[i] > value) count++;
	return count;
}

num_t Image2D::GetTresholdForCountAbove(size_t count) const
{
	num_t *sorted = new num_t[_width * _height];
	for(size_t i=0;i<_width * _height;++i)
		sorted[i] = _data[i];
	std::sort(sorted, sorted + _width * _height);
	num_t v = sorted[_width * _height - count - 1];
	delete[] sorted;
	return v;
}

void Image2D::CopyData(num_t *destination) const
{
	for(size_t i=0;i<_width * _height;++i)
		destination[i] = _data[i];
}

void Image2D::MultiplyValues(num_t factor)
{
	for(size_t i=0;i<_width * _height;++i)
		_data[i] *= factor;
}

Image2DPtr Image2D::ShrinkHorizontally(int factor) const
{
	size_t newWidth = (_width + factor - 1) / factor;

	Image2D *newImage = new Image2D(newWidth, _height);

	for(size_t x=0;x<newWidth;++x)
	{
		int binSize = factor;
		if(binSize + x*factor > _width)
			binSize = _width - x*factor;

		for(size_t y=0;y<_height;++y)
		{
			num_t sum = 0.0;
			for(int binX=0;binX<binSize;++binX)
			{
				int curX = x*factor + binX;
				sum += Value(curX, y);
			}
			newImage->SetValue(x, y, sum / (num_t) binSize);
		}
	}
	return Image2DPtr(newImage);
}

Image2DPtr Image2D::EnlargeHorizontally(int factor, size_t newWidth) const
{
	Image2D *newImage = new Image2D(newWidth, _height);

	for(size_t x=0;x<newWidth;++x)
	{
		size_t xOld = x / factor;

		for(size_t y=0;y<_height;++y)
		{
			newImage->SetValue(x, y, Value(xOld, y));
		}
	}
	return Image2DPtr(newImage);
}

Image2DPtr Image2D::Trim(unsigned long startX, unsigned long startY, unsigned long endX, unsigned long endY) const
{
	unsigned
		width = endX - startX,
		height = endY - startY;
	Image2D *image = new Image2D(width, height);
	unsigned long newPtr = 0;
	for(unsigned y=startY;y<endY;++y)
	{
		unsigned long oldPtr = y * _width + startX;
		for(unsigned x=startX;x<endX;++x)
		{
			image->_data[newPtr] = _data[oldPtr];
			image->_isSet[newPtr] = _isSet[oldPtr];
			++newPtr;
			++oldPtr;
		}
	}
	return Image2DPtr(image);
}

void Image2D::SetTrim(unsigned long startX, unsigned long startY, unsigned long endX, unsigned long endY)
{
	unsigned
		newWidth = endX - startX,
		newHeight = endY - startY;
	num_t *newData = new num_t[newWidth * newHeight];
	bool *newIsSet = new bool[newWidth * newHeight];
	unsigned long newPtr = 0;
	for(unsigned y=startY;y<endY;++y)
	{
		unsigned long oldPtr = y * _width + startX;
		for(unsigned x=startX;x<endX;++x)
		{
			newData[newPtr] = _data[oldPtr];
			newIsSet[newPtr] = _isSet[oldPtr];
			++newPtr;
			++oldPtr;
		}
	}
	delete[] _data;
	delete[] _isSet;
	_data = newData;
	_isSet = newIsSet;
	_width = newWidth;
	_height = newHeight;
}

/**
	* Returns the maximum value in the specified range.
	* @return The maximimum value.
	*/
num_t Image2D::GetMaximum(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height) const
{
	size_t count = 0;
	num_t max =0.0;
	for(unsigned long y=yOffset;y<height+yOffset;++y) {
		for(unsigned long x=xOffset;x<width+xOffset;++x)
		{
			if(IsSet(x,y))
			{
				if(Value(x,y) > max || count==0)
				{
					max = Value(x, y);
					++count;
				}
			}
		}
	}
	if(count == 0)
		return std::numeric_limits<num_t>::quiet_NaN();
	return max;
}

/**
	* Returns the minimum value in the specified range.
	* @return The minimum value.
	*/
num_t Image2D::GetMinimum(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height) const
{
	size_t count = 0;
	num_t min = 0.0;
	for(unsigned long y=yOffset;y<height+yOffset;++y) {
		for(unsigned long x=xOffset;x<width+xOffset;++x)
		{
			if(IsSet(x,y))
			{
				if(Value(x,y) < min || count==0)
				{
					min = Value(x, y);
					++count;
				}
			}
		}
	}
	if(count == 0)
		return std::numeric_limits<num_t>::quiet_NaN();
	return min;
}

