//#  GCF_PVChar.h: MAC char property type 
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

#include <GCF/GCF_PValue.h>

/**
   By means of this property type a character (0...255) value can be used.
*/

class GCFPVChar : public GCFPValue  
{
  public:
  	GCFPVChar (char val = 0) : GCFPValue(LPT_CHAR), _value(val) {;}
  	virtual ~GCFPVChar () {;}

    /** Changes the value of this object */
    inline void setValue (const char newVal) {_value = newVal;}

    /** 
     * Changes the value of this object by means of a stringbuffer, 
     * which will be translated.
     * @see GCFPValue::setValue(const string value)
     */
    virtual TGCFResult setValue (const string value);

    /** Returns the value of this object*/
    inline char getValue () const {return _value;}

    /** @see GCFPValue::clone() */
    virtual GCFPValue* clone () const;

    /** @see GCFPValue::copy() */
    virtual TGCFResult copy (const GCFPValue& value);
  
    virtual unsigned int unpack(const char* valBuf, unsigned int bufLength);

    virtual unsigned int pack(char* valBuf, unsigned int maxBufSize) const;

  private: // Private attributes
    /** The value */
    char _value;
};

#endif
