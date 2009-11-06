//# BlobField.h: Definition of a field in a blob
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BLOB_BLOBFIELD_H
#define LOFAR_BLOB_BLOBFIELD_H

// \file
// Definition of a field in a blob

#include <Common/LofarTypes.h>
#include <Common/TypeNames.h>
#include <Common/DataFormat.h>
#include <vector>

namespace LOFAR {

  // \ingroup %pkgname%
  // <group>
  
  //# Forward Declarations.
  class BlobOStream;
  class BlobIStream;
  class BlobOBufChar;
  class BlobIBufChar;
  
  // The class BlobFieldSet defines a set of fields in a blob. The blob can
  // be created and thereafter a pointer to the various fields in the blob
  // can be obtained. More information can be found in the description of
  // BlobFieldSet.
  //
  // The templated BlobField class defines a single field in a set.
  // The class BlobFieldBase serves as an abstract base class for BlobField
  // and implements some common functions.
  
  class BlobFieldBase
    {
    public:
      // Construct for a scalar.
      explicit BlobFieldBase (uint version);
      
      // Construct for an array with a possible predefined shape.
      // The shape can be left undefined or partly undefined, after which the
      // actual shape has to be set at a later time using setShape.
      // Shape length > 0 means that the dimensionality is fixed.
      // A size>0 means that the given dimension has a fixed size.
      // <br>setShape will check if the fixed sizes match.
      // <group>
      BlobFieldBase (uint version, uint32 size0);
      BlobFieldBase (uint version, uint32 size0, uint32 size1,
		     bool fortranOrder = true);
      BlobFieldBase (uint version, uint32 size0, uint32 size1, uint32 size2,
		     bool fortranOrder = true);
      BlobFieldBase (uint version, uint32 size0, uint32 size1, uint32 size2,
		     uint32 size3,
		     bool fortranOrder = true);
      BlobFieldBase (uint version, const std::vector<uint32>& shape,
		     bool fortranOrder = true);
      BlobFieldBase (uint version, const uint32* shape, uint16 ndim,
		     bool fortranOrder = true);
      // </group>
      
      virtual ~BlobFieldBase();
      
      // Make a copy of the derived object.
      virtual BlobFieldBase* clone() const = 0;
      
      // Is the field a scalar? Otherwise it is an array.
      bool isScalar() const
	{ return itsIsScalar; }
      
      // Has the field definition a fixed shape?
      // It has if all axes have a fixed size.
      bool hasFixedShape() const
	{ return itsHasFixedShape; }
      
      // Get the actual number of elements (1 for a scalar).
      uint getNelem() const
	{ return itsNelem; }
      
      // Get the actual shape of the field.
      // An empty vector is returned for a scalar.
      const std::vector<uint32>& getShape() const
	{ return itsShape; }
      
      // Set the actual shape of an array.
      // It checks if the shape matches possible fixed shape parts in the field
      // definition and throws an exception if not.
      void setShape (const std::vector<uint32>&);
      
      // Is an array in Fortran order or in C order?
      bool isFortranOrder() const
	{ return itsFortranOrder; }
      
      // Get the version of the field.
      uint getVersion() const
	{ return itsVersion; }
      
      // Use a blob header when storing the array in a blob?
      // The constructor sets it by default to no header.
      // <group>
      bool useBlobHeader() const
	{ return itsUseHeader; }
      void setUseBlobHeader (bool useBlobHeader)
	{ itsUseHeader = useBlobHeader; }
      // </group>
      
      // Get a pointer to the data in the output blob.
      // This function should be called after BlobFieldSet::createBlob.
      template<typename T> T* getData (BlobOBufChar& buf)
	{
#ifdef ENABLE_DBGASSERT
	  return static_cast<T*> (getOData (typeName((T*)0), buf));
#else
	  return static_cast<T*> (getOData (buf));
#endif
	}
      
