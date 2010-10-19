//# Step.h: Class representing a basic simulation block
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

#ifndef LOFAR_CEPFRAME_STEP_H
#define LOFAR_CEPFRAME_STEP_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
// \file
// Class representing a basic simulation block

//# Includes
#include <CEPFrame/Block.h>
#include <CEPFrame/StepRep.h>

namespace LOFAR
{

  // \addtogroup CEPFrame
  // @{

//# Forward Declarations
class Composite;


/** The Step class is the basic building block for simulations.
    In the constructor the actual worker and dataholders are defined. 
    The actual simulation work is performed in the process() method
    (which calls the WorkHolder::baseProcess() method).

    Note that the actual Step data is contained in the reference counted
    class StepRep. In this way a copy of a Step is a cheap operation.
*/

class Step : public Block
{
public:
  /** Build the Step using the given WorkHolder.
      The Step must get a unique name. To make that process easy,
      by default the suffix _n is added to the name when the Step is
      added to a Composite (where n is the sequence number starting with 0).
  */
  explicit Step (WorkHolder& worker,
		 const string& name = "aStep",
		 bool addNameSuffix = true);
  explicit Step (WorkHolder* worker,
		 const string& name = "aStep",
		 bool addNameSuffix = true);

  /// Copy constructor (reference semantics).
  Step (const Step&);

  /// Construct a Step from a StepRep (meant for internal use).
  explicit Step (StepRep*);

  virtual ~Step();

  /// Assignment (reference semantics).
  Step& operator= (const Step&);

  /// Make a correct copy (reference semantics).
  virtual Step* clone() const;

  /// Is the Step a Composite?
  bool isComposite() const;

  /// get WorkHolder.
  WorkHolder* getWorker();

protected:

  StepRep* itsStepRep;

};

inline bool Step::isComposite() const
  { return false; }

inline WorkHolder* Step::getWorker()
  { return itsStepRep->getWorker(); }


//@}

}

#endif
