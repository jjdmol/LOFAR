//#
//# BaseDataHolder.h: A parent class for DataHolder and ParamHolder
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

#ifndef LIBTRANSPORT_BASE_DATAHOLDER_H
#define LIBTRANSPORT_BASE_DATAHOLDER_H

#include <lofar_config.h>

#include <Common/lofar_string.h>
#include <Common/lofar_fstream.h>

#include <libTransport/Transporter.h>

namespace LOFAR
{

//# Forward declarations
class DataHolder;
class ParamHolder;

class BaseDataHolder
{
protected:
  struct DataPacket
  {
  public:
    DataPacket ();

    /// Set the timestamp
    void setTimeStamp (unsigned long aTimeStamp);
    /// Get the timestamp
    unsigned long getTimeStamp() const;
    /// Copy the timestamp from another DataPacket into this one
    void copyTimeStamp (const DataPacket& that);
    /** Compare the timestamp of this and that DataPacket. 
	It returns -1, 0 or 1 (for <, ==, >).
     */
    int compareTimeStamp(const DataPacket& that) const;

  private:
    unsigned long itsTimeStamp;
  };

public:
  /// Construct a BaseDataHolder with a default name
  BaseDataHolder (const string& name="aBaseDataHolder",
		  const string& type="BDH");
  
  virtual ~BaseDataHolder();
  // Make a copy
  virtual BaseDataHolder* clone() const = 0;

  /** The allocate/deallocate methods are used to manage memory for a
      BaseDataHolder. This memory is used to send from/receive into. To
      optimize communication the malloc and free routines of the
      specified transportholder are used.
      The allocate and deallocate methods are called when a
      BaseDataHolder is connected to another BaseDataHolder for
      communication.
  */
  void* allocate  (size_t size);
  void  deallocate(void*& ptr);

  /** The preprocess method is called before process is done, thus
      before any read or write is done.
      It can be used to initialize the BaseDataHolder.
      The default implementation does nothing.
  */
  void basePreprocess();
  virtual void preprocess();

  /** The postprocess method is called after process is done.
      It can be used to clean up the BaseDataHolder.
      The default implementation does nothing.
  */
  void basePostprocess();
  virtual void postprocess();

  /** Dump the BaseDataHolder contents to cout.
      The default implementation does nothing.
  */
  virtual void dump() const;

  /// Read the packet data.
  bool read();

  /// Write the packet data.
  void write();

  /** Does the data has to be handled? 
      It returns true if the 
  */
  //  bool doHandle() const;

  bool isValid() const;

  void setTimeStamp (unsigned long aTimeStamp);
  unsigned long getTimeStamp() const;
  void copyTimeStamp (const BaseDataHolder& that);
  void copyTimeStamp (const BaseDataHolder* that);
  /// Functions to deal with handling the time stamp 
  int compareTimeStamp (const BaseDataHolder& that) const;

  /** 
      Get Data Packet size (in bytes);
      See also getCurDataPacketSize() and getMaxDataPacketSize 
      for operation with variable datapackets
  */
  virtual int getDataPacketSize();

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

  /// Get the data packet
  const DataPacket& getDataPacket() const;
  void* getDataPtr();
  
  /** Get the node the BaseDataHolder runs on.
      -1 is returned if the BaseDataHolder is not used in a Step.
  */
  int getNode() const;

  /// Get the type of the BaseDataHolder.
  const string& getType() const;

  /// Set the type of the BaseDataHolder.
  void setType (const string& type);

  /// Get the name of the BaseDataHolder.
  const string& getName() const;

  /// Set the name of the BaseDataHolder.
  void setName (const string& name);

  /** Set the read delay for the BaseDataHolder.
      Only after 'delay' times an actual read is done.
  */
  void setReadDelay (int delay);
  void setWriteDelay (int delay);

  /** Get a pointer to the Transport object used to send the data
      to/from the BaseDataHolder connected to this one.
  */
  Transporter& getTransporter();
  void setTransporter(Transporter& aTransporter);

protected:
  /** Set the pointer to the data packet and set the packet's size.
      This function has to be called by the constructor of derived
      BaseDataHolder classes.
  */
  void setDataPacket (DataPacket* ptr, int size);

  /// Set the data packet to the default data packet..
  void setDefaultDataPacket();

  BaseDataHolder(const BaseDataHolder&);

  DataPacket   itsDefaultPacket;
  DataPacket*  itsDataPacketPtr;

private:

  int          itsDataPacketSize; // (Max) size in bytes
  Transporter* itsTransporter;
  string       itsName;
  string       itsType;

  /// The read delay for a BaseDataHolder.
  int         itsReadDelay;
  int         itsWriteDelay;
  int         itsReadDelayCount;
  int         itsWriteDelayCount;

   
};

inline int BaseDataHolder::getDataPacketSize()
  { return itsDataPacketSize; }

inline int BaseDataHolder::getCurDataPacketSize(){
  // overload in flexible datapackets
  return getDataPacketSize(); 
}

inline int BaseDataHolder::getMaxDataPacketSize(){
  // overload in flexible datapackets
  return getDataPacketSize(); 
}

inline Transporter& BaseDataHolder::getTransporter()
{ return *itsTransporter; }

inline const BaseDataHolder::DataPacket& BaseDataHolder::getDataPacket() const
{ return *itsDataPacketPtr; }

inline void* BaseDataHolder::getDataPtr()
{ return itsDataPacketPtr; }


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

inline void BaseDataHolder::setDefaultDataPacket()
{
  setDataPacket (&itsDefaultPacket, sizeof(DataPacket));
}

inline void BaseDataHolder::setDataPacket (DataPacket* ptr, int size)
{
  itsDataPacketPtr = ptr;
  itsDataPacketSize = size;
}

inline const string& BaseDataHolder::getName() const
  { return itsName; }

inline void BaseDataHolder::setName (const string& name)
  { itsName = name; }

inline const string& BaseDataHolder::getType () const
  { return itsType; }

inline void BaseDataHolder::setType(const string& type)
  { itsType = type; }

inline void BaseDataHolder::setTransporter(Transporter& aTransporter)
{ itsTransporter = &aTransporter; }

}

#endif 
