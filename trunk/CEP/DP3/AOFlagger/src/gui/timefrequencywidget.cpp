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
#include <AOFlagger/gui/timefrequencywidget.h>

#include <gdkmm/pixbuf.h>

#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/strategy/algorithms/thresholdconfig.h>
#include <AOFlagger/strategy/algorithms/thresholdtools.h>

#include <iostream>

#include <AOFlagger/gui/plot/horizontalplotscale.h>
#include <AOFlagger/gui/plot/verticalplotscale.h>
#include <AOFlagger/gui/plot/colorscale.h>

TimeFrequencyWidget::TimeFrequencyWidget() :
	_isInitialized(false), _showOriginalFlagging(true), _showAlternativeFlagging(true), _useColor(true), _colorMap(TFBWMap),
	_visualizedImage(TFOriginalImage),
	_highlighting(false),
	_hasImage(false),
	_segmentedImage(),
	_horiScale(0),
	_vertScale(0),
	_colorScale(0),
	_max(1.0), _min(0.0),
	_range(Winsorized)
{
	_highlightConfig = new ThresholdConfig();
	_highlightConfig->InitializeLengthsSingleSample();

	add_events(Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK);
	signal_motion_notify_event().connect(sigc::mem_fun(*this, &TimeFrequencyWidget::onMotion));
	signal_button_release_event().connect(sigc::mem_fun(*this, &TimeFrequencyWidget::onButtonReleased));
}

TimeFrequencyWidget::~TimeFrequencyWidget()
{
	Clear();
	delete _highlightConfig;
}

void TimeFrequencyWidget::Clear()
{
	if(_hasImage)
	{
		_mask.reset();
		delete _highlightConfig;
		_highlightConfig = new ThresholdConfig();
		_highlightConfig->InitializeLengthsSingleSample();
		_hasImage = false;
		_segmentedImage.reset();
	}
	if(_horiScale != 0) {
		delete _horiScale;
		_horiScale = 0;
	}
	if(_vertScale != 0) {
		delete _vertScale;
		_vertScale = 0;
	}
}

void TimeFrequencyWidget::Init()
{
	signal_expose_event().connect(sigc::mem_fun(*this, &TimeFrequencyWidget::onExposeEvent) );
}

bool TimeFrequencyWidget::onExposeEvent(GdkEventExpose *)
{
	redraw();
	return true;
}

