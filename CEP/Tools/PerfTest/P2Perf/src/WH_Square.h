//  WH_Square.h: This is an example of a WorkHolder class.
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
//  Revision 1.3  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.2  2001/08/16 15:14:23  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////

#ifndef WH_SQUARE_H
#define WH_SQUARE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/WorkHolder.h"
#include "P2Perf/DH_IntArray.h"


/**
   This is an example of a WorkHolder class.
   It has one input and one output DH_IntArray object as DataHolders.

   It shows which functions have to be implemented
*/

class WH_Square: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The argument first tells if this is the first WorkHolder in
  /// the simulation chain.
  WH_Square (const string& name="WH_Square",
	      bool first = false,
	      unsigned int nin=1, unsigned int nout=1,
	      unsigned int nbuffer=10);

  virtual ~WH_Square();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_IntArray* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_IntArray* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_Square (const WH_Square&);

  /// Forbid assignment.
  WH_Square& operator= (const WH_Square&);


  /// Pointer to the array of input DataHolders.
  DH_IntArray** itsInHolders;
  /// Pointer to the array of output DataHolders.
  DH_IntArray** itsOutHolders;

  /// Length of DH_IntArray buffers.
  int itsBufLength;
  /// Is this the first WorkHolder in the simulation chain?
  bool itsFirst;
};


#endif
