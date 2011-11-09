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

#include <limits>

#include <AOFlagger/gui/quality/baselineplotpage.h>

#include <AOFlagger/msio/measurementset.h>

#include <AOFlagger/quality/statisticscollection.h>
#include <AOFlagger/quality/statisticsderivator.h>

BaselinePlotPage::BaselinePlotPage() :
	_statisticKindFrame("Statistic kind"),
	_countButton("Count"),
	_meanButton("Mean"),
	_varianceButton("Variance"),
	_dCountButton("DCount"),
	_dMeanButton("DMean"),
	_dVarianceButton("DVariance"),
	_rfiRatioButton("RFIRatio"),
	_polarizationFrame("Polarization"),
	_polXXButton("XX"),
	_polXYButton("XY"),
	_polYXButton("YX"),
	_polYYButton("YY"),
	_polXXandYYButton("XX/2 + YY/2"),
	_polXYandYXButton("XY/2 + YX/2"),
	_phaseFrame("Phase"),
	_amplitudePhaseButton("Amplitude"),
	_phasePhaseButton("Phase"),
	_realPhaseButton("Real"),
	_imaginaryPhaseButton("Imaginary"),
	_rangeFrame("Colour range"),
	_rangeMinMaxButton("Min to max"),
	_rangeWinsorizedButton("Winsorized"),
	_rangeSpecified("Specified"),
	_logarithmicScaleButton("Logarithmic"),
	_isOpen(false),
	_selectStatisticKind(QualityTablesFormatter::VarianceStatistic)
{
	initStatisticKinds();
	initPolarizations();
	initPhaseButtons();
	initRanges();
	
	pack_start(_sideBox, Gtk::PACK_SHRINK);
	
	_imageWidget.SetCairoFilter(Cairo::FILTER_NEAREST);
	_imageWidget.SetColorMap(ImageWidget::HotColdMap);
	_imageWidget.SetRange(ImageWidget::MinMax);
	_imageWidget.SetScaleOption(ImageWidget::LogScale);
	_imageWidget.set_size_request(300, 300);
	pack_start(_imageWidget);
	
	show_all_children();
}

BaselinePlotPage::~BaselinePlotPage()
{
	close();
}

void BaselinePlotPage::initStatisticKinds()
{
	Gtk::RadioButtonGroup statGroup;
	_countButton.set_group(statGroup);
	_countButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectCount));
	_statisticKindBox.pack_start(_countButton, Gtk::PACK_SHRINK);
	
	_meanButton.set_group(statGroup);
	_meanButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectMean));
	_statisticKindBox.pack_start(_meanButton, Gtk::PACK_SHRINK);
	
	_varianceButton.set_group(statGroup);
	_varianceButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectVariance));
	_statisticKindBox.pack_start(_varianceButton, Gtk::PACK_SHRINK);
	
	_dCountButton.set_group(statGroup);
	_dCountButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectDCount));
	_statisticKindBox.pack_start(_dCountButton, Gtk::PACK_SHRINK);
	
	_dMeanButton.set_group(statGroup);
	_dMeanButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectDMean));
	_statisticKindBox.pack_start(_dMeanButton, Gtk::PACK_SHRINK);
	
	_dVarianceButton.set_group(statGroup);
	_dVarianceButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectDVariance));
	_statisticKindBox.pack_start(_dVarianceButton, Gtk::PACK_SHRINK);
	
	_rfiRatioButton.set_group(statGroup);
	_rfiRatioButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectRFIRatio));
	_statisticKindBox.pack_start(_rfiRatioButton, Gtk::PACK_SHRINK);
	
	_varianceButton.set_active();
	
	_statisticKindFrame.add(_statisticKindBox);
	
	_sideBox.pack_start(_statisticKindFrame, Gtk::PACK_SHRINK);
}

