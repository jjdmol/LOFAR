//# DataHolder.h: Abstract base class for the data holders
//#
//#  Copyright (C) 2000, 2001
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
//#
//#
//#////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_DATAHOLDER_H
#define CEPFRAME_DATAHOLDER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_fstream.h>

#include <CEPFrame/BaseSim.h>
#include <CEPFrame/TransportHolder.h>
#include <CEPFrame/Transportable.h>

namespace LOFAR
{

//# Forward Declarations
class Transport;
class StepRep;

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

class DataHolder: public Transportable
{
protected:
  /** Standard data type class
      It is the base class for datapackets in classes derived from DataHolder.
      It offers functionality to deal with the time stamp of a data packet.
  */
  struct DataPacket
  {
  public:
    DataPacket();

    /// Set the time stamp.
    void setTimeStamp (unsigned long aTimeStamp);

    /// Get the time stamp.
    unsigned long getTimeStamp() const;

    /// Copy the time stamp from another DataPacket to this one.
    void copyTimeStamp (const DataPacket& that);

    /** Compare the time stamp of this and that DataPacket.
        It returns -1, 0, or 1 (for <,==,>).
    */
    int compareTimeStamp (const DataPacket& that) const;
  private:
    unsigned long itsTimeStamp;
  };


public:
  /// Construct a DataHolder with a default name.
  DataHolder (const string& name="aDataHolder",
	      const string& type="DH");

  virtual ~DataHolder();

  // Make a copy
  virtual DataHolder* clone() const = 0;

  /** The allocate/deallocate methods are used to manage memory for a
      DataHolder. This memory is used to send from/receive into. To
      optimize communication the malloc and free routines of the
      specified transportholder are used.
      The allocate and deallocate methods are called when a
      DataHolder is connected to another DataHolder for
      communication.
  */
  void* allocate  (size_t size);
  void  deallocate(void*& ptr);

  /** The preprocess method is called before process is done, thus
      before any read or write is done.
      It can be used to initialize the DataHolder.
      The default implementation does nothing.
  */
  void basePreprocess();
  virtual void preprocess();

  /** The postprocess method is called after process is done.
      It can be used to clean up the DataHolder.
      The default implementation does nothing.
  */
  void basePostprocess();
  virtual void postprocess();

  /** Dump the DataHolder contents to cout.
      The default implementation does nothing.
  */
  virtual void dump() const;

  /// Read the packet data.
  bool read();

  /// Write the packet data.
  void write();

  /// Fill the packet data with zeroes.
  virtual void setZeroes();

  /// Fill the packet data with ones.
  virtual void setOnes();

  /** Does the data has to be handled? 
      It returns true if the 
  */
  bool doHandle() const;

  bool isValid() const;

  void setTimeStamp (unsigned long aTimeStamp);
  unsigned long getTimeStamp() const;
  void copyTimeStamp (const DataHolder& that);
  void copyTimeStamp (const DataHolder* that);
  /// Functions to deal with handling the time stamp 
  int compareTimeStamp (const DataHolder& that) const;

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
  
  /// Set the Step the DataHolder belongs to.
  void setStep (StepRep&);

  /// Get the Step the DataHolder belongs to.
  StepRep& getStep() const;

  /** Get the node the DataHolder runs on.
      -1 is returned if the DataHolder is not used in a Step.
  */
  int getNode() const;

  /// Get the type of the DataHolder.
  const string& getType() const;

  /// Set the type of the DataHolder.
  void setType (const string& type);

  /// Get the name of the DataHolder.
  const string& getName() const;

  /// Set the name of the DataHolder.
  void setName (const string& name);

  /** Set the read delay for the DataHolder.
      Only after 'delay' times an actual read is done.
  */
  void setReadDelay (int delay);
  void setWriteDelay (int delay);

  /** Force the data holder to read input from the file #inFile#.
      @exception runtime_error: "File not found"
  */
  void setInFile(const string& inFile);

  /** Force the data holder to write output to the file #outFile#.
      #setOutFile# returns false if the output file could not be created,
      otherwise it returns true.
  */
  bool setOutFile(const string& outFile);

  /// Stop reading input data from file.
  void unsetInFile();

  /// Stop writing output data to file.
  void unsetOutFile();
    
  /** Get a pointer to the Transport object used to send the data
      to/from the DataHolder connected to this one.
  */
  Transport& getTransport();

protected:
  /** Set the pointer to the data packet and set the packet's size.
      This function has to be called by the constructor of derived
      DataHolder classes.
  */
  void setDataPacket (DataPacket* ptr, int size);

  /// Set the data packet to the default data packet..
  void setDefaultDataPacket();

  DataHolder(const DataHolder&);

private:

  DataPacket  itsDefaultPacket;
  DataPacket* itsDataPacketPtr;
  int         itsDataPacketSize; // (Max) size in bytes
  Transport*  itsTransportPtr;
  string      itsName;
  string      itsType;

  // The step this DataHolder belongs to.
  StepRep* itsStep;

  /// The read delay for a DataHolder.
  int         itsReadDelay;
  int         itsWriteDelay;
  int         itsReadDelayCount;
  int         itsWriteDelayCount;

  /// Pointer to input file stream object. Used when reading input from file.
  ifstream*    itsIfsPtr;

  /// Pointer to output file stream object. Used when writing to output file.
  ofstream*    itsOfsPtr;

  /** Read from the input file stream. 
      Return true if the read operation was successful.
      #doFsRead()# should be implemented in the derived data holder classes. 
  */
  virtual bool doFsRead (ifstream&);

  /** Write to the output file stream.
      Return true if the write operation was successful.
      #doFsWrite()# should be implemented in the derived data holder classes.
  */
  virtual bool doFsWrite (ofstream&) const;
   
};

inline void DataHolder::setStep (StepRep& step)
  { itsStep = &step; }

inline StepRep& DataHolder::getStep() const
  { return *itsStep; }

inline int DataHolder::getDataPacketSize()
  { return itsDataPacketSize; }

inline int DataHolder::getCurDataPacketSize(){
  // overload in flexible datapackets
  return getDataPacketSize(); 
}

inline int DataHolder::getMaxDataPacketSize(){
  // overload in flexible datapackets
  return getDataPacketSize(); 
}

inline Transport& DataHolder::getTransport()
  { return *itsTransportPtr; }

inline const DataHolder::DataPacket& DataHolder::getDataPacket() const
  { return *itsDataPacketPtr; }

inline void* DataHolder::getDataPtr()
  { return itsDataPacketPtr; }

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

inline void DataHolder::setDefaultDataPacket()
{
  setDataPacket (&itsDefaultPacket, sizeof(DataPacket));
}

inline void DataHolder::setDataPacket (DataPacket* ptr, int size)
{
  itsDataPacketPtr = ptr;
  itsDataPacketSize = size;
}

inline const string& DataHolder::getName() const
  { return itsName; }

inline void DataHolder::setName (const string& name)
  { itsName = name; }

inline const string& DataHolder::getType () const
  { return itsType; }

inline void DataHolder::setType(const string& type)
  { itsType = type; }

}

#endif
