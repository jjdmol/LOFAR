//  WH_Evaluate.h: This class implements the controller of the blackboard.
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

#ifndef PSS3_WH_EVALUATE_H
#define PSS3_WH_EVALUATE_H

#include <lofar_config.h>

#include "CEPFrame/WorkHolder.h"


/**
 This class implements the controller of the blackboard
*/

class WH_Evaluate: public LOFAR::WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_Evaluate (const string& name="aWH_Evaluate");

  virtual ~WH_Evaluate();

  /// Make a fresh copy of the WH object.
  virtual WH_Evaluate* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_Evaluate (const WH_Evaluate&);

  /// Forbid assignment.
  WH_Evaluate& operator= (const WH_Evaluate&);

  int itsCurrentRun;
};


#endif
