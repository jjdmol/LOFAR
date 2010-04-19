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

#include <GCF/PVSS/GCF_PValue.h>
#include <Common/LofarTypes.h>

using LOFAR::TYPES::int32;

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PVSS 
  {
/**
 * By means of this property type a integer (-2,147,483,648 to 2,147,483,647) 
 * value can be used.
 */
class GCFPVInteger : public GCFPValue
{
  public:
  	explicit GCFPVInteger (int32 val = 0) : GCFPValue(LPT_INTEGER), _value(val) {;}
    GCFPVInteger(const GCFPVInteger& val) : GCFPValue(LPT_INTEGER), _value(val.getValue()) {;}

  	virtual ~GCFPVInteger () {;}
    
    /** Changes the value of this object */
    virtual void setValue (const int32 newVal) {_value = newVal;}

    /** 
     * Changes the value of this object by means of a stringbuffer, 
     * which will be translated.
     * @see GCFPValue::setValue(const string value)
     */
    virtual TGCFResult setValue (const string& value);

    // Returns the value of this object in a string
    virtual string getValueAsString(const string& format = "") const;

    /** Returns the value of this object*/
    virtual int32 getValue () const {return _value;}

    /** @see GCFPValue::clone() */
    virtual GCFPValue* clone () const;

    /** @see GCFPValue::copy() */
    virtual TGCFResult copy (const GCFPValue& value);
 
    /** @see GCFPValue::operator==() */
    virtual bool operator==(const GCFPValue& that) const;
    virtual bool operator!=(const GCFPValue& that) const { return (!(*this == that)); }
 
  private:
    /// @see GCFPValue::unpack()
    unsigned int unpackConcrete(const char* valBuf);

    /// @see GCFPValue::pack()
    unsigned int packConcrete(char* valBuf) const;

    /// @see GCFPValue::getSize()
    unsigned int getConcreteSize() const { return sizeof(int32); }
    
  private: // Private attributes
    /** The value */
    int32 _value;
};

//# ---------- inline functions ----------
inline bool GCFPVInteger::operator==(const GCFPValue&	that) const {
	return ((that.getType() == getType()) && (getValue() == ((GCFPVInteger *) &that)->getValue()));
}

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif
