//  P2Perf.h: Concrete Simulator class for performance measurements on
//            a sequence of cross-connected steps
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
//  $Log$
//  Revision 1.10  2002/04/18 07:55:03  schaaf
//  Documentation and code update
//
//  Revision 1.9  2002/04/12 15:52:18  schaaf
//  Updated for multiple source steps and cross connects
//
//  Revision 1.8  2002/03/27 09:48:00  schaaf
//  Use get{Cur/Max}DataPacketSize
//
//  Revision 1.7  2002/03/19 16:19:08  schaaf
//  increased max growsize
//
//  Revision 1.6  2002/03/08 11:38:42  wierenga
//  Upgraded from firewalls.h use to Debug.h use. This version was used for performance tests.
//
//  Revision 1.5  2001/10/31 11:34:18  wierenga
//  LOFAR CVS Repository structure change and transition to autotools (autoconf, automake and libtool).
//
//  Revision 1.4  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.3  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.2  2001/08/16 15:14:22  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef P2PERF_H
#define P2PERF_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/Simulator.h"
#include "P2Perf/WH_GrowSize.h"
#include "BaseSim/ParamBlock.h"

// define the maximum data block size used in this simulation
#define MAX_GROW_SIZE (256*1024) // 256 kWords =^ 1 MB

/**
   The P2Perf class implements a Simulator consisting of a set of data
   source steps cross-connected to a set of destination nodes. By
   using DH_Growsize and WH_Growsize, this simulator can be used for
   performance measurements on the (cross) connections between the
   steps.
   
*/

class P2Perf: public Simulator
{
public:
  P2Perf();
  virtual ~P2Perf();

  // overloaded methods from the Simulator base class
  virtual void define(const ParamBlock& params = ParamBlock());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

 private:
  /// Define pointers to the arrays with steps and workholders.
  WH_GrowSize **Sworkholders;
  WH_GrowSize **Dworkholders;
  Step        **Ssteps;
  Step        **Dsteps;

  /// Number of source steps
    int itsSourceSteps;

  /// Number of destination steps
    int itsDestSteps;
};


#endif
