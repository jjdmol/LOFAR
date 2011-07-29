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

		static Mask2D *CreateUnsetMask(unsigned width, unsigned height)
		{
			return new Mask2D(width, height);
		}
		static Mask2DPtr CreateUnsetMaskPtr(unsigned width, unsigned height)
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
		static Mask2D *CreateSetMask(unsigned width, unsigned height)
		{
			Mask2D *newMask = new Mask2D(width, height);
			for(unsigned y=0;y<height;++y)
			{
				for(unsigned x=0;x<width;++x)
					newMask->_values[y][x] = InitValue;
			}
			return newMask;
		}

		template<bool InitValue>
		static Mask2DPtr CreateSetMaskPtr(unsigned width, unsigned height)
		{
			return Mask2DPtr(CreateSetMask<InitValue>(width, height));
		}

		static Mask2D *CreateCopy(const Mask2D &source);
		static Mask2DPtr CreateCopy(Mask2DCPtr source)
		{
			return Mask2DPtr(CreateCopy(*source));
		}

		inline bool Value(unsigned x, unsigned y) const
		{
			return _values[y][x];
		}
		
		inline void SetValue(unsigned x, unsigned y, bool newValue)
		{
			_values[y][x] = newValue;
		}
		
		inline unsigned Width() const { return _width; }
		
		inline unsigned Height() const { return _height; }

		bool AllFalse() const
		{
			for(unsigned y=0;y<_height;++y)
			{
				for(unsigned x=0;x<_width;++x)
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
			for(unsigned y=0;y<_height;++y)
			{
				for(unsigned x=0;x<_width;++x)
					_values[y][x] = BoolValue;
			}
		}

		template<bool BoolValue>
		void SetAllVertically(unsigned x)
		{
			for(unsigned y=0;y<_height;++y)
				_values[y][x] = BoolValue;
		}

		template<bool BoolValue>
		void SetAllVertically(unsigned startX, unsigned endX)
		{
			for(unsigned x=startX;x<endX;++x)
			{
				for(unsigned y=0;y<_height;++y)
					_values[y][x] = BoolValue;
			}
		}

		template<bool BoolValue>
		void SetAllHorizontally(unsigned y)
		{
			for(unsigned x=0;x<_width;++x)
				_values[y][x] = BoolValue;
		}

		template<bool BoolValue>
		void SetAllHorizontally(unsigned startY, unsigned endY)
		{
			for(unsigned y=startY;y<endY;++y)
			{
				for(unsigned x=0;x<_width;++x)
					_values[y][x] = BoolValue;
			}
		}

		// This method assumes equal height and width.
		void operator=(Mask2DCPtr source)
		{
			for(unsigned y=0;y<_height;++y)
			{
				for(unsigned x=0;x<_width;++x)
					_values[y][x] = source->_values[y][x];
			}
		}

		void Invert()
		{
			for(unsigned y=0;y<_height;++y)
			{
				for(unsigned x=0;x<_width;++x)
					_values[y][x] = !_values[y][x];
			}
		}

		template<bool BoolValue>
		unsigned GetCount() const
		{
			unsigned count = 0;
			for(unsigned y=0;y<_height;++y)
			{
				for(unsigned x=0;x<_width;++x)
					if(BoolValue == _values[y][x])
						++count;
			}
			return count;
		}

		Mask2DPtr ShrinkHorizontally(int factor) const;
		Mask2DPtr ShrinkVertically(int factor) const;

		void EnlargeHorizontallyAndSet(Mask2DCPtr smallMask, int factor);
		void EnlargeVerticallyAndSet(Mask2DCPtr smallMask, int factor);

		void Join(Mask2DCPtr other)
		{
			for(unsigned y=0;y<_height;++y) {
				for(unsigned x=0;x<_width;++x)
					SetValue(x, y, other->Value(x, y) || Value(x, y));
			}
		}
		Mask2DPtr Trim(unsigned long startX, unsigned long startY, unsigned long endX, unsigned long endY) const
		{
			unsigned
				width = endX - startX,
				height = endY - startY;
			Mask2D *mask = new Mask2D(width, height);
			for(unsigned y=startY;y<endY;++y)
			{
				for(unsigned x=startX;x<endX;++x)
					mask->SetValue(x-startX, y-startY, Value(x, y));
			}
			return Mask2DPtr(mask);
		}
		
		void CopyFrom(Mask2DCPtr source, unsigned destX, unsigned destY)
		{
			unsigned
				x2 = source->_width + destX,
				y2 = source->_height + destY;
			if(x2 > _width) x2 = _width;
			if(y2 > _height) y2 = _height;
			for(unsigned y=destY;y<y2;++y)
			{
				for(unsigned x=destX;x<x2;++x)
					SetValue(x, y, source->Value(x-destX, y-destY));
			}
		}
	private:
		Mask2D(unsigned width, unsigned height);

		unsigned _width, _height;
		bool **_values;
};

#endif
