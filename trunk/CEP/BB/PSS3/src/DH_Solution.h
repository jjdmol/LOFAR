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

#ifndef PSS3_DH_SOLUTION_H
#define PSS3_DH_SOLUTION_H

#include <Common/lofar_complex.h>

#include <lofar_config.h>

#include <CEPFrame/DH_Postgresql.h>
#include <PSS3/Quality.h>

/**
   This class is a DataHolder which holds the parameters solved by
   a pSS3 knowledge source.
*/

class DH_Solution: public LOFAR::DH_Postgresql
{
public:

  explicit DH_Solution (const string& name, const string& type);

  virtual ~DH_Solution();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Database store/retrieve methods
  bool StoreInDatabase(int appId, int tag, char* buf, int size);
  bool RetrieveFromDatabase(int appId, int tag, char* buf, int size);

  // Data access methods
  int getID();
  void setID(int id);
  int getWorkOrderID();
  void setWorkOrderID(int id);
  float getRAValue(int sourceNo);
  void setRAValue(int sourceNo, float val);
  float getDECValue(int sourceNo);
  void setDECValue(int sourceNo, float val);
  float getStokesIValue(int sourceNo);
  void setStokesIValue(int sourceNo, float val);
  int getIterationNo();
  void setIterationNo(int no);
  Quality* getQuality();

  void setSolutionID(int id);  // Set id of solution to retrieve from database
  int getSolutionID();
  
protected:
  // Definition of the DataPacket type.
  class DataPacket: public DH_Database::DataPacket
  {
  public:
    DataPacket();
    int itsID;                         // Unique id
    int itsWOID;                       // ID of the corresponding WorkOrder
/*     vector<string> itsParamNames;      // Names of parameters */
/*     vector<double> itsParamValues;     // Values of parameters */
    float itsRAValues[10];              // Array containing RA.CPx values
    float itsDECValues[10];             // Array containing DEC.CPx values
    float itsStokesIValues[10];         // Array containing StokesI.CPx values
    int itsIteration;                   // Its iteration
    Quality itsQuality;                 // Its solution quality

  };

private:

  /// Forbid assignment.
  DH_Solution& operator= (const DH_Solution&);
  DH_Solution(const DH_Solution&);
  DataPacket itsDataPacket;

  string itsType;  // Its type (Control or KS). This will determine which query
                   // is executed in RetrieveFromDatabase
  int itsDBid;     // ID of solution to retrieve from database

};

inline int DH_Solution::getID()
{ return itsDataPacket.itsID; }

inline void DH_Solution::setID(int id)
{ itsDataPacket.itsID = id; }

inline int DH_Solution::getWorkOrderID()
{ return itsDataPacket.itsWOID; }

inline void DH_Solution::setWorkOrderID(int id)
{ itsDataPacket.itsWOID = id; }

inline float DH_Solution::getRAValue(int sourceNo)
{ return itsDataPacket.itsRAValues[sourceNo-1]; }

inline void DH_Solution::setRAValue(int sourceNo, float val)
{ itsDataPacket.itsRAValues[sourceNo-1] = val; }

inline float DH_Solution::getDECValue(int sourceNo)
{ return itsDataPacket.itsDECValues[sourceNo-1]; }

inline void DH_Solution::setDECValue(int sourceNo, float val)
{ itsDataPacket.itsDECValues[sourceNo-1] = val; }

inline float DH_Solution::getStokesIValue(int sourceNo)
{ return itsDataPacket.itsStokesIValues[sourceNo-1]; }

inline void DH_Solution::setStokesIValue(int sourceNo, float val)
{ itsDataPacket.itsStokesIValues[sourceNo-1] = val; }

inline int DH_Solution::getIterationNo()
{ return itsDataPacket.itsIteration; }

inline void DH_Solution::setIterationNo(int no)
{ itsDataPacket.itsIteration = no; }

inline Quality* DH_Solution::getQuality()
{ return &itsDataPacket.itsQuality; }

inline void DH_Solution::setSolutionID(int id)
{ itsDBid = id; }

inline int DH_Solution::getSolutionID()
{ return itsDBid; }

#endif 
