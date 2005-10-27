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

#ifndef LOFAR_BBS3_DH_WOSOLVE_H
#define LOFAR_BBS3_DH_WOSOLVE_H

// \file DH_WOSolve.h
// DataHolder containing workorder for the solver

#include <Common/lofar_vector.h>

#include <TransportPL/DH_PL.h>
#include <TransportPL/PO_DH_PL.h>
#include <BBS3/Quality.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

/**
   This class is a DataHolder which contains the work orders for the Solver.
*/

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
  virtual void init();

  /// Data access methods.
  int getWorkOrderID() const;
  void setNewWorkOrderID();

  int getStrategyControllerID() const;
  void setStrategyControllerID(int id);

  unsigned int getStatus() const ;
  void setStatus(unsigned int status);

  string getKSType() const;
  void setKSType(const string& ksType);

  int getIteration() const;
  void setIteration(int iter);

  bool getDoNothing() const;
  void setDoNothing(bool doNothing);

  bool getNewDomain() const;
  void setNewDomain(bool newDomain);

  bool getUseSVD() const;
  void setUseSVD(bool useSVD);

  bool getCleanUp() const;
  void setCleanUp(bool clean);

  void dump();

  void clearData();

private:
  /// Forbid assignment.
  DH_WOSolve& operator= (const DH_WOSolve&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  void setWorkOrderID(int id);

  int*          itsWOID;                    // Unique workorder id
  int*          itsSCID;                    // ID of sending StrategyController (SC)
  unsigned int* itsStatus;                  // Workorder status
  char*         itsKSType;                  // Knowledge Source type
  int*          itsIteration;               // Iteration number
  unsigned int* itsDoNothing;               // Do nothing?
  unsigned int* itsNewDomain;               // Solve on a new domain?
  unsigned int* itsUseSVD;                  // UseSVD in solver?
  unsigned int* itsCleanUp;                 // Clean up Solver when finished?
 
  PO_DH_WOSOLVE*    itsPODHWO; 

};

inline int DH_WOSolve::getWorkOrderID() const
{ return *itsWOID; }

inline void DH_WOSolve::setNewWorkOrderID()
{ *itsWOID = *itsWOID + 1; }

inline void DH_WOSolve::setWorkOrderID(int id)
{ *itsWOID = id; }

inline int DH_WOSolve::getStrategyControllerID() const
{ return *itsSCID; }

inline void DH_WOSolve::setStrategyControllerID(int id)
{ *itsSCID = id; }

inline unsigned int DH_WOSolve::getStatus() const
{ return *itsStatus; }

inline void DH_WOSolve::setStatus(unsigned int status)
{ *itsStatus = status; }

inline string DH_WOSolve::getKSType() const
{  return string(itsKSType); }

inline int DH_WOSolve::getIteration() const
{ return *itsIteration; }

inline void DH_WOSolve::setIteration(int iter)
{ *itsIteration = iter; }

inline bool DH_WOSolve::getDoNothing() const
{ return ((*itsDoNothing==0)?(false):(true)); }

inline void DH_WOSolve::setDoNothing(bool doNothing)
{ *itsDoNothing = doNothing; }

inline bool DH_WOSolve::getNewDomain() const
{ return ((*itsNewDomain==0)?(false):(true)); }

inline void DH_WOSolve::setNewDomain(bool doNewDomain)
{ *itsNewDomain = doNewDomain; }

inline bool DH_WOSolve::getUseSVD() const
{ return ((*itsUseSVD==0)?(false):(true)); }

inline void DH_WOSolve::setUseSVD(bool useSVD)
{ *itsUseSVD = useSVD; }

inline bool DH_WOSolve::getCleanUp() const
{ return *itsCleanUp; }

inline void DH_WOSolve::setCleanUp(bool clean)
{ *itsCleanUp = clean; }


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
      int          itsWOID;         // Temporarily stored in separate fields
      int          itsSCID;         // in order to facilitate debugging
      unsigned int itsStatus;
      string       itsKSType;
      int          itsIteration;
      unsigned int itsDoNothing;
      unsigned int itsNewDomain;
      unsigned int itsUseSVD;
      unsigned int itsCleanUp;
    };   
                                                      
} // end namespace PL   

// @}

} // namespace LOFAR

#endif 

