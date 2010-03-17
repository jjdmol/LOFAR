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
#ifndef LOADIMAGEFRAME_H
#define LOADIMAGEFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

#include "../../rfi/strategy/loadimageaction.h"

#include "../editstrategywindow.h"

class LoadImageFrame : public Gtk::Frame {
	public:
		LoadImageFrame(rfiStrategy::LoadImageAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("Load image"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_applyButton(Gtk::Stock::APPLY),
		_dataKindFrame("Data kind"),
		_observedDataButton("Observed"), _correctedDataButton("Corrected"), _modelDataButton("Model"), _residualDataButton("Residual"), _weightsButton("Weights"),
		_polarisationFrame("Polarisation"),
		_allDipolePolarisationButton("Dipole (xx,xy,yx,yy separately)"),
		_autoDipolePolarisationButton("Dipole auto-correlations (xx and yy)"),
		_stokesIPolarisationButton("Stokes I")
		{
			Gtk::RadioButton::Group groupA = _observedDataButton.get_group();
			_correctedDataButton.set_group(groupA);
			_modelDataButton.set_group(groupA);
			_residualDataButton.set_group(groupA);
			_weightsButton.set_group(groupA);

			_dataKindBox.pack_start(_observedDataButton);
			_observedDataButton.show();

			_dataKindBox.pack_start(_correctedDataButton);
			_correctedDataButton.show();

			_dataKindBox.pack_start(_modelDataButton);
			_modelDataButton.show();

			_dataKindBox.pack_start(_residualDataButton);
			_residualDataButton.show();

			_dataKindBox.pack_start(_weightsButton);
			_weightsButton.show();

			_dataKindFrame.add(_dataKindBox);
			_dataKindBox.show();

			_box.pack_start(_dataKindFrame);
			_dataKindFrame.show();

			switch(_action.ImageKind())
			{
				case TimeFrequencyImager::Observed:
					_observedDataButton.set_active(true); break;
				case TimeFrequencyImager::Corrected:
					_correctedDataButton.set_active(true); break;
				case TimeFrequencyImager::Model:
					_modelDataButton.set_active(true); break;
				case TimeFrequencyImager::Residual:
					_residualDataButton.set_active(true); break;
				case TimeFrequencyImager::Weight:
					_weightsButton.set_active(true); break;
			}

			Gtk::RadioButton::Group group = _allDipolePolarisationButton.get_group();
			_autoDipolePolarisationButton.set_group(group);
			_stokesIPolarisationButton.set_group(group);

			_polarisationBox.pack_start(_allDipolePolarisationButton);
			_allDipolePolarisationButton.show();

			_polarisationBox.pack_start(_autoDipolePolarisationButton);
			_autoDipolePolarisationButton.show();

			_polarisationBox.pack_start(_stokesIPolarisationButton);
			_stokesIPolarisationButton.show();

			if(_action.ReadAllPolarisations())
				_allDipolePolarisationButton.set_active(true);
			else if(_action.ReadDipoleAutoPolarisations())
				_autoDipolePolarisationButton.set_active(true);
			else if(_action.ReadStokesI())
				_stokesIPolarisationButton.set_active(true);

			_polarisationFrame.add(_polarisationBox);
			_polarisationBox.show();

			_box.pack_start(_polarisationFrame);
			_polarisationFrame.show();

			_buttonBox.pack_start(_applyButton);
			_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &LoadImageFrame::onApplyClicked));
			_applyButton.show();

			_box.pack_end(_buttonBox);
			_buttonBox.show();

			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::LoadImageAction &_action;

		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Button _applyButton;

		Gtk::Frame _dataKindFrame;
		Gtk::VBox _dataKindBox;
		Gtk::RadioButton _observedDataButton, _correctedDataButton, _modelDataButton, _residualDataButton, _weightsButton;

		Gtk::Frame _polarisationFrame;
		Gtk::VBox _polarisationBox;
		Gtk::RadioButton _allDipolePolarisationButton, _autoDipolePolarisationButton, _stokesIPolarisationButton;

		void onApplyClicked()
		{
			if(_allDipolePolarisationButton.get_active())
				_action.SetReadAllPolarisations();
			else if(_autoDipolePolarisationButton.get_active())
				_action.SetReadDipoleAutoPolarisations();
			else if(_stokesIPolarisationButton.get_active())
				_action.SetReadStokesI();

			if(_observedDataButton.get_active())
				_action.SetImageKind(TimeFrequencyImager::Observed);
			else if(_correctedDataButton.get_active())
				_action.SetImageKind(TimeFrequencyImager::Corrected);
			else if(_modelDataButton.get_active())
				_action.SetImageKind(TimeFrequencyImager::Model);
			else if(_residualDataButton.get_active())
				_action.SetImageKind(TimeFrequencyImager::Residual);
			else if(_weightsButton.get_active())
				_action.SetImageKind(TimeFrequencyImager::Weight);

			_editStrategyWindow.UpdateAction(&_action);
		}
};

#endif // LOADIMAGEFRAME_H
