//# BaseDataHolder.h: Abstract base class for DataHolder and ParamHolder
//#
//# Copyright (C) 2000
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

#ifndef LIBTRANSPORT_BASEDATAHOLDER_H
#define LIBTRANSPORT_BASEDATAHOLDER_H

//# Includes
#include <lofar_config.h>
#include <libTransport/Transporter.h>
#include <Common/lofar_string.h>
#include <Common/lofar_fstream.h>
#include <Common/BlobFieldSet.h>
#include <Common/BlobString.h>
#include <Common/BlobOBufString.h>
#include <Common/DataConvert.h>

namespace LOFAR
{

// Class BaseDataHolder is the abstract base class for classes holding
// data, in particular for DataHolder and ParamHolder.
//
// It offers some common data items and functionality.
// <ul>
// <li> Handling of a timestamp for the data to be sent or received.
// <li> A blob holding all the data and functions to form the blob.
//      It makes use of the BlobFieldSet class.

class BaseDataHolder
{
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
			     BaseDataHolder::DataPacket* buf, uint nrval);

  private:
    unsigned long itsTimeStamp;
  };


public:
  // Construct a DataHolder with a default name.
  BaseDataHolder (const string& name="aDataHolder",
		  const string& type="DH");

  virtual ~BaseDataHolder();

  // Make a copy
  virtual BaseDataHolder* clone() const = 0;

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

  // Does the data has to be handled? 
  // It returns true if the 
  //#// bool doHandle() const;

  bool isValid() const;

  void setTimeStamp (unsigned long aTimeStamp);
  unsigned long getTimeStamp() const;
  void copyTimeStamp (const BaseDataHolder& that);
  void copyTimeStamp (const BaseDataHolder* that);
  // Functions to deal with handling the timestamp 
  int compareTimeStamp (const BaseDataHolder& that) const;

  // Get Data Packet size (in bytes);
  // See also getCurDataPacketSize() and getMaxDataPacketSize 
  // for operation with variable datapackets
  int getDataPacketSize();

#ifdef abc_0
  /** 
      Get the size of the CURRENT data packet (in bytes)
      For non-flexible datapackets, this is the same as 
      getDataPacketSize()
  */
  virtual int getCurDataPacketSize();

  /** 
      Get the MAXIMUM data packet size supported(in bytes)
      For non-flexible datapackets, this is the same as 
      getDataPacketSize()
  */
  virtual int getMaxDataPacketSize();

#endif

  // Get the data packet
  const DataPacket& getDataPacket() const;

  // Get a pointer to the data (in the blob).
  void* getDataPtr();

  // Get the node the BaseDataHolder runs on.
  // -1 is returned if the BaseDataHolder is not used in a Step.
  int getNode() const;

  // Get the type of the BaseDataHolder.
  const string& getType() const;

  // Set the type of the BaseDataHolder.
  void setType (const string& type);

  // Get the name of the BaseDataHolder.
  const string& getName() const;

  // Set the name of the BaseDataHolder.
  void setName (const string& name);

  // Set the read delay for the BaseDataHolder.
  // Only after 'delay' times an actual read is done.
  void setReadDelay (int delay);
  void setWriteDelay (int delay);

  // Get a pointer to the Transport object used to send the data
  // to/from the BaseDataHolder connected to this one.
  Transporter& getTransporter();
  void setTransporter(Transporter& aTransporter);

protected:
  BaseDataHolder(const BaseDataHolder&);

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
  void createDataBlock();
  void openDataBlock();
  // 

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

public:
  // Get access to the data blob.
  // <group>
  BlobString& getDataBlock();
  const BlobString& getDataBlock() const;
  // </group>

private:
  // Get the type of BlobString needed from the transport holder.
  virtual BlobStringType blobStringType();

  // Initialize the data field set.
  void initDataFields();

  BlobFieldSet    itsDataFields;
  BlobString*     itsData;
  BlobOBufString* itsDataBlob;
  DataPacket*  itsDataPacketPtr;
  Transporter* itsTransporter;
  string       itsName;
  string       itsType;

  // The read delay for a BaseDataHolder.
  int itsReadDelay;
  int itsWriteDelay;
  int itsReadDelayCount;
  int itsWriteDelayCount;
};


inline int BaseDataHolder::getDataPacketSize()
  { return itsData->size(); }

#ifdef abc_0
inline int BaseDataHolder::getCurDataPacketSize(){
  // overload in flexible datapackets
  return getDataPacketSize(); 
}

inline int BaseDataHolder::getMaxDataPacketSize(){
  // overload in flexible datapackets
  return getDataPacketSize(); 
}
#endif


inline const BaseDataHolder::DataPacket& BaseDataHolder::getDataPacket() const
  { return *itsDataPacketPtr; }

inline void* BaseDataHolder::getDataPtr()
  { return itsData->data(); }

inline Transporter& BaseDataHolder::getTransporter()
  { return *itsTransporter; }

inline void BaseDataHolder::setTimeStamp (unsigned long aTimeStamp)
  { itsDataPacketPtr->setTimeStamp (aTimeStamp); }

inline unsigned long BaseDataHolder::getTimeStamp() const
  { return itsDataPacketPtr->getTimeStamp(); }

inline void BaseDataHolder::copyTimeStamp (const BaseDataHolder& that)
  { itsDataPacketPtr->copyTimeStamp (that.getDataPacket()); }

inline void BaseDataHolder::copyTimeStamp (const BaseDataHolder* that)
  { itsDataPacketPtr->copyTimeStamp (that->getDataPacket()); }

inline int BaseDataHolder::compareTimeStamp (const BaseDataHolder& that) const
  { return itsDataPacketPtr->compareTimeStamp (that.getDataPacket()); }


inline BaseDataHolder::DataPacket::DataPacket()
: itsTimeStamp (0)
{}

inline void BaseDataHolder::DataPacket::setTimeStamp (unsigned long aTimeStamp)
  { itsTimeStamp = aTimeStamp; }

inline unsigned long BaseDataHolder::DataPacket::getTimeStamp() const
  { return itsTimeStamp; }

inline void BaseDataHolder::DataPacket::copyTimeStamp (const DataPacket& that)
  { itsTimeStamp = that.itsTimeStamp; }

inline const string& BaseDataHolder::getName() const
  { return itsName; }

inline void BaseDataHolder::setName (const string& name)
  { itsName = name; }

inline const string& BaseDataHolder::getType () const
  { return itsType; }

inline void BaseDataHolder::setType(const string& type)
  { itsType = type; }

inline void BaseDataHolder::setTransporter(Transporter& aTransporter)
{
  itsTransporter = &aTransporter;
}


inline BlobFieldBase& BaseDataHolder::getDataField (uint fieldIndex)
{
  return itsDataFields[fieldIndex];
}
inline BlobFieldBase& BaseDataHolder::getDataField (const std::string& fieldName)
{
  return itsDataFields[fieldName];
}

template<typename T>
inline T* BaseDataHolder::getData (uint fieldIndex)
{
  return itsDataFields[fieldIndex].getData<T> (*itsDataBlob);
}
template<typename T>
inline T* BaseDataHolder::getData (const std::string& fieldName)
{
  return itsDataFields[fieldName].getData<T> (*itsDataBlob);
}

inline BlobString& BaseDataHolder::getDataBlock()
{
  return *itsData;
}

inline void dataConvert (DataFormat fmt,
			 BaseDataHolder::DataPacket* buf, uint nrval)
{
  for (uint i=0; i<nrval ;i++) {
    dataConvertDouble (fmt, &(buf[i].itsTimeStamp));
  }
}


} // end namespace



#endif
