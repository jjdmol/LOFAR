//  AH_3BlockPerf.h: Concrete Simulator class for performance measurements on a
//            simple source-heat-dest line. 
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

#ifndef AH_3BLOCKPERF_H
#define AH_3BLOCKPERF_H

#include <lofar_config.h>

#include <CEPFrame/ApplicationHolder.h>
#include <Common/KeyValueMap.h>

namespace LOFAR {

/**
   The AH_3BlockPerf class implements a Simulator consisting of a data
   source step connected to a heat step and a destination stap.
   By changing the size of the dataholder and the amount of flops per byte
   in the WH_Heat, we can do measurement of the bandwidth as a function
   of these two variables.   
*/

class AH_3BlockPerf: public LOFAR::ApplicationHolder
{
public:
  AH_3BlockPerf();
  virtual ~AH_3BlockPerf();

  // overloaded methods from the Simulator base class
  virtual void define(const LOFAR::KeyValueMap& params = LOFAR::KeyValueMap());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

 private:
  /// Define pointers to the arrays with steps and workholders.
  Step* itsSrcStep;
  Step* itsHeatStep;
  Step* itsDstStep;
};

}
#endif
