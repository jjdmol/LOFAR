//  WH_Dump: WorkHolder class for dump of correlator output
//
//  Copyright (C) 2000, 2001
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
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#ifndef WH_DUMP_H
#define WH_DUMP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/WorkHolder.h"
#include "BaseSim/DH_Empty.h"
#include "Pipeline/DH_Correlations.h"

/**
   The WH_Dump class
   Currenly, this is only a placeholder for the DH_Correlation dataholders.
   The Workholder does nothing.
   The TH_File class can be used to write data to file.
 */

class WH_Dump: public WorkHolder
{
public:

  WH_Dump(const string& name,
	  unsigned int nin,      // nr of input channels
	  int stationDim,
	  int pols);
  
  virtual ~WH_Dump();

  virtual WorkHolder* make(const string& name) const;

  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_Correlations* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_Empty* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_Dump (const WH_Dump&);

  /// Forbid assignment.
  WH_Dump& operator= (const WH_Dump&);


  /// Pointer to the array of input DataHolders.
  DH_Correlations** itsInHolders;
  /// Pointer to the array of output DataHolders.
  DH_Empty** itsOutHolders;

  int itsStationDim;
  int itsPols;

  /// Profiling States
  static int          theirProcessProfilerState; 
  
};

#endif
