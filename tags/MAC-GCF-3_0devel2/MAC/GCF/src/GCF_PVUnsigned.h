//#  GCF_PVUnsigned.h: MAC unsigned integer property type
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

#ifndef GCF_PVUNSIGNED_H
#define GCF_PVUNSIGNED_H

#include <GCF/GCF_PValue.h>

/**
 * By means of this property type a unsigned integer (0 to 4,294,967,295) value 
 * can be used.
 */
class GCFPVUnsigned : public GCFPValue
{
  public:
  	GCFPVUnsigned(unsigned int val = 0) : GCFPValue(LPT_UNSIGNED), _value(val) {;}
  	virtual ~GCFPVUnsigned() {;}
    
    /** Changes the value of this object */
     virtual inline void setValue( const unsigned int newVal) {_value = newVal;}

    /** 
     * Changes the value of this object by means of a stringbuffer, 
     * which will be translated.
     * @see GCFPValue::setValue(const string value)
     */
    virtual TGCFResult setValue(const string value);

    /** Returns the value of this object*/
    virtual inline unsigned int getValue() const {return _value;}

    /** @see GCFPValue::clone() */
    virtual GCFPValue* clone() const;

    /** @see GCFPValue::copy() */
    virtual TGCFResult copy(const GCFPValue& value);
 
    virtual unsigned int unpack(const char* valBuf, unsigned int bufLength);

    virtual unsigned int pack(char* valBuf, unsigned int maxBufSize) const;

  private: // Private attributes
    /** The value */
    unsigned int _value;
};
#endif
