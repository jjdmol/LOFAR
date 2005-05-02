//# WH_Example.h: This is an example of a WorkHolder class.
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

#ifndef CEPFRAME_WH_EXAMPLE_H
#define CEPFRAME_WH_EXAMPLE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <tinyCEP/WorkHolder.h>
#include <CEPFrame/DH_Example.h>

namespace LOFAR
{

/**
   This is an example of a WorkHolder class.
   It has one input and one output DH_Example object as DataHolders.

   It shows which functions have to be implemented
*/

class WH_Example: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_Example (const string& name="aWH_Example",
		       unsigned int nin=1,
		       unsigned int nout=1,
		       unsigned int nbuffer=10);

  virtual ~WH_Example();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const KeyValueMap&);

  /// Make a fresh copy of the WH object.
  virtual WH_Example* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_Example (const WH_Example&);

  /// Forbid assignment.
  WH_Example& operator= (const WH_Example&);

  /// Length of DH_Example buffers.
  int itsBufLength;
};

}

#endif