void BaselinePlotPage::initPolarizations()
{
	Gtk::RadioButtonGroup polGroup;
	_polXXButton.set_group(polGroup);
	_polXXButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_polarizationBox.pack_start(_polXXButton, Gtk::PACK_SHRINK);
	
	_polXYButton.set_group(polGroup);
	_polXYButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_polarizationBox.pack_start(_polXYButton, Gtk::PACK_SHRINK);

	_polYXButton.set_group(polGroup);
	_polYXButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_polarizationBox.pack_start(_polYXButton, Gtk::PACK_SHRINK);

	_polYYButton.set_group(polGroup);
	_polYYButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_polarizationBox.pack_start(_polYYButton, Gtk::PACK_SHRINK);

	_polXXandYYButton.set_group(polGroup);
	_polXXandYYButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_polarizationBox.pack_start(_polXXandYYButton, Gtk::PACK_SHRINK);

	_polXYandYXButton.set_group(polGroup);
	_polXYandYXButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_polarizationBox.pack_start(_polXYandYXButton, Gtk::PACK_SHRINK);

	_polXXandYYButton.set_active();
	
	_polarizationFrame.add(_polarizationBox);
	
	_sideBox.pack_start(_polarizationFrame, Gtk::PACK_SHRINK);
}

void BaselinePlotPage::initPhaseButtons()
{
	Gtk::RadioButtonGroup phaseGroup;
	
	_amplitudePhaseButton.set_group(phaseGroup);
	_amplitudePhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_phaseBox.pack_start(_amplitudePhaseButton, Gtk::PACK_SHRINK);
	
	_phasePhaseButton.set_group(phaseGroup);
	_phasePhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_phaseBox.pack_start(_phasePhaseButton, Gtk::PACK_SHRINK);
	
	_realPhaseButton.set_group(phaseGroup);
	_realPhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_phaseBox.pack_start(_realPhaseButton, Gtk::PACK_SHRINK);
	
	_imaginaryPhaseButton.set_group(phaseGroup);
	_imaginaryPhaseButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::updateImage));
	_phaseBox.pack_start(_imaginaryPhaseButton, Gtk::PACK_SHRINK);
	
	_amplitudePhaseButton.set_active();
	
	_phaseFrame.add(_phaseBox);
	
	_sideBox.pack_start(_phaseFrame, Gtk::PACK_SHRINK);
}

void BaselinePlotPage::initRanges()
{
	Gtk::RadioButtonGroup rangeGroup;
	_rangeMinMaxButton.set_group(rangeGroup);
	_rangeMinMaxButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectMinMaxRange));
	_rangeBox.pack_start(_rangeMinMaxButton, Gtk::PACK_SHRINK);

	_rangeWinsorizedButton.set_group(rangeGroup);
	_rangeWinsorizedButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectWinsorizedRange));
	_rangeBox.pack_start(_rangeWinsorizedButton, Gtk::PACK_SHRINK);

	_rangeSpecified.set_group(rangeGroup);
	_rangeSpecified.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onSelectSpecifiedRange));
	_rangeBox.pack_start(_rangeSpecified, Gtk::PACK_SHRINK);
	
	_logarithmicScaleButton.signal_clicked().connect(sigc::mem_fun(*this, &BaselinePlotPage::onLogarithmicScaleClicked));
	_rangeBox.pack_start(_logarithmicScaleButton, Gtk::PACK_SHRINK);
	_logarithmicScaleButton.set_active(true);
	
	_rangeFrame.add(_rangeBox);
	
	_sideBox.pack_start(_rangeFrame, Gtk::PACK_SHRINK);
}

void BaselinePlotPage::close()
{
	if(_isOpen)
	{
		delete _statCollection;
		_isOpen = false;
	}
}

