//  Source.cc
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//


#include <StationSim/Source.h>
#include <algorithm>


Source::Source (string config_file, string trajectory_file)
{
  ifstream configfile (config_file.c_str (), ifstream::in);
  string path = config_file.substr (0, config_file.rfind ('/') + 1);
  string s;
  string filename;
  int i = 0;
  itsTraject = new Trajectory (trajectory_file);

  AssertStr (configfile.is_open (), "Couldn't open source file!");

  while (!configfile.eof ()) {
    s = "";
    configfile >> s;
	transform (s.begin (), s.end (), s.begin (), tolower);
    if (s == "nsignals") {
      configfile >> s;
      if (s == ":") {
		configfile >> itsNumberOfSignals;
      }
    } else if (s == "signalfilename") {
      configfile >> s;
      if (s == ":") {
		configfile >> filename;
      }
    } else if ((s.substr (0, 2) == "am" || s == "fm" || s == "pm") && s != "amplitude") {
      double cf;
      double amp;
      double opt;
      string mod = s;
	  transform (mod.begin (), mod.end (), mod.begin (), tolower);

      configfile >> cf;
	  configfile >> amp;
	  configfile >> opt;
	  
      itsSignals[i++] = new Signal (path + filename, mod, cf, amp, opt);
    }
  }
}
