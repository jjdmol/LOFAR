//
//  FProperty.cc:
//
//  Copyright (C) 2003
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

#include "FProperty.h"

#include <string.h>

FProperty::FProperty(const char* name, FPValue::ValueType type) :
  pOnlineValue_(0),
  name_(0)
{
  name_ = new char[strlen(name) + 1];
  strcpy(name_, name);
  
  pOnlineValue_ = FPValue::createValueObject(type);
}

FProperty::FProperty(const char* name, const FPValue& newVal) :
  pOnlineValue_(0),
  name_(0)
{
  name_ = new char[strlen(name) + 1];
  strcpy(name_, name);
  
  pOnlineValue_ = newVal.clone();
}

FProperty::~FProperty()
{
  if (pOnlineValue_ != 0)
  {
    delete pOnlineValue_;
  }
  if (name_ != 0)
    delete name_;
}

/** Read property of FPValue* pOnlineValue_. */
const FPValue* FProperty::getValue() const
{
	return pOnlineValue_;
}

/** Write property of FPValue* pValue_. */
short FProperty::setValue( const FPValue& newVal)
{
  short result = 0;
  
  if (pOnlineValue_->getType() == newVal.getType())
  {
	  pOnlineValue_->copy(newVal); /* copy new value to online value*/
    result = 1;
  }
  return result;
}

/** No descriptions */
uint FProperty::unpack(const char* valBuf)
{
  return pOnlineValue_->unpack(valBuf);
}

/** No descriptions */
uint FProperty::pack(char* valBuf) const
{
  uint bytesWritten(0);
  uchar nameLength = strlen(name_);
  
  memcpy(valBuf, (void*)&nameLength, sizeof(uchar));
  memcpy(valBuf + sizeof(uchar), name_, nameLength);
  
  bytesWritten = sizeof(uchar) + nameLength;
  valBuf += bytesWritten;
  bytesWritten += pOnlineValue_->pack(valBuf);
    
  return bytesWritten;
}

