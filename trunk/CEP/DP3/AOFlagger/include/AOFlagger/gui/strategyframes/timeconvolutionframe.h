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
#ifndef TIMECONVOLUTIONFRAME_H
#define TIMECONVOLUTIONFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include <AOFlagger/rfi/strategy/timeconvolutionaction.h>

#include <AOFlagger/gui/editstrategywindow.h>

class TimeConvolutionFrame : public Gtk::Frame {
	public:
		TimeConvolutionFrame(rfiStrategy::TimeConvolutionAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("UV project"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_sincOperationButton("Sinc"),
		_projectedSincOperationButton("Projected sinc"),
		_projectedFourierOperationButton("Projected FT"),
		_extrapolatedSincOperationButton("Extrapolated sinc"),
		_sincSizeLabel("Sinc size: (relative to uv track diameter)"),
		_sincSizeScale(0, 25000, 100),
		_angleLabel("Angle: (degrees)"),
		_angleScale(-180, 180, 1),
		_applyButton(Gtk::Stock::APPLY)
		{
			Gtk::RadioButton::Group group;

			_box.pack_start(_sincOperationButton);
			_sincOperationButton.set_group(group);
			_sincOperationButton.show();
			
			_box.pack_start(_projectedSincOperationButton);
			_projectedSincOperationButton.set_group(group);
			_projectedSincOperationButton.show();

			_box.pack_start(_projectedFourierOperationButton);
			_projectedFourierOperationButton.set_group(group);
			_projectedFourierOperationButton.show();

			_box.pack_start(_extrapolatedSincOperationButton);
			_extrapolatedSincOperationButton.set_group(group);
			_extrapolatedSincOperationButton.show();

			switch(action.Operation())
			{
				case rfiStrategy::TimeConvolutionAction::SincOperation:
					_sincOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::ProjectedSincOperation:
					_projectedSincOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::ProjectedFTOperation:
					_projectedFourierOperationButton.set_active(true);
					break;
				case rfiStrategy::TimeConvolutionAction::ExtrapolatedSincOperation:
					_extrapolatedSincOperationButton.set_active(true);
					break;
			}

			_box.pack_start(_sincSizeLabel);
			_sincSizeLabel.show();

			_box.pack_start(_sincSizeScale);
			_sincSizeScale.set_value(action.SincScale());
			_sincSizeScale.show();
			
			_box.pack_start(_angleLabel);
			_angleLabel.show();

			_box.pack_start(_angleScale);
			_angleScale.set_value(action.DirectionRad()*180.0/M_PI);
			_angleScale.show();
			
			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &TimeConvolutionFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_start(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::TimeConvolutionAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::RadioButton _sincOperationButton, _projectedSincOperationButton, _projectedFourierOperationButton, _extrapolatedSincOperationButton;
		Gtk::Label _sincSizeLabel;
		Gtk::HScale _sincSizeScale;
		Gtk::Label _angleLabel;
		Gtk::HScale _angleScale;
		Gtk::Button _applyButton;

		void onApplyClicked()
		{
			_action.SetDirectionRad((num_t) _angleScale.get_value()/180.0*M_PI);
			_action.SetSincScale(_sincSizeScale.get_value());
			if(_sincOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::SincOperation);
			else if(_projectedSincOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::ProjectedSincOperation);
			else if(_projectedFourierOperationButton.get_active())
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::ProjectedFTOperation);
			else
				_action.SetOperation(rfiStrategy::TimeConvolutionAction::ExtrapolatedSincOperation);
			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // TIMECONVOLUTIONFRAME_H
