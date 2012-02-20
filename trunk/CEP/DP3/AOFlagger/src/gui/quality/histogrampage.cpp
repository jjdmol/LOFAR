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

#include <boost/bind.hpp>

#include <AOFlagger/gui/plot/plotpropertieswindow.h>
#include <AOFlagger/gui/quality/datawindow.h>

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
	_fitFrame("Fitting"),
	_fitButton("Fit"),
	_subtractFitButton("Subtract"),
	_fitAutoRangeButton("Auto range"),
	_functionFrame("Function"),
	_nsButton("N(S)"),
	_dndsButton("dN(S)/dS"),
	_plotPropertiesButton("Properties"),
	_dataExportButton("Data"),
	_slopeFrame("Slope"),
	_drawSlopeButton("Draw"),
	_slopeAutoRangeButton("Auto range"),
	_plotPropertiesWindow(0),
	_histograms(0),
	_summedPolarizationHistograms(0)
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
	
	_fitBox.pack_start(_fitButton, Gtk::PACK_SHRINK);
	_fitButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_fitBox.pack_start(_subtractFitButton, Gtk::PACK_SHRINK);
	_subtractFitButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_fitBox.pack_start(_fitAutoRangeButton, Gtk::PACK_SHRINK);
	_fitAutoRangeButton.set_active(true);
	_fitAutoRangeButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::onAutoRangeClicked));
	
	_fitBox.pack_start(_fitStartEntry, Gtk::PACK_SHRINK);
	_fitStartEntry.set_sensitive(false);
	_fitStartEntry.signal_activate().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_fitBox.pack_start(_fitEndEntry, Gtk::PACK_SHRINK);
	_fitEndEntry.set_sensitive(false);
	_fitEndEntry.signal_activate().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	
	_fitFrame.add(_fitBox);
	
	_sideBox.pack_start(_fitFrame, Gtk::PACK_SHRINK);
	
	Gtk::RadioButtonGroup group;
	_functionBox.pack_start(_nsButton, Gtk::PACK_SHRINK);
	_nsButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_nsButton.set_group(group);
	_functionBox.pack_start(_dndsButton, Gtk::PACK_SHRINK);
	_dndsButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_dndsButton.set_group(group);
	_nsButton.set_active(true);
	
	_functionFrame.add(_functionBox);
	_sideBox.pack_start(_functionFrame, Gtk::PACK_SHRINK);
	
	_plotPropertiesButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::onPlotPropertiesClicked));
	_sideBox.pack_start(_plotPropertiesButton, Gtk::PACK_SHRINK);
	
	_dataExportButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::onDataExportClicked));
	_sideBox.pack_start(_dataExportButton, Gtk::PACK_SHRINK);
	
	_slopeBox.pack_start(_slopeTextView, Gtk::PACK_SHRINK);
	_drawSlopeButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_slopeBox.pack_start(_drawSlopeButton, Gtk::PACK_SHRINK);

	_slopeBox.pack_start(_slopeAutoRangeButton, Gtk::PACK_SHRINK);
	_slopeAutoRangeButton.set_active(true);
	_slopeAutoRangeButton.signal_clicked().connect(sigc::mem_fun(*this, &HistogramPage::onSlopeAutoRangeClicked));
	
	_slopeBox.pack_start(_slopeStartEntry, Gtk::PACK_SHRINK);
	_slopeStartEntry.set_sensitive(false);
	_slopeStartEntry.signal_activate().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	_slopeBox.pack_start(_slopeEndEntry, Gtk::PACK_SHRINK);
	_slopeEndEntry.set_sensitive(false);
	_slopeEndEntry.signal_activate().connect(sigc::mem_fun(*this, &HistogramPage::updatePlot));
	
	_slopeFrame.add(_slopeBox);
	_sideBox.pack_start(_slopeFrame, Gtk::PACK_SHRINK);
	
	pack_start(_sideBox, Gtk::PACK_SHRINK);
	
	_plotWidget.SetPlot(_plot);
	pack_start(_plotWidget, Gtk::PACK_EXPAND_WIDGET);
	
	show_all_children();
	
	_dataWindow = new DataWindow();
}

HistogramPage::~HistogramPage()
{
	if(_plotPropertiesWindow != 0)
		delete _plotPropertiesWindow;
	if(_histograms != 0)
		delete _histograms;
	if(_summedPolarizationHistograms != 0)
		delete _summedPolarizationHistograms;
	delete _dataWindow;
}

void HistogramPage::readFromFile()
{
	CloseStatistics();
	HistogramTablesFormatter histogramTables(_statFilename);
	if(histogramTables.HistogramsExist())
	{
		MeasurementSet set(_statFilename);
		
		const unsigned polarizationCount = set.GetPolarizationCount();

		_histograms = new HistogramCollection(polarizationCount);
		_histograms->Load(histogramTables);
	}
}