void BaselinePlotPage::readStatistics()
{
	MeasurementSet *ms = new MeasurementSet(_filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	delete ms;

	QualityTablesFormatter formatter(_filename);
	_statCollection = new StatisticsCollection(polarizationCount);
	_statCollection->Load(formatter);
	_isOpen = true;
}

void BaselinePlotPage::updateImage()
{
	if(_isOpen)
	{
		const QualityTablesFormatter::StatisticKind kind = GetSelectedStatisticKind();
		
		const unsigned polarizationCount = _statCollection->PolarizationCount();
		std::vector<std::pair<unsigned, unsigned> > baselines = _statCollection->BaselineStatistics().BaselineList();
		StatisticsDerivator derivator(*_statCollection);
		
		const unsigned antennaCount = _statCollection->BaselineStatistics().AntennaCount();

		Image2DPtr realImages[polarizationCount];
		Image2DPtr imagImages[polarizationCount];
		Mask2DPtr mask[polarizationCount];
		for(unsigned p=0;p<polarizationCount;++p)
		{
			realImages[p] = Image2D::CreateUnsetImagePtr(antennaCount, antennaCount);
			realImages[p]->SetAll(std::numeric_limits<num_t>::quiet_NaN());
			imagImages[p] = Image2D::CreateUnsetImagePtr(antennaCount, antennaCount);
			imagImages[p]->SetAll(std::numeric_limits<num_t>::quiet_NaN());
			mask[p] = Mask2D::CreateSetMaskPtr<true>(antennaCount, antennaCount);
		}
		
		for(std::vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
		{
			const unsigned antenna1 = i->first, antenna2 = i->second;
			for(unsigned p=0;p<polarizationCount;++p)
			{
				const std::complex<float> val = derivator.GetComplexBaselineStatistic(kind, antenna1, antenna2, p);
				realImages[p]->SetValue(antenna1, antenna2, val.real());
				imagImages[p]->SetValue(antenna1, antenna2, val.imag());
				mask[p]->SetValue(antenna1, antenna2, false);
			}
		}
		TimeFrequencyData data;
		if(polarizationCount == 1)
		{
			data = TimeFrequencyData(TimeFrequencyData::ComplexRepresentation, SinglePolarisation, realImages[0], imagImages[0]);
			data.SetGlobalMask(mask[0]);
		}
		else if(polarizationCount == 2)
		{
			data = TimeFrequencyData(AutoDipolePolarisation, realImages[0], imagImages[0], realImages[1], imagImages[1]);
			data.SetIndividualPolarisationMasks(mask[0], mask[1]);
		}
		else if(polarizationCount == 4)
		{
			data = TimeFrequencyData(realImages[0], imagImages[0], realImages[1], imagImages[1], realImages[2], imagImages[2], realImages[3], imagImages[3]);
			data.SetIndividualPolarisationMasks(mask[0], mask[1], mask[2], mask[3]);
		}
		else
			throw std::runtime_error("Set has not 1, 2 or 4 polarizations (?!?)");
		
		setToSelectedPolarization(data);
		
		setToSelectedPhase(data);
		
		_imageWidget.SetImage(data.GetSingleImage());
		_imageWidget.SetOriginalMask(data.GetSingleMask());
		_imageWidget.Update();
	}
}

void BaselinePlotPage::setToSelectedPolarization(TimeFrequencyData &data)
{
	TimeFrequencyData *newData = 0;
	if(_polXXButton.get_active())
		newData = data.CreateTFData(XXPolarisation);
	else if(_polXYButton.get_active())
		newData = data.CreateTFData(XYPolarisation);
	else if(_polYXButton.get_active())
		newData = data.CreateTFData(YXPolarisation);
	else if(_polYYButton.get_active())
		newData = data.CreateTFData(YYPolarisation);
	else if(_polXXandYYButton.get_active())
		newData = data.CreateTFData(AutoDipolePolarisation);
	else if(_polXYandYXButton.get_active())
		newData = data.CreateTFData(CrossDipolePolarisation);
	if(newData != 0)
	{
		data = *newData;
		delete newData;
	}
}

void BaselinePlotPage::setToSelectedPhase(TimeFrequencyData &data)
{
	TimeFrequencyData *newData = 0;
	if(_amplitudePhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::AmplitudePart);
	else if(_phasePhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::PhasePart);
	else if(_realPhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::RealPart);
	else if(_imaginaryPhaseButton.get_active())
		newData = data.CreateTFData(TimeFrequencyData::ImaginaryPart);
	if(newData != 0)
	{
		data = *newData;
		delete newData;
	}
}

