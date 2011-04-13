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
#ifndef FREQUENCYCONVOLUTIONFRAME_H
#define FREQUENCYCONVOLUTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include <AOFlagger/strategy/actions/frequencyconvolutionaction.h>

#include <AOFlagger/gui/editstrategywindow.h>

class FrequencyConvolutionFrame : public Gtk::Frame {
	public:
		FrequencyConvolutionFrame(rfiStrategy::FrequencyConvolutionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Frequency convolution"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_sizeXLabel("Size x:"),
		_sizeYLabel("Size y"),
		_sizeXScale(1, 1024, 1),
		_sizeYScale(1, 1024, 1),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_sizeXLabel);

			_box.pack_start(_sizeXScale);
			_sizeXScale.set_value(_action.SizeX());

			_box.pack_start(_sizeYLabel);

			_box.pack_start(_sizeYScale);
			_sizeYScale.set_value(_action.SizeY());

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &FrequencyConvolutionFrame::onApplyClicked));

			_box.pack_start(_buttonBox);

			add(_box);
			_box.show_all();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::FrequencyConvolutionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _sizeXLabel, _sizeYLabel;
		Gtk::HScale _sizeXScale, _sizeYScale;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetSizeX((unsigned) _sizeXScale.get_value());
			_action.SetSizeY((unsigned)_sizeYScale.get_value());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // FREQUENCYCONVOLUTIONFRAME_H
