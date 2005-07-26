//  WH_Multiply.h: WorkHolder for the Tester test program
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

#ifndef EXAMPLESIM_WH_MULTIPLY_H
#define EXAMPLESIM_WH_MULTIPLY_H

#include <tinyCEP/WorkHolder.h>
#include <ExampleSim/DH_ExampleSim.h>

namespace LOFAR
{

/**
   This WorkHolder multiplies all inputs.
*/

class WH_Multiply: public LOFAR::WorkHolder
{
public:
  /// Construct the work holder.
  explicit WH_Multiply (const string& name = "WH_Multiply", int nin = 1);

  virtual ~WH_Multiply();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const LOFAR::KeyValueMap&);

   /// Make a fresh copy of the WH object.
  virtual WH_Multiply* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

private:
  /// Forbid copy constructor.
  WH_Multiply (const WH_Multiply&);

  /// Forbid assignment.
  WH_Multiply& operator= (const WH_Multiply&);

  int itsNInputs;

};

} // end namespace LOFAR

#endif
