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
#include <AOFlagger/gui/plot/verticalplotscale.h>

#include <AOFlagger/gui/plot/tickset.h>

VerticalPlotScale::VerticalPlotScale(Glib::RefPtr<Gdk::Drawable> drawable)
	: _plotWidth(0), _plotHeight(0), _metricsAreInitialized(false), _drawable(drawable), _tickSet(0)
{
	_cairo = _drawable->create_cairo_context();
}

VerticalPlotScale::~VerticalPlotScale()
{
	if(_tickSet != 0)
		delete _tickSet;
}

double VerticalPlotScale::GetWidth()
{
	initializeMetrics();
	return _width;
}

void VerticalPlotScale::Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX, double offsetY)
{
	_cairo = cairo;
	initializeMetrics();
	_cairo->set_source_rgb(0.0, 0.0, 0.0);
	_cairo->set_font_size(16.0);
	for(unsigned i=0;i!=_tickSet->Size();++i)
	{
		const Tick tick = _tickSet->GetTick(i);
		Cairo::TextExtents extents;
		_cairo->get_text_extents(tick.second, extents);
		_cairo->move_to(_width - extents.width - 5 + offsetX,
										(1.0-tick.first) * _plotHeight - extents.height/2  - extents.y_bearing + _topMargin + offsetY);
		_cairo->show_text(tick.second);
	}
	_cairo->stroke();
}

void VerticalPlotScale::initializeMetrics()
{
	if(!_metricsAreInitialized)
	{
		if(_tickSet != 0)
		{
			while(!ticksFit() && _tickSet->Size() > 1)
			{
				_tickSet->DecreaseTicks();
			}
			_cairo->set_font_size(16.0);
			double maxWidth = 0;
			for(unsigned i=0;i!=_tickSet->Size();++i)
			{
				Tick tick = _tickSet->GetTick(i);
				Cairo::TextExtents extents;
				_cairo->get_text_extents(tick.second, extents);
				if(maxWidth < extents.width)
					maxWidth = extents.width;
			}
			_width = maxWidth + 10;
			_metricsAreInitialized = true;
		}
	}
} 

void VerticalPlotScale::InitializeNumericTicks(double min, double max)
{
	if(_tickSet == 0)
		delete _tickSet;
	_tickSet = new NumericTickSet(min, max, 20);
}

bool VerticalPlotScale::ticksFit()
{
	//Cairo::TextExtents extents;
	//cr->get_text_extents(tick.caption, extents);
	/** TODO */
	return true;
}
