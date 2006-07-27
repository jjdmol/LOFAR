//#  DH_WOSolve.h: DataHolder containing workorder for the solver
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

#ifndef LOFAR_BBSCONTROL_DH_WOSOLVE_H
#define LOFAR_BBSCONTROL_DH_WOSOLVE_H

// \file
// DataHolder containing workorder for the solver

#include <Common/lofar_vector.h>

#include <TransportPostgres/DH_DB.h>
#include <BBS/Quality.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBS
    // @{

    // This class is a DataHolder which contains the work orders for the
    // Solver.
    class DH_WOSolve: public DH_DB
    {
    public:

      enum woStatus{New,Assigned,Executed};

      explicit DH_WOSolve (const string& name = "dh_wosolve");

      DH_WOSolve(const DH_WOSolve&);

      virtual ~DH_WOSolve();

      DataHolder* clone() const;

      /// Allocate the buffers.
      virtual void init();

      /// Data access methods.
      int getWorkOrderID() const;
      void setWorkOrderID(int id);

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

      int getMaxIterations() const;
      void setMaxIterations(int nr);

      double getFitCriterion() const;
      void setFitCriterion(double val);

      bool getUseSVD() const;
      void setUseSVD(bool useSVD);

      bool getCleanUp() const;
      void setCleanUp(bool clean);

      virtual void dump() const;

      void clearData();

    protected:
      // Methods to obtain the specific queries to insert/update this
      // DataHolder
      string createInsertStatement(TH_DB* th);

      // \note This implementation assumes only the status has changed and
      // needs to be updated
      string createUpdateStatement(TH_DB* th);

    private:
      /// Forbid assignment.
      DH_WOSolve& operator= (const DH_WOSolve&);

      // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
      virtual void fillDataPointers();

      // Unique workorder id
      int*          itsWOID;

      // ID of sending StrategyController (SC)
      int*          itsSCID;

      // Workorder status
      unsigned int* itsStatus;

      // Knowledge Source type
      char*         itsKSType;

      // Iteration number
      int*          itsIteration;

      // Do nothing?
      unsigned int* itsDoNothing;

      // Solve on a new domain?
      unsigned int* itsNewDomain;

      // Maximum number of iterations to do
      int*          itsMaxIterations;

      // Iterate until this value is obtained for the fit
      double*       itsFitCriterion;

      // UseSVD in solver?
      unsigned int* itsUseSVD;

      // Clean up Solver when finished?
      unsigned int* itsCleanUp;
 
    };

    inline int DH_WOSolve::getWorkOrderID() const
    { return *itsWOID; }

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

    inline int DH_WOSolve::getMaxIterations() const
    { return *itsMaxIterations; }

    inline void DH_WOSolve::setMaxIterations(int nr)
    { *itsMaxIterations = nr; }

    inline double DH_WOSolve::getFitCriterion() const
    { return *itsFitCriterion; }

    inline void DH_WOSolve::setFitCriterion(double val)
    { *itsFitCriterion = val; }

    inline bool DH_WOSolve::getUseSVD() const
    { return ((*itsUseSVD==0)?(false):(true)); }

    inline void DH_WOSolve::setUseSVD(bool useSVD)
    { *itsUseSVD = useSVD; }

    inline bool DH_WOSolve::getCleanUp() const
    { return *itsCleanUp; }

    inline void DH_WOSolve::setCleanUp(bool clean)
    { *itsCleanUp = clean; }

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif 

