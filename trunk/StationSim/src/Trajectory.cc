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
		itsPhi.resize (itsNpoints);
		itsTheta.resize (itsNpoints);
		itsTime.resize (itsNpoints);
		for (int i = 0; i < itsNpoints; ++i) {
		  configfile >> itsPhi (i);
		  configfile >> itsTheta (i);
		  configfile >> itsTime (i);
		}
      }
    }
  }

  //DEBUG
  itsFileOut.open("/home/alex/trajectory.txt");
}

Trajectory::~Trajectory ()
{
  itsFileOut.close ();
}

double Trajectory::getPhi (int index)
{
  Assert (index < itsFs * itsLength);
  if (itsType == "steady") 
	{
	  return itsPhi (0);
	} 
  else if (itsType == "variable") 
	{ 
	  int i;
	  // Get the good time points
	  for (i = 0; itsTime(i) * itsFs * itsLength <= index; ++i)
		;

	  int dY = (int) ((itsTime(i) - itsTime(i-1)) * itsFs * itsLength);
	  double dX =  itsPhi(i) - itsPhi(i-1);
	  
	  // DEBUG
	  itsFileOut << itsPhi(i-1) + (dX / dY) * (index - (itsTime(i-1) * itsFs * itsLength)) 
				 << "\t";

	  return itsPhi(i-1) + (dX / dY) * (index - (itsTime(i-1) * itsFs * itsLength));
	} 
  else if (itsType == "cyclic") 
	{
	  return itsPhi (index); //DEBUG
	}
  else return 0;
}

double Trajectory::getTheta (int index)
{
  Assert (index < itsFs * itsLength);
  if (itsType == "steady") 
	{
	  return itsTheta (0);
	} 
  else if (itsType == "variable") 
	{
	  int i;
	  // Get the good time points
	  for (i = 0; itsTime(i) * itsFs * itsLength <= index; ++i)
		;

	  int dY = (int) ((itsTime(i) - itsTime(i-1)) * itsFs * itsLength);
	  double dX = itsTheta(i) - itsTheta(i-1);
	  
	  // DEBUG
	  itsFileOut << itsTheta(i-1) + (dX / dY) * (index - (itsTime(i-1) * itsFs * itsLength)) 
				 << "\t" << index << endl;

	  return itsTheta(i-1) + (dX / dY) * (index - (itsTime(i-1) * itsFs * itsLength));
	} 
  else if (itsType == "cyclic") 
	{
	  return itsTheta (index); //DEBUG
	}
  else return 0;
}
