//  Source.h
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

#ifndef STATIONSIM_SOURCE_H
#define STATIONSIM_SOURCE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/Lorrays.h>
#include <Common/Debug.h>
#include <StationSim/Trajectory.h>
#include <StationSim/Signal.h>
#include <string>
#include <fstream.h>

#define MAX_SIGNALS 100

class Source
{
public:

  Source (string config_file);

  Trajectory* itsTraject;
  double      itsLength;
  long        itsCentreFrequency;
  long        itsSamplingFreq;
  int         itsNumberOfSignals;
  Signal*     itsSignals[MAX_SIGNALS];
};

#endif
