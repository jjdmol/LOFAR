//#  GCF_PValue.h: 
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

#ifndef GCF_PVALUE_H
#define GCF_PVALUE_H

#include <SAL/GSA_Defines.h>

class GCFPValue
{
  public:
    enum TMACValueType {NO_VAL, BOOL_VAL, CHAR_VAL, UNSIGNED_VAL, INTEGER_VAL, 
                    BIT32_VAL, BLOB_VAL, REF_VAL, DOUBLE_VAL, DATETIME_VAL,
                    STRING_VAL, DYNARR_VAL = 0x80,
                    DYNBOOL_VAL, DYNCHAR_VAL, DYNUNSIGNED_VAL, DYNINTEGER_VAL, 
                    DYNBIT32_VAL, DYNBLOB_VAL, DYNREF_VAL, DYNDOUBLE_VAL, DYNDATETIME_VAL,
                    DYNSTRING_VAL };
   
    GCFPValue(TMACValueType type) : _type(type) {};
    virtual ~GCFPValue() {};
    /** Read property of ValueType _type. */
    inline const TMACValueType& getType() const {return _type;}
    /** No descriptions */
    virtual GCFPValue* clone() const = 0;
    /** No descriptions */
    virtual TSAResult copy(const GCFPValue& value) = 0;
    
    virtual TSAResult setValue(const string value) = 0;

    static GCFPValue* createMACTypeObject(TMACValueType type);
   
  protected: // Protected attributes
    /**  */
    TMACValueType _type;
};

#endif
