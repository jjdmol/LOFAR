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

#include <iostream>
#include <sstream>

#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>

#include <AOFlagger/gui/imagepropertieswindow.h>
#include <AOFlagger/gui/imagewidget.h>

ImagePropertiesWindow::ImagePropertiesWindow(ImageWidget &imageWidget, const std::string &title) :
	Gtk::Window(),
	_imageWidget(imageWidget),
	_applyButton(Gtk::Stock::APPLY),
	_exportButton("Export"),
	_closeButton(Gtk::Stock::CLOSE),
	_saveImage(Gtk::Stock::SAVE, Gtk::ICON_SIZE_BUTTON),
	
	_colorMapFrame("Color map"),
	_grayScaleButton("Grayscale"),
	_invGrayScaleButton("Inverted grayscale"),
	_colorScaleButton("Coloured"),
	_RedBlueScaleButton("Red/blue"),
	_hotColdScaleButton("Hot/cold"),
	_redBlueYellowScaleButton("Red/Yellow/Blue"),
	
	_scaleFrame("Scale"),
	_minMaxScaleButton("From min to max"),
	_winsorizedScaleButton("Winsorized min and max"),
	_specifiedScaleButton("Specified:"),
	_scaleMinLabel("Scale minimum:"),
	_scaleMaxLabel("Scale maximum:"),
	_scaleMinEntry(),
	_scaleMaxEntry(),
	
	_optionsFrame("Options"),
	_normalOptionsButton("Normal scale"),
	_logScaleButton("Logarithmic scale"),
	_zeroSymmetricButton("Symmetric around zero"),
	
	_hStartScale(0, 1.01, 0.01),
	_hStopScale(0, 1.01, 0.01),
	_vStartScale(0, 1.01, 0.01),
	_vStopScale(0, 1.01, 0.01),
	
	_showAxisDescriptionsButton("Show axis descriptions")
{
	set_title(title);

	initColorMapButtons();
	initScaleWidgets();
	initOptionsWidgets();
	initZoomWidgets();
	
	_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePropertiesWindow::onApplyClicked));
	_bottomButtonBox.pack_start(_applyButton);

	_exportButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePropertiesWindow::onExportClicked));
	_exportButton.set_image(_saveImage);
	_bottomButtonBox.pack_start(_exportButton);

	_closeButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePropertiesWindow::onCloseClicked));
	_bottomButtonBox.pack_start(_closeButton);

	_topVBox.pack_start(_framesHBox);
	
	_showAxisDescriptionsButton.set_active(_imageWidget.ShowAxisDescriptions());
	_topVBox.pack_start(_showAxisDescriptionsButton);
	
	_topVBox.pack_start(_bottomButtonBox);

	add(_topVBox);
	_topVBox.show_all();
}

void ImagePropertiesWindow::initColorMapButtons()
{
	Gtk::RadioButton::Group group;

	_grayScaleButton.set_group(group);
	_colorMapBox.pack_start(_grayScaleButton);
	
	_invGrayScaleButton.set_group(group);
	_colorMapBox.pack_start(_invGrayScaleButton);
	
	_colorScaleButton.set_group(group);
	_colorMapBox.pack_start(_colorScaleButton);
	
	_RedBlueScaleButton.set_group(group);
	_colorMapBox.pack_start(_RedBlueScaleButton);
	
	_hotColdScaleButton.set_group(group);
	_colorMapBox.pack_start(_hotColdScaleButton);
	
	_redBlueYellowScaleButton.set_group(group);
	_colorMapBox.pack_start(_redBlueYellowScaleButton);
	
	switch(_imageWidget.GetColorMap())
	{
		default:
		case ImageWidget::BWMap: _grayScaleButton.set_active(true); break;
		case ImageWidget::InvertedMap: _invGrayScaleButton.set_active(true); break;
		case ImageWidget::ColorMap: _colorScaleButton.set_active(true); break;
		case ImageWidget::RedBlueMap: _RedBlueScaleButton.set_active(true); break;
		case ImageWidget::HotColdMap: _hotColdScaleButton.set_active(true); break;
		case ImageWidget::RedYellowBlueMap: _redBlueYellowScaleButton.set_active(true); break;
	}

	_colorMapFrame.add(_colorMapBox);

	_framesHBox.pack_start(_colorMapFrame);
}

