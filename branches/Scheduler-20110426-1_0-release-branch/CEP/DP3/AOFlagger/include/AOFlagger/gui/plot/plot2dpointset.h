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
#ifndef PLOT2DPOINTSET_H
#define PLOT2DPOINTSET_H

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "../../msio/types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Plot2DPointSet{
	public:
		Plot2DPointSet() { }
		~Plot2DPointSet() { }

		void SetLabel(const std::string &label) { _label = label; }
		const std::string &Label() const { return _label; }

		const std::string &YUnits() const { return _yUnits; }
		void SetYUnits(std::string yUnits) { _yUnits = yUnits; }

		void PushDataPoint(num_t x, num_t y)
		{
			_points.push_back(Point2D(x,y));
		}
		num_t GetX(size_t index) const { return _points[index].x; }
		num_t GetY(size_t index) const { return _points[index].y; }
		size_t Size() const { return _points.size(); }

		num_t MaxX() const
		{
			if(_points.empty())
				return std::numeric_limits<num_t>::quiet_NaN();
			std::vector<Point2D>::const_iterator i = _points.begin();
			num_t max = std::numeric_limits<num_t>::quiet_NaN();
			for(std::vector<Point2D>::const_iterator i = _points.begin();i!=_points.end();++i)
			{
				if((i->x > max || (!std::isfinite(max))) && std::isfinite(i->x) ) max = i->x;
			}
			return max;
		}
		num_t MinX() const
		{
			if(_points.empty())
				return std::numeric_limits<num_t>::quiet_NaN();
			std::vector<Point2D>::const_iterator i = _points.begin();
			num_t min = std::numeric_limits<num_t>::quiet_NaN();
			for(std::vector<Point2D>::const_iterator i = _points.begin();i!=_points.end();++i)
			{
				if((i->x < min || (!std::isfinite(min))) && std::isfinite(i->x) ) min = i->x;
			}
			return min;
		}
		num_t MaxY() const
		{
			if(_points.empty())
				return std::numeric_limits<num_t>::quiet_NaN();
			std::vector<Point2D>::const_iterator i = _points.begin();
			num_t max = std::numeric_limits<num_t>::quiet_NaN();
			for(std::vector<Point2D>::const_iterator i = _points.begin();i!=_points.end();++i)
			{
				if((i->y > max || (!std::isfinite(max))) && std::isfinite(i->y) ) max = i->y;
			}
			return max;
		}
		num_t MinY() const
		{
			if(_points.empty())
				return std::numeric_limits<num_t>::quiet_NaN();
			std::vector<Point2D>::const_iterator i = _points.begin();
			num_t min = std::numeric_limits<num_t>::quiet_NaN();
			for(std::vector<Point2D>::const_iterator i = _points.begin();i!=_points.end();++i)
			{
				if((i->y < min || (!std::isfinite(min))) && std::isfinite(i->y) ) min = i->y;
			}
			return min;
		}
		void Sort()
		{
			std::sort(_points.begin(), _points.end());
		}
		num_t XRangeMin() const { return _points.begin()->x; }
		num_t XRangeMax() const { return _points.rbegin()->x; }
		num_t YRangeMin() const { return MinY(); }
		num_t YRangeMax() const { return MaxY(); }

	private:
		struct Point2D
		{
			Point2D(num_t _x, num_t _y) : x(_x), y(_y) { }
			num_t x, y;
			bool operator<(const Point2D &other) const
			{
				return x < other.x;
			}
		};

		std::vector<Point2D> _points;
		std::string _label;
		std::string _yUnits;
};

#endif
