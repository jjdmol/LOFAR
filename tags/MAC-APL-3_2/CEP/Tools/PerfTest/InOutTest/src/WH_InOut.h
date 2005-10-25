//  WH_InOut.h: WorkHolder class measuring performance
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
//
//////////////////////////////////////////////////////////////////////

#ifndef LOFAR_INOUTTEST_WH_INOUT_H
#define LOFAR_INOUTTEST_WH_INOUT_H

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{

/**
   The WH_Growsize class implements a workholder with DH_Growsize
   objects as inputs and outputs. The process() method does nothing to
   the data (not even copy...) but can contains a performance measurement
   indication for the data transport bandwidth of the output DataHolders. 
 */

class WH_InOut: public WorkHolder
{
public:

  WH_InOut (const string& name="WH_InOut",
	    unsigned int nbuffer=10);   // default length of the
	                                // buffer in DH_Growsize::DataPacket 

  virtual ~WH_InOut();

  virtual WorkHolder* make(const string& name);

  //virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

private:
  /// Forbid copy constructor.
  WH_InOut (const WH_InOut&);

  /// Forbid assignment.
  WH_InOut& operator= (const WH_InOut&);

  /// Length of DH buffers.
  int itsBufLength;

};

} // end namespace LOFAR

#endif
