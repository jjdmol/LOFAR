// Trajectory.cc
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

#include <DataGen/Trajectory.h>


Trajectory::Trajectory (string config_file, int fs, double length)
: itsFs     (fs),
  itsLength (length)
{
  ifstream configfile (config_file.c_str (), ifstream::in);
  string s;
  LoVec_double pointPhi;
  LoVec_double pointTheta;
  LoVec_double pointTime;

  while (!configfile.eof () && configfile.is_open ()) {
    s = "";
    configfile >> s;
    if (s == "type") {
      configfile >> s;
      if (s == ":")
	configfile >> itsType;
    } else if (s == "npoints") {
      configfile >> s;
      if (s == ":")
	configfile >> itsNpoints;
    } else if (s == "points") {
      configfile >> s;    
	  if (s == ":") {
		pointPhi.resize (itsNpoints);
		pointTheta.resize (itsNpoints);
		pointTime.resize (itsNpoints);
		for (int i = 0; i < itsNpoints; ++i) {
		  configfile >> pointPhi (i);
		  configfile >> pointTheta (i);
		  configfile >> pointTime (i);
		}
      }
    }
  }

  // File read in now create the phi and theta vectors
  itsPhi.resize ((int)(itsFs / itsLength));
  itsTheta.resize ((int)(itsFs / itsLength));
  itsTime.resize ((int)(itsFs / itsLength));
  
  for (int i = 1; i < itsNpoints; ++i) {
	for (int j = 0; j < (itsTime(i-1) - itsTime(i)) * itsFs * itsLength; ++j) {
	  itsPhi(i) = (pointPhi(i-1) - pointPhi(i)) / (pointTheta(i-1) - pointTheta(i)) * j + pointPhi(i-1);
	  itsTheta(i) = (pointPhi(i-1) - pointPhi(i)) / (pointTheta(i-1) - pointTheta(i)) * j + pointTheta(i-1);
	}
  }
}

double Trajectory::getPhi (int index)
{
  Assert (index < itsFs * itsLength);
  return itsPhi(index);
}

double Trajectory::getTheta (int index)
{
  Assert (index < itsFs * itsLength);
  return itsTheta(index);
}
