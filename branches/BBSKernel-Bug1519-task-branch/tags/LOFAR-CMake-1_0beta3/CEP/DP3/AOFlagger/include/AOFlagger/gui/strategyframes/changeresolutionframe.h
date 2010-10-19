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
#ifndef CHANGERESOLUTIONFRAME_H
#define CHANGERESOLUTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../rfi/strategy/changeresolutionaction.h"

#include "../editstrategywindow.h"

class ChangeResolutionFrame : public Gtk::Frame {
	public:
		ChangeResolutionFrame(rfiStrategy::ChangeResolutionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Change resolution"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_decreaseFactorLabel("Decrease factor:"),
		_decreaseFactorScale(0, 128, 2),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_decreaseFactorLabel);
			_decreaseFactorLabel.show();

			_box.pack_start(_decreaseFactorScale);
			_decreaseFactorScale.set_value(_action.DecreaseFactor());
			_decreaseFactorScale.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &ChangeResolutionFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::ChangeResolutionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _decreaseFactorLabel;
		Gtk::HScale _decreaseFactorScale;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetDecreaseFactor((size_t) _decreaseFactorScale.get_value());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // CHANGERESOLUTIONFRAME_H
