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
#include <AOFlagger/gui/complexplaneplotwindow.h>

#include <set>
#include <sstream>

#include <gtkmm/messagedialog.h>

#include <AOFlagger/msio/antennainfo.h>
#include <AOFlagger/msio/date.h>
#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/rfi/rfiplots.h>
#include <AOFlagger/rfi/thresholdmitigater.h>
#include <AOFlagger/rfi/fringestoppingfitter.h>

#include <AOFlagger/util/plot.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/gui/mswindow.h>

ComplexPlanePlotWindow::ComplexPlanePlotWindow(class MSWindow &_msWindow)
	: _msWindow(_msWindow),
		_detailsFrame("Location details"),
		_detailsLabel(),
		_xPositionLabel("time start position:"),
		_yPositionLabel("frequency start position:"),
		_lengthLabel("time length:"),
		_ySumLengthLabel("Frequency averaging size:"),
		_xPositionScale(0.0, _msWindow.GetOriginalData().ImageWidth(), 1.0),
		_yPositionScale(0.0, _msWindow.GetOriginalData().ImageHeight(), 1.0),
		_lengthScale(1.0, _msWindow.GetOriginalData().ImageWidth(), 1.0),
		_ySumLengthScale(1.0, _msWindow.GetOriginalData().ImageHeight(), 1.0),
		_realVersusImaginaryButton("Real versus imaginary"),
		_timeVersusRealButton("Time versus real"),
		_allValuesButton("All values"),
		_unmaskedValuesButton("Unmasked values"),
		_maskedValuesButton("Masked values"),
		_fittedValuesButton("Fitted values (constant freq)"),
		_individualSampleFitButton("Channel fitted values"),
		_fringeFitButton("Fringe fitted (varying freq)"),
		_dynamicFringeFitButton("Dynamic fringe fitted (varying freq+amp)"),
		_plotButton("Plot"),
		_xMax(_msWindow.GetOriginalData().ImageWidth()),
		_yMax(_msWindow.GetOriginalData().ImageHeight())
{
	_detailsFrame.add(_detailsBox);
	_detailsBox.show();

	_detailsBox.pack_start(_detailsLabel);
	_detailsLabel.set_line_wrap(true);
	_detailsLabel.set_max_width_chars(40);
	_detailsLabel.show();

	_mainBox.pack_start(_detailsFrame);
	_detailsFrame.show();
	
	_mainBox.pack_start(_xPositionLabel);
	_xPositionLabel.show();
	_xPositionScale.signal_value_changed().
		connect(sigc::mem_fun(*this, &ComplexPlanePlotWindow::onTimeStartChanged));
	_mainBox.pack_start(_xPositionScale);
	_xPositionScale.show();

	_mainBox.pack_start(_yPositionLabel);
	_yPositionLabel.show();
	_yPositionScale.signal_value_changed().
		connect(sigc::mem_fun(*this, &ComplexPlanePlotWindow::onFreqChanged));
	_mainBox.pack_start(_yPositionScale);
	_yPositionScale.show();

	_mainBox.pack_start(_lengthLabel);
	_lengthLabel.show();
	_lengthScale.signal_value_changed().
		connect(sigc::mem_fun(*this, &ComplexPlanePlotWindow::onTimeDurationChanged));
	_mainBox.pack_start(_lengthScale);
	_lengthScale.show();

	_mainBox.pack_start(_ySumLengthLabel);
	_ySumLengthLabel.show();
	_ySumLengthScale.signal_value_changed().
		connect(sigc::mem_fun(*this, &ComplexPlanePlotWindow::onFreqSizeChanged));
	_mainBox.pack_start(_ySumLengthScale);
	_ySumLengthScale.show();

	_mainBox.pack_start(_realVersusImaginaryButton);
	_realVersusImaginaryButton.show();
	Gtk::RadioButtonGroup group = _realVersusImaginaryButton.get_group();
	_timeVersusRealButton.set_group(group);
	_realVersusImaginaryButton.set_active(true);
	_mainBox.pack_start(_timeVersusRealButton);
	_timeVersusRealButton.show();

	_mainBox.pack_start(_allValuesButton);
	_allValuesButton.set_active(true);
	_allValuesButton.show();

	_mainBox.pack_start(_maskedValuesButton);
	_maskedValuesButton.show();

	_mainBox.pack_start(_unmaskedValuesButton);
	_unmaskedValuesButton.show();

	_mainBox.pack_start(_fittedValuesButton);
	_fittedValuesButton.set_active(true);
	_fittedValuesButton.show();

	_mainBox.pack_start(_individualSampleFitButton);
	_individualSampleFitButton.show();

	_mainBox.pack_start(_fringeFitButton);
	_fringeFitButton.show();

	_mainBox.pack_start(_dynamicFringeFitButton);
	_dynamicFringeFitButton.show();

	_plotButton.signal_clicked().connect(sigc::mem_fun(*this, &ComplexPlanePlotWindow::onPlotPressed));
	_buttonBox.pack_start(_plotButton);
	_plotButton.show();

	_mainBox.pack_start(_buttonBox);
	_buttonBox.show();

	add(_mainBox);
	_mainBox.show();

	_observationTimes = _msWindow.TimeFrequencyMetaData()->ObservationTimes();

	setDetailsLabel();
}