void HistogramPage::CloseStatistics()
{
	_statFilename = std::string();
	if(_histograms != 0) delete _histograms;
	if(_summedPolarizationHistograms != 0) delete _summedPolarizationHistograms;
}

void HistogramPage::SetStatistics(HistogramCollection &collection)
{
	CloseStatistics();
	_histograms = new HistogramCollection(collection);
	_summedPolarizationHistograms = _histograms->CreateSummedPolarizationCollection();
	updatePlot();
}

void HistogramPage::updatePlot()
{
	if(HasStatistics())
	{
		_plot.Clear();
		
		const unsigned polarizationCount = _histograms->PolarizationCount();
		if(_xxPolarizationButton.get_active())
			plotPolarization(*_histograms, 0);
		if(_xyPolarizationButton.get_active() && polarizationCount>=2)
			plotPolarization(*_histograms, 1);
		if(_yxPolarizationButton.get_active() && polarizationCount>=3)
			plotPolarization(*_histograms, 2);
		if(_yyPolarizationButton.get_active() && polarizationCount>=4)
			plotPolarization(*_histograms, 3);
		
		_plotWidget.Update();
		updateSlopeFrame();
		updateDataWindow();
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
		
		if(_fitButton.get_active() || _subtractFitButton.get_active())
		{
			plotFit(totalHistogram, "Fit to total");
		}
	}

	LogHistogram rfiHistogram;
	histograms.GetRFIHistogramForCrossCorrelations(p, rfiHistogram);
	if(_rfiHistogramButton.get_active())
	{
		_plot.StartLine("RFI histogram", "Amplitude in arbitrary units (log)", "Frequency (log)");
		addHistogramToPlot(rfiHistogram);

		if(_fitButton.get_active() || _subtractFitButton.get_active())
		{
			plotFit(rfiHistogram, "Fit to RFI");
		}
	}
	if(_drawSlopeButton.get_active())
	{
		plotSlope(rfiHistogram, "Fitted slope");
	}
	
	if(_notRFIHistogramButton.get_active())
	{
		_plot.StartLine("Non-RFI histogram", "Amplitude in arbitrary units (log)", "Frequency (log)");
		LogHistogram histogram;
		histograms.GetTotalHistogramForCrossCorrelations(p, histogram);
		LogHistogram rfiHistogram;
		histograms.GetRFIHistogramForCrossCorrelations(p, rfiHistogram);
		histogram -= rfiHistogram;
		addHistogramToPlot(histogram);

		if(_fitButton.get_active() || _subtractFitButton.get_active())
		{
			plotFit(histogram, "Fit to Non-RFI");
		}
	}
}

void HistogramPage::plotFit(class LogHistogram &histogram, const std::string &title)
{
	double minRange, maxRange, sigmaEstimate;
	sigmaEstimate = RayleighFitter::SigmaEstimate(histogram);
	if(_fitAutoRangeButton.get_active())
	{
		RayleighFitter::FindFitRangeUnderRFIContamination(histogram.MinPositiveAmplitude(), sigmaEstimate, minRange, maxRange);
		std::stringstream minRangeStr, maxRangeStr;
		minRangeStr << minRange;
		maxRangeStr << maxRange;
		_fitStartEntry.set_text(minRangeStr.str());
		_fitEndEntry.set_text(maxRangeStr.str());
	} else {
		minRange = atof(_fitStartEntry.get_text().c_str());
		maxRange = atof(_fitEndEntry.get_text().c_str());
	}
	RayleighFitter fitter;
	double sigma = sigmaEstimate, n = RayleighFitter::NEstimate(histogram, minRange, maxRange);
	fitter.Fit(minRange, maxRange, histogram, sigma, n);
	if(_fitButton.get_active())
	{
		_plot.StartLine(title, "Amplitude in arbitrary units (log)", "Frequency (log)");
		addRayleighToPlot(histogram, sigma, n);
	}
	if(_subtractFitButton.get_active())
	{
		_plot.StartLine(title, "Amplitude in arbitrary units (log)", "Frequency (log)");
		addRayleighDifferenceToPlot(histogram, sigma, n);
	}
}

void HistogramPage::addHistogramToPlot(LogHistogram &histogram)
{
	const bool derivative = _dndsButton.get_active();
	for(LogHistogram::iterator i=histogram.begin();i!=histogram.end();++i)
	{
		if(derivative)
		{
			const double x = i.value();
			const double logx = log10(x);
			const double cslope = histogram.NormalizedSlope(x*0.5, x*2.0);
			if(std::isfinite(logx) && std::isfinite(cslope))
				_plot.PushDataPoint(logx, cslope);
		} else {
			const double x = i.value();
			const double logx = log10(x);
			const double logc = log10(i.normalizedCount());
			if(std::isfinite(logx) && std::isfinite(logc))
				_plot.PushDataPoint(logx, logc);
		}
	}
}

void HistogramPage::addRayleighToPlot(LogHistogram &histogram, double sigma, double n)
{
	const bool derivative = _dndsButton.get_active();
	double x = histogram.MinPositiveAmplitude();
	const double xend = sigma*5.0;
	const double sigmaP2 = sigma*sigma;
	while(x < xend) {
		const double logx = log10(x);
		if(derivative)
		{
			const double dc = -(pow10(2.0*x)-sigmaP2)/sigmaP2;
			if(std::isfinite(logx) && std::isfinite(dc))
				_plot.PushDataPoint(logx, dc);
		} else {
			const double c = n * x / (sigmaP2) * exp(-x*x/(2*sigmaP2));
			const double logc = log10(c);
			if(std::isfinite(logx) && std::isfinite(logc))
				_plot.PushDataPoint(logx, logc);
		}
		x *= 1.05;
	}
}

void HistogramPage::addRayleighDifferenceToPlot(LogHistogram &histogram, double sigma, double n)
{
	const double sigmaP2 = sigma*sigma;
	double minCount = histogram.MinPosNormalizedCount();
	for(LogHistogram::iterator i=histogram.begin();i!=histogram.end();++i)
	{
		const double x = i.value();
		
    const double c = n * x / (sigmaP2) * exp(-x*x/(2*sigmaP2));
		double diff = fabs(i.normalizedCount() - c);
		if(diff >= minCount)
		{
			const double logx = log10(x);
			const double logc = log10(diff);
			if(std::isfinite(logx) && std::isfinite(logc))
				_plot.PushDataPoint(logx, logc);
		}
	}
}

void HistogramPage::plotSlope(class LogHistogram &histogram, const std::string &title)
{
	double start, end;
	if(_slopeAutoRangeButton.get_active())
	{
		histogram.GetRFIRegion(start, end);
	} else {
		start = atof(_slopeStartEntry.get_text().c_str());
		end = atof(_slopeEndEntry.get_text().c_str());
	}
	double slope = histogram.NormalizedSlope(start, end);
	double offset = histogram.NormalizedSlopeOffset(start, end, slope);
	_plot.StartLine(title, "Amplitude in arbitrary units (log)", "Frequency (log)");
	double xStart = log10(start / 10.0);
	double xEnd = log10(histogram.MaxAmplitude());
	double yStart = xStart*slope + offset;
	double yEnd = xEnd*slope + offset;
	_plot.PushDataPoint(xStart, yStart);
	_plot.PushDataPoint(xEnd, yEnd);
}

void HistogramPage::onPlotPropertiesClicked()
{
	if(_plotPropertiesWindow == 0)
	{
		_plotPropertiesWindow = new PlotPropertiesWindow(_plot, "Plot properties");
		_plotPropertiesWindow->OnChangesApplied = boost::bind(&HistogramPage::updatePlot, this);
	}
	
	_plotPropertiesWindow->show();
	_plotPropertiesWindow->raise();
}

void HistogramPage::onDataExportClicked()
{
	_dataWindow->show();
	_dataWindow->raise();
	updateDataWindow();
}

void HistogramPage::updateSlopeFrame()
{
	std::stringstream str;
	str << "Slopes:";
	for(size_t p=0;p<_histograms->PolarizationCount();++p)
	{
		LogHistogram histogram;
		_histograms->GetRFIHistogramForCrossCorrelations(p, histogram);
		
		double minRange, maxRange;
		if(_slopeAutoRangeButton.get_active())
		{
			histogram.GetRFIRegion(minRange, maxRange);
			if(p==0)
			{
				std::stringstream minRangeStr, maxRangeStr;
				minRangeStr << minRange;
				maxRangeStr << maxRange;
				_slopeStartEntry.set_text(minRangeStr.str());
				_slopeEndEntry.set_text(maxRangeStr.str());
			}
		} else {
			minRange = atof(_slopeStartEntry.get_text().c_str());
			maxRange = atof(_slopeEndEntry.get_text().c_str());
		}

		double slope = histogram.NormalizedSlope(minRange, maxRange);
		str << '\n' << slope;
	}
	_slopeTextView.get_buffer()->set_text(str.str());
}

void HistogramPage::updateDataWindow()
{
	if(_dataWindow->is_visible())
		_dataWindow->SetData(_plot);
}
