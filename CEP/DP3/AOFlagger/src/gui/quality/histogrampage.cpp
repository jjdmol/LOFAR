/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include <AOFlagger/gui/quality/histogrampage.h>

#include <AOFlagger/quality/histogramtablesformatter.h>

#include <AOFlagger/msio/measurementset.h>

HistogramPage::HistogramPage()
{
	_plotWidget.SetPlot(_plot);
	pack_start(_plotWidget, Gtk::PACK_EXPAND_WIDGET);
	
	show_all_children();
}

HistogramPage::~HistogramPage()
{
}

void HistogramPage::updatePlot()
{
	if(HasStatistics())
	{
		_plot.Clear();
		
		HistogramTablesFormatter histogramTables(_statFilename);
		if(histogramTables.HistogramsExist())
		{
			MeasurementSet set(_statFilename);
			
			const unsigned polarizationCount = set.GetPolarizationCount();

			for(unsigned p=0;p<polarizationCount;++p)
			{
				const unsigned totalHistogramIndex = histogramTables.QueryTypeIndex(HistogramTablesFormatter::TotalHistogram, p);
				std::vector<HistogramTablesFormatter::HistogramItem> totalHistogram;
				histogramTables.QueryHistogram(totalHistogramIndex, totalHistogram);
				_plot.StartLine("Total histogram", "Amplitude in arbitrary units (log)", "Frequency (log)");
				addHistogramToPlot(totalHistogram);

				const unsigned rfiHistogramIndex = histogramTables.QueryTypeIndex(HistogramTablesFormatter::RFIHistogram, p);
				std::vector<HistogramTablesFormatter::HistogramItem> rfiHistogram;
				histogramTables.QueryHistogram(rfiHistogramIndex, rfiHistogram);
				_plot.StartLine("RFI histogram", "Amplitude in arbitrary units (log)", "Frequency (log)");
				addHistogramToPlot(rfiHistogram);
			}
		}
		
		_plotWidget.Update();
	}
}

void HistogramPage::addHistogramToPlot(const std::vector<HistogramTablesFormatter::HistogramItem> &histogram)
{
	for(std::vector<HistogramTablesFormatter::HistogramItem>::const_iterator i=histogram.begin();i!=histogram.end();++i)
	{
		const double b = (i->binStart + i->binEnd) * 0.5; // TODO this is actually slightly off
		const double logb = log10(b);
		const double logc = log10(i->count / (i->binEnd - i->binStart));
		if(std::isfinite(logb) && std::isfinite(logc))
			_plot.PushDataPoint(logb, logc);
	}
}
