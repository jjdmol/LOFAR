//  WH_Src.h: WorkHolder class using DH_VarSize() objects and 
//                 measuring performance
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
//
//////////////////////////////////////////////////////////////////////

#ifndef WH_Src_H
#define WH_Src_H

#ifdef HAVE_CONFIG_H
#include <lofar_config.h>
#endif

#include "tinyCEP/WorkHolder.h"
#include "P2Perf/DH_VarSize.h"
#include "P2Perf/DHGrowStrategy.h"
#include "P2Perf/StopWatch.h"

/**
   The WH_Src class implements a workholder with DH_VarSize
   objects as outputs. The process() method does nothing to
   the data (not even copy...) but can contain a performance measurement
   indication for the data transport bandwidth of the output DataHolders. 
 */

class WH_Src: public LOFAR::WorkHolder
{
public:

  WH_Src (DHGrowStrategy* DHGS, // the object that will grow the DH's
               const string& name="WH_Src",
	       unsigned int nout=1,     // nr of output channels
               unsigned int size = 100,    // size of the packet in bytes
               unsigned int measurementsPerGrowStep = 10,
               unsigned int packetsPerMeasurement = 10);
  
  virtual ~WH_Src();

  virtual WorkHolder* make(const string& name);

  /// Do a process step.
  virtual void process();

  /// determine whether performance reporting is wanted
  void setReportPerformance(bool);
  
private:
  /// Forbid copy constructor.
  WH_Src (const WH_Src&);

  /// Forbid assignment.
  WH_Src& operator= (const WH_Src&);

  /// Fixed size
  unsigned int itsSize;

  /// Used to do timing on communication
  StopWatch *watch;
  static bool itsFirstcall;
  bool itsReportPerformance;
  DHGrowStrategy * itsDHGrowStrategy;
  unsigned int itsPacketsPerMeasurement;
  unsigned int itsMeasurementsPerGrowStep;
  unsigned int itsPacketNumber;
  unsigned int itsMeasurementNumber;
};

inline void WH_Src::setReportPerformance(bool doreport){
  itsReportPerformance = doreport;
}
#endif
