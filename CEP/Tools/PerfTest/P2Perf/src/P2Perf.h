//  SeqSim.h: Concrete Simulator class for a sequence of steps
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

#ifndef RINGSIM_H
#define RINGSIM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Simulator.h"
#include "WH_GrowSize.h"
#include "ParamBlock.h"

#define MAX_GROW_SIZE (4*1024*1024)

/**
   This class is an example of a concrete Simulator.
*/

class SeqSim: public Simulator
{
public:
  SeqSim();
  virtual ~SeqSim();

  virtual void define(const ParamBlock& params = ParamBlock());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

 private:
  WH_GrowSize **workholders;
  Step        **steps;
};


#endif
