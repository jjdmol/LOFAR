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
#include <AOFlagger/gui/zoomwindow.h>

#include <gtkmm/messagedialog.h>

#include <AOFlagger/gui/mswindow.h>

ZoomWindow::ZoomWindow(class MSWindow &msWindow) : Gtk::Window(),
	_hStartScale(0, msWindow.GetOriginalData().ImageWidth(), 1),
	_hStopScale(0, msWindow.GetOriginalData().ImageWidth(), 1),
	_vStartScale(0, msWindow.GetOriginalData().ImageHeight(), 1),
	_vStopScale(0, msWindow.GetOriginalData().ImageHeight(), 1),
	_setButton("Set"),
	_msWindow(msWindow)
{
	_hMainBox.pack_start(_vStartScale, false, false, 10);
	_vStartScale.show();

	_vStopScale.set_value(msWindow.GetOriginalData().ImageHeight()-1);
	_hMainBox.pack_start(_vStopScale, false, false, 10);
	_vStopScale.show();

	_vSubBox.pack_start(_hStartScale, false, false, 3);
	_hStartScale.show();

	_hStopScale.set_value(msWindow.GetOriginalData().ImageWidth()-1);
	_vSubBox.pack_start(_hStopScale, false, false, 3);
	_hStopScale.show();

	_setButton.signal_clicked().connect(sigc::mem_fun(*this, &ZoomWindow::onSetPressed));
	_buttonBox.pack_start(_setButton);
	_setButton.show();

	_vSubBox.pack_start(_buttonBox);
	_buttonBox.show();

	_hMainBox.pack_start(_vSubBox);
	_vSubBox.show();

	add(_hMainBox);
	_hMainBox.show();
}

ZoomWindow::~ZoomWindow()
{
}

void ZoomWindow::onSetPressed()
{
	size_t
		timeStart = (size_t) _hStartScale.get_value(),
		timeEnd = (size_t) _hStopScale.get_value(),
		freqStart = (size_t) _vStartScale.get_value(),
		freqEnd = (size_t) _vStopScale.get_value();
	if(timeStart < timeEnd && freqStart < freqEnd)
	{
		_msWindow.GetTimeFrequencyWidget().SetTimeDomain(timeStart, timeEnd);
		_msWindow.GetTimeFrequencyWidget().SetFrequencyDomain(freqStart, freqEnd);
		_msWindow.GetTimeFrequencyWidget().Update();
	} else {
		Gtk::MessageDialog dialog(*this, "Invalid domain", false, Gtk::MESSAGE_ERROR);
		dialog.run();
	}
}


