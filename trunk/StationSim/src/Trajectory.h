//  Trajectory.h
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
//

#ifndef STATIONSIM_TRAJECTORY_H
#define STATIONSIM_TRAJECTORY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <string>
#include <fstream.h>


class Trajectory
{
public:

   Trajectory (string config_file, int fs, double length);

  // returns source coordinates
  double getPhi (int index);
  double getTheta (int index);

private:

  int          itsFs;
  double       itsLength;
  int          itsNpoints;
  string       itsType;
  LoVec_double itsTheta;
  LoVec_double itsPhi;
  LoVec_double itsTime;
};

#endif
