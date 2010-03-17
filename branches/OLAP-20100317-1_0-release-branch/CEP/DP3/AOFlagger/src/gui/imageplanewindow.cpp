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
#include <AOFlagger/gui/imageplanewindow.h>

ImagePlaneWindow::ImagePlaneWindow()
 : _imager(1536, 1536), _clearButton("Clear"),
	_applyWeightsButton("Apply weights"),
	_uvPlaneButton("UV plane"), _imagePlaneButton("Image plane"),
	_zoomXd4Button("x1/4"), _zoomXd2Button("x1/2"),
	_zoomX1Button("x1"), _zoomX2Button("x2"), _zoomX4Button("x4"),
	_zoomX8Button("x8"), _zoomX16Button("x16"), _zoomX32Button("x32"),
	_zoomX64Button("x64"), _zoomX128Button("x128"),
	_zoom(1.0L), _displayingUV(true)
{
	set_default_size(400,400);

	// Add the plane radio buttons
	Gtk::RadioButtonGroup group;
	_topBox.pack_start(_uvPlaneButton, false, true);
	_uvPlaneButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onUVPlaneButtonClicked));
	_uvPlaneButton.set_group(group);
	_uvPlaneButton.set_active(true);
	_uvPlaneButton.show();

	_topBox.pack_start(_imagePlaneButton, false, true);
	_imagePlaneButton.set_group(group);
	_imagePlaneButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onImagePlaneButtonClicked));
	_imagePlaneButton.show();

	// Add the clear button
	_buttonBox.pack_start(_clearButton, false, true);
	_clearButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onClearClicked));
	_clearButton.show();

	_buttonBox.pack_start(_applyWeightsButton, false, true);
	_applyWeightsButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onApplyWeightsClicked));
	_applyWeightsButton.show();

	_topBox.pack_start(_buttonBox, false, true);
	_buttonBox.show();

	// Add the zoom buttons
	Gtk::RadioButtonGroup zoomGroup;
	_topBox.pack_start(_zoomXd4Button, false, true);
	_zoomXd4Button.set_group(zoomGroup);
	_zoomXd4Button.set_active(true);
	_zoomXd4Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomXd4Button.show();

	_topBox.pack_start(_zoomXd2Button, false, true);
	_zoomXd2Button.set_group(zoomGroup);
	_zoomXd2Button.set_active(true);
	_zoomXd2Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomXd2Button.show();

	_topBox.pack_start(_zoomX1Button, false, true);
	_zoomX1Button.set_group(zoomGroup);
	_zoomX1Button.set_active(true);
	_zoomX1Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX1Button.show();

	_topBox.pack_start(_zoomX2Button, false, true);
	_zoomX2Button.set_group(zoomGroup);
	_zoomX2Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX2Button.show();

	_topBox.pack_start(_zoomX4Button, false, true);
	_zoomX4Button.set_group(zoomGroup);
	_zoomX4Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX4Button.show();

	_topBox.pack_start(_zoomX8Button, false, true);
	_zoomX8Button.set_group(zoomGroup);
	_zoomX8Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX8Button.show();

	_topBox.pack_start(_zoomX16Button, false, true);
	_zoomX16Button.set_group(zoomGroup);
	_zoomX16Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX16Button.show();

	_topBox.pack_start(_zoomX32Button, false, true);
	_zoomX32Button.set_group(zoomGroup);
	_zoomX32Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX32Button.show();

	_topBox.pack_start(_zoomX64Button, false, true);
	_zoomX64Button.set_group(zoomGroup);
	_zoomX64Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX64Button.show();

	_topBox.pack_start(_zoomX128Button, false, true);
	_zoomX128Button.set_group(zoomGroup);
	_zoomX128Button.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onZoomButtonClicked));
	_zoomX128Button.show();

	// Show containers
	_box.pack_start(_topBox, false, true);
	_topBox.show();

	_box.pack_start(_imageWidget);
	_imageWidget.show();

	add(_box);
	_box.show();
}


ImagePlaneWindow::~ImagePlaneWindow()
{
}

void ImagePlaneWindow::AddData(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
{
	for(size_t y=0;y<data.ImageHeight();++y)
		_imager.Image(data, metaData, y);
	Update();
}

void ImagePlaneWindow::onClearClicked()
{
	_imager.Empty();
	Update();
}

void ImagePlaneWindow::onUVPlaneButtonClicked()
{
	if(!_displayingUV)
	{
		Update();
	}
}

void ImagePlaneWindow::onImagePlaneButtonClicked()
{
	if(_displayingUV)
		Update();
}

void ImagePlaneWindow::onZoomButtonClicked()
{
	double zoom = 1;
	if(_zoomXd4Button.get_active()) zoom = 0.25;
	else if(_zoomXd2Button.get_active()) zoom = 0.5;
	else if(_zoomX2Button.get_active()) zoom = 2;
	else if(_zoomX4Button.get_active()) zoom = 4;
	else if(_zoomX8Button.get_active()) zoom = 8;
	else if(_zoomX16Button.get_active()) zoom = 16;
	else if(_zoomX32Button.get_active()) zoom = 32;
	else if(_zoomX64Button.get_active()) zoom = 64;
	else if(_zoomX128Button.get_active()) zoom = 128;
	if(_zoom != zoom)
	{
		_imager.Empty();
		_imager.SetUVScaling(0.0001L * (long double) zoom); // TODO
		Update();
		_zoom = zoom;
	}
}

void ImagePlaneWindow::Update()
{
	if(_uvPlaneButton.get_active())
	{
		if(_imager.HasUV()) {
			_imageWidget.SetImage(Image2D::CreateCopyPtr(_imager.RealUVImage()));
			_imageWidget.Update();
			_displayingUV = true;
		}
	}
	else
	{
		if(!_imager.HasFFT() && _imager.HasUV())
			_imager.PerformFFT();

		if(_imager.HasFFT()) {
			_imageWidget.SetImage(Image2D::CreateCopyPtr(_imager.FTReal()));
			_imageWidget.Update();
			_displayingUV = false;
		}
	}
}

void ImagePlaneWindow::onApplyWeightsClicked()
{
	_imager.ApplyWeightsToUV();
	Update();
}
