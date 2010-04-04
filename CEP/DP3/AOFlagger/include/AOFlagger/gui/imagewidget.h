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
#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <gtkmm/drawingarea.h>

#include <vector>

#include "../msio/image2d.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ImageWidget : public Gtk::DrawingArea {
	public:
		ImageWidget();
		~ImageWidget();

		void Update();
		void SetImage(Image2DCPtr image) { _image = image; }
		Image2DCPtr Image() { return _image; }
		void Clear() { _image = Image2DCPtr(); _pixbuf=Glib::RefPtr<Gdk::Pixbuf>(); _isInitialized = false; } 
		void SetMin(num_t min)
		{
			_automaticMin = false;
			_min = min;
		}
		void SetAutomaticMin()
		{
			_automaticMin = true;
		}
	private:
		void redraw();
		void findMinMax(Image2DCPtr image, num_t &min, num_t &max);
		bool onExposeEvent(GdkEventExpose* ev);


		Glib::RefPtr<Gdk::Pixbuf> _pixbuf;
		Image2DCPtr _image;

		bool _isInitialized, _winsorizedStretch;
		bool _automaticMin;

		num_t _min;
};

#endif
