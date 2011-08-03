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

VerticalPlotScale::VerticalPlotScale()
	: _plotWidth(0), _plotHeight(0), _metricsAreInitialized(false), _tickSet(0), _isLogarithmic(false)
{
}

VerticalPlotScale::~VerticalPlotScale()
{
	if(_tickSet != 0)
		delete _tickSet;
}

double VerticalPlotScale::GetWidth(Cairo::RefPtr<Cairo::Context> cairo)
{
	initializeMetrics(cairo);
	return _width;
}

void VerticalPlotScale::Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX, double offsetY)
{
	initializeMetrics(cairo);
	cairo->set_source_rgb(0.0, 0.0, 0.0);
	cairo->set_font_size(16.0);
	for(unsigned i=0;i!=_tickSet->Size();++i)
	{
		const Tick tick = _tickSet->GetTick(i);
		Cairo::TextExtents extents;
		cairo->get_text_extents(tick.second, extents);
		cairo->move_to(_width - extents.width - 5 + offsetX,
										getTickYPosition(tick) - extents.height/2  - extents.y_bearing + offsetY);
		cairo->show_text(tick.second);
	}
	cairo->stroke();
}

void VerticalPlotScale::initializeMetrics(Cairo::RefPtr<Cairo::Context> cairo)
{
	if(!_metricsAreInitialized)
	{
		if(_tickSet != 0)
		{
			_tickSet->Reset();
			while(!ticksFit(cairo) && _tickSet->Size() > 2)
			{
				_tickSet->DecreaseTicks();
			}
			cairo->set_font_size(16.0);
			double maxWidth = 0;
			for(unsigned i=0;i!=_tickSet->Size();++i)
			{
				Tick tick = _tickSet->GetTick(i);
				Cairo::TextExtents extents;
				cairo->get_text_extents(tick.second, extents);
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
	_tickSet = new NumericTickSet(min, max, 25);
	_isLogarithmic = false;
	_metricsAreInitialized = false;
}

void VerticalPlotScale::InitializeLogarithmicTicks(double min, double max)
{
	if(_tickSet == 0)
		delete _tickSet;
	_tickSet = new LogarithmicTickSet(min, max, 25);
	_isLogarithmic = true;
	_metricsAreInitialized = false;
}

double VerticalPlotScale::getTickYPosition(const Tick &tick)
{
	return (1.0-tick.first) * _plotHeight + _topMargin;
}

bool VerticalPlotScale::ticksFit(Cairo::RefPtr<Cairo::Context> cairo)
{
	cairo->set_font_size(16.0);
	double prevTopY = _plotHeight*2.0;
	for(unsigned i=0;i!=_tickSet->Size();++i)
	{
		const Tick tick = _tickSet->GetTick(i);
		Cairo::TextExtents extents;
		cairo->get_text_extents(tick.second, extents);
		// we want a distance of at least one x height between the text, hence height
		const double
			bottomY = getTickYPosition(tick) + extents.height,
			topY = bottomY - extents.height*2;
		if(bottomY > prevTopY)
			return false;
		prevTopY = topY;
	}
	return true;
}
