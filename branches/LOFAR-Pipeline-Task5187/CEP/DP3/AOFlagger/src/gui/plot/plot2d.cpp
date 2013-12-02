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
#include <AOFlagger/gui/plot/plot2d.h>

#include <iostream>

Plot2D::Plot2D()
{
}

Plot2D::~Plot2D()
{
	Clear();
}

void Plot2D::Clear()
{
	for(std::vector<Plot2DPointSet*>::iterator i=_pointSets.begin();i!=_pointSets.end();++i)
		delete *i;
	_pointSets.clear();
	_system.Clear();
}

void Plot2D::Render(Gtk::DrawingArea &drawingArea)
{
	_system.Clear();
	for(std::vector<Plot2DPointSet*>::iterator i=_pointSets.begin();i!=_pointSets.end();++i)
		_system.AddToSystem(**i);

	Cairo::RefPtr<Cairo::Context> cr = drawingArea.get_window()->create_cairo_context();

	Gtk::Allocation allocation = drawingArea.get_allocation();
	_width = allocation.get_width();
	_height = allocation.get_height();

	cr->set_line_width(2);

	cr->set_source_rgba(1, 1, 1, 1);
  cr->paint();
	cr->fill();

	size_t c = 0;

	for(std::vector<Plot2DPointSet*>::iterator i=_pointSets.begin();i!=_pointSets.end();++i)
	{
		switch(c)
		{
			case 0: cr->set_source_rgba(1, 0, 0, 1); break;
			case 1: cr->set_source_rgba(0, 1, 0, 1); break;
			case 2: cr->set_source_rgba(0, 0, 1, 1); break;
			case 3: cr->set_source_rgba(0, 0, 0, 1); break;
			case 4: cr->set_source_rgba(1, 1, 0, 1); break;
			case 5: cr->set_source_rgba(1, 0, 1, 1); break;
			case 6: cr->set_source_rgba(0, 1, 1, 1); break;
			case 7: cr->set_source_rgba(0.5, 0.5, 0.5, 1); break;
		}
		c = (c+1)%8;

		render(cr, **i);
	}
}

void Plot2D::render(Cairo::RefPtr<Cairo::Context> cr, Plot2DPointSet &pointSet)
{
	pointSet.Sort();

	num_t
		xLeft = _system.XRangeMin(pointSet),
		xRight = _system.XRangeMax(pointSet),
		yTop = _system.YRangeMin(pointSet),
		yBottom = _system.YRangeMax(pointSet);
	if(!std::isfinite(xLeft) || !std::isfinite(xRight) || xLeft == xRight)
	{
		xLeft -= 1;
		xRight += 1;
	}
	if(!std::isfinite(yTop) || !std::isfinite(yBottom) || yTop == yBottom)
	{
		yTop -= 1;
		yBottom += 1;
	}

	double fx = (double) _width / (xRight - xLeft);
	double fy = (double) _height / (yBottom - yTop);

	bool hasPrevPoint = false;

	for(size_t i=0;i<pointSet.Size()-1;++i)
	{
		num_t
			x1 = (pointSet.GetX(i) - xLeft) * fx,
			x2 = (pointSet.GetX(i+1) - xLeft) * fx,
			y1 = (yBottom - pointSet.GetY(i)) * fy,
			y2 = (yBottom - pointSet.GetY(i+1)) * fy;

		if(std::isfinite(x1) && std::isfinite(x2) && std::isfinite(y1) && std::isfinite(y2))
		{
			if(!hasPrevPoint)
				cr->move_to(x1, y1);
			cr->line_to(x2, y2);
			hasPrevPoint = true;
		} else {
			hasPrevPoint = false;
		}
	}
	cr->stroke();

	// Draw zero y-axis
	if(yTop <= 0.0 && yBottom >= 0.0)
	{
		cr->set_source_rgba(0.2, 0.2, 0.2, 1);
		cr->move_to(0, yBottom * fy);
		cr->line_to(_width, yBottom * fy);
		cr->stroke();
	}
}
