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
 : _imager(1536*2, 1536*2), _clearButton("Clear"),
	_applyWeightsButton("Apply weights"),
	_refreshCurrentButton("R"),
	_memoryStoreButton("MS"),
	_memoryRecallButton("MR"),
	_memoryMultiplyButton("Mx"),
	_sqrtButton("sqrt"),
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

	_topBox.pack_start(_refreshCurrentButton, false, true);
	_refreshCurrentButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onRefreshCurrentClicked));
	_refreshCurrentButton.show();

	_topBox.pack_start(_memoryStoreButton, false, true);
	_memoryStoreButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemoryStoreClicked));
	_memoryStoreButton.show();

	_topBox.pack_start(_memoryRecallButton, false, true);
	_memoryRecallButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemoryRecallClicked));
	_memoryRecallButton.show();

	_topBox.pack_start(_memoryMultiplyButton, false, true);
	_memoryMultiplyButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemoryMultiplyClicked));
	_memoryMultiplyButton.show();

	_topBox.pack_start(_sqrtButton, false, true);
	_sqrtButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onSqrtClicked));
	_sqrtButton.show();

	// Show containers
	_box.pack_start(_topBox, false, true);
	_topBox.show();

	_box.pack_start(_imageWidget);
	_imageWidget.show();
	_imageWidget.add_events(Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK);
	_imageWidget.signal_button_release_event().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onButtonReleased));

	add(_box);
	_box.show();

	onZoomButtonClicked();
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
			_imageWidget.SetAutomaticMin();
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
			_imageWidget.SetMin(0.0);
			_imageWidget.Update();
		printStats();
			_displayingUV = false;
		}
	}
}

void ImagePlaneWindow::onApplyWeightsClicked()
{
	_imager.ApplyWeightsToUV();
	Update();
}

void ImagePlaneWindow::onRefreshCurrentClicked()
{
	_imageWidget.SetImage(Image2D::CreateCopyPtr(_imager.FTReal()));
	_imageWidget.Update();
}

void ImagePlaneWindow::onMemoryStoreClicked()
{
	_memory = _imageWidget.Image();
}

void ImagePlaneWindow::onMemoryRecallClicked()
{
	_imageWidget.SetImage(_memory);
	_imageWidget.Update();
}

void ImagePlaneWindow::onMemoryMultiplyClicked()
{
	Image2DPtr multiplied = Image2D::CreateCopy(_memory);
	Image2DCPtr old = _imageWidget.Image();
	for(size_t y=0;y<multiplied->Height();++y)
	{
		for(size_t x=0;x<multiplied->Width();++x)
		{
			multiplied->SetValue(x, y, multiplied->Value(x, y) * old->Value(x, y));
		}
	}
	_imageWidget.SetImage(multiplied);
	_imageWidget.Update();
	printStats();
}

void ImagePlaneWindow::onSqrtClicked()
{
	Image2DPtr sqrtImage = Image2D::CreateCopy(_imageWidget.Image());
	for(size_t y=0;y<sqrtImage->Height();++y)
	{
		for(size_t x=0;x<sqrtImage->Width();++x)
		{
			if(sqrtImage->Value(x, y) >= 0.0)
				sqrtImage->SetValue(x, y, sqrt(sqrtImage->Value(x, y)));
			else
				sqrtImage->SetValue(x, y, -sqrt(-sqrtImage->Value(x, y)));
		}
	}
	_imageWidget.SetImage(sqrtImage);
	_imageWidget.Update();
	printStats();
}

void ImagePlaneWindow::printStats()
{
	num_t topLeftRMS = _imageWidget.Image()->GetRMS(0, 0, _imageWidget.Image()->Width()/3, _imageWidget.Image()->Height()/3);
	std::cout << "RMS=" << _imageWidget.Image()->GetRMS()
		<< ", max=" << _imageWidget.Image()->GetMaximum()
		<< ", min=" << _imageWidget.Image()->GetMinimum()
		<< ", top left RMS=" << topLeftRMS
		<< ", SNR=" << _imageWidget.Image()->GetMaximum()/topLeftRMS
		<< std::endl;
}

bool ImagePlaneWindow::onButtonReleased(GdkEventButton *event)
{
	if(_imageWidget.IsInitialized())
	{
		int 
			width = _imageWidget.Image()->Width(),
			height = _imageWidget.Image()->Height(),
			posX = (size_t) roundl((long double) event->x * width / _imageWidget.get_width() - 0.5L),
			posY = (size_t) roundl((long double) event->y * height / _imageWidget.get_height() - 0.5L);
			
		if(posX >= width)
			posX = width - 1;
		if(posY >= height)
			posY = height - 1;
	
		int left = posX - 3, right = posX + 3, top = posY - 3, bottom = posY + 3;
		if(left < 0) left = 0;
		if(right >= width) right = width - 1;
		if(top < 0) top = 0;
		if(bottom >= height) bottom = height - 1;
		
		num_t rms = _imageWidget.Image()->GetRMS(left, top, right-left, bottom-top);
		num_t max = _imageWidget.Image()->GetMaximum(left, top, right-left, bottom-top);
		std::cout << "RMS=" << rms << ", max=" << max
			<< std::endl;
	}
	return true;
}
