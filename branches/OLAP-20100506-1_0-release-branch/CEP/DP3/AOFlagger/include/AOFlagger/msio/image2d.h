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
/** @file
 * This is the header file for the Image2D class.
 * @author André Offringa <offringa@gmail.com>
 */
#ifndef IMAGE2D_H
#define IMAGE2D_H

#include "../baseexception.h"
#include "colormap.h"
#include "types.h"

#include <boost/shared_ptr.hpp>

#include <exception>

typedef boost::shared_ptr<class Image2D> Image2DPtr;
typedef boost::shared_ptr<const class Image2D> Image2DCPtr;

/**
 * This class represents a two dimensional single-valued (=gray scale) image. It can be
 * read from and written to a @c .fits file and written to a @c .png file. A new Image2D can
 * be constructed with e.g. the CreateFromFits(), CreateEmptyImage() or CreateFromDiff() static methods.
 */
class Image2D {
	public:
		
		/**
		 * Creates an image containing unset zeros.
		 * @param width Width of the new image.
		 * @param height Height of the new image.
		 * @return The new created image. Should be deleted by the caller.
		 */
		static Image2D *CreateEmptyImage(long width, long height);
		static Image2DPtr CreateEmptyImagePtr(long width, long height)
		{
			return Image2DPtr(CreateEmptyImage(width, height));
		}
		
		/**
		 * Creates an image containing zeros that are "set".
		 * @param width Width of the new image.
		 * @param height Height of the new image.
		 * @return The new created image. Should be deleted by the caller.
		 */
		static Image2D *CreateZeroImage(long width, long height);
		static Image2DPtr CreateZeroImagePtr(long width, long height)
		{
			return Image2DPtr(CreateZeroImage(width, height));
		}

		/**
		 * Destructor.
		 */
		~Image2D();
		
		/**
		 * Creates a new image by subtracting two images of the same size.
		 * @param imageA first image.
		 * @param imageB second image.
		 * @return The new created image. Should be deleted by the caller.
		 * @throws FitsIOException if the images do not match in size.
		 */
		static Image2D *CreateFromSum(const Image2D &imageA, const Image2D &imageB);
		static Image2DPtr CreateFromSum(Image2DCPtr imageA, Image2DCPtr imageB)
		{
			return Image2DPtr(CreateFromSum(*imageA, *imageB));
		}
		/**
		 * Creates a new image by subtracting two images of the same size.
		 * @param imageA first image.
		 * @param imageB second image.
		 * @return The new created image. Should be deleted by the caller.
		 * @throws FitsIOException if the images do not match in size.
		 */
		static Image2D *CreateFromDiff(const Image2D &imageA, const Image2D &imageB);
		static Image2DPtr CreateFromDiff(Image2DCPtr imageA, Image2DCPtr imageB)
		{
			return Image2DPtr(CreateFromDiff(*imageA, *imageB));
		}

		static Image2D *CreateCopy(const Image2D &image);
		static Image2DPtr CreateCopy(Image2DCPtr image)
		{
			return Image2DPtr(CreateCopy(*image));
		}
		static Image2DPtr CreateCopyPtr(const Image2D &image)
		{
			return Image2DPtr(CreateCopy(image));
		}

		void Clear();

		/**
		 * Retrieves the average value of the image.
		 * @return The average value.
		 */
		num_t GetAverage() const;
		
		/**
		 * Returns the maximum value in the image.
		 * @return The maximimum value.
		 */
		num_t GetMaximum() const;
		
		/**
		 * Returns the maximum value in the specified range.
		 * @return The maximimum value.
		 */
		num_t GetMaximum(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height) const;

		/**
		 * Returns the minimum value in the image.
		 * @return The minimum value.
		 */
		num_t GetMinimum() const;
		
		/**
		 * Returns the minimum value in the specified range.
		 * @return The minimum value.
		 */
		num_t GetMinimum(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height) const;

		/**
		 * Returns the maximum value in the image.
		 * @return The maximimum value.
		 */
		num_t GetMaximumFinite() const;
		
		/**
		 * Returns the minimum value in the image.
		 * @return The minimum value.
		 */
		num_t GetMinimumFinite() const;
		
		/**
		 * Retrieves the number of unset values in the image.
		 * @return The number of unset values.
		 */
		unsigned long GetUnsetValueCount() const;
		
		/**
		 * Retrieves the value at a specific position.
		 * @param x x-coordinate
		 * @param y y-coordinate
		 * @return The value.
		 */
		inline num_t Value(long x, long y) const { return _data[y*_width+x]; }
		
