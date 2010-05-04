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

#include <iostream>

#include <gtkmm/messagedialog.h>

#include <AOFlagger/gui/msoptionwindow.h>

#include <AOFlagger/msio/timefrequencyimager.h>

#include <AOFlagger/rfi/strategy/msimageset.h>

#include <AOFlagger/gui/mswindow.h>

MSOptionWindow::MSOptionWindow(MSWindow &msWindow, const std::string &filename) :
	Gtk::Window(),
	_msWindow(msWindow),
	_filename(filename),
	_openButton("Open"),
	_dataKindFrame("Columns to read"),
	_polarisationFrame("Polarisation to read"),
	_partitioningFrame("Partitioning"),
	_observedDataButton("Observed"), _correctedDataButton("Corrected"), _modelDataButton("Model"), _residualDataButton("Residual"), _weightsButton("Weights"),
	_allDipolePolarisationButton("Dipole (xx,xy,yx,yy separately)"),
	_autoDipolePolarisationButton("Dipole auto-correlations (xx and yy)"),
	_stokesIPolarisationButton("Stokes I"),
	_noPartitioningButton("No partitioning"),
	_max2500ScansButton("Split when >2.500 scans"),
	_max10000ScansButton("Split when >10.000 scans"),
	_max25000ScansButton("Split when >25.000 scans"),
	_max100000ScansButton("Split when >100.000 scans")
{
	set_title("Options for opening a measurement set");

	initDataTypeButtons();
	initPolarisationButtons();

	_openButton.signal_clicked().connect(sigc::mem_fun(*this, &MSOptionWindow::onOpen));
	_bottomButtonBox.pack_start(_openButton);
	_openButton.show();

	_leftVBox.pack_start(_bottomButtonBox);
	_bottomButtonBox.show();

	_topHBox.pack_start(_leftVBox);
	_leftVBox.show();

	initPartitioningButtons();

	add(_topHBox);
	_topHBox.show();
}

MSOptionWindow::~MSOptionWindow()
{
}

void MSOptionWindow::initDataTypeButtons()
{
	Gtk::RadioButton::Group group = _observedDataButton.get_group();
	_correctedDataButton.set_group(group);
	_modelDataButton.set_group(group);
	_residualDataButton.set_group(group);
	_weightsButton.set_group(group);

	_dataKindBox.pack_start(_observedDataButton);
	_dataKindBox.pack_start(_correctedDataButton);
	_dataKindBox.pack_start(_modelDataButton);
	_dataKindBox.pack_start(_residualDataButton);
	_dataKindBox.pack_start(_weightsButton);

	_dataKindBox.show();
	_dataKindFrame.add(_dataKindBox);

	_dataKindFrame.show();
	_leftVBox.pack_start(_dataKindFrame);

	_observedDataButton.show();
	_correctedDataButton.show();
	_modelDataButton.show();
	_residualDataButton.show();
	_weightsButton.show();

	_dataKindFrame.show();
}

void MSOptionWindow::initPolarisationButtons()
{
	Gtk::RadioButton::Group group = _allDipolePolarisationButton.get_group();
	_autoDipolePolarisationButton.set_group(group);
	_stokesIPolarisationButton.set_group(group);

	_polarisationBox.pack_start(_allDipolePolarisationButton);
	_polarisationBox.pack_start(_autoDipolePolarisationButton);
	_polarisationBox.pack_start(_stokesIPolarisationButton);

	_polarisationFrame.add(_polarisationBox);
	_leftVBox.pack_start(_polarisationFrame);

	_allDipolePolarisationButton.show();
	_autoDipolePolarisationButton.show();
	_stokesIPolarisationButton.show();
	_polarisationBox.show();
	_polarisationFrame.show();
}

void MSOptionWindow::initPartitioningButtons()
{
	Gtk::RadioButton::Group group = _noPartitioningButton.get_group();
	_max2500ScansButton.set_group(group);
	_max10000ScansButton.set_group(group);
	_max25000ScansButton.set_group(group);
	_max100000ScansButton.set_group(group);

	_partitioningBox.pack_start(_noPartitioningButton);
	_partitioningBox.pack_start(_max2500ScansButton);
	_partitioningBox.pack_start(_max10000ScansButton);
	_partitioningBox.pack_start(_max25000ScansButton);
	_partitioningBox.pack_start(_max100000ScansButton);

	_partitioningFrame.add(_partitioningBox);
	_topHBox.pack_start(_partitioningFrame);

	_noPartitioningButton.show();
	_max2500ScansButton.show();
	_max10000ScansButton.show();
	_max25000ScansButton.show();
	_max100000ScansButton.show();
	_partitioningBox.show();
	_partitioningFrame.show();
}

void MSOptionWindow::onOpen()
{
	std::cout << "Opening " << _filename << std::endl;
	try
	{
		rfiStrategy::ImageSet *imageSet = rfiStrategy::ImageSet::Create(_filename);
		if(dynamic_cast<rfiStrategy::MSImageSet*>(imageSet) != 0)
		{
			rfiStrategy::MSImageSet *msImageSet = static_cast<rfiStrategy::MSImageSet*>(imageSet);
			if(_observedDataButton.get_active())
				msImageSet->SetImageKind(TimeFrequencyImager::Observed);
			else if(_correctedDataButton.get_active())
				msImageSet->SetImageKind(TimeFrequencyImager::Corrected);
			else if(_modelDataButton.get_active())
				msImageSet->SetImageKind(TimeFrequencyImager::Model);
			else if(_residualDataButton.get_active())
				msImageSet->SetImageKind(TimeFrequencyImager::Residual);
			else if(_weightsButton.get_active())
				msImageSet->SetImageKind(TimeFrequencyImager::Weight);
	
			if(_allDipolePolarisationButton.get_active())
				msImageSet->SetReadAllPolarisations();
			else if(_autoDipolePolarisationButton.get_active())
				msImageSet->SetReadDipoleAutoPolarisations();
			else
				msImageSet->SetReadStokesI();
	
			if(_max2500ScansButton.get_active())
				msImageSet->SetMaxScanCounts(2500);
			else if(_max10000ScansButton.get_active())
				msImageSet->SetMaxScanCounts(10000);
			else if(_max25000ScansButton.get_active())
				msImageSet->SetMaxScanCounts(25000);
			else if(_max100000ScansButton.get_active())
				msImageSet->SetMaxScanCounts(100000);
			else
				msImageSet->SetMaxScanCounts(0);
		}
		imageSet->Initialize();
	
		_msWindow.SetImageSet(imageSet);
	} catch(std::exception &e)
	{
		Gtk::MessageDialog dialog(*this, e.what(), false, Gtk::MESSAGE_ERROR);
		dialog.run();
	}
	hide();
}

