// ParamHolder.h: Class to hold parameters
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_PARAMHOLDER_H
#define CEPFRAME_PARAMHOLDER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/BaseSim.h"
#include <Common/lofar_string.h>
#include "CEPFrame/Lock.h"

//Forward declarations
class StepRep;
class ParamTransportManager;

/**
  Class ParamHolder is the abstract base class for all parameter holders
  in the CEPFrame environment. Its main purpose is to offer a common interface
  to a class like WorkHolder. Apart from that it also offers some common
  functionality to the classes derived from it.

  ParamHolder has an internal class called ParamPacket. This class holds
  the data of a ParamHolder class. A Class derived from ParamHolder
  should also have an internal class to hold its data. That class should
  be derived from ParamHolder::ParamPacket.

  The class derived from ParamHolder should always call the function 
  setParamPacket in order to make their ParamPacket object known to this 
  base class.

*/

class ParamHolder
{
public:
  /// Standard Parameter type class
  // It is the base class for parampackets in classes derived from ParamHolder.
  struct ParamPacket
  { 
    public:
      ParamPacket();
      /// Set the time stamp.
      void setTimeStamp (unsigned long aTimeStamp);

      /// Get the time stamp.
      unsigned long getTimeStamp() const;

      char          itsEndianness;
      unsigned long itsTimeStamp;
      int           itsFinalMsg;
  };

  ParamHolder(const string& name="aParamHolder",
	      const string& type="PH");

  virtual ~ParamHolder();

  virtual ParamHolder* clone() const = 0;

  void basePreprocess();
  virtual void preprocess();

  // Get the ParamPacket
  void* getParamPacket();

  // Get the ParamPacket size
  int getParamPacketSize();

  // Set the timestamp
  void setTimeStamp (unsigned long aTimeStamp);

  // Get the timestamp
  unsigned long getTimeStamp() const;

  /** Show the contents of the object on cout.
      The default implementation outputs the string "ParamHolder".
  */
  virtual void dump() const;

  /** @name Comparison functions
      @memo Compare the serial number of this and other
      @doc Compare this and the other ParameterHolder using their
      serial number.
  */
  //@{
  bool operator== (const ParamHolder& other) const;
  bool operator!= (const ParamHolder& other) const;
  bool operator< (const ParamHolder& other) const;
  //@}

  /// Set the Step the ParamHolder belongs to.
  void setStep (StepRep&);

  /// Get the Step the ParamHolder belongs to.
  StepRep& getStep() const;

  // Get the name of the ParamHolder
  const string& getName() const;

  // Set the name of the ParamHolder
  void setName(const string& name);

  // Get the type of the ParamHolder
  const string& getType() const;

  // Set the type of the ParamHolder
  void setType(const string& type);

  // Get ownership info
  bool isParamOwner() const;

  // Set ownership info
  void setParamOwner(bool owner);

  // Get ParamTransportManager
  ParamTransportManager* getPTManager();

  // Set ParamTransportManager
  void setPTManager(ParamTransportManager* ptManager);

  /** Get the node the ParamHolder runs on.
      -1 is returned if the ParamHolder is not used in a Step.
  */
  int getNode() const;

  // Locking paramPacket
  void readLock();
  void readUnlock();
  void writeLock();
  void writeUnlock();

protected:
  ParamHolder& operator= (const ParamHolder& that);
  
  ParamHolder(const ParamHolder&);
  
  // Set the pointer to the param packet and set the packet's size
  void setParamPacket(ParamPacket* ptr, int size);

private:
  static int         theirSerial;
  int                itsSerial;
  ThreadRWLock       itsLock;

  // The step this ParamHolder belongs to.
  StepRep*                itsStep;
  string                  itsName;
  string                  itsType;
  bool                    itsIsParamOwner; // Owner has right to 'publish'
                                           // new value
  ParamPacket             itsParamPacket;
  ParamPacket*            itsParamPacketPtr;
  int                     itsParamPacketSize;
  ParamTransportManager*  itsPTManager;    // Pointer to transport manager

};

inline void ParamHolder::setStep (StepRep& step)
{ itsStep = &step; }

inline StepRep& ParamHolder::getStep() const
{ return *itsStep; }

inline const string& ParamHolder::getName() const
{ return itsName; }

inline void ParamHolder::setName(const string& name)
{ itsName = name; }

inline const string& ParamHolder::getType() const
{ return itsType; }

inline bool ParamHolder::isParamOwner() const
{ return itsIsParamOwner; }

inline void ParamHolder::setParamOwner(bool owner)
{ itsIsParamOwner = owner; }

inline void ParamHolder::setType(const string& type)
{ itsType = type; }

inline ParamTransportManager* ParamHolder::getPTManager()
{ return itsPTManager; }

inline void ParamHolder::setPTManager(ParamTransportManager* ptManager)
{ itsPTManager = ptManager; }

inline void ParamHolder::setParamPacket(ParamPacket* ptr, int size)
{
  itsParamPacketPtr = ptr;
  itsParamPacketSize = size;
}

inline void* ParamHolder::getParamPacket()
{
  return itsParamPacketPtr;
}

inline int ParamHolder::getParamPacketSize()
{
  return itsParamPacketSize;
}

inline void ParamHolder::readLock()
{ itsLock.ReadLock(); }

inline void ParamHolder::readUnlock()
{ itsLock.ReadUnlock(); }

inline void ParamHolder::writeLock()
{ itsLock.WriteLock(); }

inline void ParamHolder::writeUnlock()
{ itsLock.WriteUnlock(); }

inline void ParamHolder::setTimeStamp (unsigned long aTimeStamp)
  { itsParamPacketPtr->setTimeStamp (aTimeStamp); }

inline unsigned long ParamHolder::getTimeStamp() const
  { return itsParamPacketPtr->getTimeStamp(); }

inline ParamHolder::ParamPacket::ParamPacket()
  : itsEndianness(0),
    itsTimeStamp(0),
    itsFinalMsg(0)
{}

inline void ParamHolder::ParamPacket::setTimeStamp (unsigned long aTimeStamp)
  { itsTimeStamp = aTimeStamp; }

inline unsigned long ParamHolder::ParamPacket::getTimeStamp() const
  { return itsTimeStamp; }


#endif 
