//#  GCF_PVString.h: 
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

#ifndef GCF_PVSTRING_H
#define GCF_PVSTRING_H

#include "GCF_PValue.h"

class GCFPVString : public GCFPValue
{
  public:
  	GCFPVString(string val = "") : GCFPValue(STRING_VAL), _value(val) {;}
  	virtual ~GCFPVString() {;}
    /** Write property of string value. */
    virtual inline void setValue(const string newVal) {_value = newVal;}
    /** Read property of string value. */
    virtual inline const string& getValue() const {return _value;}
    /** No descriptions */
    virtual GCFPValue* clone() const;
    /** No descriptions */
    virtual TSAResult copy(const GCFPValue& value);
 
  private: // Private attributes
    /**  */
    string _value;
};
#endif
