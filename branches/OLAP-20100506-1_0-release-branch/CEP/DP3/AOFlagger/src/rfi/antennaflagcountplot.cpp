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
#include <fstream>
#include <iostream>

#include <AOFlagger/util/plot.h>

#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/msio/timefrequencymetadata.h>

#include <AOFlagger/rfi/antennaflagcountplot.h>

void AntennaFlagCountPlot::Add(class TimeFrequencyData &data, TimeFrequencyMetaDataCPtr meta)
{
	for(size_t maskIndex=0;maskIndex<data.MaskCount();++maskIndex)
	{
		Mask2DCPtr mask = data.GetMask(maskIndex);
		long long count = 0;

		for(size_t y=0;y<mask->Height();++y)
		{
			for(size_t x=0;x<mask->Width();++x)
			{
				if(mask->Value(x, y))
					++count;
			}
		}

		long long total = (long long) mask->Width() * (long long) mask->Height();

		MapItem item1;
		if(_counts.count(meta->Antenna1().id)==0)
			item1.name = meta->Antenna1().name;
		else
			item1 = _counts[meta->Antenna1().id];

		if(meta->Antenna1().id == meta->Antenna2().id)
		{
			item1.autoTotal += total;
			item1.autoCount += count;
			_counts[meta->Antenna1().id] = item1;
		} else {
			MapItem item2;
			if(_counts.count(meta->Antenna2().id)==0)
				item2.name = meta->Antenna2().name;
			else
				item2 = _counts[meta->Antenna2().id];

			item1.crossTotal += total;
			item1.crossCount += count;
			item2.crossTotal += total;
			item2.crossCount += count;
			_counts[meta->Antenna1().id] = item1;
			_counts[meta->Antenna2().id] = item2;
		}
	}
} 

void AntennaFlagCountPlot::MakePlot()
{
	Plot plot("antenna-flag-counts.pdf");
	plot.SetXAxisText("Antenna (index)");
	plot.SetYAxisText("Flagged (%)");
	plot.StartBoxes("Auto-correlation");
	plot.SetXRangeAutoMax(0);
	plot.SetYRangeAutoMax(0);
	for(std::map<int, MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
	{
		plot.PushDataPoint(i->first, 100.0L * (long double) i->second.autoCount / (long double) i->second.autoTotal);
	}
	plot.StartBoxes("Cross-correlation");
	for(std::map<int, MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
	{
		plot.PushDataPoint((long double) i->first + 0.3, 100.0L * (long double) i->second.crossCount / (long double) i->second.crossTotal);
	}
	plot.Close();
	plot.Show();
}

void AntennaFlagCountPlot::Report()
{
	for(std::map<int, MapItem>::const_iterator i=_counts.begin();i!=_counts.end();++i)
	{
		std::cout << "Flagged in autocorrelations of antenna " << i->second.name << ": " << (100.0L * (long double) i->second.autoCount / (long double) i->second.autoTotal)
			<< "%\nFlagged in cross correlations with antenna " << i->second.name << ": " << (100.0L * (long double) i->second.crossCount / (long double) i->second.crossTotal) << "%\n";
	}
}
