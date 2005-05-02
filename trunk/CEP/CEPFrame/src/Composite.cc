//#  Composite.cc:
//#
//#  Copyright (C) 2000, 2001
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CEPFrame/Composite.h>
#include <CEPFrame/NetworkBuilder.h>

namespace LOFAR
{

Composite::Composite()
: Step(),
  itsComposite(0)
{}

Composite::Composite (WorkHolder& worker, 
	      const string& name,
	      bool addNameSuffix,
	      bool controllable,
	      bool monitor)
: Step()
{
  itsComposite = new CompositeRep (worker, name, addNameSuffix, controllable, monitor);
  itsRep = itsComposite;
}

Composite::Composite (WorkHolder* worker, 
	      const string& name,
	      bool addNameSuffix,
	      bool controllable,
	      bool monitor)
: Step()
{
  itsComposite = new CompositeRep (*worker, name, addNameSuffix, controllable, monitor);
  itsRep = itsComposite;
}

Composite::Composite (NetworkBuilder& aBuilder, 
	      const string& name,
	      bool addNameSuffix,
	      bool controllable,
	      bool monitor)
: Step()
{
  itsComposite = new CompositeRep (aBuilder.getWorker(), name, addNameSuffix,
			   controllable, monitor);
  itsRep = itsComposite;
  aBuilder.buildNetwork (*this);
}

Composite::Composite (const Composite& that)
: Step (that)
{
  itsComposite = that.itsComposite;
}

Composite::Composite (CompositeRep* rep)
: Step (rep)
{
  itsComposite = rep;
}

Composite& Composite::operator= (const Composite& that)
{
  if (this != &that) {
    Step::operator= (that);
    itsComposite = that.itsComposite;
  }
  return *this;
}

Composite::~Composite() 
{}


Composite* Composite::clone() const
{
  return new Composite (*this);
}


bool Composite::connectInputArray (Composite* aComposite[],
				   int    nrItems,
				   const TransportHolder& prototype,
				   bool blockingComm)
{
  if (nrItems < 0) {  // set nr_items automatically
    nrItems = getWorker()->getDataManager().getInputs();
  }
  Step** stepPtrs = new Step* [nrItems];
  for (int i=0; i<nrItems; i++) {
    stepPtrs[i] = aComposite[i];
  }
  bool result = itsRep->connectInputArray (stepPtrs, nrItems, prototype, blockingComm);
  delete [] stepPtrs;
  return result;
}

}