      // Get a pointer to the data in the input blob.
      // This function should be called after BlobFieldSet::openBlob.
      // A null pointer is returned if the field does not exist which can happen
      // if the version of the field is higher than the version of the blob.
      template<typename T> const T* getData (BlobIBufChar& buf) const
	{
#ifdef ENABLE_DBGASSERT
	  return static_cast<const T*> (getIData (typeName((T*)0), buf));
#else
	  return static_cast<const T*> (getIData (buf));
#endif
	}
 
      // Set the required field alignment (on a multiple of nbytes bytes).
      // Usually this function is only used by BlobFieldSet::add.
      void setAlignment (uint nbytes)
        { itsAlignment = nbytes; }

      // Get the alignment.
      uint getAlignment() const
        { return itsAlignment; }

      // Return the greatest power of 2 that divides the value.
      // It is used to make the default alignment a power of 2.
      // It the result exceeds the maximum, it is set to the maximum.
      static uint p2divider (uint value, uint maxval=8);

      // Helper functions for BlobFieldSet.
      // <group>
      // Reserve space for this field in an output blob and set its offset.
      void setSpace (BlobOStream& bs)
	{ setOSpace (bs); }
      // Set the offset of this field in an input blob.
      // The version of the field can be higher than the blob version meaning
      // that an old blob is used in which this field does not exist. In that
      // case the offset is set to -1.
      void getSpace (BlobIStream&, uint version);
      // Convert this field in the buffer from the given to the local format.
      // The conversion is done in place.
      virtual void convertData (BlobIBufChar& buf,
				LOFAR::DataFormat fmt) const = 0;
      // </group>
      
    protected:
      // Helper functions for derived classes.
      // <group>
      void setOffset (int64 offset, int64 arrayOffset)
	{ itsOffset = offset; itsArrayOffset = arrayOffset; }
      int64 getOffset() const
	{ return itsOffset; }
      int64 getArrayOffset() const
	{ return itsArrayOffset; }
      bool& rwFortranOrder()
	{ return itsFortranOrder; }
      // </group>
      
    private:
      // Helper functions for this class.
      // <group>
      void init();
      void fillNelem();
      virtual void setOSpace (BlobOStream&) = 0;
      virtual void getISpace (BlobIStream&) = 0;
      virtual void* getOData (BlobOBufChar&) const = 0;
      virtual const void* getIData (BlobIBufChar&) const = 0;
      virtual void* getOData (const std::string&,
			      BlobOBufChar&) const = 0;
      virtual const void* getIData (const std::string&,
				    BlobIBufChar&) const = 0;
      // </group>
      
      int64               itsOffset;
      int64               itsArrayOffset;     // offset of array header
      uint                itsVersion;
      uint                itsNelem;
      std::vector<uint32> itsShapeDef;
      std::vector<uint32> itsShape;
      bool                itsFortranOrder;
      bool                itsIsScalar;
      bool                itsUseHeader;
      bool                itsHasFixedShape;
      uint                itsAlignment;
    };
  
  
  // The templated BlobField class defines a field in a BlobFieldSet.
  // The template parameter defines the data type of the field.
  // All basic data types (including fcomplex and dcomplex) can be used.
  // However, bool cannot be used because bools are stored as bits in blobs.
  // Furthermore POD types can be used as template parameter. A POD is a
  // plain old data type, thus a struct or class containing contiguous data
  // (thus no pointers to other data).
  // This means that std::string cannot be used as template parameter.
  //
  // A field can be a scalar or an array of any dimensionality.
  // Care is taken that the field is aligned properly when the blob is created.
  // By default a field is aligned on a multiple of its basic element size
  // (e.g. 8 bytes for a double), but it is possible to specify the alignment
  // explicitly (which has to be a power of 2). E.g. to be able to use SSE
  // instructions an alignment of 16 should be specified explicitly.
  // The basic element size is the greatest divider of the element size
  // that is a power of 2. E.g. for an array of structs containing 3 floats,
  // the element size is 12, but the basic element size is 4.
  // For a struct containing 2 floats the basic element size would be 8.
  // The default alignment is maximum 8.
  //
  // Because a blob can be used to exchange data between different systems,
  // it must be possible to convert data as needed (for instance from big to
  // little endian). This is done by the dataConvert functions. These functions
  // exist for all basic types, but not for user defined types. So if a POD type
  // is used as template parameter, a proper dataConvert function has to be
  // implemented in the namespace LOFAR. For instance for a struct XX:
  // <srcblock>
  // struct X
  // {
  //   X(int i, float f) : f1(i), f2(f) {};
  //   bool operator==(const X& that) const {return f1==that.f1 && f2==that.f2;}
  //   int32 f1;
  //   float f2;
  // };
  //
  // namespace LOFAR {
  //   // Convert XX from given format to local format.
  //   void dataConvert (DataFormat fmt, X* buf, uint nrval)
  //   {
  //     for (uint i=0; i<nrval ;i++) {
  //       dataConvert32 (fmt, &(buf[i].f1));
  //       dataConvertFloat (fmt, &(buf[i].f2));
  //     }
  //   }
  // }
  // </srcblock>
  // Note that in the example above the data type int32 is used in class XX.
  // This is done to ensure that on all possible machines 32 bits are used
  // for this integer.
  //
  // Because class BlobField is templated and because its implementation is
  // not included in the .h file, the template has to be
  // instantiated manually. It can be done by putting the following lines
  // <srcblock>
  // //# Needed for instantiating BlobField<XX>
  // #include <Blob/BlobField.tcc>
  // template class BlobField<XX>;
  // </srcblock>
  // in XX.cc or in a separate .cc file.
  
  template<typename T>
    class BlobField : public BlobFieldBase
    {
    public:
      // Define a scalar.
      explicit BlobField (uint version);
      
      // Define a vector.
      BlobField (uint version, uint32 size0);
      
      // Define a 2-dim array with axes by default in Fortran order.
      // The default alignment is set to the greatest power of 2 that
      // divided sizeof(T).
      // E.g. for a struct containing 3 floats, the default alignment is 4.
      // For a struct containing 2 floats the default alignment is 8.
      BlobField (uint version, uint32 size0, uint32 size1,
		 bool fortranOrder = true);
      BlobField (uint version, uint32 size0, uint32 size1, uint32 size2,
		 bool fortranOrder = true);
      BlobField (uint version, uint32 size0, uint32 size1, uint32 size2,
		 uint32 size3,
		 bool fortranOrder = true);
      BlobField (uint version, const std::vector<uint32>& shape,
		 bool fortranOrder = true);
      BlobField (uint version, const uint32* shape, uint16 ndim,
		 bool fortranOrder = true);
      
      virtual ~BlobField();
      
      virtual BlobFieldBase* clone() const;
      
      // Reserve space in the BlobOStream.
      virtual void setOSpace (BlobOStream&);
      
      // Get the space from the BlobIStream.
      virtual void getISpace (BlobIStream&);
      
      // Get the pointer to the data in the buffer.
      // <group>
      virtual void* getOData (BlobOBufChar&) const;
      virtual const void* getIData (BlobIBufChar&) const;
      // </group>
      
      // Get the pointer to the data in the buffer.
      // It checks if the data type is correct.
      // <group>
      virtual void* getOData (const std::string&, BlobOBufChar&) const;
      virtual const void* getIData (const std::string&, BlobIBufChar&) const;
      // </group>
      
      // Convert this field in the buffer from the given to the local format.
      // The conversion is done in place.
      virtual void convertData (BlobIBufChar& buf,
				LOFAR::DataFormat fmt) const;
    };
  
  
  // Make sure BlobField cannot be used for a field of bools and strings.
  template <> class BlobField<bool> : public BlobFieldBase
    {
    private:
      BlobField() : BlobFieldBase(0) {};
    };
  
  template <> class BlobField<std::string> : public BlobFieldBase
    {
    private:
      BlobField() : BlobFieldBase(0) {};
    };
  
// </group>

} // end namespace

#endif
