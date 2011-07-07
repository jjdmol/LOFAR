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

#include <AOFlagger/util/ffttools.h>
#include <AOFlagger/util/plot.h>
#include <AOFlagger/util/ffttools.h>

#include <AOFlagger/strategy/algorithms/sinusfitter.h>

ImagePlaneWindow::ImagePlaneWindow()
 : _imager(1536*2, 1536*2), _clearButton("Clear"),
	_applyWeightsButton("Apply weights"),
	_refreshCurrentButton("R"),
	_memoryStoreButton("MS"),
	_memoryRecallButton("MR"),
	_memoryMultiplyButton("Mx"),
	_memorySubtractButton("M-"),
	_sqrtButton("sqrt"),
	_fixScaleButton("S"),
	_plotHorizontalButton("H"), _plotVerticalButton("V"),
	_angularTransformButton("AT"),
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
	_topBox.pack_start(_clearButton, false, true);
	_clearButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onClearClicked));
	_clearButton.show();

	_topBox.pack_start(_applyWeightsButton, false, true);
	_applyWeightsButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onApplyWeightsClicked));
	_applyWeightsButton.show();

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

	_topBox.pack_start(_memorySubtractButton, false, true);
	_memorySubtractButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onMemorySubtractClicked));
	_memorySubtractButton.show();

	_topBox.pack_start(_sqrtButton, false, true);
	_sqrtButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onSqrtClicked));
	_sqrtButton.show();

	_topBox.pack_start(_fixScaleButton, false, true);
	_fixScaleButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onFixScaleClicked));
	_fixScaleButton.show();
	
	_topBox.pack_start(_plotHorizontalButton, false, true);
	_plotHorizontalButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onPlotHorizontally));
	_plotHorizontalButton.show();
	
	_topBox.pack_start(_plotVerticalButton, false, true);
	_plotVerticalButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onPlotVertically));
	_plotVerticalButton.show();
	
	_topBox.pack_start(_angularTransformButton, false, true);
	_angularTransformButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePlaneWindow::onAngularTransformButton));
	_angularTransformButton.show();
	
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
	_imager.Image(data, metaData);
	_lastMetaData = metaData;
	Update();
}

void ImagePlaneWindow::AddData(const TimeFrequencyData &data, class SpatialMatrixMetaData *spatialMetaData)
{
	_imager.Image(data, spatialMetaData);
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

void ImagePlaneWindow::onMemorySubtractClicked()
{
	Image2DPtr subtracted = Image2D::CreateCopy(_memory);
	Image2DCPtr old = _imageWidget.Image();
	for(size_t y=0;y<subtracted->Height();++y)
	{
		for(size_t x=0;x<subtracted->Width();++x)
		{
			subtracted->SetValue(x, y, subtracted->Value(x, y) - old->Value(x, y));
		}
	}
	_imageWidget.SetImage(subtracted);
	_imageWidget.Update();
	printStats();
}

void ImagePlaneWindow::onSqrtClicked()
{
	Image2DPtr sqrtImage = Image2D::CreateCopy(_imageWidget.Image());
	FFTTools::SignedSqrt(sqrtImage);
	_imageWidget.SetImage(sqrtImage);
	_imageWidget.Update();
	printStats();
}

void ImagePlaneWindow::onFixScaleClicked()
{
	if(_fixScaleButton.get_active())
		_imageWidget.FixScale();
	else
		_imageWidget.SetAutomaticScale();
}

void ImagePlaneWindow::onPlotHorizontally()
{
	Plot plot("Image-horizontal-axis.pdf");
	plot.SetXAxisText("RA index");
	plot.SetYAxisText("Amplitude");
	//plot.SetLogScale(false, true);
	plot.StartLine();
	Image2DCPtr image = _imageWidget.Image();
	for(size_t x=0;x<image->Width();++x)
	{
		num_t sum = 0.0;
		for(size_t y=0;y<image->Height();++y)
		{
			sum += image->Value(x, y);
		}
		plot.PushDataPoint(x, sum);
	}
	plot.Close();
	plot.Show();
}

void ImagePlaneWindow::onPlotVertically()
{
	Plot plot("Image-vertical-axis.pdf");
	plot.SetXAxisText("Declination index");
	plot.SetYAxisText("Amplitude");
	//plot.SetLogScale(false, true);
	plot.StartLine();
	Image2DCPtr image = _imageWidget.Image();
	for(size_t y=0;y<image->Height();++y)
	{
		num_t sum = 0.0;
		for(size_t x=0;x<image->Width();++x)
		{
			sum += image->Value(x, y);
		}
		plot.PushDataPoint(y, sum);
	}
	plot.Close();
	plot.Show();
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
		
		const BandInfo band = _lastMetaData->Band();
		num_t frequencyHz = band.channels[band.channelCount/2].frequencyHz;
		num_t rms = _imageWidget.Image()->GetRMS(left, top, right-left, bottom-top);
		num_t max = _imageWidget.Image()->GetMaximum(left, top, right-left, bottom-top);
		num_t xRel = posX-width/2.0, yRel = posY-height/2.0;
		const numl_t
			dist = sqrtnl(xRel*xRel + yRel*yRel),
			delayRa = _lastMetaData->Field().delayDirectionRA,
			delayDec = _lastMetaData->Field().delayDirectionDec;
		std::cout << "Clicked at: " << xRel << "," << yRel << '\n';
		double
			distanceRad = _imager.ImageDistanceToDecRaDistance(dist);
		std::cout << "RMS=" << rms << ", max=" << max
			<< ", angle=" << (SinusFitter::Phase(xRel, -yRel)*180.0/M_PI) << ", dist=" << dist << "\n"
			<< "Distance ~ "
			<< distanceRad << " rad = "
			<< Angle::ToString(distanceRad) << " = "
			<< (1.0/_imager.ImageDistanceToFringeSpeedInSamples(dist, frequencyHz, _lastMetaData)) << " samples/fringe.\n";
		numl_t
			centerX = cosn(delayRa) * delayDec,
			centerY = -sinn(delayRa) * delayDec,
			dx = _imager.ImageDistanceToDecRaDistance(-xRel) + centerX,
			dy = _imager.ImageDistanceToDecRaDistance(yRel) + centerY,
			ra = 2.0*M_PInl - SinusFitter::Phase(dx, dy),
			dec = sqrtnl(dx*dx + dy*dy);
		std::cout << "Delay = " << RightAscension::ToString(delayRa) << ", " << Declination::ToString(delayDec) << " (@" << dx << "," << dy << ")\n";
		std::cout << "RA = " << RightAscension::ToString(ra) << ", DEC = " << Declination::ToString(dec) << "\n";
	}

	return true;
}

void ImagePlaneWindow::onAngularTransformButton()
{
	Image2DPtr transformedImage = FFTTools::AngularTransform(_imageWidget.Image());
	_imageWidget.SetImage(transformedImage);
	_imageWidget.Update();
}

