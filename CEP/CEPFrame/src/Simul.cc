//  Simul.cc:
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
//  Revision 1.20  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.19  2002/05/02 12:20:20  schaaf
//  Added CorbaMonitor object to C'tor
//
//  Revision 1.18  2002/03/26 13:29:14  gvd
//  Moved CorbaController object to SimulRep (for better copy ctor semantics)
//  Use fewer pointers and use switch instead of if
//
//  Revision 1.17  2002/03/26 11:37:33  schaaf
//  Added CorbaController and access to VirtualMachine
//
//  Revision 1.16  2002/01/09 09:56:35  gvd
//  Added getParent to Step (for Robbert)
//
//  Revision 1.15  2001/11/02 14:29:41  gvd
//  Added clone to Step/Simul and use it in SimulRep
//
//  Revision 1.14  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.13  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.12  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.11  2001/09/18 12:07:28  gvd
//  Changed to resolve Step and Simul memory leaks
//  Introduced ref.counted StepRep and SimulRep classes for that purposes
//  Changed several functions to pass by reference instead of pass by pointer
//
//  Revision 1.10  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.9  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.8  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.7  2001/02/05 14:53:04  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/Simul.h"
#include "BaseSim/SimulBuilder.h"


Simul::Simul()
: Step(),
  itsSimul(0)
{}

Simul::Simul (const WorkHolder& worker, 
	      const string& name,
	      bool addNameSuffix,
	      bool controllable,
	      bool monitor)
: Step()
{
  itsSimul = new SimulRep (worker, name, addNameSuffix, controllable, monitor);
  itsRep = itsSimul;
}

Simul::Simul (const WorkHolder* worker, 
	      const string& name,
	      bool addNameSuffix,
	      bool controllable,
	      bool monitor)
: Step()
{
  itsSimul = new SimulRep (*worker, name, addNameSuffix, controllable, monitor);
  itsRep = itsSimul;
}

Simul::Simul (const SimulBuilder& aBuilder, 
	      const string& name,
	      bool addNameSuffix,
	      bool controllable,
	      bool monitor)
: Step()
{
  itsSimul = new SimulRep (aBuilder.getWorker(), name, addNameSuffix,
			   controllable, monitor);
  itsRep = itsSimul;
  aBuilder.buildSimul (*this);
}

Simul::Simul (const Simul& that)
: Step (that)
{
  itsSimul = that.itsSimul;
}

Simul::Simul (SimulRep* rep)
: Step (rep)
{
  itsSimul = rep;
}

Simul& Simul::operator= (const Simul& that)
{
  if (this != &that) {
    Step::operator= (that);
    itsSimul = that.itsSimul;
  }
  return *this;
}

Simul::~Simul() 
{}


Simul* Simul::clone() const
{
  return new Simul (*this);
}


bool Simul::connectInputArray (Simul* aSimul[],
			       int    nrItems,
			       const TransportHolder& prototype)
{
  if (nrItems < 0) {  // set nr_items automatically
    nrItems = getWorker()->getInputs();
  }
  Step** stepPtrs = new Step* [nrItems];
  for (int i=0; i<nrItems; i++) {
    stepPtrs[i] = aSimul[i];
  }
  bool result = itsRep->connectInputArray (stepPtrs, nrItems, prototype);
  delete [] stepPtrs;
  return result;
}