void ImagePropertiesWindow::initScaleWidgets()
{
	_scaleFrame.add(_scaleBox);
	
	Gtk::RadioButton::Group group;
	
	_scaleBox.pack_start(_minMaxScaleButton);
	_minMaxScaleButton.set_group(group);
	_minMaxScaleButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePropertiesWindow::onScaleChanged));

	_scaleBox.pack_start(_winsorizedScaleButton);
	_winsorizedScaleButton.set_group(group);
	_winsorizedScaleButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePropertiesWindow::onScaleChanged));
	_scaleBox.pack_start(_specifiedScaleButton);
	
	_specifiedScaleButton.set_group(group);
	_specifiedScaleButton.signal_clicked().connect(sigc::mem_fun(*this, &ImagePropertiesWindow::onScaleChanged));
	
	switch(_imageWidget.Range())
	{
		default:
		case ImageWidget::MinMax: _minMaxScaleButton.set_active(true); break;
		case ImageWidget::Winsorized: _winsorizedScaleButton.set_active(true); break;
		case ImageWidget::Specified: _specifiedScaleButton.set_active(true); break;
	}
	onScaleChanged();

	updateMinMaxEntries();

	_scaleBox.pack_start(_scaleMinLabel);
	_scaleBox.pack_start(_scaleMinEntry);
	
	_scaleBox.pack_start(_scaleMaxLabel);
	_scaleBox.pack_start(_scaleMaxEntry);
	
	_framesHBox.pack_start(_scaleFrame);
}

void ImagePropertiesWindow::initOptionsWidgets()
{
	Gtk::RadioButton::Group group;
	
	_optionsBox.pack_start(_normalOptionsButton);
	_normalOptionsButton.set_group(group);

	_optionsBox.pack_start(_logScaleButton);
	_logScaleButton.set_group(group);

	_optionsBox.pack_start(_zeroSymmetricButton);
	_zeroSymmetricButton.set_group(group);
	
	switch(_imageWidget.ScaleOption())
	{
		default:
		case ImageWidget::NormalScale: _normalOptionsButton.set_active(true); break;
		case ImageWidget::LogScale: _logScaleButton.set_active(true); break;
		case ImageWidget::ZeroSymmetricScale: _zeroSymmetricButton.set_active(true); break;
	}

	_optionsFrame.add(_optionsBox);
	
	_framesHBox.pack_start(_optionsFrame);
}

void ImagePropertiesWindow::initZoomWidgets()
{
	_zoomHBox.pack_start(_vStartScale, false, false, 10);
	_vStartScale.set_inverted(true);

	_vStopScale.set_inverted(true);
	_zoomHBox.pack_start(_vStopScale, false, false, 10);

	_zoomVSubBox.pack_start(_hStartScale, false, false, 3);

	_zoomVSubBox.pack_start(_hStopScale, false, false, 3);

	_vStopScale.set_value(1.0);
	_hStopScale.set_value(1.0);
	
	_zoomHBox.pack_start(_zoomVSubBox);

	_zoomFrame.add(_zoomHBox);
	_topVBox.pack_start(_zoomFrame);
}

void ImagePropertiesWindow::updateMinMaxEntries()
{
	std::stringstream minStr;
	minStr << _imageWidget.Min();
	_scaleMinEntry.set_text(minStr.str());
	
	std::stringstream maxStr;
	maxStr << _imageWidget.Max();
	_scaleMaxEntry.set_text(maxStr.str());
}

