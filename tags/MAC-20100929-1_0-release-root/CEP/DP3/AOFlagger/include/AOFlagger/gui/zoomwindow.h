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
#ifndef ZOOMWINDOW_H
#define ZOOMWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>
 
#include "../rfi/strategy/types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ZoomWindow : public Gtk::Window {
	public:
		ZoomWindow(class MSWindow &msWindow);
		~ZoomWindow();
	private:
		void onSetPressed();

		Gtk::HScale _hStartScale, _hStopScale;
		Gtk::VScale _vStartScale, _vStopScale;
		Gtk::Button _setButton;
		Gtk::HBox _hMainBox;
		Gtk::VBox _vSubBox;
		Gtk::HButtonBox _buttonBox;

		class MSWindow &_msWindow;
};

#endif
