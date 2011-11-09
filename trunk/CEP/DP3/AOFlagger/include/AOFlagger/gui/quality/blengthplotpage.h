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
#ifndef GUI_QUALITY__BLENGTHPLOTPAGE_H
#define GUI_QUALITY__BLENGTHPLOTPAGE_H

#include "twodimensionalplotpage.h"

#include <AOFlagger/quality/statisticscollection.h>

#include <AOFlagger/msio/measurementset.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class BLengthPlotPage : public TwoDimensionalPlotPage {
	protected:
		virtual void processStatistics(class StatisticsCollection *statCollection, const std::string &filename)
		{
			const BaselineStatisticsMap &map = statCollection->BaselineStatistics();
			
			MeasurementSet ms(filename);
			unsigned antennaCount = ms.AntennaCount();
			AntennaInfo antennas[antennaCount];
			for(unsigned a=0;a<antennaCount;++a)
			{
				antennas[a] = ms.GetAntennaInfo(a);
			}
			vector<std::pair<unsigned, unsigned> > baselines = map.BaselineList();
			for(vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
			{
				Baseline bline(antennas[i->first], antennas[i->second]);
				_statistics.insert(std::pair<double, Statistics>(bline.Distance(), map.GetStatistics(i->first, i->second)));
			}
		}
		
		virtual const std::map<double, class Statistics> &GetStatistics() const
		{
			return _statistics;
		}
		
		virtual void StartLine(Plot2D &plot, const std::string &name)
		{
			plot.StartLine(name, "Baseline length (m)", "Value (Jy)", false, Plot2DPointSet::DrawPoints);
		}
		
	private:
		std::map<double, Statistics> _statistics;
};

#endif
