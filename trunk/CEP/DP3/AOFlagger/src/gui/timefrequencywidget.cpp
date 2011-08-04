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

#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/strategy/algorithms/thresholdconfig.h>
#include <AOFlagger/strategy/algorithms/thresholdtools.h>

#include <iostream>

#include <AOFlagger/gui/plot/horizontalplotscale.h>
#include <AOFlagger/gui/plot/verticalplotscale.h>
#include <AOFlagger/gui/plot/colorscale.h>

TimeFrequencyWidget::TimeFrequencyWidget() :
	_isInitialized(false), _showOriginalFlagging(true), _showAlternativeFlagging(true), _useColor(true), _colorMap(BWMap),
	_visualizedImage(TFOriginalImage),
	_highlighting(false),
	_hasImage(false),
	_segmentedImage(),
	_horiScale(0),
	_vertScale(0),
	_colorScale(0),
	_useLogScale(false),
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
	if(get_width() == (int) _initializedWidth && get_height() == (int) _initializedHeight)
		redrawWithoutChanges(get_window()->create_cairo_context(), get_width(), get_height());
	else
		Update();
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
		update(get_window()->create_cairo_context(), get_width(), get_height());
	}
}

void TimeFrequencyWidget::SavePdf(const std::string &filename)
{
	unsigned width = get_width(), height = get_height();
	Cairo::RefPtr<Cairo::PdfSurface> surface = Cairo::PdfSurface::create(filename, width, height);
	Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(surface);
	if(_hasImage)
	{
		AOLogger::Debug << "Saving PDF of " <<get_width() << " x " << get_height() << "\n";
		update(cairo, width, height);
	}
	cairo->show_page();
	// We finish the surface. This might be required, because some of the subclasses store the cairo context. In that
	// case, it won't be written.
	surface->finish();
}

void TimeFrequencyWidget::SaveSvg(const std::string &filename)
{
	unsigned width = get_width(), height = get_height();
	Cairo::RefPtr<Cairo::SvgSurface> surface = Cairo::SvgSurface::create(filename, width, height);
	Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(surface);
	if(_hasImage)
	{
		AOLogger::Debug << "Saving SVG of " << get_width() << " x " << get_height() << "\n";
		update(cairo, width, height);
	}
	cairo->show_page();
	surface->finish();
}

void TimeFrequencyWidget::SavePng(const std::string &filename)
{
	unsigned width = get_width(), height = get_height();
	Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, width, height);
	Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(surface);
	if(_hasImage)
	{
		AOLogger::Debug << "Saving PNG of " << get_width() << " x " << get_height() << "\n";
		update(cairo, width, height);
	}
	surface->write_to_png(filename);
}

void TimeFrequencyWidget::update(Cairo::RefPtr<Cairo::Context> cairo, unsigned width, unsigned height)
{
	size_t
		imageWidth = _endTime - _startTime,
		imageHeight = _endFrequency - _startFrequency;

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
	_vertScale = new VerticalPlotScale();
	_vertScale->SetDrawWithDescription(_showAxisDescriptions);
	_horiScale = new HorizontalPlotScale();
	_horiScale->SetDrawWithDescription(_showAxisDescriptions);
	_colorScale = new ColorScale(cairo);
	_colorScale->SetDrawWithDescription(_showAxisDescriptions);
	if(_metaData != 0) {
		_vertScale->InitializeNumericTicks(_metaData->Band().channels[_startFrequency].frequencyHz / 1e6, _metaData->Band().channels[_endFrequency-1].frequencyHz / 1e6);
		_vertScale->SetUnitsCaption("Frequency");
		_horiScale->InitializeTimeTicks(_metaData->ObservationTimes()[_startTime], _metaData->ObservationTimes()[_endTime-1]);
		_horiScale->SetUnitsCaption("Time");
		_colorScale->SetUnitsCaption(_metaData->DataDescription() + " (" + _metaData->DataUnits() + ")");
	} else {
		_vertScale->InitializeNumericTicks(_startFrequency, _endFrequency-1);
		_horiScale->InitializeNumericTicks(_startTime, _endTime-1);
	}
	if(_useLogScale)
		_colorScale->InitializeLogarithmicTicks(min, max);
	else
		_colorScale->InitializeNumericTicks(min, max);

	// The scale dimensions are depending on each other. However, since the height of the horizontal scale is practically
	// not dependent on other dimensions, we give the horizontal scale temporary width/height, so that we can calculate its
	// height:
	_horiScale->SetPlotDimensions(width, height, 0.0, 0.0);
	_bottomBorderSize = _horiScale->GetHeight(cairo);
	_rightBorderSize = _horiScale->GetRightMargin(cairo);
	
	_topBorderSize = 10;
	_vertScale->SetPlotDimensions(width - _rightBorderSize + 5.0, height - _topBorderSize - _bottomBorderSize, _topBorderSize);
	_leftBorderSize = _vertScale->GetWidth(cairo);
	_colorScale->SetPlotDimensions(width - _rightBorderSize, height-_topBorderSize - _bottomBorderSize - 10.0, _topBorderSize + 10.0);
	_rightBorderSize += _colorScale->GetWidth() + 5.0;
	_horiScale->SetPlotDimensions(width - _rightBorderSize + 5.0, height -_topBorderSize - _bottomBorderSize, _topBorderSize, 	_vertScale->GetWidth(cairo));

	class ColorMap *colorMap = createColorMap();
	const double
		minLog10 = min>0.0 ? log10(min) : 0.0,
		maxLog10 = max>0.0 ? log10(max) : 0.0;
	for(unsigned x=0;x<256;++x)
	{
		num_t colorVal = (2.0 / 256.0) * x - 1.0;
		num_t imageVal;
		if(_useLogScale)
			imageVal = exp10((x / 256.0) * (log10(max) - minLog10) + minLog10);
		else 
			imageVal = (max-min) * x / 256.0 + min;
		double
			r = colorMap->ValueToColorR(colorVal),
			g = colorMap->ValueToColorG(colorVal),
			b = colorMap->ValueToColorB(colorVal);
		_colorScale->SetColorValue(imageVal, r/255.0, g/255.0, b/255.0);
	}
	
	_imageSurface.clear();
	_imageSurface =
		Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, imageWidth, imageHeight);

	unsigned char *data = _imageSurface->get_data();
	size_t rowStride = _imageSurface->get_stride();

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

				if(_useLogScale)
					val = (log10(_image->Value(x, y)) - minLog10) * 2.0 / (maxLog10 - minLog10) - 1.0;
				else
					val = (_image->Value(x, y) - min) * 2.0 / (max - min) - 1.0;
				if(val < -1.0) val = -1.0;
				else if(val > 1.0) val = 1.0;
				r = colorMap->ValueToColorR(val);
				g = colorMap->ValueToColorG(val);
				b = colorMap->ValueToColorB(val);
				a = colorMap->ValueToColorA(val);
			}
			rowpointer[xa]=b;
			rowpointer[xa+1]=g;
			rowpointer[xa+2]=r;
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

	while(_imageSurface->get_width() > (int) (width*2))
	{
		shrinkImageBufferHorizontally();
	}

	_isInitialized = true;
	_initializedWidth = width;
	_initializedHeight = height;
	redrawWithoutChanges(cairo, width, height);
} 

