//  WH_Tester.h: WorkHolder for the Tester test program
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
//  Revision 1.11  2002/06/27 10:49:55  schaaf
//  %[BugId: 57]%
//  Modified getIn/OutHolder methods and use in process() and dump() methods.
//
//  Revision 1.10  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.9  2002/03/15 13:28:09  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.8  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.7  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
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
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_WH_TESTER_H
#define BASESIM_WH_TESTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/WorkHolder.h"
#include "BaseSim/DH_Tester.h"


/**
   This is the work holder for the Tester test program.
*/

class WH_Tester: public WorkHolder
{
public:
  /// Construct the work holder.
  explicit WH_Tester (const string& name = "WH_Tester");

  virtual ~WH_Tester();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

   /// Make a fresh copy of the WH object.
  virtual WH_Tester* make (const string& name) const;

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_Tester* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_Tester* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_Tester (const WH_Tester&);

  /// Forbid assignment.
  WH_Tester& operator= (const WH_Tester&);


  DH_Tester itsInDataHolder;
  DH_Tester itsOutDataHolder;
};


#endif
