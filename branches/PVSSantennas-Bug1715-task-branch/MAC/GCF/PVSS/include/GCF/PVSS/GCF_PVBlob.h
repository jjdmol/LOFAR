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

#include <GCF/PVSS/GCF_PValue.h>
#include <Common/LofarTypes.h>


namespace LOFAR 
{
using TYPES::uint16;
 namespace GCF 
 {
  namespace PVSS 
  {

/**
 * By means of this property type bulk of data with a specified size can be used.
 */
class GCFPVBlob : public GCFPValue
{
  public:
    /** 
     * @param value a not 0 terminated buffer
     * @param length length of the buffer
     * @param clone buffer must be cloned 
     */
  	explicit GCFPVBlob(unsigned char* val = 0, uint16 length = 0, bool clone = false);

    GCFPVBlob(const GCFPVBlob& val) 
      : GCFPValue(LPT_BLOB), _value(0), _length(0), _isDataHolder(false) 
      { copy(val);}

  	virtual ~GCFPVBlob() {if (_isDataHolder) delete [] _value;}
    
    /** Changes the value of this object 
     * @param value a not 0 terminated buffer
     * @param length length of the buffer
     * @param clone buffer must be cloned 
     */
    virtual TGCFResult setValue(unsigned char* value, uint16 length, bool clone = false);
    virtual TGCFResult setValue(const string& value);
    
    // Returns the value of this object in a string
    virtual string getValueAsString(const string& format = "") const;
    
    /** Returns the value of this object*/
    virtual unsigned char* getValue() const {return _value;}
    virtual uint16 getLen() const {return _length;}

    /** @see GCFPValue::clone() */
    virtual GCFPValue* clone() const;

    /** @see GCFPValue::copy() */
    virtual TGCFResult copy(const GCFPValue& value);
 
    /** @see GCFPValue::operator==() */
    virtual bool operator==(const GCFPValue& that) const;
    virtual bool operator!=(const GCFPValue& that) const { return (!(*this == that)); }
 
private:
    /// @see GCFPValue::unpack()
    unsigned int unpackConcrete(const char* valBuf);

    /// @see GCFPValue::pack()
    unsigned int packConcrete(char* valBuf) const;

    /// @see GCFPValue::getSize()
    unsigned int getConcreteSize() const { return sizeof(_length) + _length; }
    
private: // Private attributes
    /// The value (buffer)
    unsigned char* _value;
    /// length of the buffer
    uint16 _length;
    /**
     * This boolean indicates wether the buffer space for value is newed in this
     * class or not. The "caller" of the constructor or the first of the setValue 
     * methods can specify wether the data must be cloned (newed and copied) or 
     * only the pointer needed to be copied.
     * The clone() and the second setValue methods clones always.
     */ 
    bool  _isDataHolder;
};
  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif
