//# WH_Sync.h: 
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef ONLINEPROTO_WH_SYNC_H
#define ONLINEPROTO_WH_SYNC_H

#include <lofar_config.h>

#include "CEPFrame/WorkHolder.h"
#include "OnLineProto/DH_Beamlet.h"
#include "OnLineProto/DH_CorrectionMatrix.h"
#include <ACC/ParameterSet.h>
#include <OnLineProto/definitions.h>

namespace LOFAR
{
class WH_Sync: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_Sync (const string& name, 
			  const int nbeamlets, 
			  const ParameterSet& ps,
			  const int StationID);

  virtual ~WH_Sync();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, 
				const int nbeamlets, 
				const ParameterSet& ps,
				const int StationID);

  /// Make a fresh copy of the WH object.
  virtual WH_Sync* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_Sync (const WH_Sync&);

  /// Forbid assignment.
  WH_Sync& operator= (const WH_Sync&);

  ParameterSet itsPS;
  int itsStationID;
  int itsCounter;
};

}

#endif
