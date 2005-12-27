//  DH_ParmSol.h: DataHolder which holds a solution for one parameter.
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

#ifndef LOFAR_BBS3_DH_PARMSOL_H
#define LOFAR_BBS3_DH_PARMSOL_H

// \file
// DataHolder which holds a solution for one parameter.

#include <Common/lofar_vector.h>
#include <TransportPL/DH_PL.h>
#include <TransportPL/PO_DH_PL.h>
#include <BBS3/Quality.h>
#include <BBS3/ParmData.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

/**
   This class is a DataHolder which holds a solution for one parameter
*/

class DH_ParmSol: public LOFAR::DH_PL
{
public:
  typedef PL::TPersistentObject<DH_ParmSol> PO_DH_SOL;

  explicit DH_ParmSol (const string& name="dh_solution");

  virtual ~DH_ParmSol();

  DataHolder* clone() const;

  // Get a reference to the PersistentObject.
  virtual PL::PersistentObject& getPO() const;

  // Create a TPO object and set the table name in it.
  virtual void initPO (const string& tableName);

  /// Allocate the buffers.
  virtual void init();

  // Data access methods
  int getWorkOrderID() const;
  void setWorkOrderID(const int id);

  int getIteration() const;
  void setIteration(const int iter);

  string getParmName() const;
  void setParmName(const string& parmName);

  double getFit() const;
  int getRank() const;
  void setQuality(const Quality& quality);

  double getStartFreq() const;
  double getEndFreq() const;
  double getStartTime() const;
  double getEndTime() const;
  void setDomain(double fStart, double fEnd, double tStart, double tEnd);

  double getCoeff0() const;
  double getCoeff1() const;
  double getCoeff2() const;
  double getCoeff3() const;
  void setCoefficients(double c0, double c1=0, double c2=0, double c3=0);

  // Resets (clears) the contents of its DataPacket 
  void clearData();
  
  void dump();

private:
  /// Forbid assignment.
  DH_ParmSol& operator= (const DH_ParmSol&);
  DH_ParmSol(const DH_ParmSol&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  PO_DH_SOL*    itsPODHSOL; 
  int*          itsWOID;             // Workorder identifier
  char*         itsParmName;         // Parameter name
  int*          itsIter;             // Iteration number
  double*       itsFit;              // LSQ fit value
  int*          itsRank;
  double*       itsStartFreq;        // Start frequency of the domain
  double*       itsEndFreq;          // End frequency of the domain
  double*       itsStartTime;        // Start time of the domain
  double*       itsEndTime;          // End time of the domain
  double*       itsCoeff0;           // Polynomial coefficients
  double*       itsCoeff1;
  double*       itsCoeff2;
  double*       itsCoeff3;

};

inline int DH_ParmSol::getWorkOrderID() const
{ return *itsWOID; }

inline void DH_ParmSol::setWorkOrderID(const int id)
{ *itsWOID = id; }

inline int DH_ParmSol::getIteration() const
{ return *itsIter; }

inline void DH_ParmSol::setIteration(const int iter)
{ *itsIter = iter; }

inline string DH_ParmSol::getParmName() const
{ return string(itsParmName); }

inline double DH_ParmSol::getFit() const
{ return *itsFit; }

inline int DH_ParmSol::getRank() const
{ return *itsRank; }

inline double DH_ParmSol::getStartFreq() const
{ return *itsStartFreq; }

inline double DH_ParmSol::getEndFreq() const
{ return *itsEndFreq; }

inline double DH_ParmSol::getStartTime() const
{ return *itsStartTime; }

inline double DH_ParmSol::getEndTime() const
{ return *itsEndTime; }

inline double DH_ParmSol::getCoeff0() const
{ return *itsCoeff0; }

inline double DH_ParmSol::getCoeff1() const
{ return *itsCoeff1; }

inline double DH_ParmSol::getCoeff2() const
{ return *itsCoeff2; }

inline double DH_ParmSol::getCoeff3() const
{ return *itsCoeff3; }

// Define the class needed to tell PL that there should be
// extra fields stored in the database table.
namespace PL {  
  template<>                                               
  class DBRep<DH_ParmSol> : public DBRep<DH_PL>               
  {                                                             
    public:                                                     
      void bindCols (dtl::BoundIOs& cols);                      
      void toDBRep (const DH_ParmSol&);                        
    private: 
                           
      int    itsWOID;
      string itsParmName;
      int    itsIter;
      double itsFit;
      int    itsRank;
      double itsStartFreq;
      double itsEndFreq;
      double itsStartTime;
      double itsEndTime;
      double itsCoeff0;
      double itsCoeff1;
      double itsCoeff2;
      double itsCoeff3;
    };   
                                                      
} // end namespace PL   

// @}

} // namespace LOFAR

#endif 
