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
#include <AOFlagger/quality/histogramcollection.h>
#include <AOFlagger/quality/rayleighfitter.h>

HistogramPage::HistogramPage() :
	_histogramTypeFrame("Histogram"),
	_totalHistogramButton("Total"),
	_rfiHistogramButton("RFI"),
	_notRFIHistogramButton("Not RFI"),
	_xxPolarizationButton("XX"),
	_xyPolarizationButton("XY"),
	_yxPolarizationButton("YX"),
	_yyPolarizationButton("YY"),
	_fitButton("Fit")
{
	_histogramTypeBox.pack_start(_totalHistogramButton, Gtk::PACK_SHRINK);
	_totalHistogramButton.set_active(true);
	_totalHistogramButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_histogramTypeBox.pack_start(_rfiHistogramButton, Gtk::PACK_SHRINK);
	_rfiHistogramButton.set_active(false);
	_rfiHistogramButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_histogramTypeBox.pack_start(_notRFIHistogramButton, Gtk::PACK_SHRINK);
	_notRFIHistogramButton.set_active(false);
	_notRFIHistogramButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	
	_histogramTypeFrame.add(_histogramTypeBox);
	
	_sideBox.pack_start(_histogramTypeFrame, Gtk::PACK_SHRINK);
	
	_polarizationBox.pack_start(_xxPolarizationButton, Gtk::PACK_SHRINK);
	_xxPolarizationButton.set_active(true);
	_xxPolarizationButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_polarizationBox.pack_start(_xyPolarizationButton, Gtk::PACK_SHRINK);
	_xyPolarizationButton.set_active(false);
	_xyPolarizationButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_polarizationBox.pack_start(_yxPolarizationButton, Gtk::PACK_SHRINK);
	_yxPolarizationButton.set_active(false);
	_yxPolarizationButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_polarizationBox.pack_start(_yyPolarizationButton, Gtk::PACK_SHRINK);
	_yyPolarizationButton.set_active(true);
	_yyPolarizationButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));

	_polarizationFrame.add(_polarizationBox);
	
	_sideBox.pack_start(_polarizationFrame, Gtk::PACK_SHRINK);
	
	_sideBox.pack_start(_fitButton, Gtk::PACK_SHRINK);
	
	pack_start(_sideBox, Gtk::PACK_SHRINK);
	
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

			HistogramCollection histograms(polarizationCount);
			histograms.Load(histogramTables);
			
			if(_xxPolarizationButton.get_active())
				plotPolarization(histograms, 0);
			if(_xyPolarizationButton.get_active() && polarizationCount>=1)
				plotPolarization(histograms, 1);
			if(_yxPolarizationButton.get_active() && polarizationCount>=2)
				plotPolarization(histograms, 2);
			if(_yyPolarizationButton.get_active() && polarizationCount>=3)
				plotPolarization(histograms, 3);
		}
		
		_plotWidget.Update();
	}
}

void HistogramPage::plotPolarization(class HistogramCollection &histograms, unsigned p)
{
	if(_totalHistogramButton.get_active())
	{
		_plot.StartLine("Total histogram", "Amplitude in arbitrary units (log)", "Frequency (log)");
		LogHistogram totalHistogram;
		histograms.GetTotalHistogramForCrossCorrelations(p, totalHistogram);
		addHistogramToPlot(totalHistogram);
		
		if(_fitButton.get_active())
		{
			plotFit(totalHistogram, "Fit to total");
		}
	}

	if(_rfiHistogramButton.get_active())
	{
		_plot.StartLine("RFI histogram", "Amplitude in arbitrary units (log)", "Frequency (log)");
		LogHistogram rfiHistogram;
		histograms.GetRFIHistogramForCrossCorrelations(p, rfiHistogram);
		addHistogramToPlot(rfiHistogram);

		if(_fitButton.get_active())
		{
			plotFit(rfiHistogram, "Fit to RFI");
		}
	}
}

void HistogramPage::plotFit(class LogHistogram &histogram, const std::string &title)
{
	_plot.StartLine(title, "Amplitude in arbitrary units (log)", "Frequency (log)");
	double minRange, maxRange, sigmaEstimate;
	sigmaEstimate = RayleighFitter::SigmaEstimate(histogram);
	RayleighFitter::FindFitRangeUnderRFIContamination(histogram.MinPositiveAmplitude(), sigmaEstimate, minRange, maxRange);
	RayleighFitter fitter;
	double sigma = sigmaEstimate, n = RayleighFitter::NEstimate(histogram, minRange, maxRange);
	fitter.Fit(minRange, maxRange, histogram, sigma, n);
	addRayleighToPlot(histogram, sigma, n);
}

void HistogramPage::addHistogramToPlot(LogHistogram &histogram)
{
	for(LogHistogram::iterator i=histogram.begin();i!=histogram.end();++i)
	{
		const double x = i.value(); // TODO this is actually slightly off
		const double logx = log10(x);
		const double logc = log10(i.normalizedCount());
		if(std::isfinite(logx) && std::isfinite(logc))
			_plot.PushDataPoint(logx, logc);
	}
}

void HistogramPage::addRayleighToPlot(LogHistogram &histogram, double sigma, double n)
{
	double x = histogram.MinPositiveAmplitude();
	const double xend = sigma*5.0;
	const double sigmaP2 = sigma*sigma;
	while(x < xend) {
		const double logx = log10(x);
    const double c = n * x / (sigmaP2) * exp(-x*x/(2*sigmaP2));
		const double logc = log10(c);
		if(std::isfinite(logx) && std::isfinite(logc))
			_plot.PushDataPoint(logx, logc);
		x *= 1.05;
	}
}
