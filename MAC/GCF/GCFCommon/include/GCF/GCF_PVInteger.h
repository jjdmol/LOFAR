//#  GCF_PVInteger.h: MAC integer property type
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

#ifndef GCF_PVINTEGER_H
#define GCF_PVINTEGER_H

#include <GCF/GCF_PValue.h>

/**
 * By means of this property type a integer (-2,147,483,648 to 2,147,483,647) 
 * value can be used.
 */
class GCFPVInteger : public GCFPValue
{
  public:
  	explicit GCFPVInteger (int val = 0) : GCFPValue(LPT_INTEGER), _value(val) {;}
    explicit GCFPVInteger(const GCFPVInteger& val) : GCFPValue(LPT_INTEGER), _value(val.getValue()) {;}

  	virtual ~GCFPVInteger () {;}
    
    /** Changes the value of this object */
    virtual inline void setValue (const int newVal) {_value = newVal;}

    /** 
     * Changes the value of this object by means of a stringbuffer, 
     * which will be translated.
     * @see GCFPValue::setValue(const string value)
     */
    virtual TGCFResult setValue (const string value);

    /** Returns the value of this object*/
    virtual inline int getValue () const {return _value;}

    /** @see GCFPValue::clone() */
    virtual GCFPValue* clone () const;

    /** @see GCFPValue::copy() */
    virtual TGCFResult copy (const GCFPValue& value);
 
    /// @see GCFPValue::unpack()
    virtual unsigned int unpackConcrete(const char* valBuf);

    /// @see GCFPValue::pack()
    virtual unsigned int packConcrete(char* valBuf) const;

    /// @see GCFPValue::getSize()
    virtual unsigned int getConcreteSize() const { return sizeof(int); }
    
  private: // Private attributes
    /** The value */
    int _value;
};
#endif
