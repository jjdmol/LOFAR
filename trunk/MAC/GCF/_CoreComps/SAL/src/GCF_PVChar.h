//#  GCF_PVChar.h: 
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

#ifndef GCF_PVCHAR_H
#define GCF_PVCHAR_H

#include "GCF_PValue.h"

class GCFPVChar : public GCFPValue  
{
  public:
  	GCFPVChar(char val = 0) : GCFPValue(CHAR_VAL), _value(val) {;}
  	virtual ~GCFPVChar() {;}
    /** Write property of char value. */
    inline void setValue( const char newVal) {_value = newVal;}
    /** No descriptions */
    virtual TSAResult setValue(const string value);
    /** Read property of char value. */
    inline char getValue() const {return _value;}
    /** No descriptions */
    virtual GCFPValue* clone() const;
    /** No descriptions */
    virtual TSAResult copy(const GCFPValue& value);
  
  private: // Private attributes
    /**  */
    char _value;
};

#endif
