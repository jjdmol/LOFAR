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
#ifndef IMAGEPROPERTIESWINDOW_H
#define IMAGEPROPERTIESWINDOW_H

#include <string>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ImagePropertiesWindow : public Gtk::Window {
	public:
		ImagePropertiesWindow(class MSWindow &msWindow, const std::string &filename);
		~ImagePropertiesWindow() { }
	private:
		void onApplyClicked();
		void onCloseClicked();

		class ImageWidget &_imageWidget;

		Gtk::HButtonBox _bottomButtonBox;
		Gtk::VBox _topVBox;
		Gtk::Button _openButton;
		Gtk::Frame _modeFrame;
		Gtk::VBox _modeBox;
		Gtk::RadioButton _allBeamletsButton, _singleBeamletButton, _channelBeamletButton, _statisticsButton;
		Gtk::Label _beamletsInSetLabel, _timeBlockSizeLabel;
		Gtk::HScale _beamletsInSetScale, _timeBlockSizeScale;
};

#endif // IMAGEPROPERTIESWINDOW_H
