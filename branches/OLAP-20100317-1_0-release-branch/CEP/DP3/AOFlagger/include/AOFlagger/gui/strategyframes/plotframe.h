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
#ifndef STRATEGYFRAMES_PLOTFRAME_H
#define STRATEGYFRAMES_PLOTFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>

#include "../../rfi/strategy/plotaction.h"

#include "../editstrategywindow.h"

class StrategyPlotFrame : public Gtk::Frame {
	public:
		StrategyPlotFrame(rfiStrategy::PlotAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("For each baseline"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_plotKindLabel("Plot kind:"),
		_antennaVsFlagsButton("Antenna vs. flags"),
		_frequencyVsFlagsButton("Frequency vs. flags"),
		_frequencyVsPowerButton("Frequency vs. power"),
		_timeVsFlagsButton("Time vs. flgs"),
		_applyButton(Gtk::Stock::APPLY)
		{
			_box.pack_start(_plotKindLabel);
			_plotKindLabel.show();

			Gtk::RadioButton::Group group;

			_box.pack_start(_antennaVsFlagsButton);
			_antennaVsFlagsButton.set_group(group);
			_antennaVsFlagsButton.show();

			_box.pack_start(_frequencyVsFlagsButton);
			_frequencyVsFlagsButton.set_group(group);
			_frequencyVsFlagsButton.show();

			_box.pack_start(_frequencyVsPowerButton);
			_frequencyVsPowerButton.set_group(group);
			_frequencyVsPowerButton.show();

			_box.pack_start(_timeVsFlagsButton);
			_timeVsFlagsButton.set_group(group);
			_timeVsFlagsButton.show();

			switch(_action.PlotKind())
			{
				case rfiStrategy::PlotAction::AntennaFlagCountPlot:
				_antennaVsFlagsButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::FrequencyFlagCountPlot:
				_frequencyVsFlagsButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::FrequencyPowerPlot:
				_frequencyVsPowerButton.set_active(true);
					break;
				case rfiStrategy::PlotAction::TimeFlagCountPlot:
				_timeVsFlagsButton.set_active(true);
					break;
			}

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &StrategyPlotFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::PlotAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Label _plotKindLabel;
		Gtk::RadioButton
			_antennaVsFlagsButton, _frequencyVsFlagsButton, _frequencyVsPowerButton, _timeVsFlagsButton;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			if(_antennaVsFlagsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::AntennaFlagCountPlot);
			else if(_frequencyVsFlagsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::FrequencyFlagCountPlot);
			else if(_frequencyVsPowerButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::FrequencyPowerPlot);
			else if(_timeVsFlagsButton.get_active())
				_action.SetPlotKind(rfiStrategy::PlotAction::TimeFlagCountPlot);
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // STRATEGYFRAMES_PLOTFRAME_H
