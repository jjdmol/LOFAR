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
#include "FPByteValue.h"
#include "FPWordValue.h"
#include "FPDWordValue.h"
#include "FPFloatValue.h"
#include "FPDoubleValue.h"
#include "FPBoolValue.h"
#include "FPLongLongValue.h"

#include <string.h>

FProperty::FProperty(const char* name, FPValue::ValueType type) :
  pOldValue_(0),
  pOnlineValue_(0),
  pOriginalValue_(0),
  subscribed_(false),
  accessMode_(FP_SUBSCRIPTION_AND_GET_ALLOWED | FP_SET_ALLOWED | FP_SET_IMMEDIATE),
  name_(0)
{
  name_ = new char[strlen(name) + 1];
  strcpy(name_, name);
  
  switch (type)
  {
  case FPValue::LPT_BOOL:
    pOldValue_ = new FPBoolValue();
    pOriginalValue_ = new FPBoolValue();
    pOnlineValue_ = new FPBoolValue();
    break;
  case FPValue::ULPT_CHAR:
    pOldValue_ = new FPUCharValue();
    pOriginalValue_ = new FPUCharValue();
    pOnlineValue_ = new FPUCharValue();
    break;
  case FPValue::LPT_CHAR:
    pOldValue_ = new FPCharValue();
    pOriginalValue_ = new FPCharValue();
    pOnlineValue_ = new FPCharValue();
    break;
  case FPValue::USHORT_VAL:
    pOldValue_ = new FPUShortValue();
    pOriginalValue_ = new FPUShortValue();
    pOnlineValue_ = new FPUShortValue();
    break;
  case FPValue::SHORT_VAL:
    pOldValue_ = new FPShortValue();
    pOriginalValue_ = new FPShortValue();
    pOnlineValue_ = new FPShortValue();
    break;
  case FPValue::ULONG_VAL:
    pOldValue_ = new FPULongValue();
    pOriginalValue_ = new FPULongValue();
    pOnlineValue_ = new FPULongValue();
    break;
  case FPValue::LONG_VAL:
    pOldValue_ = new FPLongValue();
    pOriginalValue_ = new FPLongValue();
    pOnlineValue_ = new FPLongValue();
    break;
  case FPValue::ULONGLONG_VAL:
    pOldValue_ = new FPULongLongValue();
    pOriginalValue_ = new FPULongLongValue();
    pOnlineValue_ = new FPULongLongValue();
    break;
  case FPValue::LONGLONG_VAL:
    pOldValue_ = new FPLongLongValue();
    pOriginalValue_ = new FPLongLongValue();
    pOnlineValue_ = new FPLongLongValue();
    break;
  case FPValue::FLOAT_VAL:
    pOldValue_ = new FPFloatValue();
    pOriginalValue_ = new FPFloatValue();
    pOnlineValue_ = new FPFloatValue();
    break;
  case FPValue::COMPLEX_FLOAT_VAL:
    pOldValue_ = new FPComplexFloatValue();
    pOriginalValue_ = new FPComplexFloatValue();
    pOnlineValue_ = new FPComplexFloatValue();
    break;
  case FPValue::LPT_DOUBLE:
    pOldValue_ = new FPDoubleValue();
    pOriginalValue_ = new FPDoubleValue();
    pOnlineValue_ = new FPDoubleValue();
    break;
  case FPValue::COMPLEX_LPT_DOUBLE:
    pOldValue_ = new FPComplexDoubleValue();
    pOriginalValue_ = new FPComplexDoubleValue();
    pOnlineValue_ = new FPComplexDoubleValue();
    break;
  default:
    break; 
  }
}

FProperty::~FProperty()
{
  if (pOldValue_ != 0)
  {
    delete pOldValue_;
  }
  if (pOriginalValue_ != 0)
  {
    delete pOriginalValue_;
  }
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
    pOldValue_->copy(pOnlineValue_); /* copy online value to old value*/
    pOriginalValue_->copy(&newVal); /* copy new value to original value*/
	  pOnlineValue_->copy(&newVal); /* copy new value to online value*/
    result = 1;
  }
  return result;
}

/** No descriptions */
uint FProperty::unpack(const char* valBuf)
{
  FPValue* pTempValue(0);
  FPValue* pCurrentValue(0);
  uint bytesRead(0);

  if ((~accessMode_) & FP_SET_ALLOWED)
    return bytesRead;
    
  if (accessMode_ & FP_SET_IMMEDIATE)
    pCurrentValue = pOnlineValue_;
  else
    pCurrentValue = pOriginalValue_;  /* delayed setting; online value will be updated by syncValue */
  
  pTempValue = pCurrentValue->clone();
  bytesRead = pCurrentValue->unpack(valBuf);
  if (bytesRead > 0)
  {
    if (accessMode_ & FP_SET_IMMEDIATE)
    {
      pOriginalValue_->copy(pOnlineValue_);
    }
    delete pOldValue_;  
    pOldValue_ = pTempValue;
  }
  else
    delete pTempValue;
  
  return bytesRead;
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

/** Read property of FPValue* pOldValue_. */
const FPValue* FProperty::getOldValue() const
{
	return pOldValue_;
}

/** Write property of int accessMode_. */
void FProperty::setAccessMode( const uchar& newVal)
{
	accessMode_ = newVal;
}

/** Write property of bool subscribed_. */
int FProperty::enableSubscription( const bool& newVal)
{
  int result = 0;
  if ((newVal == true && subscribed_ == false && (accessMode_ & FP_SUBSCRIPTION_AND_GET_ALLOWED)) ||
      subscribed_ == true)
  {
    subscribed_ = newVal;
    result = 1;  
  }
  return result;
}

/** No descriptions */
void FProperty::syncValues()
{
  if ((~accessMode_) & FP_SET_IMMEDIATE)
  {
    pOnlineValue_->copy(pOriginalValue_);
  }
}
