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
#ifndef FRINGESTOPPINGFRAME_H
#define FRINGESTOPPINGFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../rfi/strategy/fringestopaction.h"

#include "../editstrategywindow.h"

class FringeStoppingFrame : public Gtk::Frame {
	public:
		FringeStoppingFrame(rfiStrategy::FringeStopAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Fringe stopping recovery"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_fringesToConsiderLabel("Considered fringes:"),
		_fringesToConsiderScale(0, 25.0L, 0.25L),
		_windowSizeLabel("Window size:"),
		_windowSizeScale(0, 512, 1),
		_fitChannelsIndividuallyButton("Fit channels individually"),
		_onlyFringeStopButton("No fit, only fringe stop"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_fringesToConsiderLabel);
			_fringesToConsiderLabel.show();

			_box.pack_start(_fringesToConsiderScale);
			_fringesToConsiderScale.set_value(_action.FringesToConsider());
			_fringesToConsiderScale.show();

			_box.pack_start(_windowSizeLabel);
			_windowSizeLabel.show();

			_box.pack_start(_windowSizeScale);
			_windowSizeScale.set_value(_action.WindowSize());
			_windowSizeScale.show();

			_box.pack_start(_fitChannelsIndividuallyButton);
			_fitChannelsIndividuallyButton.set_active(_action.FitChannelsIndividually());
			_fitChannelsIndividuallyButton.show();

			_box.pack_start(_onlyFringeStopButton);
			_onlyFringeStopButton.set_active(_action.OnlyFringeStop());
			_onlyFringeStopButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &FringeStoppingFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::FringeStopAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _fringesToConsiderLabel;
		Gtk::HScale _fringesToConsiderScale;
		Gtk::Label _windowSizeLabel;
		Gtk::HScale _windowSizeScale;
		Gtk::CheckButton _fitChannelsIndividuallyButton;
		Gtk::CheckButton _onlyFringeStopButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetFringesToConsider(_fringesToConsiderScale.get_value());
			_action.SetWindowSize((size_t) _windowSizeScale.get_value());
			_action.SetFitChannelsIndividually(_fitChannelsIndividuallyButton.get_active());
			_action.SetOnlyFringeStop(_onlyFringeStopButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // FRINGESTOPPINGFRAME_H
