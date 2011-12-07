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
#ifndef ITERATIONSPLOT_H
#define ITERATIONSPLOT_H

#include <string>
#include <map>

#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/timefrequencymetadata.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class IterationsPlot
{
public:
	IterationsPlot() { }
	~IterationsPlot() { }

	void Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta );
	void MakePlot();
	bool HasData() { return !_stats.empty(); }
private:
	struct Item
	{
		Item() : flaggedRatio(0.0), mode(0.0), winsorizedMode(0.0)
		{
		}
		double flaggedRatio;
		num_t mode, winsorizedMode;
	};
	std::vector<Item> _stats;
};

#endif
