//  WH_Control.h: This class implements the controller of the blackboard.
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

#ifndef LOFAR_BBS3_WH_CONTROL_H
#define LOFAR_BBS3_WH_CONTROL_H

// \file WH_Control.h
// This class implements the controller of the blackboard.

#include <Common/lofar_vector.h>
#include <tinyCEP/WorkHolder.h>
#include <Common/KeyValueMap.h>

#include <Common/lofar_list.h>
namespace LOFAR
{

// \addtogroup BBS3
// @{

class StrategyController;

/**
 This class implements the controller of the blackboard
*/

class WH_Control: public LOFAR::WorkHolder
{

public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_Control (const string& name, int nrPrediffers,
		       const KeyValueMap& args);

  virtual ~WH_Control();

  /// Make a fresh copy of the WH object.
  virtual WH_Control* make (const string& name);

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int nrPrediffers,
				const KeyValueMap& args);

  /// Preprocess
  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Postprocess
  virtual void postprocess();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  typedef list<StrategyController*> ControllerList;

  /// Forbid copy constructor.
  WH_Control (const WH_Control&);

  /// Forbid assignment.
  WH_Control& operator= (const WH_Control&);

  // Private help functions to create strategy controller(s)
  void createStrategyControllers();

  int                      itsNrPrediffers;  // Number of available prediffers
  KeyValueMap              itsArgs;          // Input parameters
  ControllerList           itsControllers;   // All StrategyControllers   
  ControllerList::iterator itsCtrlIter;      // Iterator indicating the currently
                                             // active strategy controller.
                                             // At the moment each process() call 
                                             // executes one StrategyController
  bool                     itsFirstCall;

};

// @}

} // namespace LOFAR

#endif
