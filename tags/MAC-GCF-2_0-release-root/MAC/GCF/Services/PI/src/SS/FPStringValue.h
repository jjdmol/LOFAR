//#  FPStringValue.h: 
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef FPSTRINGVALUE_H
#define FPSTRINGVALUE_H

#include "FPValue.h"
#include <string>

class FPStringValue : public FPValue
{
  public: 
    FPStringValue(string val = "") : FPValue(LPT_STRING), value_(val) {;}
    virtual ~FPStringValue() {;}
    
    /** Write property of string value_. */
    inline void setValue( const string& newVal) {value_ = newVal;};
    /** Read property of string value_. */
    inline const string& getValue() const {return value_;};
    /** No descriptions */
    virtual FPValue* clone() const;
    /** No descriptions */
    virtual void copy(const FPValue& value);
    /** No descriptions */
    virtual uint unpack(const char* valBuf);
    /** No descriptions */
    virtual uint pack(char* valBuf) const;
    
  private: // Private attributes
    /**  */
    string value_;
};
#endif
