//# WH_DFTServer.h: WorkHolder for the DFTServer 
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

#ifndef DFTSERVER_WH_DFTSERVER_H
#define DFTSERVER_WH_DFTSERVER_H

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>
#include <PSS3/DH_DFTRequest.h>
#include <PSS3/DH_DFTResult.h>

namespace LOFAR
{

/**
   This is the work holder for the DFTServer program.
*/

class WH_DFTServer: public WorkHolder
{
public:
  /// Construct the work holder.
  explicit WH_DFTServer (const string& name = "WH_DFTServer");

  virtual ~WH_DFTServer();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const KeyValueMap&);

   /// Make a fresh copy of the WH object.
  virtual WH_DFTServer* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_DFTServer (const WH_DFTServer&);

  /// Forbid assignment.
  WH_DFTServer& operator= (const WH_DFTServer&);

};

}

#endif
