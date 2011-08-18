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
#include <AOFlagger/gui/imagewidget.h>

#include <AOFlagger/strategy/algorithms/thresholdtools.h>

#include <iostream>

ImageWidget::ImageWidget() : _image(), _isInitialized(false), _winsorizedStretch(false), _automaticMin(true), _automaticMax(true)
{
	signal_expose_event().connect(sigc::mem_fun(*this, &ImageWidget::onExposeEvent) );
}

ImageWidget::~ImageWidget()
{
}

bool ImageWidget::onExposeEvent(GdkEventExpose *)
{
	redraw();
	return true;
}

void ImageWidget::redraw()
{
	if(_isInitialized && get_width() > 1 && get_height() > 1) {
		//std::cout << "Displaying pixbuf to " << get_width() << " x " << get_height() << " area." << std::endl;
			_pixbuf->scale_simple(get_width(), get_height(), Gdk::INTERP_BILINEAR)->render_to_drawable(get_window(), get_style()->get_black_gc(),
		0, 0, 0, 0, get_width(), get_height(), Gdk::RGB_DITHER_NONE, 0, 0);
		//std::cout << "Done displaying pixbuf to " << get_width() << " x " << get_height() << " area." << std::endl;
	}
}

void ImageWidget::Update()
{
	if(_image != 0)
	{
		MonochromeMap *colorMap = new MonochromeMap();
		
		//std::cout << "Creating pix buf of " << _image->Width() << " x " << _image->Height() << std::endl;

		unsigned sampleSize = 8;
		_pixbuf =
			Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, sampleSize, _image->Width(), _image->Height());
	
		num_t min, max;
		if(_automaticMin || _automaticMax)
			findMinMax(_image, min, max);
		if(!_automaticMin)
			min = _min;
		if(!_automaticMax)
			max = _max;
	
		if(min != max || !std::isfinite(min) || !std::isfinite(max))
		{
			guint8* data = _pixbuf->get_pixels();
			size_t rowStride = _pixbuf->get_rowstride();

			for(unsigned long y=0;y<_image->Height();++y) {
				guint8* rowpointer = data + rowStride * y;
				for(unsigned long x=0;x<_image->Width();++x) {
					int xa = x * 4;
					char r,g,b,a;
					bool altMap = false;
					if(!std::isfinite(_image->Value(x, y))) {
						// Not set; output purely transparent pixel
						r = 0; g = 0; b = 0; a = 255;
					} else {
						num_t val = (_image->Value(x, y) - min) * 2.0 / (max - min) - 1.0;
						if(val < -1.0)
						{
							altMap = true;
							val = -2.0-val;
							if(val > 1.0)
								val = 1.0;
						}
						else if(val > 1.0) val = 1.0;
						g = 255-colorMap->ValueToColorG(val);
						b = 255-colorMap->ValueToColorB(val);
						if(altMap)
						{
							r = 255;
						} else {
							r = 255-colorMap->ValueToColorR(val);
						}
						a = colorMap->ValueToColorA(val);
					}
					rowpointer[xa]=r;
					rowpointer[xa+1]=g;
					rowpointer[xa+2]=b;
					rowpointer[xa+3]=a;
				}
			}
			_isInitialized = true;
			redraw();
		}
		delete colorMap;
	}
} 

void ImageWidget::findMinMax(Image2DCPtr image, num_t &min, num_t &max)
{
	if(_winsorizedStretch) {
		num_t mean, stddev, genMax, genMin;
		ThresholdTools::WinsorizedMeanAndStdDev(image, mean, stddev);
		genMax = image->GetMaximumFinite();
		genMin = image->GetMinimumFinite();

		max = mean + stddev*3.0;
		min = mean - stddev*3.0;
		if(genMin > min) min = genMin;
		if(genMax < max) max = genMax;
	} else {
		max = image->GetMaximumFinite();
		min = image->GetMinimumFinite();
	}
}
