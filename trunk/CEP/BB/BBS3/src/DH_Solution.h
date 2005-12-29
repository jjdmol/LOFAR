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

// \file
// DataHolder for BlackBoard solutions

#include <Common/lofar_vector.h>
#include <TransportPostgres/DH_DB.h>
#include <BBS3/Quality.h>
#include <BBS3/ParmData.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

/**
   This class is a DataHolder which holds the parameters solved by
   a BBS3 Solver knowledge source.
*/

class DH_Solution: public LOFAR::DH_DB
{
public:
  explicit DH_Solution (const string& name="dh_solution", bool writeIndivParms=false,
			const string& parmTableName="bbs3parmsolutions");

  virtual ~DH_Solution();

  DataHolder* clone() const;

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

  bool hasConverged() const;
  void setConverged(bool converged);
  
  // Resets (clears) the contents of its DataPacket 
  void clearData();
  
  void dump();

 protected:
  virtual string createInsertStatement(TH_DB* th);

private:
  /// Forbid assignment.
  DH_Solution& operator= (const DH_Solution&);
  DH_Solution(const DH_Solution&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*          itsWOID;
  int*          itsIteration;
  double*       itsFit;
  //  int*          itsRank;
  double*       itsMu;
  double*       itsStdDev;
  double*       itsChi;
  
  double*       itsStartFreq;        // Start frequency of the domain
  double*       itsEndFreq;          // End frequency of the domain
  double*       itsStartTime;        // Start time of the domain
  double*       itsEndTime;          // End time of the domain

  unsigned int* itsHasConverged;     // Does the solution comply with the convergence criterion?
  
  bool          itsWriteIndivParms;  // Write parameters individually in a subtable? 
  string        itsParmTableName;    // Table name of individual parameter storage
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

inline bool DH_Solution::hasConverged() const
{ return ((*itsHasConverged==0)?(false):(true)); }

inline void DH_Solution::setConverged(bool converged)
{ *itsHasConverged = converged; }

// @}

} // namespace LOFAR

#endif 
