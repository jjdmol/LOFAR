//  Step.cc:
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
//  Revision 1.26  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.25  2002/05/02 12:17:55  schaaf
//  Added CorbaMonitor object
//
//  Revision 1.24  2002/01/09 09:56:35  gvd
//  Added getParent to Step (for Robbert)
//
//  Revision 1.23  2001/12/07 13:58:20  gvd
//  Changes to make connect by name possible
//  Avoid leaks in firewall
//  Replace resolveComm by a new simplifyConnections
//
//  Revision 1.22  2001/11/02 14:29:41  gvd
//  Added clone to Step/Simul and use it in SimulRep
//
//  Revision 1.21  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.20  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.19  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.18  2001/09/18 12:07:28  gvd
//  Changed to resolve Step and Simul memory leaks
//  Introduced ref.counted StepRep and SimulRep classes for that purposes
//  Changed several functions to pass by reference instead of pass by pointer
//
//  Revision 1.17  2001/09/05 15:00:58  wierenga
//  Replace !defined(NOMPI_) with defined(MPI_)
//
//  Revision 1.16  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.15  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//  Revision 1.14  2001/06/22 09:09:30  schaaf
//  Use TRANSPORTERINCLUDE to select the TH_XXX.h include files
//
//  Revision 1.13  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.12  2001/03/01 13:32:32  gvd
//  Improved error message in Step.cc when comparing DH types
//
//  Revision 1.10  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/Step.h"
#include "BaseSim/Simul.h"


Step::Step (const WorkHolder& worker, 
	    const string& aName,
	    bool addNameSuffix,
	    bool monitor)
: itsRep(0)
{
  itsRep = new StepRep (worker, aName, addNameSuffix, monitor);
}

Step::Step (const WorkHolder* worker, 
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

Simul Step::getParent() const
{
  return Simul(itsRep->getParent());
}
