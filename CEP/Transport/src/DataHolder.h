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
#include <Common/DataConvert.h>
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
  to a class like WorkHolder. Apart from that it also offers some common
  functionality to the classes derived from it.

  DataHolder has an internal class called DataPacket. This class holds
  the data of a DataHolder class. A Class derived from DataHolder
  should also have an internal class to hold its data. That class should
  be derived from DataHolder::DataPacket.
  The basic DataPacket class offers some functions to set, get, and
  compare the timestamp of a data packet.

  The constructors of a class derived from DataHolder should always
  call the function setDataPacket in order to make their DataPacket object
  known to this base class.

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
  // Standard data containing a timestamp.
  struct DataPacket
  {
  public:
    DataPacket();

    // Set the timestamp.
    void setTimeStamp (unsigned long aTimeStamp);

    // Get the timestamp.
    unsigned long getTimeStamp() const;

    // Copy the timestamp from another DataPacket to this one.
    void copyTimeStamp (const DataPacket& that);

    // Compare the timestamp of this and that DataPacket.
    //    It returns -1, 0, or 1 (for <,==,>).
    int compareTimeStamp (const DataPacket& that) const;

    // Define the function to convert DataPacket from given format
    // to local format.
    friend void dataConvert (DataFormat fmt,
			     DataHolder::DataPacket* buf, uint nrval);

  private:
    unsigned long itsTimeStamp;
  };


public:
  // Construct a DataHolder with a default name.
  DataHolder (const string& name="aDataHolder",
	      const string& type="DH");

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

  /// Read the packet data.
  bool read();

  /// Write the packet data.
  void write();

  /// Is the Transporter of this DataHolder valid?
  bool isValid() const;

  /// Connect to another DataHolder.
  // (data flows from this object to thatDH).
  bool connectTo(DataHolder& thatDH, const TransportHolder& prototype,
		 bool blockingComm = true);

  /// Initialization must be called after connect.
  void init();   

  void setTimeStamp (unsigned long aTimeStamp);
  unsigned long getTimeStamp() const;
  void copyTimeStamp (const DataHolder& that);
  void copyTimeStamp (const DataHolder* that);
  // Functions to deal with handling the timestamp 
  int compareTimeStamp (const DataHolder& that) const;

  // Get data size (in bytes);
  // See also getCurDataSize() and getMaxDataSize()
  // for operation with variable datas
  int getDataSize();

  // Get the size of the CURRENT data block (in bytes)
  // For non-flexible data blocks, this is the same as 
  // getDataSize()
  virtual int getCurDataSize();

  // Get the MAXIMUM data block size supported (in bytes)
  // For non-flexible data blocks, this is the same as 
  // getDataSize()
  virtual int getMaxDataSize();

  // Get a pointer to the data (the beginning of the blob).
  void* getDataPtr();

  // Get the data packet
  const DataPacket& getDataPacket() const;

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

  // Get the Transport object used to send the data
  // to/from the DataHolder connected to this one.
  Transporter& getTransporter();

protected:
  DataHolder(const DataHolder&);

  // Add a field to the data block definition.
  // Optionally a (unique) name can be given to the field.
  // It returns the index of the field.
  // Note that the DataPacket (timestamp) is always the first
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

public:
  // Initialize the extra output blob holding arbitrary fields.
  // The return reference can be used to store the fields in.
  // It is meant for DataHolders writing data.
  BlobOStream& createExtraBlob();
  // Get access to the extra input blob holding arbitrary fields.
  // It is meant for DataHolders reading data.
  // It fills the version of the extra data blob.
  BlobIStream& openExtraBlob (int& version);

  // Get access to the data blob.
  // <group>
  BlobString& getDataBlock();
  const BlobString& getDataBlock() const;
  // </group>

  // Get the size of a blob header.
  uint getHeaderSize() const;

  // Extract the size from the blob header in the buffer.
  uint getDataLength (const void* buffer) const;

  // Resize the buffer to the given size (if needed).
  void resizeBuffer (uint newSize);

