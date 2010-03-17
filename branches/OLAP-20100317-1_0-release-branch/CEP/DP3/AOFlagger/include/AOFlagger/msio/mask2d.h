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
#ifndef MASK2D_H
#define MASK2D_H

#include <boost/shared_ptr.hpp>

#include "image2d.h"

typedef boost::shared_ptr<class Mask2D> Mask2DPtr;
typedef boost::shared_ptr<const class Mask2D> Mask2DCPtr;

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Mask2D {
	public:
		~Mask2D();

		static Mask2D *CreateUnsetMask(size_t width, size_t height)
		{
			return new Mask2D(width, height);
		}
		static Mask2DPtr CreateUnsetMaskPtr(size_t width, size_t height)
		{
			return Mask2DPtr(new Mask2D(width, height));
		}

		static Mask2D *CreateUnsetMask(const class Image2D &templateImage);
		static Mask2DPtr CreateUnsetMask(Image2DCPtr templateImage)
		{
			return Mask2DPtr(CreateUnsetMask(*templateImage));
		}

		template<bool InitValue>
		static Mask2D *CreateSetMask(const class Image2D &templateImage);

		template<bool InitValue>
		static Mask2DPtr CreateSetMask(Image2DCPtr templateImage)
		{
			return Mask2DPtr(CreateSetMask<InitValue>(*templateImage));
		}

		template<bool InitValue>
		static Mask2D *CreateSetMask(size_t width, size_t height)
		{
			Mask2D *newMask = new Mask2D(width, height);
			for(size_t y=0;y<height;++y)
			{
				for(size_t x=0;x<width;++x)
					newMask->_values[y][x] = InitValue;
			}
			return newMask;
		}

		template<bool InitValue>
		static Mask2DPtr CreateSetMaskPtr(size_t width, size_t height)
		{
			return Mask2DPtr(CreateSetMask<InitValue>(width, height));
		}

		static Mask2D *CreateCopy(const Mask2D &source);
		static Mask2DPtr CreateCopy(Mask2DCPtr source)
		{
			return Mask2DPtr(CreateCopy(*source));
		}

		inline bool Value(size_t x, size_t y) const
		{
			return _values[y][x];
		}
		inline void SetValue(size_t x, size_t y, bool newValue) const
		{
			_values[y][x] = newValue;
		}
		inline size_t Width() const { return _width; }
		inline size_t Height() const { return _height; }

		bool AllFalse() const
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
				{
					if(_values[y][x])
						return false;
				}
			}
			return true;
		}

		template<bool BoolValue>
		void SetAll()
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					_values[y][x] = BoolValue;
			}
		}

		template<bool BoolValue>
		void SetAllVertically(size_t x)
		{
			for(size_t y=0;y<_height;++y)
				_values[y][x] = BoolValue;
		}

		template<bool BoolValue>
		void SetAllVertically(size_t startX, size_t endX)
		{
			for(size_t x=startX;x<endX;++x)
			{
				for(size_t y=0;y<_height;++y)
					_values[y][x] = BoolValue;
			}
		}

		template<bool BoolValue>
		void SetAllHorizontally(size_t y)
		{
			for(size_t x=0;x<_width;++x)
				_values[y][x] = BoolValue;
		}

		template<bool BoolValue>
		void SetAllHorizontally(size_t startY, size_t endY)
		{
			for(size_t y=startY;y<endY;++y)
			{
				for(size_t x=0;x<_width;++x)
					_values[y][x] = BoolValue;
			}
		}

		// This method assumes equal height and width.
		void operator=(Mask2DCPtr source)
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					_values[y][x] = source->_values[y][x];
			}
		}

		void Invert()
		{
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					_values[y][x] = !_values[y][x];
			}
		}

		template<bool BoolValue>
		size_t GetCount() const
		{
			size_t count = 0;
			for(size_t y=0;y<_height;++y)
			{
				for(size_t x=0;x<_width;++x)
					if(BoolValue == _values[y][x])
						++count;
			}
			return count;
		}

		Mask2DPtr ShrinkHorizontally(int factor) const;

		void EnlargeHorizontallyAndSet(Mask2DCPtr smallMask, int factor);

		void Join(Mask2DCPtr other)
		{
			for(unsigned y=0;y<_height;++y) {
				for(unsigned x=0;x<_width;++x)
					SetValue(x, y, other->Value(x, y) || Value(x, y));
			}
		}
	private:
		Mask2D(size_t width, size_t height);

		size_t _width, _height;
		bool **_values;
};

#endif
