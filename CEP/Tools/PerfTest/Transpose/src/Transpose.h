//  Transpose.h: Concrete Simulator class for performance measurements on
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
//  Revision 1.2  2002/05/14 11:39:41  gvd
//  Changed for new build environment
//
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef TRANSPOSE_H
#define TRANSPOSE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/Simulator.h"
class WH_FillTFMatrix;
class WH_Transpose;
class WH_Correlate;
class ParamBlock ;

/**
   The Transpose class implements a Simulator consisting of a set of data
   source steps cross-connected to a set of destination nodes. 
   ....
*/

class Transpose: public Simulator
{
 public:
  Transpose();
  virtual ~Transpose();
  
  // overloaded methods from the Simulator base class
  virtual void define(const ParamBlock& params = ParamBlock());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();
  
  void undefine();
  
 private:
  /// Define pointers to the arrays with steps and workholders.
    WH_FillTFMatrix **Sworkholders;
    WH_Transpose    **Dworkholders;
    WH_Correlate    **Cworkholders;
    Step            **Ssteps;
    Step            **Dsteps;
    Step            **Csteps;
    
    /// Number of source steps
      int itsSourceSteps;
      
      /// Number of destination steps
	int itsDestSteps;
};

#endif


