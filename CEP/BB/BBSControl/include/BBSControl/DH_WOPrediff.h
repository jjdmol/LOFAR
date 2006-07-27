//#  DH_WOPrediff.h: DataHolder containing Prediffer workorders
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

#ifndef LOFAR_BBSCONTROL_DH_WOPREDIFF_H
#define LOFAR_BBSCONTROL_DH_WOPREDIFF_H

// \file
// DataHolder containing Prediffer workorders

#include <Common/lofar_vector.h>
#include <APS/ParameterSet.h>
#include <TransportPostgres/DH_DB.h>
#include <BBS/Quality.h>

namespace LOFAR
{
  namespace BBS
  {

    using ACC::APS::ParameterSet;

    // \addtogroup BBS
    // @{

    /**
       This class is a DataHolder which contains the work orders.
    */

    class DH_WOPrediff: public DH_DB
    {
    public:

      enum woStatus{New,Assigned,Executed};

      explicit DH_WOPrediff (const string& name = "dh_woprediff");

      DH_WOPrediff(const DH_WOPrediff&);

      virtual ~DH_WOPrediff();

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

      bool getDoNothing() const;
      void setDoNothing(bool doNothing);

      bool getNewBaselines() const;
      void setNewBaselines(bool newBaselines);

      bool getNewDomain() const;
      void setNewDomain(bool newDomain);

      bool getNewPeelSources() const;
      void setNewPeelSources(bool newSources);

      bool getSubtractSources() const;
      void setSubtractSources(bool subtract);

      bool getWritePredData() const;
      void setWritePredData(bool write);

      bool getWriteInDataCol() const;
      void setWriteInDataCol(bool write);

      int getMaxIterations() const;
      void setMaxIterations(int nr);

      int getStartChannel() const;
      void setStartChannel(int chan);

      int getEndChannel() const;
      void setEndChannel(int chan);

      double getStartTime() const;
      void setStartTime(double time);

      double getTimeLength() const;
      void setTimeLength(double time);

      string getModelType() const;
      void setModelType(const string& type);

      bool getCalcUVW() const;
      void setCalcUVW(bool calc);

      bool getUseAutoCorrelations() const;
      void setUseAutoCorrelations(bool use);

      bool getCleanUp() const;
      void setCleanUp(bool clean);

      bool getUpdateParms() const;
      void setUpdateParms(bool update);

      int getSolutionID() const;
      void setSolutionID(int id);

      void setVarData(const ParameterSet& predArgs,
		      vector<int>& antNrs,
		      vector<string>& pNames,
		      vector<string>& exPNames,
		      vector<int>& peelSrcs,
		      vector<int>& corrs);
      bool getVarData(ParameterSet& predArgs,
		      vector<int>& antNrs,
		      vector<string>& pNames,
		      vector<string>& exPNames,
		      vector<int>& peelSrcs,
		      vector<int>& corrs);

      virtual void dump() const;

      void clearData();

      // Get maximum scid present in prediffer workorder table. Returns 0 if
      // empty.
      int getMaxSCID(TH_DB* th);
      // Get maximum woid present in prediffer workorder table. Returns 0 if
      // empty.
      int getMaxWOID(TH_DB* th);

    protected:
      // Methods to obtain the specific queries to insert/update this
      // DataHolder
      string createInsertStatement(TH_DB* th);

      // \note This implementation assumes only the
      // status has changed and needs to be updated
      string createUpdateStatement(TH_DB* th);
    private:
      /// Forbid assignment.
      DH_WOPrediff& operator= (const DH_WOPrediff&);

      // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
      virtual void fillDataPointers();

      // Unique workorder id
      int*          itsWOID;

      // ID of the sending StrategyController (SC)
      int*          itsSCID;

      // Workorder status
      unsigned int* itsStatus;

      // Knowledge Source type
      char*         itsKSType;

      // Do nothing?
      unsigned int* itsDoNothing;

      // New baseline selection?
      unsigned int* itsNewBaselines;

      // New domain selection?
      unsigned int* itsNewDomain;

      // New peel sources selection?
      unsigned int* itsNewPeelSources;

      // Subtract peel sources?
      unsigned int* itsSubtractSources;

      // Write predicted data?
      unsigned int* itsWritePredData;

      // Write predicted data in DATA column?
      unsigned int* itsWriteInDataCol;

      // Maximum number of iterations to do
      int*          itsMaxIterations;

      // Start channel
      int*          itsStartChan;

      // End channel
      int*          itsEndChan;

      // Start time of time interval
      double*       itsStartTime;

      // Time interval size (s)
      double*       itsTimeLength;

      char*         itsModelType;
      unsigned int* itsCalcUVW;
      unsigned int* itsUseAutoCorr;

