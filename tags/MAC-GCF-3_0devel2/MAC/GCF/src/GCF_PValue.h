//#  GCF_PValue.h: abstract class for all MAC types
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

#ifndef GCF_PVALUE_H
#define GCF_PVALUE_H

#include <GCF/GCF_Defines.h>

/**
   This is the abstract value type class, which will be used to transport values 
   through a whole MAC application in a generic way. Instances of 
   specialisations of this class will normally used to hold values in local 
   properties or transport property values from PVSS to the MAC application or 
   visa versa.
*/

class GCFPValue
{
  public:
    /**
     * The enumeration of possible MAC property types
     * In case a dynamic array will be used the type ID enumeration starts on 
     * 0x80.
     */
    enum TMACValueType {NO_LPT, LPT_BOOL, LPT_CHAR, LPT_UNSIGNED, LPT_INTEGER, 
                    LPT_BIT32, LPT_BLOB, LPT_REF, LPT_DOUBLE, LPT_DATETIME,
                    LPT_STRING, LPT_DYNARR = 0x80,
                    LPT_DYNBOOL, LPT_DYNCHAR, LPT_DYNUNSIGNED, LPT_DYNINTEGER, 
                    LPT_DYNBIT32, LPT_DYNBLOB, LPT_DYNREF, LPT_DYNDOUBLE, LPT_DYNDATETIME,
                    LPT_DYNSTRING };
    /**
     * The constructor
     * Sets the type ID for each subclassed property value type class
     * @param type MAC property type ID
     */
    GCFPValue (TMACValueType type) : _type(type) {};
    
    /**
     * The destructor
     */
    virtual ~GCFPValue () {};

    /** 
     * Returns MAC type ID.
     * @return MAC type ID 
     */
    inline const TMACValueType& getType () const {return _type;}

    /** 
     * Pure virtual method
     * @return a hard copy of this object 
     * <b>IMPORTANT: must be deleted by "user" of this method</b>
     */
    virtual GCFPValue* clone () const = 0;

    /** 
     * Pure virtual method
     * Copys the arguments value to the affected object
     * @param value value to be copied into <b>this</b> object 
     * @return GCF_DIFFERENT_TYPES if type of <b>value<b> is different to 
     * <b>this</b> object. Otherwise GCF_NO_ERROR.
     */
    virtual TGCFResult copy (const GCFPValue& value) = 0;
    
    /** 
     * Pure virtual method
     * Sets a value to the affected object by means of a string buffer. 
     * This value will be translated by the concrete subclassed value type class.
     * @param value value to be translated to value of <b>this</b> object 
     * @return GCF_VALUESTRING_NOT_VALID if <b>value<b> could not be translated 
     * to the value of <b>this</b> object. Otherwise GCF_NO_ERROR.
     */
    virtual TGCFResult setValue (const string value) = 0;

    /** 
     * Static method
     * Creates a property value object of MAC type <b>type<b>
     * @param type property type to created
     * @return pointer to created property value type object
     * <b>IMPORTANT: must be deleted by "user" of this method</b>
     */
    static GCFPValue* createMACTypeObject (TMACValueType type);

    static GCFPValue* unpackValue (const char* valBuf, unsigned int maxBufSize);

    virtual unsigned int unpack(const char* valBuf, unsigned int maxBufSize) = 0;

    virtual unsigned int pack(char* valBuf, unsigned int maxBufSize) const = 0;
   
  protected:
    unsigned int unpackBase(const char* valBuf, unsigned int maxBufSize);
 
    unsigned int packBase(char* valBuf, unsigned int maxBufSize) const;
 
  private: // private data members
    /** Holds MAC property value type ID*/
    TMACValueType _type;
};

#endif