void TimeFrequencyWidget::SetNewData(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
{
	Clear();
	_mask =  data.GetSingleMask();
	_original = data;
	_revised = _original;
	_revised.SetImagesToZero();
	_contaminated = _original;
	_metaData = metaData;
	_hasImage = true;

	_startTime = 0;
	_endTime = data.ImageWidth();
	_startFrequency = 0;
	_endFrequency = data.ImageHeight();
}

const TimeFrequencyData TimeFrequencyWidget::getDifference() const
{
	Image2DPtr image = Image2D::CreateFromDiff(_original.GetSingleImage(), _revised.GetSingleImage());
	return TimeFrequencyData(TimeFrequencyData::AmplitudePart, SinglePolarisation, image);
}

void TimeFrequencyWidget::Update()
{
	if(_hasImage)
	{
		size_t width = _endTime - _startTime;
		size_t height = _endFrequency - _startFrequency;

		switch(_visualizedImage)
		{
			case TFOriginalImage: _image = _original.GetSingleImage(); break;
			case TFRevisedImage: _image = _revised.GetSingleImage(); break;
			case TFContaminatedImage: _image = _contaminated.GetSingleImage(); break;
			case TFDifferenceImage:
				_image = Image2D::CreateFromDiff(_original.GetSingleImage(), _revised.GetSingleImage());
				break;
		}
	
		num_t min, max;
		Mask2DCPtr mask = GetActiveMask();
		findMinMax(_image, mask, min, max);
		
		if(_horiScale != 0)
			delete _horiScale;
		if(_vertScale != 0)
			delete _vertScale;
		if(_colorScale != 0)
			delete _colorScale;
		if(_metaData != 0) {
			_vertScale = new VerticalPlotScale(get_window());
			_vertScale->InitializeNumericTicks(_metaData->Band().channels[_startFrequency].frequencyHz / 1e6, _metaData->Band().channels[_endFrequency-1].frequencyHz / 1e6);
			
			_horiScale = new HorizontalPlotScale(get_window());
			_horiScale->InitializeTimeTicks(_metaData->ObservationTimes()[_startTime], _metaData->ObservationTimes()[_endTime-1]);
		} else {
			_vertScale = new VerticalPlotScale(get_window());
			_vertScale->InitializeNumericTicks(_startFrequency, _endFrequency-1);
			_horiScale = new HorizontalPlotScale(get_window());
			_horiScale->InitializeNumericTicks(_startTime, _endTime-1);
		}
		_colorScale = new ColorScale(get_window());
		_colorScale->InitializeNumericTicks(min, max);

		_leftBorderSize = _vertScale->GetWidth();
		_rightBorderSize = _horiScale->GetRightMargin();
		_topBorderSize = 10;
		_bottomBorderSize = _horiScale->GetHeight();

		ColorMap *colorMap = createColorMap();
		for(unsigned x=0;x<256;++x)
		{
			const num_t
				colorVal = (2.0 / 256.0) * x - 1.0,
				imageVal = (max-min) * x / 256.0 + min;
			double
				r = colorMap->ValueToColorR(colorVal),
				g = colorMap->ValueToColorG(colorVal),
				b = colorMap->ValueToColorB(colorVal);
			_colorScale->SetColorValue(imageVal, r/255.0, g/255.0, b/255.0);
		}
		
		unsigned sampleSize = 8;
		_pixbuf.clear();
		_pixbuf =
			Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, sampleSize, width, height);
	
		guint8* data = _pixbuf->get_pixels();
		size_t rowStride = _pixbuf->get_rowstride();

		Mask2DPtr highlightMask;
		if(_highlighting)
		{
			highlightMask = Mask2D::CreateSetMaskPtr<false>(_image->Width(), _image->Height());
			_highlightConfig->Execute(_image, highlightMask, true, 10.0);
		}
		Mask2DCPtr altMask = _contaminated.GetSingleMask();
		
		for(unsigned long y=_startFrequency;y<_endFrequency;++y) {
			guint8* rowpointer = data + rowStride * (_endFrequency - y - 1);
			for(unsigned long x=_startTime;x<_endTime;++x) {
				int xa = (x-_startTime) * 4;
				char r,g,b,a;
				bool highlighted = _highlighting && highlightMask->Value(x, y) != 0;
				bool originallyFlagged = _mask->Value(x, y);
				bool altFlagged = altMask->Value(x, y);
				if(highlighted) {
					r = 255; g = 0; b = 0; a = 255;
				} else if(_showOriginalFlagging && originallyFlagged) {
					r = 255; g = 0; b = 255; a = 255;
				} else if(_showAlternativeFlagging && altFlagged) {
					r = 255; g = 255; b = 0; a = 255;
				} else {
					num_t val = _image->Value(x, y);
					if(val > max) val = max;
					else if(val < min) val = min;
	
					val = (_image->Value(x, y) - min) * 2.0 / (max - min) - 1.0;
					if(val < -1.0) val = -1.0;
					else if(val > 1.0) val = 1.0;
					r = colorMap->ValueToColorR(val);
					g = colorMap->ValueToColorG(val);
					b = colorMap->ValueToColorB(val);
					a = colorMap->ValueToColorA(val);
				}
				rowpointer[xa]=r;
				rowpointer[xa+1]=g;
				rowpointer[xa+2]=b;
				rowpointer[xa+3]=a;
			}
		}
		delete colorMap;

		if(_segmentedImage != 0)
		{
			for(unsigned long y=_startFrequency;y<_endFrequency;++y) {
				guint8* rowpointer = data + rowStride * (y - _startFrequency);
				for(unsigned long x=_startTime;x<_endTime;++x) {
					if(_segmentedImage->Value(x,y) != 0)
					{
						int xa = (x-_startTime) * 4;
						rowpointer[xa]=IntMap::R(_segmentedImage->Value(x,y));
						rowpointer[xa+1]=IntMap::G(_segmentedImage->Value(x,y));
						rowpointer[xa+2]=IntMap::B(_segmentedImage->Value(x,y));
						rowpointer[xa+3]=IntMap::A(_segmentedImage->Value(x,y));
					}
				}
			}
		}

		_pixBufWidth = width;
		_pixBufHeight = height;
		while(_pixBufWidth > (unsigned) (get_width()*2))
		{
			ShrinkPixBufHorizontally();
		}

		_isInitialized = true;
		redraw();
	}
} 

