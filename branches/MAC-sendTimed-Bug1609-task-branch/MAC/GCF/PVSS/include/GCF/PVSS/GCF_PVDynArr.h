//#  GCF_PVDynArr.h: MAC dynamic array property type
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

#ifndef GCF_PVDYNARR_H
#define GCF_PVDYNARR_H

#include <GCF/PVSS/GCF_PValue.h>
#include <Common/lofar_vector.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PVSS 
  {
typedef vector<GCFPValue*> GCFPValueArray;

/**
 * By means of this complex property type a dynamic list of property values of 
 * the same simple type can be managed.
 */

class GCFPVDynArr : public GCFPValue
{
  public:
    explicit GCFPVDynArr(TMACValueType itemType);
  	GCFPVDynArr(TMACValueType itemType, const GCFPValueArray& val);

    GCFPVDynArr (const GCFPVDynArr& valArray) : GCFPValue(valArray.getType())
      { setValue(valArray.getValue()); }

  	virtual ~GCFPVDynArr();

    /** Changes the value of this object */
    virtual void setValue(const GCFPValueArray& newVal);

    /** 
     * Changes the value of this object by means of a stringbuffer, 
     * which will be translated.
     * NOT YET IMPLEMENTED
     * @see GCFPValue::setValue(const string value)
     */
    virtual TGCFResult setValue(const string& value);

    // Returns the value of this object in a string
    virtual string getValueAsString(const string& format = "") const;

    /** Returns the value of this object*/
    virtual const GCFPValueArray& getValue() const {return _values;}

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
    unsigned int getConcreteSize() const;
    
  private: // help members
    /** cleanup the array item objects */
    void cleanup();
    
  private: // Private attributes
    /**  The values*/
    GCFPValueArray _values;
};

//# ---------- inline functions ----------
inline bool GCFPVDynArr::operator==(const GCFPValue&	that) const {
	return ((that.getType() == getType()) && (getValue() == ((GCFPVDynArr *) &that)->getValue()));
}

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif
