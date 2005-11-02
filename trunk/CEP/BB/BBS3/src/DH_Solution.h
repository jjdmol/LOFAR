//  DH_Solution.h: DataHolder for BlackBoard solutions
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

#ifndef LOFAR_BBS3_DH_SOLUTION_H
#define LOFAR_BBS3_DH_SOLUTION_H

// \file DH_Solution.h
// DataHolder for BlackBoard solutions

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
   This class is a DataHolder which holds the parameters solved by
   a PSS3 knowledge source.
*/

class DH_Solution: public LOFAR::DH_PL
{
public:
  typedef PL::TPersistentObject<DH_Solution> PO_DH_SOL;

  explicit DH_Solution (const string& name="dh_solution");

  virtual ~DH_Solution();

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
  void setIteration(int iter);

  void setSolutionID(const int id);  // Set id of solution to retrieve from database
  int getSolutionID() const;

  Quality getQuality() const;
  void setQuality(const Quality& quality);

  bool getSolution(vector<ParmData>& pData);
  void setSolution(const vector<ParmData>& pData);

  double getStartFreq() const;
  double getEndFreq() const;
  double getStartTime() const;
  double getEndTime() const;
  void setDomain(double fStart, double fEnd, double tStart, double tEnd);

  // Resets (clears) the contents of its DataPacket 
  void clearData();
  
  void dump();

private:
  /// Forbid assignment.
  DH_Solution& operator= (const DH_Solution&);
  DH_Solution(const DH_Solution&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*          itsWOID;
  int*          itsIteration;
  double*       itsFit;
  double*       itsRank;
  double*       itsMu;
  double*       itsStdDev;
  double*       itsChi;
  
  PO_DH_SOL*    itsPODHSOL; 

  int itsCurDataSize;
  double*       itsStartFreq;        // Start frequency of the domain
  double*       itsEndFreq;          // End frequency of the domain
  double*       itsStartTime;        // Start time of the domain
  double*       itsEndTime;          // End time of the domain

};

inline int DH_Solution::getWorkOrderID() const
{ return *itsWOID; }

inline void DH_Solution::setWorkOrderID(const int id)
{ *itsWOID = id; }

inline int DH_Solution::getIteration() const
{ return *itsIteration; }

inline void DH_Solution::setIteration(int iter)
{ *itsIteration = iter; }

inline double DH_Solution::getStartFreq() const
{ return *itsStartFreq; }

inline double DH_Solution::getEndFreq() const
{ return *itsEndFreq; }

inline double DH_Solution::getStartTime() const
{ return *itsStartTime; }

inline double DH_Solution::getEndTime() const
{ return *itsEndTime; }


// Define the class needed to tell PL that there should be
// extra fields stored in the database table.
namespace PL {  
  template<>                                               
  class DBRep<DH_Solution> : public DBRep<DH_PL>               
  {                                                             
    public:                                                     
      void bindCols (dtl::BoundIOs& cols);                      
      void toDBRep (const DH_Solution&);                        
    private: 
                                   // Temporarily stored in separate fields
      int    itsWOID;              // in order to facilitate debugging
      int    itsIteration;
      double itsFit;
      double itsRank;
      double itsMu;
      double itsStdDev;
      double itsChi;
      double itsStartFreq;
      double itsEndFreq;
      double itsStartTime;
      double itsEndTime;
    };   
                                                      
} // end namespace PL   

// @}

} // namespace LOFAR

#endif 
