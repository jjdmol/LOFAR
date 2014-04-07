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
#ifndef GUI_QUALITY__DATA_WINDOW_H
#define GUI_QUALITY__DATA_WINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class DataWindow : public Gtk::Window {
	public:
		DataWindow()
		{
			_scrolledWindow.add(_textView);
			_textView.show();
			
			add(_scrolledWindow);
			_scrolledWindow.show();
			set_default_size(200, 200);
		}
    ~DataWindow()
    {
		}
		void SetData(const std::string &data)
		{
			_textView.get_buffer()->set_text(data);
		}
	private:
		Gtk::ScrolledWindow _scrolledWindow;
		Gtk::TextView _textView;
};

#endif
