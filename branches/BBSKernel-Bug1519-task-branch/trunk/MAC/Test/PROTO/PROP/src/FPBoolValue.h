//
//  FPBoolValue.h: 
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

#ifndef FPBOOLVALUE_H
#define FPBOOLVALUE_H

#include "FPValue.h"

/**
  *@author pvss
  */

class FPBoolValue : public FPValue
{
public: 
	FPBoolValue(bool val = false) : FPValue(LPT_BOOL), value_(val) {;}
	virtual ~FPBoolValue() {;}
  
  /** Write property of float value_. */
  inline void setValue( const bool newVal) {value_ = newVal;};
  /** Read property of float value_. */
  inline bool getValue() const {return value_;};
  /** No descriptions */
  virtual FPValue* clone() const;
  /** No descriptions */
  virtual void copy(const FPValue* pValue);
  
private: // Private attributes
  /** No descriptions */
  virtual uint unpack(const char* valBuf);
  /** No descriptions */
  virtual uint pack(char* valBuf) const;
  /**  */
  volatile bool value_;
};
#endif
