//#  Step.cc:
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

#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>

namespace LOFAR
{

Step::Step (WorkHolder& worker, 
	    const string& aName,
	    bool addNameSuffix,
	    bool monitor)
: itsRep(0)
{
  itsRep = new StepRep (worker, aName, addNameSuffix, monitor);
}

Step::Step (WorkHolder* worker, 
	    const string& aName,
	    bool addNameSuffix,
	    bool monitor)
: itsRep(0)
{
  itsRep = new StepRep (*worker, aName, addNameSuffix, monitor);
}

Step::Step (const Step& that)
{
  itsRep = that.itsRep;
  if (itsRep) {
    itsRep->incrRefCount();
  }
}

Step::Step (StepRep* rep)
{
  itsRep = rep;
  if (itsRep) {
    itsRep->incrRefCount();
  }
}

Step& Step::operator= (const Step& that)
{
  if (this != &that) {
    if (itsRep  &&  itsRep->decrRefCount() <= 0) {
      delete itsRep;
    }
    itsRep = that.itsRep;
    if (itsRep) {
      itsRep->incrRefCount();
    }
  }
  return *this;
}

Step::~Step() 
{
  if (itsRep  &&  itsRep->decrRefCount() <= 0) {
    delete itsRep;
  }
}

Step* Step::clone() const
{
  return new Step (*this);
}

Composite Step::getParent() const
{
  return Composite(itsRep->getParent());
}

}