protected:
  // Handle the data read (check and convert it as needed).
  void handleDataRead();

  // Get the data field set.
  const BlobFieldSet& dataFieldSet() const;

private:
  // Get the type of BlobString needed from the transport holder.
  virtual BlobStringType blobStringType();

  // Initialize the data field set.
  void initDataFields();

  // Put the extra data block into the main data blob.
  // If possible and needed the buffer is resized.
  // If resized, the data pointers are refilled.
  void putExtra (const void* data, uint size);

  // Fill all data pointers (of DataPacket and in derived class).
  void fillAllDataPointers();

  // Let the derived class fill its pointers to the data in the blob.
  // This function is called when the blob is created and when its layout
  // has changed.
  // The default implementation does nothing.
  virtual void fillDataPointers();


  BlobFieldSet    itsDataFields;
  BlobString*     itsData;
  BlobOBufString* itsDataBlob;
  DataPacket*  itsDataPacketPtr;
  Transporter  itsTransporter;
  string       itsName;
  string       itsType;
  int          itsReadConvert;   //# data conversion needed after a read?
                                 //# 0=no, 1=yes, else=not known yet
  DataBlobExtra* itsExtraPtr;
};


inline int DataHolder::getDataSize()
  { return itsData->size(); }

#ifndef abc_0
inline int DataHolder::getCurDataSize(){
  // overload in flexible datas
  return getDataSize(); 
}

inline int DataHolder::getMaxDataSize(){
  // overload in flexible datas
  return getDataSize(); 
}
#endif

inline void* DataHolder::getDataPtr()
  { return itsData->data(); }

inline const DataHolder::DataPacket& DataHolder::getDataPacket() const
  { return *itsDataPacketPtr; }

inline Transporter& DataHolder::getTransporter()
  { return itsTransporter; }

inline void DataHolder::setTimeStamp (unsigned long aTimeStamp)
  { itsDataPacketPtr->setTimeStamp (aTimeStamp); }

inline unsigned long DataHolder::getTimeStamp() const
  { return itsDataPacketPtr->getTimeStamp(); }

inline void DataHolder::copyTimeStamp (const DataHolder& that)
  { itsDataPacketPtr->copyTimeStamp (that.getDataPacket()); }

inline void DataHolder::copyTimeStamp (const DataHolder* that)
  { itsDataPacketPtr->copyTimeStamp (that->getDataPacket()); }

inline int DataHolder::compareTimeStamp (const DataHolder& that) const
  { return itsDataPacketPtr->compareTimeStamp (that.getDataPacket()); }


inline DataHolder::DataPacket::DataPacket()
: itsTimeStamp (0)
{}

inline void DataHolder::DataPacket::setTimeStamp (unsigned long aTimeStamp)
  { itsTimeStamp = aTimeStamp; }

inline unsigned long DataHolder::DataPacket::getTimeStamp() const
  { return itsTimeStamp; }

inline void DataHolder::DataPacket::copyTimeStamp (const DataPacket& that)
  { itsTimeStamp = that.itsTimeStamp; }

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

inline const BlobFieldSet& DataHolder::dataFieldSet() const
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

inline void dataConvert (DataFormat fmt,
			 DataHolder::DataPacket* buf, uint nrval)
{
  for (uint i=0; i<nrval ;i++) {
    dataConvertDouble (fmt, &(buf[i].itsTimeStamp));
  }
}

inline void DataHolder::fillAllDataPointers()
{
  itsDataPacketPtr = itsDataFields[0].getData<DataPacket> (*itsDataBlob);
  fillDataPointers();
}

inline uint DataHolder::getHeaderSize() const
{
  return sizeof(BlobHeader);
}

inline uint DataHolder::getDataLength (const void* buffer) const
{
  return static_cast<const BlobHeader*>(buffer)->getLength();
}


} // end namespace

#endif