void ImagePropertiesWindow::onApplyClicked()
{
	if(_grayScaleButton.get_active())
		_imageWidget.SetColorMap(ImageWidget::BWMap);
	else if(_invGrayScaleButton.get_active())
		_imageWidget.SetColorMap(ImageWidget::InvertedMap);
	else if(_colorScaleButton.get_active())
		_imageWidget.SetColorMap(ImageWidget::ColorMap);
	else if(_RedBlueScaleButton.get_active())
		_imageWidget.SetColorMap(ImageWidget::RedBlueMap);
	else if(_hotColdScaleButton.get_active())
		_imageWidget.SetColorMap(ImageWidget::HotColdMap);
	else if(_redBlueYellowScaleButton.get_active())
		_imageWidget.SetColorMap(ImageWidget::RedYellowBlueMap);
	
	if(_minMaxScaleButton.get_active())
		_imageWidget.SetRange(ImageWidget::MinMax);
	else if(_winsorizedScaleButton.get_active())
		_imageWidget.SetRange(ImageWidget::Winsorized);
	else if(_specifiedScaleButton.get_active())
	{
		_imageWidget.SetRange(ImageWidget::Specified);
		_imageWidget.SetMin(atof(_scaleMinEntry.get_text().c_str()));
		_imageWidget.SetMax(atof(_scaleMaxEntry.get_text().c_str()));
	}
	
	if(_normalOptionsButton.get_active())
		_imageWidget.SetScaleOption(ImageWidget::NormalScale);
	else if(_logScaleButton.get_active())
		_imageWidget.SetScaleOption(ImageWidget::LogScale);
	else if(_zeroSymmetricButton.get_active())
		_imageWidget.SetScaleOption(ImageWidget::ZeroSymmetricScale);
	
	double
		timeStart = _hStartScale.get_value(),
		timeEnd = _hStopScale.get_value(),
		freqStart = _vStartScale.get_value(),
		freqEnd = _vStopScale.get_value();
		
	_imageWidget.SetHorizontalDomain(timeStart, timeEnd);
	_imageWidget.SetVerticalDomain(freqStart, freqEnd);
		
	_imageWidget.SetShowAxisDescriptions(_showAxisDescriptionsButton.get_active());
	
	_imageWidget.Update();
	
	updateMinMaxEntries();
}

void ImagePropertiesWindow::onCloseClicked()
{
	hide();
}

void ImagePropertiesWindow::onExportClicked()
{
	if(_imageWidget.HasImage())
	{
		Gtk::FileChooserDialog dialog("Specify image filename", Gtk::FILE_CHOOSER_ACTION_SAVE);
		dialog.set_transient_for(*this);

		//Add response buttons the the dialog:
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button("Save", Gtk::RESPONSE_OK);

		Gtk::FileFilter pdfFilter;
		std::string pdfName = "Portable Document Format (*.pdf)";
		pdfFilter.set_name(pdfName);
		pdfFilter.add_pattern("*.pdf");
		pdfFilter.add_mime_type("application/pdf");
		dialog.add_filter(pdfFilter);

		Gtk::FileFilter svgFilter;
		std::string svgName = "Scalable Vector Graphics (*.svg)";
		svgFilter.set_name(svgName);
		svgFilter.add_pattern("*.svg");
		svgFilter.add_mime_type("image/svg+xml");
		dialog.add_filter(svgFilter);

		Gtk::FileFilter pngFilter;
		std::string pngName = "Portable Network Graphics (*.png)";
		pngFilter.set_name(pngName);
		pngFilter.add_pattern("*.png");
		pngFilter.add_mime_type("image/png");
		dialog.add_filter(pngFilter);

		int result = dialog.run();

		if(result == Gtk::RESPONSE_OK)
		{
			const Gtk::FileFilter *filter = dialog.get_filter();
			if(filter->get_name() == pdfName)
				_imageWidget.SavePdf(dialog.get_filename());
			else if(filter->get_name() == svgName)
				_imageWidget.SaveSvg(dialog.get_filename());
			else
				_imageWidget.SavePng(dialog.get_filename());
		}
	}
}
