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
// ParamHolder.cpp: implementation of the ParamHolder class.
//
//////////////////////////////////////////////////////////////////////


#include "ParamHolder.h"
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
