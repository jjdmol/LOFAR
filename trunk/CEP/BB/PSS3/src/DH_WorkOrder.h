//  DH_WorkOrder.h: Example DataHolder
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

#ifndef PSS3_DH_WORKORDER_H
#define PSS3_DH_WORKORDER_H

#include <Common/lofar_complex.h>

#include <lofar_config.h>

#include <CEPFrame/DH_Postgresql.h>
#include <PSS3/Quality.h>

#define MAX_STRAT_ARGS_SIZE 32  // Change this value to the maximum
                                // argument size of all strategy 
                                // implementations

/**
   This class is an example DataHolder which is only used in the
   Example programs.
*/

class DH_WorkOrder: public LOFAR::DH_Postgresql
{
public:

  enum woStatus{New,Assigned,Executed};

  explicit DH_WorkOrder (const string& name);

  DH_WorkOrder(const DH_WorkOrder&);

  virtual ~DH_WorkOrder();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Database store/retrieve methods
  bool StoreInDatabase(int appId, int tag, char* buf, int size);
  bool RetrieveFromDatabase(int appId, int tag, char* buf, int size);

  /// Data access functions.
  int getWorkOrderID();
  void setWorkOrderID(int id);

  woStatus getStatus();
  void setStatus(woStatus status);

  const string& getKSType();
  void setKSType(const string& ksType);

  unsigned int getStrategyNo();
  void setStrategyNo(unsigned int no);

  int getArgSize();
  void setArgSize(int size);
  char* getVarArgsPtr();

  const string& getParam1Name();
  void setParam1Name(const string& name);
  const string& getParam2Name();
  void setParam2Name(const string& name);
  const string& getParam3Name();
  void setParam3Name(const string& name);

  void useSolutionNumber(int id);
  int getSolutionNumber();

protected:
  // Definition of the DataPacket type.
  // N.B.This datapacket can only be correctly transported with TH_Database
  class DataPacket: public DH_Database::DataPacket
  {
  public:
    DataPacket();
    ~DataPacket() {};
    int          itsWOID;

    woStatus     itsStatus;        // WorkOrder status
    string       itsKSType;        // Knowledge source type which is allowed to 
                                   // execute the WorkOrder
    unsigned int itsStrategyNo;    // Strategy number
    int          itsArgSize;       // Size of strategy specific arguments in bytes
    char         itsVarArgs[MAX_STRAT_ARGS_SIZE];
                                   // Strategy specific arguments
    string       itsParam1Name;    // Names of three parameters
    string       itsParam2Name;
    string       itsParam3Name;
    int          itsStartSolution; // Use this solution
  };

private:
  /// Forbid assignment.
  DH_WorkOrder& operator= (const DH_WorkOrder&);
  DataPacket itsDataPacket;

  static int theirWriteCount;
  static int theirReadCount;

};

inline int DH_WorkOrder::getWorkOrderID()
{ return itsDataPacket.itsWOID; }

inline void DH_WorkOrder::setWorkOrderID(int id)
{ itsDataPacket.itsWOID = id; }

inline const string& DH_WorkOrder::getParam1Name()
{ return itsDataPacket.itsParam1Name;}

inline void DH_WorkOrder::setParam1Name(const string& name)
{ itsDataPacket.itsParam1Name = name;}

inline const string& DH_WorkOrder::getParam2Name()
{ return itsDataPacket.itsParam2Name;}

inline void DH_WorkOrder::setParam2Name(const string& name)
{ itsDataPacket.itsParam2Name = name;}

inline const string& DH_WorkOrder::getParam3Name()
{ return itsDataPacket.itsParam3Name;}

inline void DH_WorkOrder::setParam3Name(const string& name)
{ itsDataPacket.itsParam3Name = name;}

inline DH_WorkOrder::woStatus DH_WorkOrder::getStatus()
{ return itsDataPacket.itsStatus; }

inline void DH_WorkOrder::setStatus(DH_WorkOrder::woStatus status)
{ itsDataPacket.itsStatus = status; }

inline const string& DH_WorkOrder::getKSType()
{ return itsDataPacket.itsKSType; }

inline void DH_WorkOrder::setKSType(const string& ksType)
{ itsDataPacket.itsKSType = ksType; }

inline unsigned int DH_WorkOrder::getStrategyNo()
{ return itsDataPacket.itsStrategyNo; }

inline void DH_WorkOrder::setStrategyNo(unsigned int no)
{ itsDataPacket.itsStrategyNo = no; }

inline int DH_WorkOrder::getArgSize()
{ return itsDataPacket.itsArgSize; }

inline char* DH_WorkOrder::getVarArgsPtr()
{ return itsDataPacket.itsVarArgs; }

inline void DH_WorkOrder::useSolutionNumber(int id)
{ itsDataPacket.itsStartSolution = id; }

inline int DH_WorkOrder::getSolutionNumber()
{ return itsDataPacket.itsStartSolution; }

#endif 
