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

class GCFPValue
{
  public:
    enum TMACValueType {BOOL_VAL = 1, CHAR_VAL, UNSIGNED_VAL, INTEGER_VAL, 
                    BIT32_VAL, BLOB_VAL, REF_VAL, FLOAT_VAL, DATETIME_VAL,
                    STRING_VAL, DYNARR_VAL = 0x80};
   
    GCFPValue(TMACValueType type) : _type(type) {};
    virtual ~GCFPValue() {};
    /** Read property of ValueType _type. */
    inline const TMACValueType& getType() const {return _type;}
    /** No descriptions */
    virtual GCFPValue* clone() const = 0;
    /** No descriptions */
    virtual void copy(const GCFPValue& value) = 0;
   
  protected: // Protected attributes
    /**  */
    TMACValueType _type;
};

#endif
