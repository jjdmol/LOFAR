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

Source::Source (string config_file)
{
  ifstream configfile (config_file.c_str (), ifstream::in);
  string s;
  string filename;
  int i = 0;

  while (!configfile.eof () && configfile.is_open ()) {
    s = "";
    configfile >> s;
    if (s == "length") {
      configfile >> s;
      if (s == ":") {
	configfile >> itsLength;
      }
    } else if (s == "fs") {
      configfile >> s;
      if (s == ":") {
	configfile >> itsSamplingFreq;
      }
    } else if (s == "nsignals") {
      configfile >> s;
      if (s == ":") {
	configfile >> itsNumberOfSignals;
      }
    } else if (s == "SignalFilename") {
      configfile >> s;
      if (s == ":") {
	configfile >> filename;
      }
    } else if (s == "TrajectoryFilename") {
      configfile >> s;
      if (s == ":")
	configfile >> s;
      itsTraject = new Trajectory (s, itsSamplingFreq, itsLength);
    } else if (s == "f_centre") {
      configfile >> s;
      if (s == ":") {
	configfile >> itsCentreFrequency;
      }
    } else if (s.substr (0, 2) == "AM" || s == "FM" || s == "PM") {
      double cf;
      double amp;
      double opt;
      string mod = s;

      configfile >> cf;
      configfile >> amp;
      configfile >> opt;

      itsSignals[i++] = new Signal (filename, mod, cf, amp, opt);
    }
  }
}
