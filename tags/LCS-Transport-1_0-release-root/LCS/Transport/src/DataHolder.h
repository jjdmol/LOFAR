///# DataHolder.h: Abstract base class for the data holders
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

#include <lofar_config.h>

//# Includes
#include <Transport/Transporter.h>
#include <Common/BlobFieldSet.h>
#include <Common/BlobHeader.h>
#include <Common/BlobString.h>
#include <Common/BlobOBufString.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

//# Forward Declarations
class BlobOStream;
class BlobIStream;
class DataBlobExtra;

/**
  Class DataHolder is the abstract base class for all data holders
  in the CEPFrame environment. Its main purpose is to offer a common interface
  to the data and common functionality to derived classes.
  The data (defined in derived classes) is stored in a blob.

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

  // Make a copy
  virtual DataHolder* clone() const = 0;

  // The preprocess method is called before process is done, thus
  // before any read or write is done.
  // It can be used to initialize the DataHolder.
  // The default implementation does nothing.
  void basePreprocess();
  virtual void preprocess();

  // The postprocess method is called after process is done.
  // It can be used to clean up the DataHolder.
  // The default implementation does nothing.
  void basePostprocess();
  virtual void postprocess();

  // Dump the DataHolder contents to cout.
  // The default implementation does nothing.
  virtual void dump() const;

  // Read the packet data.
  bool read();

  // Write the packet data.
  void write();

  // Is the Transporter of this DataHolder valid?
  bool isValid() const;

  // Connect to another DataHolder.
  // The data will flow from this object to thatDH.
  bool connectTo(DataHolder& thatDH, const TransportHolder& prototype,
		 bool blockingComm = true);

  // Connect to another DataHolder.
  // The data flow is bidirectional.
  bool connectBidirectional(DataHolder& thatDH, 
			    const TransportHolder& thisTH,
			    const TransportHolder& thatTH,
			    bool blockingComm = true);

  // Initialization (must be called after connect).
  void init();   

  // Getting, setting and comparing of a timestamp.
  void setTimeStamp (unsigned long aTimeStamp);
  unsigned long getTimeStamp() const;
  void copyTimeStamp (const DataHolder& that);
  void copyTimeStamp (const DataHolder* that);
  // Functions to deal with handling the timestamp 
  int compareTimeStamp (const DataHolder& that) const;

  // Set maximum data size.
  // If used, it should be called before preprocess.
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

  // Get data size (in bytes);
  int getDataSize() const;

  // Get a pointer to the data (the beginning of the blob).
  void* getDataPtr() const;

   // Set/get the ID
  void setID(int aID);
  int getID() const;

  // Get communication type
  bool isBlocking();

  // Get the type of the DataHolder.
  const string& getType() const;

  // Set the type of the DataHolder.
  void setType (const string& type);

  // Get the name of the DataHolder.
  const string& getName() const;

  // Set the name of the DataHolder.
  void setName (const string& name);

  // Get the Transporter object used to send the data
  // to/from the DataHolder connected to this one.
  Transporter& getTransporter();

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

  // Get access to the data blob.
  // <group>
  BlobString& getDataBlock();
  const BlobString& getDataBlock() const;
  // </group>

  // Get the size of a blob header.
  uint getHeaderSize() const;

  // Extract the size from the blob header in the buffer.
  static uint getDataLength (const void* buffer);

  // Resize the buffer to the given size (if needed).
  void resizeBuffer (uint newSize);

protected:
  // Copy constructor
  DataHolder(const DataHolder&);

  // Add a field to the data block definition.
  // Optionally a (unique) name can be given to the field.
  // It returns the index of the field.
  // Note that the timestamp is always the first
  // field of the block.
  // <group>
  uint addField (const BlobFieldBase&);
  uint addField (const std::string& fieldName, const BlobFieldBase&);
  // </group>

  // Setup the data block.
  // This function needs to be called only once (in preprocess) if
  // all data fields have a fixed shape.
  // It fields have a variable shape, the function has to be called again
  // (in process) when a shape changes.
  // <group>
  void createDataBlock();
  void openDataBlock();
  // </group>

  // Get access to the BlobField.
  // It makes it possible to get the shape of a variable shaped input array
  // or to set the shape of a variable shaped output array.
  // <group>
  BlobFieldBase& getDataField (uint fieldIndex);
  BlobFieldBase& getDataField (const std::string& fieldName);
  // </group>

  // Get a pointer to a data field in the blob (by index or by name)
  // <group>
  template<typename T> T* getData (uint fieldIndex);
  template<typename T> T* getData (const std::string& fieldName);
  // </group>

  // If told in the DataHolder constructor, it is possible to add
  // arbitrary fields to the data blob using the ordinary operator<<
  // functions on blob streams. Operator>> functions can be used to read them.
  // An exception is thrown if these functions are used without telling the
  // constructor that 
  // Tell that the extra data block will be used.
  void setExtraBlob (const string& name, int version);

  // Handle the data read (check and convert it as needed).
  void handleDataRead();

  // Get the data field set.
  BlobFieldSet& dataFieldSet();

  // Initialize the data field set.
  void initDataFields();

  // Write the extra data block into the main blob
  void writeExtra();

private:
  // Get the type of BlobString needed from the transport holder.
  virtual BlobStringType blobStringType();

  // Put the extra data block into the main data blob.
  // If possible and needed the buffer is resized.
  // If resized, the data pointers are refilled.
  void putExtra (const void* data, uint size);

  // Fill all data pointers (of timestamp and in derived class).
  void fillAllDataPointers();

  // Let the derived class fill its pointers to the data in the blob.
  // This function is called when the blob is created and when its layout
  // has changed.
  // The default implementation does nothing.
  virtual void fillDataPointers();


  BlobFieldSet    itsDataFields;
  BlobString*     itsData;
  BlobOBufString* itsDataBlob;
  Transporter  itsTransporter;
  int          itsMaxDataSize;   //# <0 is not filled in
  bool         itsIsAddMax;
  string       itsName;
  string       itsType;
  int          itsVersion;
  int          itsReadConvert;   //# data conversion needed after a read?
                                 //# 0=no, 1=yes, else=not known yet
  uint64*      itsTimeStampPtr;
  DataBlobExtra* itsExtraPtr;
};


inline void DataHolder::setMaxDataSize (uint nbytes, bool isAddMax)
  { itsMaxDataSize = nbytes; itsIsAddMax = isAddMax; }

inline int DataHolder::getDataSize() const
  { return itsData->size(); }

inline void* DataHolder::getDataPtr() const
  { return itsData->data(); }

inline Transporter& DataHolder::getTransporter()
  { return itsTransporter; }

inline void DataHolder::setTimeStamp (unsigned long aTimeStamp)
  { *itsTimeStampPtr = aTimeStamp; }

inline unsigned long DataHolder::getTimeStamp() const
  { return *itsTimeStampPtr; }

inline void DataHolder::copyTimeStamp (const DataHolder& that)
  { *itsTimeStampPtr = that.getTimeStamp(); }

inline void DataHolder::copyTimeStamp (const DataHolder* that)
  { *itsTimeStampPtr = that->getTimeStamp(); }

inline const string& DataHolder::getName() const
  { return itsName; }

inline void DataHolder::setName (const string& name)
  { itsName = name; }

inline const string& DataHolder::getType () const
  { return itsType; }

inline void DataHolder::setType(const string& type)
  { itsType = type; }

inline void DataHolder::setID(int aID)
  { itsTransporter.setItsID(aID); }

inline int DataHolder::getID() const
  { return itsTransporter.getItsID(); }

inline bool DataHolder::isBlocking()
  { return itsTransporter.isBlocking(); }

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

inline void DataHolder::fillAllDataPointers()
{
  itsTimeStampPtr = itsDataFields[0].getData<uint64> (*itsDataBlob);
  fillDataPointers();
}

inline uint DataHolder::getHeaderSize() const
{
  return sizeof(BlobHeader);
}

inline uint DataHolder::getDataLength (const void* buffer)
{
  return static_cast<const BlobHeader*>(buffer)->getLength();
}


} // end namespace

#endif
