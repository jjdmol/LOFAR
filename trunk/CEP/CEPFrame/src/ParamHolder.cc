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


#include "CEPFrame/ParamHolder.h"
#include <Common/lofar_iostream.h>
#include "CEPFrame/StepRep.h"
#include "CEPFrame/ParamTransport.h"
#include "CEPFrame/ParamTransportManager.h"
#include TRANSPORTERINCLUDE
#include "Common/Debug.h"

namespace LOFAR
{

int ParamHolder::theirSerial = 0;


ParamHolder::ParamHolder(const string& name, const string& type)
  : Transportable(),
    itsStep(0),
    itsName(name),
    itsType(type),
    itsIsParamOwner(false),
    itsParamPacketPtr(0),
    itsPTManager(0)
{
  itsSerial = theirSerial++;
  //  cout << "itsSerial = " << itsSerial << endl;
}

ParamHolder::~ParamHolder()
{}

ParamHolder::ParamHolder(const ParamHolder& that)
  :  Transportable(),
     itsStep(that.itsStep),
     itsName(that.itsName),
     itsType(that.itsType),
     itsIsParamOwner(that.itsIsParamOwner),
     itsParamPacketPtr(0),
     itsPTManager(that.itsPTManager)
{
  itsSerial = theirSerial++;
}

void ParamHolder::basePreprocess()
{

}

void ParamHolder::preprocess()
{}

int ParamHolder::getNode() const
{
  return itsStep==0  ?  -1 : itsStep->getNode();
} 

ParamHolder& ParamHolder::operator= (const ParamHolder& that)
{
  if (this != &that) {
    itsParamPacket = that.itsParamPacket;
    itsStep = that.itsStep;
    itsName = that.itsName;
    itsType = that.itsType;
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

}
