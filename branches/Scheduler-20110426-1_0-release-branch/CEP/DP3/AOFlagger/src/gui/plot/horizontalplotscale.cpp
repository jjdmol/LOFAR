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
#include <AOFlagger/gui/plot/horizontalplotscale.h>

#include <AOFlagger/gui/plot/tickset.h>

HorizontalPlotScale::HorizontalPlotScale(Glib::RefPtr<Gdk::Drawable> drawable)
	: _plotWidth(0), _plotHeight(0), _metricsAreInitialized(false), _drawable(drawable), _tickSet(0)
{
	_cairo = _drawable->create_cairo_context();
}

HorizontalPlotScale::~HorizontalPlotScale()
{
	if(_tickSet != 0)
		delete _tickSet;
}

double HorizontalPlotScale::GetHeight()
{
	initializeMetrics();
	return _height;
}

double HorizontalPlotScale::GetRightMargin()
{
	initializeMetrics();
	return _rightMargin;
}

void HorizontalPlotScale::Draw(Cairo::RefPtr<Cairo::Context> cairo)
{
	_cairo = cairo;
	initializeMetrics();
	_cairo->set_source_rgb(0.0, 0.0, 0.0);
	_cairo->set_font_size(16.0);
	for(unsigned i=0;i!=_tickSet->Size();++i)
	{
		const Tick tick = _tickSet->GetTick(i);
		double x = tick.first * (_plotWidth - _verticalScaleWidth) + _verticalScaleWidth;
		_cairo->move_to(x, _topMargin + _plotHeight);
		_cairo->line_to(x, _topMargin + _plotHeight + 3);
		Cairo::TextExtents extents;
		_cairo->get_text_extents(tick.second, extents);
		_cairo->move_to(x - extents.width/2, _topMargin + _plotHeight - extents.y_bearing + extents.height);
		_cairo->show_text(tick.second);
	}
	_cairo->stroke();
}

void HorizontalPlotScale::initializeMetrics()
{
	if(!_metricsAreInitialized)
	{
		if(_tickSet != 0)
		{
			while(!ticksFit())
			{
				_tickSet->DecreaseTicks();
			}
			_cairo->set_font_size(16.0);
			double maxHeight = 0;
			for(unsigned i=0;i!=_tickSet->Size();++i)
			{
				const Tick tick = _tickSet->GetTick(i);
				Cairo::TextExtents extents;
				_cairo->get_text_extents(tick.second, extents);
				if(maxHeight < extents.height)
					maxHeight = extents.height;
			}
			_height = maxHeight*2 + 10;
			
			Cairo::TextExtents extents;
			_cairo->get_text_extents(_tickSet->GetTick(_tickSet->Size()-1).second, extents);
			_rightMargin = extents.width/2+5 > 10 ? extents.width/2+5 : 10;
			
			_metricsAreInitialized = true;
		}
	}
} 

void HorizontalPlotScale::InitializeNumericTicks(double min, double max)
{
	if(_tickSet != 0)
		delete _tickSet;
	_tickSet = new NumericTickSet(min, max, 14);
}

void HorizontalPlotScale::InitializeTimeTicks(double timeMin, double timeMax)
{
	if(_tickSet != 0)
		delete _tickSet;
	_tickSet = new TimeTickSet(timeMin, timeMax, 14);
}

bool HorizontalPlotScale::ticksFit()
{
	/*double pos = 0.0;
	for(std::map<double, Tick>::const_iterator i= ticks.begin();i!=ticks.end();++i)
	{
		const Tick &tick = i->second;
		Cairo::TextExtents extents;
		_cairo->get_text_extents(tick.caption, extents);
		double thisPos = tick.normValue * (_plotWidth - _verticalScaleWidth) - extents.width/2  - extents.y_bearing;
		if(extents.width > thisPos - pos)
			return false;
		pos = thisPos;
	}*/
	return true;
}
