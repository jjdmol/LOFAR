//  WH_GrowSize.h: This is an example of a WorkHolder class.
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

#include "WorkHolder.h"
#include "DH_GrowSize.h"
#include "StopWatch.h"


/**
   This is an example of a WorkHolder class.
   It has one input and one output DH_IntArray object as DataHolders.

   It shows which functions have to be implemented
*/

class WH_GrowSize: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The argument first tells if this is the first WorkHolder in
  /// the simulation chain.
  WH_GrowSize (const string& name="WH_GrowSize",
	      bool first = false,
	      unsigned int nin=1, unsigned int nout=1,
	      unsigned int nbuffer=10);

  virtual ~WH_GrowSize();

  virtual WorkHolder* make(const string& name) const;

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_GrowSize* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_GrowSize* getOutHolder (int channel);

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
  /// Is this the first WorkHolder in the simulation chain?
  bool itsFirst;

  /// Used to do timing on communication
  StopWatch watch;
  int         itsIteration;
  static int  itsMeasurements;
  static bool itsFirstcall;
  int         itsTime;
};


#endif