		/**
		 * Retrieves the value at a specific offset (y * width + x)
		 * @param ptr The pointer to the position.
		 * @return The value.
		 */
		inline num_t Value(long ptr) const { return _data[ptr]; }
		
		/**
		 * Tests whether the value has been set.
		 * @param x x-coordinate
		 * @param y y-coordinate
		 * @return @c true if value has been set.
		 */
		inline bool IsSet(long x, long y) const { return _isSet[y*_width+x]; }
		
		/**
		 * Tests whether the value has been set.
		 * @param ptr The pointer to the position.
		 * @return @c true if value has been set.
		 */
		inline bool IsSet(long ptr) const { return _isSet[ptr]; }
		
		/**
		 * Retrieve the width of the image.
		 * @return Width of the image.
		 */
		inline unsigned long Width() const { return _width; }
		
		/**
		 * Retrieve the height of the image.
		 * @return Height of the image.
		 */
		inline unsigned long Height() const { return _height; }
		
		/**
		 * Change a value at a specific position.
		 * @param x x-coordinate of value to change.
		 * @param y y-coordinate of value to change.
		 * @param newValue New value.
		 */
		inline void SetValue(long x, long y, num_t newValue)
		{
			long i = y*_width+x;
			_isSet[i] = true;
			_data[i] = newValue;
		}

		void SetValues(const Image2D &source);

		void SetZero();
		
		inline void AddValue(long x, long y, num_t addValue)
		{
			long i = y*_width+x;
			if(_isSet[i])
				_data[i] += addValue;
			else
				_data[i] = addValue;
			_isSet[i] = true;
		}
		
		/**
		 * Check whether this value is completely empty.
		 * @return @c true if the value only contains zeros or unset values.
		 */
		bool ContainsOnlyZeros() const;
		
		/**
		 * Retrieve a factor to multiply the values with to normalise them.
		 * @return Normalisation factor.
		 */
		num_t GetMaxMinNormalizationFactor() const;

		num_t GetStdDev() const;

		num_t GetRMS() const
		{
			return GetRMS(0, 0, _width, _height);
		}

		num_t GetRMS(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height) const;

		/**
		 * Normalize the data so that the variance is 1.
		 */
		void NormalizeVariance();

		/**
		* Create a new image instance by reading a fitsfile.
		* @param file The fits file.
		* @param imageNumber The number of the image.
		* @return The new created image. Should be deleted by the caller.
		* @throws FitsIOException if something goes wrong during reading the .fits file.
		*/
		static Image2D *CreateFromFits(class FitsFile &file, int imageNumber);

		/**
		* Number of images that can be read from the current HUD block
		* in the fits file.
		* @param file Fits file.
		* @return Number of images.
		*/
		static long GetImageCountInHUD(class FitsFile &file);

		/**
		* Save the image to a fits file.
		* @param filename Fits filename.
		* @throws IOException if something goes wrong during writing
		*/
		void SaveToFitsFile(const std::string &filename) const;

		/**
		 * Count the number of values that are above a specified value.
		 */
		size_t GetCountAbove(num_t value) const;
		size_t GetCountBelowOrEqual(num_t value) const
		{
			return _width*_height - GetCountAbove(value);
		}

		/**
		 * Returns a threshold for which #count values are above the
		 * the threshold. That is, GetCountAbove(GetTresholdForCountAbove(x)) = x.
		 */
		num_t GetTresholdForCountAbove(size_t count) const;

		/**
		 * Copies all values to the specified array. The array should be of size width*height.
		 */
		void CopyData(num_t *destination) const;

		/**
		 * Multiply all values with a factor.
		 */
		void MultiplyValues(num_t factor);

		/**
		 * Resample the image horizontally by decreasing the width
		 * with an integer factor.
		 */
		Image2DPtr ShrinkHorizontally(int factor) const;

		/**
		 * Resample the image horizontally by inreasing the width
		 * with an integer factor.
		 */
		Image2DPtr EnlargeHorizontally(int factor, size_t newWidth) const;

		Image2DPtr Trim(unsigned long startX, unsigned long startY, unsigned long endX, unsigned long endY) const;
		void SetTrim(unsigned long startX, unsigned long startY, unsigned long endX, unsigned long endY);
	private:
		Image2D(long width, long height);
		unsigned long _width, _height;
		num_t *_data;
		bool *_isSet;
};

#endif
