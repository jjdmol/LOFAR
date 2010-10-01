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
#ifndef HORIZONTALNUMERICSCALE_H
#define HORIZONTALNUMERICSCALE_H

#include <sstream>

#include "horizontalplotscale.h"

class HorizontalNumericScale : private HorizontalPlotScale
{
	public:
		HorizontalNumericScale(Glib::RefPtr<Gdk::Drawable> drawable, double min, double max)
			: HorizontalPlotScale(drawable)
		{
			for(size_t i=0;i<=100;++i)
			{
				std::stringstream s;
				s << ((max - min) * i / 100.0 + min);
				if(i % 10 == 0)
					HorizontalPlotScale::AddLargeTick(i / 100.0, s.str());
				else
					HorizontalPlotScale::AddSmallTick(i / 100.0, s.str());
			}
		}
		void Draw(Cairo::RefPtr<Cairo::Context> cairo) { HorizontalPlotScale::Draw(cairo); }
		double GetHeight() { return HorizontalPlotScale::GetHeight(); }
		void SetPlotDimensions(double plotWidth, double plotHeight, double topMargin, double verticalScaleWidth)
		{
			HorizontalPlotScale::SetPlotDimensions(plotWidth, plotHeight, topMargin, verticalScaleWidth);
		}
};

#endif // HORIZONTALNUMERICSCALE_H
