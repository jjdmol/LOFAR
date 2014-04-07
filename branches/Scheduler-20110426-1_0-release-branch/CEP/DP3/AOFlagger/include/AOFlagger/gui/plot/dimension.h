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
#ifndef DIMENSION_H
#define DIMENSION_H

#include "plot2dpointset.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Dimension {
	public:
		Dimension() : _pointSets(0) { }
		~Dimension() { }

		void AdjustRanges(Plot2DPointSet &pointSet)
		{
			if(_pointSets == 0)
			{
				_xRangeMin = pointSet.XRangeMin();
				_xRangeMax = pointSet.XRangeMax();
				_yRangeMin = pointSet.YRangeMin();
				_yRangeMax = pointSet.YRangeMax();
			} else {
				if(_xRangeMin > pointSet.XRangeMin())
					_xRangeMin = pointSet.XRangeMin();
				if(_xRangeMax < pointSet.XRangeMax())
					_xRangeMax = pointSet.XRangeMax();
				if(_yRangeMin > pointSet.YRangeMin())
					_yRangeMin = pointSet.YRangeMin();
				if(_yRangeMax < pointSet.YRangeMax())
					_yRangeMax = pointSet.YRangeMax();
			}
			++_pointSets;
		}

		num_t XRangeMin() const { return _xRangeMin; }
		num_t XRangeMax() const { return _xRangeMax; }
		num_t YRangeMin() const { return _yRangeMin; }
		num_t YRangeMax() const { return _yRangeMax; }
	private:
		size_t _pointSets;
		num_t _xRangeMin, _xRangeMax;
		num_t _yRangeMin, _yRangeMax;
};

#endif
