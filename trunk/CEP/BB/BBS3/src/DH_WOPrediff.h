//  DH_WOPrediff.h: Example DataHolder
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

#ifndef BBS3_DH_WOPREDIFF_H
#define BBS3_DH_WOPREDIFF_H

#include <Common/lofar_vector.h>

#include <TransportPL/DH_PL.h>
#include <TransportPL/PO_DH_PL.h>
#include <BBS3/Quality.h>

namespace LOFAR
{

/**
   This class is a DataHolder which contains the work orders.
*/
 //# Forward Declarations
class KeyValueMap;

class DH_WOPrediff: public DH_PL
{
public:

  enum woStatus{New,Assigned,Executed};

  typedef PL::TPersistentObject<DH_WOPrediff> PO_DH_WOPrediff;

  explicit DH_WOPrediff (const string& name = "dh_woprediff");

  DH_WOPrediff(const DH_WOPrediff&);

  virtual ~DH_WOPrediff();

  DataHolder* clone() const;

  // Get a reference to the PersistentObject.
  virtual PL::PersistentObject& getPO() const;

  // Create a TPO object and set the table name in it.
  virtual void initPO (const string& tableName);

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Data access methods.
  int getWorkOrderID() const;
  void setWorkOrderID(int id);

  unsigned int getStatus() const ;
  void setStatus(unsigned int status);

  string getKSType() const;
  void setKSType(const string& ksType);

  bool getInitialize() const;
  void setInitialize(bool doInitialize);

  bool getNextInterval() const;
  void setNextInterval(bool doNextInterval);

  int getFirstChannel() const;
  void setFirstChannel(int nr);

  int getLastChannel() const;
  void setLastChannel(int nr);

  int getTimeInterval() const;
  void setTimeInterval(int time);

  int getDDID() const;
  void setDDID(int ddid);

  string getModelType() const;
  void setModelType(const string& type);

  bool getCalcUVW() const;
  void setCalcUVW(bool calc);

  bool getLockMappedMemory() const;
  void setLockMappedMemory(bool lock);

  void setVarData(const KeyValueMap& predArgs,
		  vector<int>& antNrs,
		  vector<string>& pNames,
		  vector<int>& peelSrcs);
  bool getVarData(KeyValueMap& predArgs,
		  vector<int>& antNrs,
		  vector<string>& pNames,
		  vector<int>& peelSrcs);

  void dump();

  void clearData();

private:
  /// Forbid assignment.
  DH_WOPrediff& operator= (const DH_WOPrediff&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*          itsWOID;                    // Unique workorder id
  unsigned int* itsStatus;                  // Workorder status
  char*         itsKSType;                  // Knowledge Source type
  unsigned int* itsInitialize;              // Do initialization?
  unsigned int* itsNextInterval;            // Do nextInterval?
  int*          itsFirstChan;               // First frequency channel
  int*          itsLastChan;                // Last frequency channel
  int*          itsTimeInterval;            // Time interval size (s)
  int*          itsDDID;
  char*         itsModelType;
  unsigned int* itsCalcUVW;
  unsigned int* itsLockMappedMem;
  
  PO_DH_WOPrediff* itsPODHWO; 

};

inline int DH_WOPrediff::getWorkOrderID() const
{ return *itsWOID; }

inline void DH_WOPrediff::setWorkOrderID(int id)
{ *itsWOID = id; }

inline unsigned int DH_WOPrediff::getStatus() const
{ return *itsStatus; }

inline void DH_WOPrediff::setStatus(unsigned int status)
{ *itsStatus = status; }

inline string DH_WOPrediff::getKSType() const
{  return string(itsKSType); }

inline bool DH_WOPrediff::getInitialize() const
{ return ((*itsInitialize==0)?(false):(true)); }

inline void DH_WOPrediff::setInitialize(bool doInitialize)
{ *itsInitialize = doInitialize; }

inline bool DH_WOPrediff::getNextInterval() const
{ return ((*itsNextInterval==0)?(false):(true)); }

inline void DH_WOPrediff::setNextInterval(bool doNextInterval)
{ *itsNextInterval = doNextInterval; }

inline int DH_WOPrediff::getFirstChannel() const
{ return *itsFirstChan; }

inline void DH_WOPrediff::setFirstChannel(int nr)
{ *itsFirstChan = nr; }

inline int DH_WOPrediff::getLastChannel() const
{ return *itsLastChan; }

inline void DH_WOPrediff::setLastChannel(int nr)
{ *itsLastChan = nr; }

inline int DH_WOPrediff::getTimeInterval() const
{ return *itsTimeInterval; }

inline void DH_WOPrediff::setTimeInterval(int time)
{ *itsTimeInterval = time; }

inline int DH_WOPrediff::getDDID() const
{ return *itsDDID; }

inline void DH_WOPrediff::setDDID(int ddid)
{ *itsDDID = ddid; }

inline bool DH_WOPrediff::getCalcUVW() const
{ return ((*itsCalcUVW==0)?(false):(true)); }

inline void DH_WOPrediff::setCalcUVW(bool calc)
{ *itsCalcUVW = calc; }

inline bool DH_WOPrediff::getLockMappedMemory() const
{ return *itsLockMappedMem; }

inline void DH_WOPrediff::setLockMappedMemory(bool lock)
{ *itsLockMappedMem = lock; }

// Define the class needed to tell PL that there should be
// extra fields stored in the database table.
namespace PL {  
  template<>                                                 
  class DBRep<DH_WOPrediff> : public DBRep<DH_PL>               
  {                                                             
    public:                                                     
      void bindCols (dtl::BoundIOs& cols);                      
      void toDBRep (const DH_WOPrediff&);                        
    private:                                                    
      int itsWOID;                    // Temporarily stored in separate fields
      unsigned int itsStatus;         // in order to facilitate debugging
      string itsKSType;
      unsigned int itsInitialize;
      int itsNextInterval;
      int itsFirstChan;
      int itsLastChan;
      int itsTimeInterval;
    };   
                                                      
} // end namespace PL   

} // namespace LOFAR

#endif 