ColorMap *TimeFrequencyWidget::createColorMap()
{
	switch(_colorMap) {
		case TFLogMap:
			return new PosMonochromeLogMap();
		case TFBWMap:
			return new MonochromeMap();
		case TFColorMap:
			return new ColdHotMap();
		default:
			return 0;
	}
}

void TimeFrequencyWidget::findMinMax(Image2DCPtr image, Mask2DCPtr mask, num_t &min, num_t &max)
{
	switch(_range)
	{
		case MinMax:
			max = ThresholdTools::MaxValue(image, mask);
			min = ThresholdTools::MinValue(image, mask);
		break;
		case Winsorized:
		{
			num_t mean, stddev, genMax, genMin;
			ThresholdTools::WinsorizedMeanAndStdDev(image, mask, mean, stddev);
			genMax = ThresholdTools::MaxValue(image, mask);
			genMin = ThresholdTools::MinValue(image, mask);
			max = mean + stddev*3.0;
			min = mean - stddev*3.0;
			if(genMin > min) min = genMin;
			if(genMax < max) max = genMax;
		}
		break;
		case Specified:
			min = _min;
			max = _max;
		break;
	}
	_max = max;
	_min = min;
}

void TimeFrequencyWidget::redraw()
{
	if(_isInitialized) {
		Cairo::RefPtr<Cairo::Context> cairo = get_window()->create_cairo_context();
		cairo->set_source_rgb(1.0, 1.0, 1.0);
		cairo->set_line_width(1.0);
		cairo->rectangle(0, 0, get_width(), get_height());
		cairo->fill();
		
		double rightBorder = _rightBorderSize;
		_colorScale->SetPlotDimensions(get_width() - rightBorder, get_height()-_topBorderSize - _bottomBorderSize - 10.0, _topBorderSize + 10.0);
		rightBorder += _colorScale->GetWidth() + 5.0;
		_vertScale->SetPlotDimensions(get_width() - rightBorder, get_height() - _topBorderSize - _bottomBorderSize, _topBorderSize);
		_horiScale->SetPlotDimensions(get_width() - rightBorder, get_height()-_topBorderSize - _bottomBorderSize, _topBorderSize, _vertScale->GetWidth());
		
		int
			width = get_width() - (int) floor(_leftBorderSize + rightBorder),
			height = get_height() - (int) floor(_topBorderSize + _bottomBorderSize);
		_pixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR)->render_to_drawable(get_window(), get_style()->get_black_gc(),
		0, 0, (int) round(_leftBorderSize), (int) round(_topBorderSize), -1, -1, Gdk::RGB_DITHER_NONE, 0, 0);
		cairo->set_source_rgb(0.0, 0.0, 0.0);
		cairo->rectangle(round(_leftBorderSize), round(_topBorderSize), width, height);
		cairo->stroke();

		_colorScale->Draw(cairo);
		_vertScale->Draw(cairo);
		_horiScale->Draw(cairo);
	}
}

