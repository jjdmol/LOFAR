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
#ifndef HORIZONTALPLOTSCALE_H
#define HORIZONTALPLOTSCALE_H

#include <string>
#include <vector>

#include <gtkmm/drawingarea.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class HorizontalPlotScale {
	public:
		HorizontalPlotScale(Glib::RefPtr<Gdk::Drawable> drawable);
		virtual ~HorizontalPlotScale();
		void AddLargeTick(double normValue, std::string caption)
		{
			_largeTicks.push_back(Tick(normValue, caption));
			_metricsAreInitialized = false;
		}
		void AddSmallTick(double normValue, std::string caption)
		{
			_smallTicks.push_back(Tick(normValue, caption));
			_metricsAreInitialized = false;
		}
		void SetPlotDimensions(double plotWidth, double plotHeight, double topMargin, double verticalScaleWidth)
		{
			_plotWidth = plotWidth;
			_plotHeight = plotHeight;
			_topMargin = topMargin;
			_verticalScaleWidth = verticalScaleWidth;
			_metricsAreInitialized = false;
		}
		double GetHeight();
		void Draw(Cairo::RefPtr<Cairo::Context> cairo);
	private:
		struct Tick {
			Tick(double _normValue, std::string _caption) :
				normValue(_normValue), caption(_caption)
			{ }
			Tick(const Tick &source) :
				normValue(source.normValue), caption(source.caption)
			{ }
			Tick &operator=(const Tick &rhs)
			{
				normValue = rhs.normValue; caption = rhs.caption;
				return *this;
			}
			double normValue;
			std::string caption;
		};
		void setVisibleTicks();
		bool ticksFit(std::map<double, Tick> &ticks);
		void initializeMetrics(); 

		double _plotWidth, _plotHeight, _topMargin, _verticalScaleWidth;
		std::vector<Tick> _largeTicks, _smallTicks;
		std::vector<Tick> _visibleLargeTicks;
		bool _metricsAreInitialized;
		double _width;
		Glib::RefPtr<Gdk::Drawable> _drawable;
		Cairo::RefPtr<Cairo::Context> _cairo;
};

#endif
