//# NetworkBuilder.h:
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef CEPFRAME_NETWORKBUILDER_H_
#define CEPFRAME_NETWORKBUILDER_H_

#include <lofar_config.h>

#include "CEPFrame/Composite.h"

namespace LOFAR
{

/**
   The NetworkBuilder is part of the implementation of a Builder pattern
   (Gamma). The NetworkBuilder class is an abstract class from which
   concrete builders should inherit and overload the virtual methods.
   A special constructor in the Composite class will first call
   the Step constructor (which will call getWorker())in order to
   correctly set the internal references to the WorkHolder.)
   Then the Composite constructor will call the buildNetwork() method which
   will build the Composite.

   The destructor should delete all objects created in the other
   calls, for example all Steps created with new.

   See Change Request 26 for an example.
*/

class NetworkBuilder 
{
public:
  NetworkBuilder();

  /** The destructor.
  */
  virtual ~NetworkBuilder();

  /** The buildNetwork method contains all the code that will build the
      actual simul. The argument is a Composite object which has to build.
  */
  virtual void buildNetwork (Composite&) const = 0;

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
  NetworkBuilder (const NetworkBuilder&);

  /// Forbid assignment..
  NetworkBuilder& operator= (const NetworkBuilder&);


  WorkHolder* itsWorker;
};




inline NetworkBuilder::NetworkBuilder()
: itsWorker (0)
{}

inline WorkHolder& NetworkBuilder::getWorker()
{ return *itsWorker; }

inline void NetworkBuilder::setWorker (WorkHolder& worker)
{ itsWorker = worker.baseMake(); }

}

#endif
