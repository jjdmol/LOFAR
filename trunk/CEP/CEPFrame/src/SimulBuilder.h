//  SimulBuilder.h:
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
//  Revision 1.12  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.11  2002/03/04 12:54:01  gvd
//  Let WorkHolder copy the name of DataHolders; done by creating baseMake
//
//  Revision 1.10  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.9  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.8  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.7  2001/09/18 12:07:28  gvd
//  Changed to resolve Step and Simul memory leaks
//  Introduced ref.counted StepRep and SimulRep classes for that purposes
//  Changed several functions to pass by reference instead of pass by pointer
//
//  Revision 1.6  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.5  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.4  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_SIMULBUILDER_H_
#define BASESIM_SIMULBUILDER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/Simul.h"


/**
   The SimulBuilder is part of the implementation of a Builder pattern
   (Gamma). The SimulBuilder class is an abstract class from which
   concrete builders should inherit and overload the virtual methods.
   A special constructor in the Simul class will first call
   the Step constructor (which will call getWorker())in order to
   correctly set the internal references to the WorkHolder.)
   Then the Simul constructor will call the buildSimul() method which
   will build the Simul.

   The destructor should delete all objects created in the other
   calls, for example all Steps created with new.

   See Change Request 26 for an example.
*/

class SimulBuilder 
{
public:
  SimulBuilder();

  /** The destructor.
  */
  virtual ~SimulBuilder();

  /** The buildSimul method contains all the code that will build the
      actual simul. The argument is a Simul object which has to build.
  */
  virtual void buildSimul (Simul&) const = 0;

  /** The getWorker method is used by the Step constructor to obtain a
      reference to the workholder.
  */
  const WorkHolder& getWorker() const;

  /** The setWorker method is used by constructors of derived classes
      to set the itsWorker.
  */
  void setWorker (const WorkHolder&);

private:
  /// Forbid copy constructor.
  SimulBuilder (const SimulBuilder&);

  /// Forbid assignment..
  SimulBuilder& operator= (const SimulBuilder&);


  WorkHolder* itsWorker;
};




inline SimulBuilder::SimulBuilder()
: itsWorker (0)
{}

inline const WorkHolder& SimulBuilder::getWorker() const
{ return *itsWorker; }

inline void SimulBuilder::setWorker (const WorkHolder& worker)
{ itsWorker = worker.baseMake(); }


#endif
