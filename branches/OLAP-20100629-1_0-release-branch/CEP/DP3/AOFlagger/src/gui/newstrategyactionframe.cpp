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
#include <gtkmm/stock.h>
#include <gtkmm/enums.h>

#include <AOFlagger/gui/editstrategywindow.h>
#include <AOFlagger/gui/newstrategyactionframe.h>

#include <AOFlagger/rfi/strategy/actionfactory.h>

NewStrategyActionFrame::NewStrategyActionFrame(EditStrategyWindow &editStrategyWindow)
	: Gtk::Frame("New action"),
	_editStrategyWindow(editStrategyWindow)
{
	std::vector<std::string> actions = rfiStrategy::ActionFactory::GetActionList();

	for(std::vector<std::string>::const_iterator i=actions.begin();i!=actions.end();++i)
	{
		Gtk::Button *button = new Gtk::Button(*i);
		_buttonBox.pack_start(*button);
		button->signal_clicked().connect(sigc::bind<const std::string>(sigc::mem_fun(*this, &NewStrategyActionFrame::onButtonClicked), *i));
		button->show();

		_buttons.push_back(button);
	}

	add(_buttonBox);
	_buttonBox.show();
}

NewStrategyActionFrame::~NewStrategyActionFrame()
{
	for(std::vector<Gtk::Button *>::const_iterator i=_buttons.begin();i!=_buttons.end();++i)
		delete *i;
}

void NewStrategyActionFrame::onButtonClicked(const std::string str)
{
 	_editStrategyWindow.AddAction(rfiStrategy::ActionFactory::CreateAction(str));
}
