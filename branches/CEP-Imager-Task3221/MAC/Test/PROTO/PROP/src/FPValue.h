//
//  FPValue.h: 
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

#ifndef FPVALUE_H
#define FPVALUE_H

#include <sys/time.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long longlong;
typedef unsigned long long ulonglong;

class FProperty;

class FPValue
{
public:
  enum ValueType {LPT_BOOL = 1, ULPT_CHAR, LPT_CHAR, USHORT_VAL, SHORT_VAL, ULONG_VAL, LONG_VAL,
                  ULONGLONG_VAL, LONGLONG_VAL, LPT_DOUBLE, COMPLEX_LPT_DOUBLE, FLOAT_VAL,
                  COMPLEX_FLOAT_VAL, STRING,
                  ARRAY_VAL = 0x80};
 
	FPValue(ValueType type) : type_(type) {;}
	virtual ~FPValue() {;}
  /** Read property of ValueType type_. */
  inline const ValueType& getType() const {return type_;}
  /** No descriptions */
  virtual FPValue* clone() const = 0;
  /** No descriptions */
  virtual void copy(const FPValue* value) = 0;
 
protected: // Protected attributes
  /** No descriptions */
  virtual uint unpack(const char* valBuf) = 0;
  /** No descriptions */
  virtual uint pack(char* valBuf) const = 0;
  /** No descriptions */
  uint unpackBase(const char* valBuf);
  /** No descriptions */
  uint packBase(char* valBuf) const;
  /**  */
  ValueType type_;
  timeval setTime_;
  friend class FProperty;
};

#endif
