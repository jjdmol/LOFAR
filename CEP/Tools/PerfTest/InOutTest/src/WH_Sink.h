//  WH_Sink.h: WorkHolder class measuring performance
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

#ifndef LOFAR_INOUTTEST_WH_SINK_H
#define LOFAR_INOUTTEST_WH_SINK_H

#include <tinyCEP/WorkHolder.h>
#include <InOutTest/StopWatch.h>

namespace LOFAR
{

/**
   The WH_Growsize class implements a workholder with DH_Growsize
   objects as inputs and outputs. The process() method does nothing to
   the data (not even copy...) but can contains a performance measurement
   indication for the data transport bandwidth of the output DataHolders. 
 */

class WH_Sink: public LOFAR::WorkHolder
{
public:

  WH_Sink (const string& name="WH_Sink",
	       bool first = false,
	       unsigned int nbuffer=10, // default length of the buffer
	       unsigned int niterations=1);   // number of iterations

  virtual ~WH_Sink();

  virtual WorkHolder* make(const string& name);

  //virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_Sink (const WH_Sink&);

  /// Forbid assignment.
  WH_Sink& operator= (const WH_Sink&);

  /// Length of DH_Sink buffers.
  int itsBufLength;

  /// Is this the first WorkHolder in the simulation chain?
  bool itsFirst;

  /// Used to do timing on communication
  StopWatch   watch;
  int         itsIteration;
  int         itsNIterations;
  static int  itsMeasurements;
  static bool itsFirstcall;
  int         itsTime;


  /// Monitoring Values
  int itsLastSize;
  int itsLastPerf;

};

} // end namespace LOFAR

#endif
