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
#ifndef VERTICALPLOTSCALE_H
#define VERTICALPLOTSCALE_H

#include <string>
#include <vector>

#include <gtkmm/drawingarea.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class VerticalPlotScale {
	public:
		VerticalPlotScale(Glib::RefPtr<Gdk::Drawable> drawable);
		virtual ~VerticalPlotScale();
		void SetPlotDimensions(double plotWidth, double plotHeight, double topMargin)
		{
			_plotWidth = plotWidth;
			_plotHeight = plotHeight;
			_topMargin = topMargin;
			_metricsAreInitialized = false;
		}
		double GetWidth();
		void Draw(Cairo::RefPtr<Cairo::Context> cairo, double offsetX=0.0, double offsetY=0.0);
		void InitializeNumericTicks(double min, double max);
	private:
		bool ticksFit();
		void initializeMetrics(); 

		double _plotWidth, _plotHeight, _topMargin;
		bool _metricsAreInitialized;
		double _width;
		Glib::RefPtr<Gdk::Drawable> _drawable;
		Cairo::RefPtr<Cairo::Context> _cairo;
		class TickSet *_tickSet;
};

#endif
