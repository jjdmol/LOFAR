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
#ifndef HORIZONTALTIMESCALE_H
#define HORIZONTALTIMESCALE_H

#include <string>

#include "horizontalplotscale.h"

#include <AOFlagger/msio/date.h>

class HorizontalTimeScale : private HorizontalPlotScale
{
	public:
		HorizontalTimeScale(Glib::RefPtr<Gdk::Drawable> drawable, double minAipsTime, double maxAipsTime)
			: HorizontalPlotScale(drawable)
		{
			for(size_t i=0;i<=100;++i)
			{
				double val = ((maxAipsTime - minAipsTime) * i / 100.0 + minAipsTime);
				std::string s = Date::AipsMJDToTimeString(val) + "\n" + Date::AipsMJDToDateString(val);
				if(i % 10 == 0)
					HorizontalTimeScale::AddLargeTick(i / 100.0, s);
				else
					HorizontalTimeScale::AddSmallTick(i / 100.0, s);
			}
		}
		void Draw(Cairo::RefPtr<Cairo::Context> cairo) { HorizontalPlotScale::Draw(cairo); }
		double GetHeight() { return HorizontalPlotScale::GetHeight(); }
		void SetPlotDimensions(double plotWidth, double plotHeight, double topMargin, double verticalScaleWidth)
		{
			HorizontalPlotScale::SetPlotDimensions(plotWidth, plotHeight, topMargin, verticalScaleWidth);
		}
};

#endif // HORIZONTALTIMESCALE_H
