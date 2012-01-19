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

#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include <boost/function.hpp>

#include <set>

#include "plot2d.h"

class PlotManager
{
	public:
		Plot2D &NewPlot2D()
		{
			Plot2D *plot = new Plot2D();
			_items.insert(plot);
			_onChange();
			return *plot;
		}
		
		boost::function<void()> &OnChange() { return _onChange; }
		
	private:
		std::set<Plot2D*> _items;
		
		boost::function<void()> _onChange;
};

#endif
