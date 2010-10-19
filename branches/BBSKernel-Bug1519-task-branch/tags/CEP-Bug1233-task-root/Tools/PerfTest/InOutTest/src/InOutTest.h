//#  InOutTest.h: Concrete Simulator class for performance measurements on
//#               inDataHolder is outDataHolder
//#
//#  Copyright (C) 2000, 2002
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

#ifndef INOUTTEST_INOUTTEST_H
#define INOUTTEST_INOUTTEST_H


#include <CEPFrame/ApplicationHolder.h>
#include <Blob/KeyValueMap.h>

// define the maximum data block size used in this simulation
#define MAX_GROW_SIZE (256*1024) // 256 kWords =^ 1 MB

/**
   The P2Perf class implements a Simulator consisting of a set of data
   source steps cross-connected to a set of destination nodes. By
   using DH_Growsize and WH_Growsize, this simulator can be used for
   performance measurements on the (cross) connections between the
   steps.
   
*/

class InOutTest: public LOFAR::ApplicationHolder
{
public:
  InOutTest();
  virtual ~InOutTest();

  // overloaded methods from the Simulator base class
  virtual void define(const LOFAR::KeyValueMap& params = LOFAR::KeyValueMap());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

 private:
  /// 0 = variable size packets, > 0 fixed size packets
    int itsFixedSize;
    int itsNPerf;
    bool itsIOshared;
      
};


#endif
