//# DataHolder.h: Abstract base class for the data holders
//#
//# Copyright (C) 2000, 2001
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

#ifndef TRANSPORT_DATAHOLDER_H
#define TRANSPORT_DATAHOLDER_H

// \file
// Abstract base class for the data holders

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Blob/BlobFieldSet.h>
#include <Blob/BlobHeader.h>
#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

// \addtogroup Transport
// @{

//# Forward Declarations
class BlobOStream;
class BlobIStream;
class DataBlobExtra;

/**
  Class DataHolder is the abstract base class for all data holders
  in the CEPFrame environment. Its main purpose is to offer a common interface
  to the data and common functionality to derived classes.
  The data (defined in derived classes) is stored in a blob.

  The blob is constructed from the fields added using the addField function.
  It is possible to specify the alignment of a field. By default a field is
  aligned on its element size with a maximum of 8 (e.g. a single precision
  complex is aligned on 8 bytes). The alignment must be a power of 2.
  The blob buffer containing all fields is aligned on the maximum of the
  alignment of its fields, so it is ensured that the data of a fields are
  aligned properly in memory.

  Prove \code list<table> \endcode or \<table\>.
  \code
    main()
    {
       list<table> l;
    }
  \endcode
*/

class DataHolder
{
friend class DataBlobExtra;

public:
  // Construct a DataHolder with a default name, type and version.
  DataHolder (const string& name="aDataHolder",
	      const string& type="DH",
	      int version=0);

  // Destructor
  virtual ~DataHolder();

  // Initialization. The default implementation does nothing.
  virtual void init();   

  // Make a copy
  virtual DataHolder* clone() const = 0;

  // Pack the data (use before writing)
  virtual void pack();

  // Unpack the data (use after reading). Checks and converts it as needed.
  virtual void unpack();

  // Dump the DataHolder contents to cout.
  // The default implementation does nothing.
  virtual void dump() const;

  // Get data size (in bytes);
  int getDataSize() const;

  // Get a pointer to the data (the beginning of the blob).
  void* getDataPtr() const;

  // Get the size of a blob header.
  uint getHeaderSize() const;

  // Extract the size from the blob header in the buffer.
  static uint getDataLength (const void* buffer);

  // Resize the buffer to the given size (if needed).
  void resizeBuffer (uint newSize);

  // Does this DataHolder have a fixed size?
  bool hasFixedSize();

  // Get the type of the DataHolder.
  const string& getType() const;

  // Set the type of the DataHolder.
  void setType (const string& type);

  // Get the name of the DataHolder.
  const string& getName() const;

  // Set the name of the DataHolder.
  void setName (const string& name);

  // Set maximum data size.
  // If used, it should be called before initialization.
  // Normally this is not needed, but for TH_ShMem transports it is useful,
  // because a buffer in TH_ShMem cannot grow. For other TransportHolders
  // it is not necessary, but can be useful in some cases.
  // It should be used (with TH_ShMem) if the data in the blob
  // is variable sized and/or if an extra blob is added.
  // <br>nbytes=0 means no maximum.
  // <br>isAddMax=true means that the given size (nbytes) is additive,
  // so the maximum data size is the size of the main data blob plus nbytes.
  // <br>The default maximum data size is 0, but for TransportHolders not
  // supporting growable data, it is the size of all data fields.
  void setMaxDataSize (uint nbytes, bool isAddMax=false);

  // Get the MAXIMUM data block size supported (in bytes).
  // 0 means no maximum.
  int getMaxDataSize() const;

  // Tell the DataHolder the properties how the buffer allocation is done.
  void setAllocationProperties (bool		dataCanGrow,
				BlobStringType	aBlobType);

  // Get the version of this DataHolder.
  int getVersion();

  // Check if this DataHolder had been initialized.
  bool isInitialized();

  // \name Get access to the data blob.
  // <group>
  BlobString& getDataBlock();                    // Used by PO_DH_PL
  const BlobString& getDataBlock() const;
  // </group>

protected:
  // Copy DataHolder
  DataHolder(const DataHolder&);

  // \name Add a field to the data block definition.
  // Optionally a (unique) name can be given to the field.
  // It is possible to specify the alignment; 0 means use default
  // alignment which is the length of a field element.
  // It returns the index of the field.
  // <group>
  uint addField (const BlobFieldBase&, uint alignment=0);
  uint addField (const std::string& fieldName, const BlobFieldBase&,
		 uint alignment=0);
  // </group>

  // \name Setup the data block.
  // This function needs to be called only once (in preprocess) if
  // all data fields have a fixed shape.
  // It fields have a variable shape, the function has to be called again
  // (in process) when a shape changes.
  // <group>
  void createDataBlock();
  void openDataBlock();
  // </group>

  // \name Get access to the BlobField.
  // It makes it possible to get the shape of a variable shaped input array
  // or to set the shape of a variable shaped output array.
  // <group>
  BlobFieldBase& getDataField (uint fieldIndex);
  BlobFieldBase& getDataField (const std::string& fieldName);
  // </group>

