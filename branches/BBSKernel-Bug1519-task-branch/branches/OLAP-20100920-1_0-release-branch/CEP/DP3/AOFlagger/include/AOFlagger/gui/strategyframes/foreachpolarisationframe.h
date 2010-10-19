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
#ifndef FOREACHPOLARISATIONFRAME_H
#define FOREACHPOLARISATIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>

#include "../../rfi/strategy/foreachpolarisationblock.h"

#include "../editstrategywindow.h"

class ForEachPolarisationFrame : public Gtk::Frame {
	public:
		ForEachPolarisationFrame(rfiStrategy::ForEachPolarisationBlock &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("For each baseline"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_iterateStokesComponentsButton("Iterate stokes components"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_iterateStokesComponentsButton);
			_iterateStokesComponentsButton.set_active(_action.IterateStokesValues());
			_iterateStokesComponentsButton.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &ForEachPolarisationFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::ForEachPolarisationBlock &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::CheckButton _iterateStokesComponentsButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetIterateStokesValues(_iterateStokesComponentsButton.get_active());
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // FOREACHPOLARISATIONFRAME_H