ComplexPlanePlotWindow::~ComplexPlanePlotWindow()
{
}

void ComplexPlanePlotWindow::onPlotPressed()
{
	if(_msWindow.HasImage())
	{
		try {
			Plot plot("dist.pdf");
			size_t x = (size_t) _xPositionScale.get_value();
			size_t y = (size_t) _yPositionScale.get_value();
			size_t length = (size_t) _lengthScale.get_value();
			size_t avgSize = (size_t) _ySumLengthScale.get_value();
			bool realVersusImaginary = _realVersusImaginaryButton.get_active();
			const TimeFrequencyData &data = _msWindow.GetActiveData();

			if(_allValuesButton.get_active())
			{
				plot.StartLine("Time connected measurement");
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(_msWindow.AltMask()->Width(), _msWindow.AltMask()->Height());
				RFIPlots::MakeComplexPlanePlot(plot, data, x, length, y, avgSize, mask, realVersusImaginary, false);
	
				if(!realVersusImaginary)
				{
					plot.StartLine("Time connected measurement (I)");
					RFIPlots::MakeComplexPlanePlot(plot, data, x, length, y, avgSize, mask, realVersusImaginary, true);
				}
			}

			if(_unmaskedValuesButton.get_active())
			{
				plot.StartLine("Without RFI");
				RFIPlots::MakeComplexPlanePlot(plot, data, x, length, y, avgSize, _msWindow.AltMask(), realVersusImaginary, false);
				if(!realVersusImaginary)
				{
					plot.StartLine("Without RFI (I)");
					RFIPlots::MakeComplexPlanePlot(plot, data, x, length, y, avgSize, _msWindow.AltMask(), realVersusImaginary, true);
				}
			}
	
			if(_maskedValuesButton.get_active())
			{
				plot.StartLine("Only RFI");
				Mask2DPtr mask = Mask2D::CreateCopy(_msWindow.AltMask());
				mask->Invert();
				RFIPlots::MakeComplexPlanePlot(plot, data, x, length, y, avgSize, mask, realVersusImaginary, false);
				if(!realVersusImaginary)
				{
					plot.StartLine("Only RFI (I)");
					RFIPlots::MakeComplexPlanePlot(plot, data, x, length, y, avgSize, mask, realVersusImaginary, true);
				}
			}
	
			if(_fittedValuesButton.get_active())
			{
				plot.StartLine("Single fit");
				size_t middleY = (2*y + avgSize) / 2;
				double timeStart = _observationTimes[x];
				double deltaTime;
				if(_observationTimes.size()>1)
					deltaTime = _observationTimes[1] - _observationTimes[0];
				else
					deltaTime = 1.0;
				double timeEnd = _observationTimes[x+length-1]+deltaTime;
				long double frequency = _msWindow.TimeFrequencyMetaData()->Band().channels[middleY].frequencyHz;
				Baseline baseline(_msWindow.TimeFrequencyMetaData()->Antenna1(), _msWindow.TimeFrequencyMetaData()->Antenna2());
				long double delayRA = _msWindow.TimeFrequencyMetaData()->Field().delayDirectionRA;
				long double delayDec = _msWindow.TimeFrequencyMetaData()->Field().delayDirectionDec;
				long double intFringeFreq =
					UVImager::GetIntegratedFringeStopFrequency(timeStart, timeEnd, baseline, delayRA, delayDec, frequency, 1000);
				long double sampleFrequency = intFringeFreq * deltaTime;
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(_msWindow.AltMask()->Width(), _msWindow.AltMask()->Height());
				RFIPlots::MakeFittedComplexPlot(plot, data, x, length, y, avgSize, mask, sampleFrequency, realVersusImaginary, false);
				if(!realVersusImaginary)
				{
					plot.StartLine("Single fit (I)");
					RFIPlots::MakeFittedComplexPlot(plot, data, x, length, y, avgSize, mask, sampleFrequency, realVersusImaginary, true);
				}
			}

			if(_individualSampleFitButton.get_active())
			{
				FringeStoppingFitter fitter;
				fitter.Initialize(data);
				fitter.SetFitChannelsIndividually(true);
				fitter.SetFringesToConsider(1.0L);
				fitter.SetMaxWindowSize(256);
				fitter.SetReturnFittedValue(true);
				fitter.SetReturnMeanValue(false);
				
				fitter.SetMetaData(_msWindow.TimeFrequencyMetaData());
				fitter.PerformFitOnOneChannel(y);

				plot.StartLine("Fit on each sample");
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(_msWindow.AltMask()->Width(), _msWindow.AltMask()->Height());
				RFIPlots::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, false);
	
				fitter.SetReturnFittedValue(false);
				fitter.SetReturnMeanValue(true);
				if(!realVersusImaginary)
				{
					plot.StartLine("Fit on each sample (I)");
					RFIPlots::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, true);
				}

				fitter.PerformFitOnOneChannel(y);

				plot.StartLine("Center");
				RFIPlots::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, false);
	
				if(!realVersusImaginary)
				{
					plot.StartLine("Center (I)");
					RFIPlots::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, true);
				}
			}
	
			if(_fringeFitButton.get_active() || _dynamicFringeFitButton.get_active())
			{
				/*FringeStoppingFitter fitter;
				Image2DPtr zero = Image2D::CreateZeroImagePtr( _msWindow.GetTimeFrequencyData().ImageWidth(), _msWindow.GetTimeFrequencyData().ImageHeight());
				Image2DPtr ones = Image2D::CreateZeroImagePtr( _msWindow.GetTimeFrequencyData().ImageWidth(), _msWindow.GetTimeFrequencyData().ImageHeight());
				for(size_t yi=0;yi<ones->Height();++yi)
					for(size_t xi=0;xi<ones->Width();++xi)
						ones->SetValue(xi, yi, 1.0L);
				TimeFrequencyData data(TimeFrequencyData::StokesI, ones, zero);
				fitter.Initialize(data);
				fitter.SetFitChannelsIndividually(true);
				
				std::vector<double> *times = _msWindow.CreateObservationTimesVector();
				fitter.SetBaselineInfo(_msWindow.GetFieldInfo(), _msWindow.GetBandInfo(), _msWindow.GetAntenna1Info(), _msWindow.GetAntenna2Info(), *times);
				fitter.PerformFringeStop();

				plot.StartLine("Fringe rotation");
				Image2DPtr mask = Image2D::CreateZeroImagePtr(_msWindow.AltMask()->Width(), _msWindow.AltMask()->Height());
				RFIStatistics::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, false);
	
				if(!realVersusImaginary)
				{
					plot.StartLine("Fringe rotation (I)");
					RFIStatistics::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, true);
				}

				delete times;*/

				FringeStoppingFitter fitter;
				fitter.Initialize(data);
				
				fitter.SetMetaData(_msWindow.TimeFrequencyMetaData());
				fitter.PerformFringeStop();
				/*if(_dynamicFringeFitButton.get_active())
					fitter.PerformRFIFit(y, y + avgSize, 200);
				else
					fitter.PerformRFIFit(y, y + avgSize);*/

				plot.StartLine("RFI fit");
				Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(_msWindow.AltMask()->Width(), _msWindow.AltMask()->Height());
				RFIPlots::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, false);
	
				if(!realVersusImaginary)
				{
					plot.StartLine("RFI fit (I)");
					RFIPlots::MakeComplexPlanePlot(plot, fitter.Background(), x, length, y, avgSize, mask, realVersusImaginary, true);
				}
			}

			plot.Close();
			plot.Show();
		} catch(std::exception &e)
		{
			Gtk::MessageDialog dialog(*this, e.what(), false, Gtk::MESSAGE_ERROR);
			dialog.run();
		}
	}
}

