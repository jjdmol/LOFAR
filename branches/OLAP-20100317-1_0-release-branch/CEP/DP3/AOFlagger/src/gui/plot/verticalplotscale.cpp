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

#include <map>

VerticalPlotScale::VerticalPlotScale(Glib::RefPtr<Gdk::Drawable> drawable)
	: _plotWidth(0), _plotHeight(0), _metricsAreInitialized(false), _drawable(drawable)
{
	_cairo = _drawable->create_cairo_context();
}

VerticalPlotScale::~VerticalPlotScale()
{
}

double VerticalPlotScale::GetWidth()
{
	initializeMetrics();
	return _width;
}

void VerticalPlotScale::Draw(Cairo::RefPtr<Cairo::Context> cairo)
{
	_cairo = cairo;
	initializeMetrics();
	_cairo->set_source_rgb(0.0, 0.0, 0.0);
	_cairo->set_font_size(16.0);
	for(std::vector<Tick>::const_iterator i=_visibleLargeTicks.begin();i!=_visibleLargeTicks.end();++i)
	{
		Cairo::TextExtents extents;
		_cairo->get_text_extents(i->caption, extents);
		_cairo->move_to(_width - extents.width - 5, i->normValue * _plotHeight - extents.height/2  - extents.y_bearing + _topMargin);
		_cairo->show_text(i->caption);
	}
	_cairo->stroke();
}

void VerticalPlotScale::initializeMetrics()
{
	if(!_metricsAreInitialized)
	{
		setVisibleTicks();
		_cairo->set_font_size(16.0);
		double maxWidth = 0;
		for(std::vector<Tick>::const_iterator i=_visibleLargeTicks.begin();i!=_visibleLargeTicks.end();++i)
		{
			Tick tick = *i;
			Cairo::TextExtents extents;
			_cairo->get_text_extents(tick.caption, extents);
			if(maxWidth < extents.width)
				maxWidth = extents.width;
		}
		_width = maxWidth + 10;
		_metricsAreInitialized = true;
	}
} 

void VerticalPlotScale::setVisibleTicks()
{

	std::map<double, Tick> ticks;
	for(std::vector<Tick>::const_iterator i=_largeTicks.begin();i!=_largeTicks.end();++i)
		ticks.insert(std::pair<double, Tick>(i->normValue, *i));

	if(!ticksFit(ticks))
	{
		size_t tryCount = 2;
	
		while(tryCount < _largeTicks.size())
		{
			tryCount *= 2;
			ticks.clear();
			for(size_t i=0;i<tryCount;++i)
			{
				size_t index = (i * _largeTicks.size() / tryCount);
				Tick tick = _largeTicks[index];
				ticks.insert(std::pair<double, Tick>(tick.normValue, tick));
			}
		}
		tryCount /= 2;
		ticks.clear();
		for(size_t i=0;i<tryCount;++i)
		{
			size_t index = (i * _largeTicks.size() / tryCount);
			Tick tick = _largeTicks[index];
			ticks.insert(std::pair<double, Tick>(tick.normValue, tick));
		}
	}
	_visibleLargeTicks.clear();
	for(std::map<double, Tick>::const_iterator i=ticks.begin();i!=ticks.end();++i)
		_visibleLargeTicks.push_back(i->second);
}

bool VerticalPlotScale::ticksFit(std::map<double, Tick> &/*ticks*/)
{
	//Cairo::TextExtents extents;
	//cr->get_text_extents(tick.caption, extents);
	/** TODO */
	return true;
}
