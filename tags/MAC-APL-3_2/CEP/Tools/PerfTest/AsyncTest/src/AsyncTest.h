//  AsyncTest.h: Concrete ApplicationHolder class for performance measurements
//               on a number of steps with buffered (asynchronous) transport
//
//  Copyright (C) 2000, 2002
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
//////////////////////////////////////////////////////////////////////////

#ifndef LOFAR_ASYNCTEST_ASYNCTEST_H
#define LOFAR_ASYNCTEST_ASYNCTEST_H

#include <CEPFrame/ApplicationHolder.h>
#include <Common/KeyValueMap.h>
#include <AsyncTest/WH_Source.h>
#include <AsyncTest/WH_Sink.h>

namespace LOFAR
{

// define the maximum data block size used in this simulation
#define MAX_GROW_SIZE (256*1024) // 256 kWords =^ 1 MB

class Step;

/**
   The AsyncTest class implements a ApplicationHolder consisting of 
   a set of data source steps cross-connected to a set of destination 
   nodes.    
*/

class AsyncTest: public LOFAR::ApplicationHolder
{
public:
  AsyncTest();
  virtual ~AsyncTest();

  // overloaded methods from the Simulator base class
  virtual void define(const LOFAR::KeyValueMap& params = LOFAR::KeyValueMap());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

 private:
  /// Define pointers to the arrays with steps and workholders.
  WH_Source **Sworkholders;
  WH_Sink **Dworkholders;
  Step      **Ssteps;
  Step      **Dsteps;

  /// Number of source steps
    int itsSourceSteps;

  /// Number of destination steps
    int itsDestSteps;

  /// 0 = variable size packets, > 0 fixed size packets
    int itsFixedSize;

  /// 1 = synchronous, 0 = asynchronous
    bool itsSyncRW;
      
};

} // end namespace LOFAR

#endif
