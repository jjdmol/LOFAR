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
  string path = config_file.substr (0, config_file.rfind ('/') + 1);

  AssertStr (configfile.is_open (), "Couldn't open datagenerator file!");

  while (!configfile.eof ()) {	  
	s = "";
	configfile >> s;
	if (s == "nsources") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> itsNumberOfSources;
	  }
	} else if (s == "nfft") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> itsNumberOfFFT;
	  }
	} else if (s == "fs") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> itsSamplingFreq;
	  }
	} else if (s == "time") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> itsTimeLength;
	  }
	} else if (s == "nant") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> itsNumberOfAntennas;
	  }
	} else if (s == "snaptogrid") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> itsSnapToGrid;
	  }
	} else if (s == "nullgrid") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> itsNullGrid;
	  }
	} else if (s == "arrayfile") {
	  configfile >> s;
	  if (s == ":") {
		configfile >> s;
	  }
	  itsArray = new ArrayConfig (path + s);
	} else if (s == "Source") {
	  string t;
	  configfile >> s;
	  if (s == ":") {
		configfile >> s;
		configfile >> t;
	  }		  
	  itsSources[i++] = new Source (path + s, path + t);
	}
  }
}
