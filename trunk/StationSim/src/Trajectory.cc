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


Trajectory::Trajectory (string config_file)
{
  conf_file = config_file;
  ifstream configfile (config_file.c_str (), ifstream::in);
  int x; 
  string s;
  int y;
  
  AssertStr (configfile.is_open (), "Couldn't open trajectory file!");
 
  configfile >> x;
  configfile >> s;
  configfile >> y;
  
  configfile.seekg (0); // place file pointer back to the begining of the file
  itsPhiAndTheta.resize (x, y);
  
  configfile >> itsPhiAndTheta;
}

Trajectory::~Trajectory ()
{
}

double Trajectory::getPhi (int index)
{
  AssertStr (index < itsPhiAndTheta.rows (), "Trajectory size too small!");
  return itsPhiAndTheta (index, 0);
}

double Trajectory::getTheta (int index)
{
  AssertStr (index < itsPhiAndTheta.rows (), "Trajectory size too small!");
  return itsPhiAndTheta (index, 1);
}
