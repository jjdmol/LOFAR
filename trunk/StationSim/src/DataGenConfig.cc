//  DataGenConfig.cc
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

#include <StationSim/DataGenConfig.h>


DataGenerator::DataGenerator (string config_file)
{
  ifstream configfile (config_file.c_str (), ifstream::in);
  string s;
  int i = 0;

  while (!configfile.eof () && configfile.is_open ()) {
    s = "";
    configfile >> s;
    if (s == "length") {
      configfile >> s;
      if (s == ":") {
		configfile >> itsLength;
	  }
    } else if (s == "nsources") {
      configfile >> s;
      if (s == ":") {
		configfile >> itsNumberOfSources;
	  }
    } else if (s == "SignalFilename") {
      configfile >> s;
      if (s == ":") {
		configfile >> s;
	  }
      itsSources[i++] = new Source (s);
    } else if (s == "ArrayFilename") {
      configfile >> s;
      if (s == ":") {
		configfile >> s;
	  }
      itsArray = new ArrayConfig (s);
    }
  }
}
