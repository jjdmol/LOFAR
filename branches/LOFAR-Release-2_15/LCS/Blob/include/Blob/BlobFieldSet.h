//# BlobFieldSet.h: Definition all fields in a blob
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BLOB_BLOBFIELDSET_H
#define LOFAR_BLOB_BLOBFIELDSET_H

// \file
// Definition all fields in a blob

#include <Blob/BlobField.h>
#include <Blob/BlobIBufChar.h>
#include <Common/LofarLogger.h>
#include <vector>
#include <string>
#include <map>

namespace LOFAR {

  // \ingroup %pkgname%
  // <group>

  //# Forward Declarations.
  class BlobOStream;
  class BlobIStream;
  
  // The class BlobFieldSet defines a set of fields in a blob. The blob can
  // be created and thereafter a pointer to the various fields in the blob
  // can be obtained. In this way it is possible to define a consecutive 
  // data structure as a blob that can, for instance, be sent directly in
  // the CEPFrame framework.
  //
  // In a similar way BlobFieldSet can be used to interpret an input blob
  // created via a BlobFieldSet. If it is known that each time the blob has
  // exactly the same structure, the interpretation needs to be done only once.
  //
  // The fields in the blob are defined by the templated class BlobField.
  // It is possible to have multiple versions of a BlobFieldSet. That is to say:
  // over time fields can be added to a BlobFieldSet making it possible that
  // a field is missing in older sets retrieved from, say, a data base.
  // This is done by defining a version in the BlobField. The original fields
  // should have version 1. Fields added later on should get a higher version.
  // The version of a BlobFieldSet is the highest version of its fields.
  // In this way the openBlob function is able to recognize that a field is
  // missing in older blobs.
  //
  // Fields to a set are added using the add function and they can be found
  // using its index in operator []. Some or all fields can be given a unique
  // name making it possible to find the field by name.
  // A field will be aligned on the implicitly or explicitly given
  // alignment. It is important that a set is constructed in the same way
  // on the reader and the writer side.
  //
  // The blob has to created (in memory) using the createBlob function.
  // Thereafter pointers to the various data in the BlobOBufChar buffer object
  // (or in a derived class) can be obtained using the templated
  // BlobFieldBase::getData function. If the structure of a blob does not
  // change (thus the same array shapes, etc.), the same buffer can be used
  // over and over again without having to do createBlob and getData again..
  // <br>When a blob is received, it can be read from a BlobIBufChar buffer
  // object using the openBlob function. Again pointers to data can thereafter
  // be obtained using getData. Also, openBlob and getData do not need to be
  // done if the structure of received blobs does not change.
  // <br>It is possible to use BlobFieldSet::checkHeader to check if the
  // header and possibly size of the blob is as expected. It is a good idea
  // to do that certainly in debug mode.
  // <br>Because the data format of a received blob is usually unknown,
  // it is needed to call convertData on each received blob to do the
  // possibly needed data conversion (which is done in place).
  
  class BlobFieldSet
    {
    public:
      // Construct for the given blob type.
      // The blob version is the highest version of all its fields.
      explicit BlobFieldSet (const std::string& name);
      
      // Copy constructor (copy semantics).
      BlobFieldSet (const BlobFieldSet& that);
      
      // Assignment (copy semantics).
      BlobFieldSet& operator= (const BlobFieldSet& that);
      
      // Destructor.
      ~BlobFieldSet();
      
      // Has the set a fixed shape?
      // It has if all its fields have a fixed shape.
      bool hasFixedShape() const
	{ return itsHasFixedShape; }
      
      // Add an unnamed field to the definition and return its index.
      // It is possible to specify the alignment; 0 means use default
      // alignment which is the length of a field element.
      int add (const BlobFieldBase& field, uint alignment=0);
      
      // Add a named field to the definition and return its index.
      // The name has to be unique.
      // It is possible to specify the alignment; 0 means use default
      // alignment which is the length of a field element.
      int add (const std::string& name, const BlobFieldBase& field,
	       uint alignment=0);
  
      // Get the version of the set.
      uint version() const
	{ return itsVersion; }

      // Get the alignment of the set.
      uint getAlignment() const
        { return itsAlignment; }

      // Find the length of the blob to be created.
      uint64 findBlobSize();
      
      // Create the blob for output.
      // It defines the offsets of the data in the various fields.
      // Pointers to the data can be retrieved using BlobFieldBase::getData.
      // <group>
      // Append the blob to the existing stream. The stream should use
      // a memory buffer.
      void createBlob (BlobOStream& bs);
      // Put the blob in the buffer (that is first resized to zero).
      void createBlob (BlobOBufChar&);
      // </group>
      
      // Put an extra blob at the end of the data blob in outbuf.
      // The extra blob should be contained in inbuf.
      // The length of the blob is given by size. It can be 0.
      // If size>0, it is checked if the length of the blob in inbuf matches size.
      void putExtraBlob (BlobOBufChar& outbuf, const void* inbuf, uint64 size);
      
      // Get the extra blob from the input.
      // The size of the returned buffer is the size of the extra blob.
      BlobIBufChar getExtraBlob (BlobIBufChar& inbuf);
      
      // Read a blob held in the buffer.
      // It finds the offsets of the data in the various fields in the blob.
      // Pointers to the data can be retrieved using BlobFieldBase::getData.
      void openBlob (BlobIBufChar&);
      
      // Get the i-th field in the set.
      // Note that the add function returns the index of a field.
      // <group>
      BlobFieldBase& operator[] (int i)
	{ return *(itsFields[i]); }
      const BlobFieldBase& operator[] (int i) const
	{ return*(itsFields[i]); }
      // </group>
      
      // Get the field with the given name.
      // An exception is thrown if the name is unknown.
      // <group>
      BlobFieldBase& operator[] (const std::string& name);
      const BlobFieldBase& operator[] (const std::string& name) const;
      // </group>
      
      // Check the header of the blob in the buffer and throw an exception
      // in case of a mismatch.
      // <br>It always checks the magic value.
      // <br>If the object type is given (no null pointer), the object type and the
      // version are checked.
      // Version<0 means that its absolute value has to match exactly.
      // Version>=0 means that its value has to be >= the version in
      // the blob (indicating that up to version is supported).
      // <br>Size is checked if given as > 0.
      // It returns if it is needed to convert the data.
      static bool checkHeader (BlobIBufChar& buf, const char* objectType,
			       int version, uint64 size);
      
      // Convert all fields in the buffer to the local format.
      // The conversion is done in place.
      void convertData (BlobIBufChar& buf) const;
      
    private:
      typedef std::map<std::string,int> NameMap;
      
      std::string                 itsName;
      uint                        itsVersion;
      uint                        itsAlignment;
      bool                        itsHasFixedShape;
      uint64                      itsNormalSize;
      std::vector<BlobFieldBase*> itsFields;
      NameMap                     itsNameMap;
      ALLOC_TRACER_CONTEXT
    };

// </group>

} // end namespace

#endif
