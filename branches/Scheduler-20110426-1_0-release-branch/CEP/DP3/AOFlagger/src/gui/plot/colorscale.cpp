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
#include <AOFlagger/gui/plot/colorscale.h>

const double ColorScale::BAR_WIDTH = 15.0;

ColorScale::ColorScale(Glib::RefPtr<Gdk::Drawable> drawable)
: _width(0.0), _drawable(drawable),  _verticalPlotScale(drawable)
{
	_cairo = _drawable->create_cairo_context();
}

void ColorScale::initWidth()
{
	if(_width == 0.0)
	{
		_scaleWidth = _verticalPlotScale.GetWidth();
		_width = _scaleWidth + BAR_WIDTH;
	}
}

void ColorScale::Draw(Cairo::RefPtr<Cairo::Context> cairo)
{
	_verticalPlotScale.SetPlotDimensions(_plotWidth, _plotHeight, _topMargin);
	initWidth();
	_cairo = cairo;
	ColorValue backValue;
	if(!_colorValues.empty())
	{
		backValue = _colorValues.begin()->second;
	} else {
		backValue.red = 1.0;
		backValue.green = 1.0;
		backValue.blue = 1.0;
	}
	_cairo->rectangle(_plotWidth - _width + _scaleWidth, _topMargin,
										BAR_WIDTH, _plotHeight);
	_cairo->set_source_rgb(backValue.red, backValue.green, backValue.blue);
	_cairo->fill();
	
	for(std::map<double, ColorValue>::const_iterator i=_colorValues.begin();
		i!=_colorValues.end();++i)
	{
		double val = (i->first - _min) / (_max - _min);
		if(val < 0.0) val = 0.0;
		if(val > 1.0) val = 1.0;
		double height = _plotHeight * (1.0 - val);
		const ColorValue &color = i->second;
		_cairo->set_source_rgb(color.red, color.green, color.blue);
		_cairo->rectangle(_plotWidth - _width + _scaleWidth, _topMargin,
											BAR_WIDTH, height);
		_cairo->fill();
	}
	
	_cairo->rectangle(_plotWidth - _width + _scaleWidth, _topMargin,
										BAR_WIDTH, _plotHeight);
	_cairo->set_source_rgb(0.0, 0.0, 0.0);
	_cairo->stroke();
	
	_verticalPlotScale.Draw(cairo, _plotWidth - _width, 0.0);
}
