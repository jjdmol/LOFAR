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

#include <StationSim/Trajectory.h>


Trajectory::Trajectory (string config_file, int fs, double length)
: itsFs     (fs),
  itsLength (length)
{
  ifstream configfile (config_file.c_str (), ifstream::in);
  string s;
  LoVec_double pointPhi;
  LoVec_double pointTheta;
  LoVec_double pointTime;

  // DEBUG
  itsFileOut.open ("/home/alex/interpolation.txt");

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

  // DEBUG
  itsFileOut << "Length : " << itsFs * itsLength << endl;
  itsFileOut << "Type   : " << itsType << endl << endl;
  itsFileOut << "Time\t" << "Phi\t" << "Theta\t" << endl;

  // File read in now create the phi and theta vectors
  
  if (itsType == "steady") {
	itsPhi.resize (1);
	itsTheta.resize (1);
	itsPhi (0) = pointPhi (0);
	itsTheta (0) = pointTheta (0);
  } else if (itsType == "variable") {
	  itsPhi.resize ((int)(itsFs * itsLength));
	  itsTheta.resize ((int)(itsFs * itsLength));
	   
	  for (int i = 1; i < itsNpoints; ++i) {
		int b = (int) (pointTime(i-1) * itsFs * itsLength);
		int e = (int) ((pointTime(i) - pointTime(i-1)) * itsFs * itsLength);
		for (int j = 0; j < e; ++j) {
		  itsPhi (b+j) = (pointPhi(i) - pointPhi(i-1)) / e * j + pointPhi(i-1);
		  itsTheta (b+j) = (pointTheta(i) - pointTheta(i-1)) / e * j + pointTheta(i-1);
		  
		  // DEBUG
		  itsFileOut << b+j << "\t" << itsPhi(b+j) << "\t" << itsTheta(b+j) << "\t" << endl;
		}
	  }
  } else if (itsType == "cyclic") {
	// do cyclic stuff
  } else {
	// ERROR
  }
}

Trajectory::~Trajectory ()
{
  itsFileOut.close ();
}

double Trajectory::getPhi (int index)
{
  Assert (index < itsFs * itsLength);
  if (itsType == "steady") {
	return itsPhi (0);
  } else if (itsType == "variable") {
	return itsPhi (index);
  } else if (itsType == "cyclic") {
	return itsPhi (index); //DEBUG
  }
}

double Trajectory::getTheta (int index)
{
  Assert (index < itsFs * itsLength);
  if (itsType == "steady") {
	return itsTheta (0);
  } else if (itsType == "variable") {
	return itsTheta (index);
  } else if (itsType == "cyclic") {
	return itsPhi (index); //DEBUG
  }
}
