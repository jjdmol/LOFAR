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
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_SIMULBUILDER_H_
#define CEPFRAME_SIMULBUILDER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/Simul.h"

namespace LOFAR
{

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
  WorkHolder& getWorker();

  /** The setWorker method is used by constructors of derived classes
      to set the itsWorker.
  */
  void setWorker (WorkHolder&);

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

inline WorkHolder& SimulBuilder::getWorker()
{ return *itsWorker; }

inline void SimulBuilder::setWorker (WorkHolder& worker)
{ itsWorker = worker.baseMake(); }

}

#endif
