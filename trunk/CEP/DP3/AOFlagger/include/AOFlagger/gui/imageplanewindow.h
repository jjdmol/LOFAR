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
#ifndef IMAGEPLANEWINDOW_H
#define IMAGEPLANEWINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/radiobutton.h>

#include "../msio/timefrequencydata.h"
#include "../msio/timefrequencymetadata.h"

#include "../imaging/uvimager.h"

#include "imagewidget.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ImagePlaneWindow : public Gtk::Window {
	public:
		ImagePlaneWindow();
		~ImagePlaneWindow();

		void AddData(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData);
		UVImager *GetImager() throw() { return &_imager; }
		void Update();
	private:
		UVImager _imager;
		ImageWidget _imageWidget;
		Gtk::VBox _box;
		Gtk::HBox _topBox;
		Gtk::HButtonBox _buttonBox;
		Gtk::Button _clearButton;
		Gtk::Button _applyWeightsButton;
		Gtk::Button _memoryStoreButton, _memoryMultiplyButton, _sqrtButton;
		Gtk::RadioButton _uvPlaneButton, _imagePlaneButton;
		Gtk::RadioButton
			_zoomXd4Button, _zoomXd2Button, _zoomX1Button, _zoomX2Button, _zoomX4Button, _zoomX8Button,
			_zoomX16Button, _zoomX32Button, _zoomX64Button, _zoomX128Button;
		long double _zoom;

		bool _displayingUV;
		Image2DCPtr _memory;
		
		void onClearClicked();
		void onApplyWeightsClicked();
		void onUVPlaneButtonClicked();
		void onImagePlaneButtonClicked();
		void onZoomButtonClicked();
		void onMemoryStoreClicked();
		void onMemoryMultiplyClicked();
		void onSqrtClicked();

		void printStats();
};

#endif
