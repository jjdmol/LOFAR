//#  GCF_PVDouble.h: 
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

#ifndef GCF_PVDOUBLE_H
#define GCF_PVDOUBLE_H

#include "GCF_PValue.h"

class GCFPVDouble : public GCFPValue
{
  public: 
  	GCFPVDouble(double val = 0.0) : GCFPValue(DOUBLE_VAL), _value(val) {;}
  	virtual ~GCFPVDouble() {;}
    
    /** Write property of float value_. */
    inline void setValue( const double newVal) {_value = newVal;};
    /** No descriptions */
    virtual TSAResult setValue(const string value);
    /** Read property of float value_. */
    inline double getValue() const {return _value;};
    /** No descriptions */
    virtual GCFPValue* clone() const;
    /** No descriptions */
    virtual TSAResult copy(const GCFPValue& value);
    
  private: // Private attributes
    /**  */
    volatile double _value;
};
#endif