  // \name Get a pointer to a data field in the blob
  // <group>
  // Get a pointer by index.
  template<typename T> T* getData (uint fieldIndex);
  // Get a pointer by name.
  template<typename T> T* getData (const std::string& fieldName);
  // </group>

  // If told in the DataHolder constructor, it is possible to add
  // arbitrary fields to the data blob using the ordinary operator<<
  // functions on blob streams. Operator>> functions can be used to read them.
  // An exception is thrown if these functions are used without telling the
  // constructor that 
  // Tell that the extra data block will be used.
  void setExtraBlob (const string& name, int version);

 // Initialize the extra output blob holding arbitrary fields.
  // The return reference can be used to store the fields in.
  // It is meant for DataHolders writing data.
  BlobOStream& createExtraBlob();
  // Clear the extra blob output buffer.
  // This is needed, because an extra blob is kept until overwritten.
  void clearExtraBlob();
  // Get read access to the extra blob last created or read.
  // If a created blob is used, only the data written so far can be accessed.
  // <br>found=false is set if there is no extra blob. The first version
  // throws an exception if there is no extra blob.
  // <group>
  BlobIStream& getExtraBlob();
  BlobIStream& getExtraBlob (bool& found, int& version);
  // </group>

  // Get the data field set.
  BlobFieldSet& dataFieldSet();

  // Initialize the data field set.
  void initDataFields();

  //  // The basePostprocess method cleans up the DataHolder.
  //  void basePostprocess();          // TBD: Is this needed?

private:
  // Get the type of BlobString needed from the transport holder.
  virtual BlobStringType getBlobStringType();

  // Put the extra data block into the main data blob.
  // If possible and needed the buffer is resized.
  // If resized, the data pointers are refilled.
  void putExtra (const void* data, uint size);

  // Let the derived class fill its pointers to the data in the blob.
  // This function is called when the blob is created and when its layout
  // has changed.
  // The default implementation does nothing.
  virtual void fillDataPointers();

  BlobFieldSet    itsDataFields;
  BlobString*     itsData;
  BlobOBufString* itsDataBlob;
  BlobStringType* itsBlobType;
  int             itsMaxDataSize;   //# <0 is not filled in
  bool		  itsDataCanGrow;
  string          itsName;
  string          itsType;
  int             itsVersion;
  int             itsReadConvert;   //# data conversion needed after a read?
                                    //# 0=no, 1=yes, else=not known yet
  DataBlobExtra*  itsExtraPtr;
  bool            itsInitialized;
};


inline void DataHolder::setMaxDataSize (uint nbytes, bool isAddMax)
  { itsMaxDataSize = nbytes; itsDataCanGrow = !isAddMax; }

inline int DataHolder::getDataSize() const
  { return itsData->size(); }

inline void* DataHolder::getDataPtr() const
  { return itsData->data(); }

inline bool DataHolder::hasFixedSize()
  { return (itsDataFields.hasFixedShape()  && itsDataFields.version() == 1
	    && itsExtraPtr == 0);  }

inline const string& DataHolder::getName() const
  { return itsName; }

inline void DataHolder::setName (const string& name)
  { itsName = name; }

inline const string& DataHolder::getType () const
  { return itsType; }

inline void DataHolder::setType(const string& type)
  { itsType = type; }

inline int DataHolder::getVersion()
  { return itsVersion; }

inline BlobFieldSet& DataHolder::dataFieldSet()
  { return itsDataFields; }

inline BlobFieldBase& DataHolder::getDataField (uint fieldIndex)
{
  return itsDataFields[fieldIndex];
}
inline BlobFieldBase& DataHolder::getDataField (const std::string& fieldName)
{
  return itsDataFields[fieldName];
}

template<typename T>
inline T* DataHolder::getData (uint fieldIndex)
{
  return itsDataFields[fieldIndex].getData<T> (*itsDataBlob);
}
template<typename T>
inline T* DataHolder::getData (const std::string& fieldName)
{
  return itsDataFields[fieldName].getData<T> (*itsDataBlob);
}

inline BlobString& DataHolder::getDataBlock()
{
  return *itsData;
}
inline const BlobString& DataHolder::getDataBlock() const
{
  return *itsData;
}

inline uint DataHolder::getHeaderSize() const
{
  return sizeof(BlobHeader);
}

inline uint DataHolder::getDataLength (const void* buffer)
{
  return static_cast<const BlobHeader*>(buffer)->getLength();
}

inline bool DataHolder::isInitialized()
{
  return itsInitialized;
}

inline void DataHolder::setAllocationProperties(bool  		dataCanGrow,
					        BlobStringType aBlobType)
{
  DBGASSERTSTR (!itsInitialized, "Allocation Properties are already set");

  itsDataCanGrow = dataCanGrow;
  itsBlobType    = new BlobStringType(aBlobType);
  itsInitialized = true;
}

// @} // Doxygen endgroup Transport

} // end namespace

#endif
