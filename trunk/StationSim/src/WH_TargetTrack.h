//#  WH_TargetTrack.h:
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

#ifndef STATIONSIM_WH_TARGETTRACK_H
#define STATIONSIM_WH_TARGETTRACK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_TargetTrack.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>

/**
   This workholder reads the target tracks from a file and sends them
   to the beamformer.
   Later another workholder can be used which calculates the  target
   tracks in real time.
*/

class WH_TargetTrack: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  WH_TargetTrack (const string& name, unsigned int MaxNtrack,
		  const string& fileName);

  virtual ~WH_TargetTrack();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_TargetTrack* make (const string& name) const;

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// There are no input DataHolders.
  virtual DataHolder* getInHolder (int channel);

  /// Get a pointer to the output DataHolder.
  virtual DH_TargetTrack* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_TargetTrack (const WH_TargetTrack&);

  /// Forbid assignment.
  WH_TargetTrack& operator= (const WH_TargetTrack&);


  DH_TargetTrack itsOutHolder;

  /// Length of buffers.
  int      itsMaxNtrack;
  string   itsFileName;
  ifstream itsFile;
  int      itsLineNr;
};


#endif
