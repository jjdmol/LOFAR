//  WH_GrowSize.h: WorkHolder class using DH_Growsize() objects and 
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
//  $Log$
//  Revision 1.7  2002/05/02 12:21:56  schaaf
//  Produce simple monitoring data in getMonitorValue() method
//
//  Revision 1.6  2002/04/18 07:55:03  schaaf
//  Documentation and code update
//
//  Revision 1.5  2002/04/12 15:51:44  schaaf
//  Explicit definition of source and destination side
//
//  Revision 1.4  2001/12/17 16:30:00  schaaf
//  new logic in process() measurements counting
//
//  Revision 1.3  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.2  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.1  2001/08/16 15:14:23  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef WH_GROWSIZE_H
#define WH_GROWSIZE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/WorkHolder.h"
#include "P2Perf/DH_GrowSize.h"
#include "P2Perf/StopWatch.h"

/**
   The WH_Growsize class implements a workholder with DH_Growsize
   objects as inputs and outputs. The process() method does nothing to
   the data (not even copy...) but can contains a performance measurement
   indication for the data transport bandwidth of the output DataHolders. 
 */

class WH_GrowSize: public WorkHolder
{
public:

  WH_GrowSize (const string& name="WH_GrowSize",
	       bool first = false,
	       unsigned int nin=1,      // nr of input channels
	       unsigned int nout=1,     // nr of output channels
	       unsigned int nbuffer=10, // default length of the
	                                // buffer in DH_Growsize::DataPacket 
	       bool destside=false);    // determine whether this is
                                        // the send or recieve side in
                                        // a connection to other
                                        // WH_Growsize objects
  
  virtual ~WH_GrowSize();

  virtual WorkHolder* make(const string& name) const;

  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_GrowSize* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_GrowSize* getOutHolder (int channel);

  /// determine whether performance reporting is wanted
  void setReportPerformance(bool);

  /// Monitoring output
  virtual int getMonitorValue(const char* name);

private:
  /// Forbid copy constructor.
  WH_GrowSize (const WH_GrowSize&);

  /// Forbid assignment.
  WH_GrowSize& operator= (const WH_GrowSize&);


  /// Pointer to the array of input DataHolders.
  DH_GrowSize** itsInHolders;
  /// Pointer to the array of output DataHolders.
  DH_GrowSize** itsOutHolders;

  /// Length of DH_GrowSize buffers.
  int itsBufLength;

  /** indicate destination side WH in order to manage the increasesize 
      call correctly
  */
  bool itsIsDestSide;

  /// Is this the first WorkHolder in the simulation chain?
  bool itsFirst;

  /// Used to do timing on communication
  StopWatch   watch;
  int         itsIteration;
  static int  itsMeasurements;
  static bool itsFirstcall;
  int         itsTime;
  bool        itsReportPerformance;

  /// Monitoring Values
  int itsLastSize;
  int itsLastPerf;
  int itsReportPerf;
};

inline void WH_GrowSize::setReportPerformance(bool doreport){
  TRACER4("Set output for " << getName() << " to " << doreport);
  itsReportPerformance = doreport;
}
#endif