ColorMap *TimeFrequencyWidget::createColorMap()
{
	switch(_colorMap) {
		case BWMap:
			return new MonochromeMap();
		case InvertedMap:
			return new class InvertedMap();
		case ColorMap:
			return new ColdHotMap();
		case RedBlueMap:
			return new class RedBlueMap();
		case RedYellowBlueMap:
			return new class RedYellowBlueMap();
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

void TimeFrequencyWidget::redrawWithoutChanges(Cairo::RefPtr<Cairo::Context> cairo, unsigned width, unsigned height)
{
	if(_isInitialized) {
		cairo->set_source_rgb(1.0, 1.0, 1.0);
		cairo->set_line_width(1.0);
		cairo->rectangle(0, 0, width, height);
		cairo->fill();
		
		int
			destWidth = width - (int) floor(_leftBorderSize + _rightBorderSize),
			destHeight = height - (int) floor(_topBorderSize + _bottomBorderSize),
			sourceWidth = _imageSurface->get_width(),
			sourceHeight = _imageSurface->get_height();
		cairo->save();
		cairo->translate((int) round(_leftBorderSize), (int) round(_topBorderSize));
		cairo->scale((double) destWidth / (double) sourceWidth, (double) destHeight / (double) sourceHeight);
		Cairo::RefPtr<Cairo::SurfacePattern> pattern = Cairo::SurfacePattern::create(_imageSurface);
		pattern->set_filter(Cairo::FILTER_BEST);
		cairo->set_source(pattern);
		cairo->rectangle(0, 0, sourceWidth, sourceHeight);
		cairo->clip();
		cairo->paint();
		cairo->restore();
		cairo->set_source_rgb(0.0, 0.0, 0.0);
		cairo->rectangle(round(_leftBorderSize), round(_topBorderSize), destWidth, destHeight);
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

void TimeFrequencyWidget::shrinkImageBufferHorizontally()
{
	const unsigned newWidth = _imageSurface->get_width()/2;
	const unsigned height = _imageSurface->get_height();
	Cairo::RefPtr<Cairo::ImageSurface> newImageSurface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, newWidth, height);

	unsigned char* newData = newImageSurface->get_data();
	size_t rowStrideOfNew = newImageSurface->get_stride();

	guint8* oldData = _imageSurface->get_data();
	size_t rowStrideOfOld = _imageSurface->get_stride();

	for(unsigned long y=0;y<height;++y) {
		guint8* rowpointerToNew = newData + rowStrideOfNew * y;
		guint8* rowpointerToOld = oldData + rowStrideOfOld * y;
		for(unsigned long x=0;x<newWidth;++x) {
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

	_imageSurface = newImageSurface;
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
