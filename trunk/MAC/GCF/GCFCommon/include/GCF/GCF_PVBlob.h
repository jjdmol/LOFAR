//#  GCF_PVBlob.h: MAC string property type
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

#ifndef GCF_PVBLOB_H
#define GCF_PVBLOB_H

#include <GCF/GCF_PValue.h>

/**
 * By means of this property type bulk of data with a specified size can be used.
 */
class GCFPVBlob : public GCFPValue
{
  public:
  	explicit GCFPVBlob(unsigned char* val = 0, unsigned int size = 0) : GCFPValue(LPT_BLOB), _value(val), _size(size) {;}
  	virtual ~GCFPVBlob() {;}
    
    /** Changes the value of this object */
    virtual TGCFResult setValue(unsigned char* value, unsigned int size);
    virtual TGCFResult setValue(const string value);

    /** Returns the value of this object*/
    virtual unsigned char* getValue() const {return _value;}
    virtual unsigned int getLen() const {return _size;}

    /** @see GCFPValue::clone() */
    virtual GCFPValue* clone() const;

    /** @see GCFPValue::copy() */
    virtual TGCFResult copy(const GCFPValue& value);
 
    virtual unsigned int unpack(const char* valBuf);

    virtual unsigned int pack(char* valBuf) const;

    virtual unsigned int getSize() const { return sizeof(_size) + _size + getBaseSize(); }
    
  private: // Private attributes
    /** The value*/
    unsigned char* _value;
    unsigned int _size;
};
#endif
