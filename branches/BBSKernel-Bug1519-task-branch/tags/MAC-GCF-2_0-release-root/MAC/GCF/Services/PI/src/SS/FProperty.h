//
//  FProperty.h: 
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

#ifndef FPROPERTY_H
#define FPROPERTY_H


#include "FPValue.h"

class FProperty
{
public:
  FProperty(const char* name, FPValue::ValueType type);
  FProperty(const char* name, const FPValue& value);
  
  virtual ~FProperty();
  /** Read property of FPValue* pOnlineValue_. */
  virtual const FPValue* getValue() const;
  /** No descriptions */
  inline const char* getName() const {return name_;}
  /** No descriptions */
  virtual uint unpack(const char* valBuf);
  /** No descriptions */
  virtual uint pack(char* valBuf) const;
  /** Write property of FPValue* pOriginalValue_.*/
  virtual short setValue( const FPValue& pNewVal);

protected: // protected attributes
  /**  */
  FPValue* pOnlineValue_;
  /** */
  char* name_;
};

#endif