      // Clean up Prediffer object when finished?
      unsigned int* itsCleanUp;

      // Update solvable parameters?
      unsigned int* itsUpdateParms;

      // (wo)ID of solution in solution table
      int*          itsSolutionID; 
  
    };

    inline int DH_WOPrediff::getWorkOrderID() const
    { return *itsWOID; }

    inline void DH_WOPrediff::setWorkOrderID(int id)
    { *itsWOID = id; }

    inline int DH_WOPrediff::getStrategyControllerID() const
    { return *itsSCID; }

    inline void DH_WOPrediff::setStrategyControllerID(int id)
    { *itsSCID = id; }

    inline unsigned int DH_WOPrediff::getStatus() const
    { return *itsStatus; }

    inline void DH_WOPrediff::setStatus(unsigned int status)
    { *itsStatus = status; }

    inline string DH_WOPrediff::getKSType() const
    { return string(itsKSType); }

    inline bool DH_WOPrediff::getDoNothing() const
    { return ((*itsDoNothing==0)?(false):(true)); }

    inline void DH_WOPrediff::setDoNothing(bool doNothing)
    { *itsDoNothing = doNothing; }

    inline bool DH_WOPrediff::getNewBaselines() const
    { return ((*itsNewBaselines==0)?(false):(true)); }

    inline void DH_WOPrediff::setNewBaselines(bool newBaselines)
    { *itsNewBaselines = newBaselines; }

    inline bool DH_WOPrediff::getNewDomain() const
    { return ((*itsNewDomain==0)?(false):(true)); }

    inline void DH_WOPrediff::setNewDomain(bool newDomain)
    { *itsNewDomain = newDomain; }

    inline bool DH_WOPrediff::getNewPeelSources() const
    { return ((*itsNewPeelSources==0)?(false):(true)); }

    inline void DH_WOPrediff::setNewPeelSources(bool newSources)
    { *itsNewPeelSources = newSources; }

    inline bool DH_WOPrediff::getSubtractSources() const
    { return ((*itsSubtractSources==0)?(false):(true)); }

    inline void DH_WOPrediff::setSubtractSources(bool subtract)
    { *itsSubtractSources = subtract; }

    inline bool DH_WOPrediff::getWritePredData() const
    { return ((*itsWritePredData==0)?(false):(true)); }

    inline void DH_WOPrediff::setWritePredData(bool write)
    { *itsWritePredData = write; }

    inline bool DH_WOPrediff::getWriteInDataCol() const
    { return ((*itsWriteInDataCol==0)?(false):(true)); }

    inline void DH_WOPrediff::setWriteInDataCol(bool write)
    { *itsWriteInDataCol = write; }

    inline int DH_WOPrediff::getMaxIterations() const
    { return *itsMaxIterations; }

    inline void DH_WOPrediff::setMaxIterations(int nr)
    { *itsMaxIterations = nr; }

    inline int DH_WOPrediff::getStartChannel() const
    { return *itsStartChan; }

    inline void DH_WOPrediff::setStartChannel(int chan)
    { *itsStartChan = chan; }

    inline int DH_WOPrediff::getEndChannel() const
    { return *itsEndChan; }

    inline void DH_WOPrediff::setEndChannel(int chan)
    { *itsEndChan = chan; }

    inline double DH_WOPrediff::getStartTime() const
    { return *itsStartTime; }

    inline void DH_WOPrediff::setStartTime(double time)
    { *itsStartTime = time; }

    inline double DH_WOPrediff::getTimeLength() const
    { return *itsTimeLength; }

    inline void DH_WOPrediff::setTimeLength(double time)
    { *itsTimeLength = time; }

    inline bool DH_WOPrediff::getCalcUVW() const
    { return ((*itsCalcUVW==0)?(false):(true)); }

    inline void DH_WOPrediff::setCalcUVW(bool calc)
    { *itsCalcUVW = calc; }

    inline bool DH_WOPrediff::getUseAutoCorrelations() const
    { return ((*itsUseAutoCorr==0)?(false):(true)); }

    inline void DH_WOPrediff::setUseAutoCorrelations(bool use)
    { *itsUseAutoCorr = use; }

    inline bool DH_WOPrediff::getCleanUp() const
    { return *itsCleanUp; }

    inline void DH_WOPrediff::setCleanUp(bool clean)
    { *itsCleanUp = clean; }

    inline bool DH_WOPrediff::getUpdateParms() const
    { return *itsUpdateParms; }

    inline void DH_WOPrediff::setUpdateParms(bool update)
    { *itsUpdateParms = update; }

    inline int DH_WOPrediff::getSolutionID() const
    { return *itsSolutionID; }
 
    inline void DH_WOPrediff::setSolutionID(int id)
    { *itsSolutionID = id; }

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif 

