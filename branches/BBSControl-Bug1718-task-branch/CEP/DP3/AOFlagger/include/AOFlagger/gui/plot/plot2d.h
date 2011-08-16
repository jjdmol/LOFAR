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
#ifndef PLOT2D_H
#define PLOT2D_H

#include <gtkmm/drawingarea.h>

#include <stdexcept>
#include <string>

#include "plotable.h"
#include "plot2dpointset.h"
#include "system.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class Plot2D : public Plotable {
	public:
		Plot2D();
		~Plot2D();

		void Clear();
		void StartLine(const std::string &label)
		{
			Plot2DPointSet *newSet = new Plot2DPointSet();
			newSet->SetLabel(label);
			_pointSets.push_back(newSet);
		}
		void PushDataPoint(num_t x, num_t y)
		{
			if(_pointSets.size() > 0)
				(*_pointSets.rbegin())->PushDataPoint(x,y);
			else
				throw std::runtime_error("Trying to push a data point into a plot without point sets (call StartLine first).");
		}
		size_t PointSetCount() const { return _pointSets.size(); }
		virtual void Render(Gtk::DrawingArea &drawingArea);
	private:
		void render(Cairo::RefPtr<Cairo::Context> cr, Plot2DPointSet &pointSet);

		std::vector<Plot2DPointSet*> _pointSets;
		int _width, _height;
		System _system;
};

#endif
