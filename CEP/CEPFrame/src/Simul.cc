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
//
//////////////////////////////////////////////////////////////////////

#include "CEPFrame/Simul.h"
#include "CEPFrame/SimulBuilder.h"


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
