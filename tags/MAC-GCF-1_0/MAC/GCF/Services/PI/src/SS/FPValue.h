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
    enum ValueType {NO_LPT = 0, LPT_BOOL, LPT_CHAR, LPT_INTEGER, LPT_UNSIGNED, LPT_DOUBLE,
                    LPT_STRING, LPT_BLOB, BIT32_VAL, LPT_DATETIME, 
                    LPT_DYNARR = 0x80,
                    LPT_DYNBOOL, LPT_DYNCHAR, LPT_DYNINTEGER, LPT_DYNUNSIGNED, LPT_DYNDOUBLE,
                    LPT_DYNSTRING, LPT_DYNBLOB, DYNBIT32_VAL, LPT_DYNDATETIME};
   
  	FPValue(ValueType type) : type_(type) {;}
  	virtual ~FPValue() {;}
    /** Read property of ValueType type_. */
    inline const ValueType& getType() const {return type_;}
    /** No descriptions */
    virtual FPValue* clone() const = 0;
    /** No descriptions */
    virtual void copy(const FPValue& value) = 0;
    /** No descriptions */
    virtual uint unpack(const char* valBuf) = 0;
    /** No descriptions */
    virtual uint pack(char* valBuf) const = 0;
    
    static FPValue* createValueObject(ValueType type);
    
  protected: // Protected attributes
    /** No descriptions */
    uint unpackBase(const char* valBuf);
    /** No descriptions */
    uint packBase(char* valBuf) const;
    /**  */
    ValueType type_;
    timeval setTime_;
};

#endif
