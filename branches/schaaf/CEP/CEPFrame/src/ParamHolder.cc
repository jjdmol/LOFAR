//  ParamHolder.cc:
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
//  Revision 1.7  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.6  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.5  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.4  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.3  2001/02/05 14:53:04  loose
//  Added GPL headers
//

// ParamHolder.cpp: implementation of the ParamHolder class.
//
//////////////////////////////////////////////////////////////////////


#include "BaseSim/ParamHolder.h"
#include <Common/lofar_iostream.h>

int ParamHolder::theirSerial = 0;


ParamHolder::ParamHolder()
{
  itsSerial = theirSerial++;
  //  cout << "itsSerial = " << itsSerial << endl;
}

ParamHolder::~ParamHolder()
{}

ParamHolder& ParamHolder::operator= (const ParamHolder& that)
{
  if (this != &that) {
    itsDataPacket = that.itsDataPacket;
  }
  return *this;
}

bool ParamHolder::operator== (const ParamHolder& aPH)  const
{
  return itsSerial == aPH.itsSerial;
}

bool ParamHolder::operator!= (const ParamHolder& aPH) const
{
  return itsSerial != aPH.itsSerial;
}

bool ParamHolder::operator< (const ParamHolder& aPH) const
{
  return itsSerial < aPH.itsSerial;
}

void ParamHolder::dump() const
{
  cout << "ParamHolder" << endl;
}