void TimeFrequencyWidget::ClearBackground()
{
	_revised.SetImagesToZero();
}

void TimeFrequencyWidget::ShrinkPixBufHorizontally()
{
	_pixBufWidth /= 2;
	Glib::RefPtr<Gdk::Pixbuf> newPixBuf =
		Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, _pixBufWidth, _pixBufHeight);

	guint8* newData = newPixBuf->get_pixels();
	size_t rowStrideOfNew = newPixBuf->get_rowstride();

	guint8* oldData = _pixbuf->get_pixels();
	size_t rowStrideOfOld = _pixbuf->get_rowstride();

	for(unsigned long y=0;y<_pixBufHeight;++y) {
		guint8* rowpointerToNew = newData + rowStrideOfNew * y;
		guint8* rowpointerToOld = oldData + rowStrideOfOld * y;
		for(unsigned long x=0;x<_pixBufWidth;++x) {
			unsigned char r1 = (*rowpointerToOld); ++rowpointerToOld;
			unsigned char g1 = (*rowpointerToOld); ++rowpointerToOld;
			unsigned char b1 = (*rowpointerToOld); ++rowpointerToOld;
			unsigned char a1 = (*rowpointerToOld); ++rowpointerToOld;
			unsigned char r2 = (*rowpointerToOld); ++rowpointerToOld;
			unsigned char g2 = (*rowpointerToOld); ++rowpointerToOld;
			unsigned char b2 = (*rowpointerToOld); ++rowpointerToOld;
			unsigned char a2 = (*rowpointerToOld); ++rowpointerToOld;
			(*rowpointerToNew) = (r1 + r2)/2;
			 ++rowpointerToNew;
			(*rowpointerToNew) = (g1 + g2)/2;
			 ++rowpointerToNew;
			(*rowpointerToNew) = (b1 + b2)/2;
			 ++rowpointerToNew;
			(*rowpointerToNew) = (a1 + a2)/2;
			 ++rowpointerToNew;
		}
	}

	_pixbuf = newPixBuf;
}

Mask2DCPtr TimeFrequencyWidget::GetActiveMask() const
{
	if(_showOriginalFlagging)
	{
		if(_showAlternativeFlagging)
		{
			Mask2DPtr mask = Mask2D::CreateCopy(_original.GetSingleMask()); 
			mask->Join(_contaminated.GetSingleMask());
			return mask;
		} else
			return _original.GetSingleMask();
	} else {
		if(_showAlternativeFlagging)
			return _contaminated.GetSingleMask();
		else
			return Mask2D::CreateSetMaskPtr<false>(_original.ImageWidth(), _original.ImageHeight());
	}
}

bool TimeFrequencyWidget::toUnits(double mouseX, double mouseY, int &posX, int &posY)
{
	const unsigned
		width = _endTime - _startTime,
		height = _endFrequency - _startFrequency;
	double rightBorder = _rightBorderSize;
	rightBorder += _colorScale->GetWidth() + 5.0;
	posX = (int) round((mouseX - _leftBorderSize) * width / (get_width() - rightBorder - _leftBorderSize) - 0.5);
	posY = (int) round((mouseY - _topBorderSize) * height / (get_height() - _bottomBorderSize - _topBorderSize) - 0.5);
	bool inDomain = posX >= 0 && posY >= 0 && posX < (int) width && posY < (int) height;
	posX += _startTime;
	posY = _endFrequency - posY - 1;
	return inDomain;
}

bool TimeFrequencyWidget::onMotion(GdkEventMotion *event)
{
	if(HasImage())
	{
		int posX, posY;
		if(toUnits(event->x, event->y, posX, posY))
			_onMouseMoved(posX, posY);
	}
	return true;
}

bool TimeFrequencyWidget::onButtonReleased(GdkEventButton *event)
{
	if(HasImage())
	{
		int posX, posY;
		if(toUnits(event->x, event->y, posX, posY))
			_onButtonReleased(posX, posY);
	}
	return true;
}