void ComplexPlanePlotWindow::setDetailsLabel()
{
	size_t x = (size_t) _xPositionScale.get_value();
	size_t y = (size_t) _yPositionScale.get_value();
	size_t length = (size_t) _lengthScale.get_value();
	size_t avgSize = (size_t) _ySumLengthScale.get_value();
	size_t middleY = (2*y + avgSize) / 2;

	double timeStart = _observationTimes[x];
	double deltaTime;
	if(_observationTimes.size()>1)
		deltaTime = _observationTimes[1] - _observationTimes[0];
	else
		deltaTime = 1.0;
	double timeEnd = _observationTimes[x+length-1]+deltaTime;
	long double frequency = _msWindow.TimeFrequencyMetaData()->Band().channels[middleY].frequencyHz;
	Baseline baseline(_msWindow.TimeFrequencyMetaData()->Antenna1(), _msWindow.TimeFrequencyMetaData()->Antenna2());
	long double delayRA = _msWindow.TimeFrequencyMetaData()->Field().delayDirectionRA;
	long double delayDec = _msWindow.TimeFrequencyMetaData()->Field().delayDirectionDec;
	long double intFringeFreq =
		UVImager::GetFringeCount(x, x+length, y, _msWindow.TimeFrequencyMetaData());
	long double midFringeFreq =
		UVImager::GetFringeStopFrequency((timeStart + timeEnd)/2, baseline, delayRA, delayDec, frequency);

	std::stringstream s;
	s << "Start time: " << Date::AipsMJDToString(timeStart) << std::endl
		<< "Frequency: " << frequency/1000000.0L << "Mhz" << std::endl
		<< "Baseline: " << baseline.Distance() << "m" << std::endl
		<< "Delay direction: " << delayRA << "RA, " << delayDec << "dec." << std::endl
		<< "(= " << (delayRA/M_PIn*180.0L) << " deg or " << (delayRA/M_PIn*12.0L) << " hrs RA, " << (delayDec/M_PIn*180.0L) << " deg dec.)" << std::endl
		<< "Mid fringe stopping freq: " << midFringeFreq << "(Hz)" << std::endl
		<< "Fringe count: " << intFringeFreq << std::endl
		<< "Fringe length: " << 1.0L/intFringeFreq << "(s)" << std::endl
		<< "Time step: " << deltaTime << "(s)" << std::endl
		<< "Samples/fringe: " << (1.0L / (deltaTime * intFringeFreq)) << std::endl
		<< "Fringes in domain: " << intFringeFreq << std::endl;
	
	_detailsLabel.set_text(s.str());
}

