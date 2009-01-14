//#  GCF_PValue.h: abstract class for all MAC types
//#
//#  Copyright (C) 2002-2007
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

#include <GCF/PVSS/GCF_Defines.h>
#include <Common/DataFormat.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

class GCFPVDynArr;

// This is the abstract value type class, which will be used to transport values 
// through a whole MAC application in a generic way. Instances of 
// specialisations of this class will normally used to hold values in local 
// properties or transport property values from PVSS to the MAC application or 
// visa versa.

class GCFPValue
{
public:

	// The destructor
	virtual ~GCFPValue () {};

	// Returns MAC type ID.
	// @return MAC type ID 
	const TMACValueType& getType () const {return _type;}

	// Pure virtual method
	// @return a hard copy of this object 
	// <b>IMPORTANT: must be deleted by "user" of this method</b>
	virtual GCFPValue* clone () const = 0;

	// Pure virtual method
	// Copys the arguments value to the affected object
	// @param value value to be copied into <b>this</b> object 
	// @return GCF_DIFFERENT_TYPES if type of <b>value<b> is different to 
	// <b>this</b> object. Otherwise GCF_NO_ERROR.
	virtual TGCFResult copy (const GCFPValue& value) = 0;

	// Assignment of value object does the same as <b>this</b> copy
	GCFPValue& operator= (const GCFPValue& value) { copy(value); return *this; }

	// Pure virtual method
	// Check if the value of both objects is the same.
	virtual bool operator== (const GCFPValue& that) const = 0;
	virtual bool operator!= (const GCFPValue& that) const = 0;

	// Pure virtual method
	// Sets a value to the affected object by means of a string buffer. 
	// This value will be translated by the concrete subclassed value type class.
	// @param value value to be translated to value of <b>this</b> object 
	// @return GCF_VALUESTRING_NOT_VALID if <b>value<b> could not be translated 
	// to the value of <b>this</b> object. Otherwise GCF_NO_ERROR.
	virtual TGCFResult setValue (const string& value) = 0;

	// returns the value of the concrete type class in a string
	// the default format is like the expected format for the setValue(string) method
	virtual string getValueAsString(const string& format = "") const = 0;

	// Static method
	// Creates a property value object of MAC type <b>type</b>
	// @param type property type to created
	// @return pointer to created property value type object
	// <b>IMPORTANT: must be deleted by "user" of this method</b>
	static GCFPValue* createMACTypeObject (TMACValueType type);

	// Static method
	// Creates a property value object of MAC type <b>type</b> based on the data
	// passed in the parameter <b>valBuf</b>
	// @param valBuf buffer data containing a MAC value, which is packet with <b>pack</b>
	// @return pointer to created property value type object
	// <b>IMPORTANT: must be deleted by "caller" of this method</b>
	static GCFPValue* unpackValue (const char* valBuf);

	// unpacks (copies) the data of the value into the object data space
	// for now it only unpacks the type; later it also can unpack a timestamp or else
	// calls the unpackConcrete method to unpack the concrete data to the specific 
	// value type object data space
	// @param valBuf buffer with the data 
	// @return number of unpacked bytes 
	virtual unsigned int unpack(const char* valBuf);

	// packs (copies) the data of the value object into a buffer
	// for now it only packs the type; later it also can pack a timestamp or else
	// calls the packConcrete method to pack the concrete data of a specific 
	// value type object
	// @param valBuf buffer space in which the data can be stored
	// @return number of packed bytes
	virtual unsigned int pack(char* valBuf) const;

	// size of value object if it would be packed
	// calls the getConcreteSize method, which returns the concrete size of the
	// specific value object data
	// @return size of the object
	virtual unsigned int getSize() const { return 2 + getConcreteSize();}


	// @return true if local dataformat (ENDIANES) differes to the dataformat of this value
	virtual bool mustConvert() { return (_dataFormat != LOFAR::dataFormat()); }

	/// provides the possibility to change the dataformat of a value
	void setDataFormat(LOFAR::DataFormat dfmt = LOFAR::dataFormat()) {_dataFormat = dfmt; }

	virtual string getTypeName() const;

protected:
	friend class GCFPVDynArr;
	// The constructor
	// Sets the type ID for each subclassed property value type class
	// @param type MAC property type ID
	explicit GCFPValue (TMACValueType type) : _type(type), _dataFormat(LOFAR::dataFormat()) {};

	// Pure virtual method
	// the concrete unpack method of the concrete value object
	// @see unpack
	virtual unsigned int unpackConcrete(const char* valBuf) = 0;

	// Pure virtual method
	// the concrete pack method of the concrete value object
	// @see pack
	virtual unsigned int packConcrete(char* valBuf) const = 0;

	// Pure virtual method
	// the concrete getSize method of the concrete value object
	// @see getSize
	virtual unsigned int getConcreteSize() const = 0;        

private:
	// Don't allow copying this object.
	GCFPValue (const GCFPValue&);
	GCFPValue();

private: 
	// ----- data members -----
	TMACValueType       _type;		 // Holds MAC property value type ID
	LOFAR::DataFormat   _dataFormat; // Holds the dataformat (ENDIANES) of this value

};

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif
