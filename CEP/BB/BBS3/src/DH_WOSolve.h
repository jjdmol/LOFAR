//  DH_WOSolve.h: DataHolder containing workorder for the solver
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

#ifndef BBS3_DH_WOSOLVE_H
#define BBS3_DH_WOSOLVE_H

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

class DH_WOSolve: public DH_PL
{
public:

  enum woStatus{New,Assigned,Executed};

  typedef PL::TPersistentObject<DH_WOSolve> PO_DH_WOSOLVE;

  explicit DH_WOSolve (const string& name = "dh_wosolve");

  DH_WOSolve(const DH_WOSolve&);

  virtual ~DH_WOSolve();

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
  int getKSTypeLength();
  void setKSTypeLength(int length);

  bool getInitialize() const;
  void setInitialize(bool doInitialize);

  bool getNextInterval() const;
  void setNextInterval(bool doNextInterval);

  bool getUseSVD() const;
  void setUseSVD(bool useSVD);

  void setVarData(const KeyValueMap& msArgs, 
		  int timeInterval,
		  vector<string>& pNames);
  bool getVarData(KeyValueMap& msArgs,
		  int& timeInterval,
		  vector<string>& pNames);

  void dump();

  void clearData();

private:
  /// Forbid assignment.
  DH_WOSolve& operator= (const DH_WOSolve&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*          itsWOID;                    // Unique workorder id
  unsigned int* itsStatus;                  // Workorder status
  char*         itsKSType;                  // Knowledge Source type
  unsigned int* itsInitialize;              // Do initialization?
  unsigned int* itsNextInterval;            // Do nextInterval?
  unsigned int* itsUseSVD;                  // UseSVD in solver?
 
  PO_DH_WOSOLVE*    itsPODHWO; 

};

inline int DH_WOSolve::getWorkOrderID() const
{ return *itsWOID; }

inline void DH_WOSolve::setWorkOrderID(int id)
{ *itsWOID = id; }

inline unsigned int DH_WOSolve::getStatus() const
{ return *itsStatus; }

inline void DH_WOSolve::setStatus(unsigned int status)
{ *itsStatus = status; }

inline string DH_WOSolve::getKSType() const
{  return string(itsKSType); }

inline bool DH_WOSolve::getInitialize() const
{ return ((*itsInitialize==0)?(false):(true)); }

inline void DH_WOSolve::setInitialize(bool doInitialize)
{ *itsInitialize = doInitialize; }

inline bool DH_WOSolve::getNextInterval() const
{ return ((*itsNextInterval==0)?(false):(true)); }

inline void DH_WOSolve::setNextInterval(bool doNextInterval)
{ *itsNextInterval = doNextInterval; }

inline bool DH_WOSolve::getUseSVD() const
{ return ((*itsUseSVD==0)?(false):(true)); }

inline void DH_WOSolve::setUseSVD(bool useSVD)
{ *itsUseSVD = useSVD; }


// Define the class needed to tell PL that there should be
// extra fields stored in the database table.
namespace PL {  
  template<>                                                 
  class DBRep<DH_WOSolve> : public DBRep<DH_PL>               
  {                                                             
    public:                                                     
      void bindCols (dtl::BoundIOs& cols);                      
      void toDBRep (const DH_WOSolve&);                        
    private:                                                    
      int itsWOID;                    // Temporarily stored in separate fields
      unsigned int itsStatus;         // in order to facilitate debugging
      string itsKSType;
      unsigned int itsInitialize;
      unsigned int itsNextInterval;
      unsigned int itsUseSVD;
    };   
                                                      
} // end namespace PL   

} // namespace LOFAR

#endif 

