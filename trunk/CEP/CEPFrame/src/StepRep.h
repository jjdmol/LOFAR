//# StepRep.h: Class representing a basic simulation block
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef CEPFRAME_STEPREP_H
#define CEPFRAME_STEPREP_H

#include <lofar_config.h>

#include "CEPFrame/BaseSim.h"
#include "CEPFrame/WorkHolder.h"
#include "CEPFrame/TransportHolder.h"
#include <stdlib.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

class Step;
class SimulRep;
class CorbaMonitor;

/** The Step class is the basic building block for simulations.
    In the constructor the actial worker and dataholders are defined. 
    The actual simulation work is performed in the process() method
    (which calls the Workholder::baseProcess() method)
*/

class StepRep
{
public:
  /** Normally used basic constructor
      a pointer to a workholder (containing the dataholders) is passed 
  */
  StepRep (WorkHolder& worker, 
	   const string& name,
	   bool addNameSuffix,
	   bool monitor);

  virtual ~StepRep();

  /// Increment reference count.
  void incrRefCount();

  /// Decremented reference count and return decremented count.
  int decrRefCount();

  /** The preprocess method is called before process.
      It can be used to initialize WorkHolders, etc.
  */
  virtual void preprocess();

  /** The process method is the basical simulation step.
      It will first read the data for the in DataHolders in the workholder.
      Then it will call Workholder->process().
      Finally it will write the data from the DataHolders
      (e.g to another MPI process).
  */
  virtual void process();

  /** The postprocess method is called after process.
      It can be used to clean up WorkHolders, etc.
  */
  virtual void postprocess();

  /// Dump information to user
  virtual void dump() const;

  /// This methods distinguishes between StepRep and SimulRep class.
  virtual bool isSimul() const;

  /// returns pointer to the WorkHolder.
  WorkHolder* getWorker();

  /// Get i-th input DataHolder.
  DataHolder& getInData (int dhIndex);
  /// Get i-th output DataHolder.
  DataHolder& getOutData (int dhIndex);

  /// Get Transport object for i-th input DataHolder.
  Transport& getInTransport (int dhIndex) const;
  /// Get Tranport object for i-th output DataHolder.
  Transport& getOutTransport (int dhIndex) const;

  /// Get ID of the StepRep.
  int getID() const;  

  /// Get the node number for this StepRep.
  int getNode() const; 

  /// Get the application number for this StepRep.
  int getAppl() const; 

  /** Set Node numbers for both In and OutData
      Set application number as well.
  */
  virtual void runOnNode(int aNode, int applNr);

  /// return this StepRep's name
  const string& getName() const;

  /// set the Step's name
  void setName (const string& name);

  /** Get the parent simul object.
      0 means that the Step is not used in a Simul.
  */
  SimulRep* getParent() const;

  /// Set the parent simul object.
  void setParent (SimulRep& parent);

  /** Get the sequence number of the Step in its parent Simul. 
      -1 means thar the Step is not used in a Simul.
  */
  int getSeqNr() const;

  /** Set the sequence number.
      If needed, it is also added as a suffix to the name.
  */
  void setSeqNr (int seqNr);

  /// Basic helper function for ConnectXXXX methods.
  bool connectRep (StepRep* aStep, 
		   int thisDHIndex,
		   int thatDHIndex,
		   int nrDH,
		   const TransportHolder& prototype);
  
  /**
     Connect the output DataHolders of aStep to the input DHs of the current
     Step. This is the normal way of connecting two Steps to each other.
  */
  bool connectInput (Step* aStep,
		     const TransportHolder& prototype);

  /**
     Connect all output DataHolders in the array of Steps (aStep[]) to
     the input DataHolders of the current step. This is the normal way
     of connecting the output of multiple Steps to a Simul object.
     If the nrSteps argument is -1, it is assumed that
     each of the Steps in the array has only one input DataHolder
     which will be connected to the output DataHolders of the Simul.
  */
  bool connectInputArray (Step* aStep[], // pointer to array of ptrs to Steps
			  int   nrSteps, // nr of Steps in aStep[] array
			  const TransportHolder& prototype);

  /**
     Connect all input DataHolders in the array of Steps (aStep[]) to
     the output DataHolders of the current step. This is the normal
     way of connecting the output of a Simul object to Steps.
     If the nrSteps argument is -1, it is assumed that
     each of the Steps in the array has only one output DataHolder
     which will be connected to the input DataHolders of the Simul.
  */
  bool connectOutputArray (Step* aStep[], // pointer to array of ptrs to Steps
			   int   nrSteps, // nr of Steps in aStep[] array
			   const TransportHolder& prototype);

  // Connect named parameter of aStep to that of the current step
  bool connectParam(const string& name, Step* aStep,
		    const TransportHolder& prototype);

  // Check the connection.
  virtual bool checkConnections (ostream&, const StepRep* parent);

  /** Shortcut the connections by removing all possible Simul
      connections. In this way the steps in different simuls communicate
      directly.
  */
  virtual void shortcutConnections();

  /** Simplify the connections by using TH_Mem for all connections between
      Steps running on the same node.
  */
  virtual void simplifyConnections();
 
 /** Optimize connections by replacing a TransportHolder with
      a possibly more efficient TransportHolder (newTH)
  */
  virtual void optimizeConnectionsWith(const TransportHolder& newTH);
 
  /** SetRate methods:
      These methods set the rate at which input/output dataholders are
      read/written.
      The rate is the fraction of the events that is to be read/written.
      The dhIndex argument selects only the given input/output
      DataHolder; dhIndex=-1 sets all input/output DataHolders.
      setRate calls both setInRate and setOutRate and also sets the
      Step::itsRate.
  */
  bool setRate (int rate, int dhIndex);
  bool setInRate (int rate, int dhIndex);
  bool setOutRate (int rate, int dhIndex);

  /// Get the rate of this Step.
  int getRate() const;

  /// Get the application number of the current run.
  static int getCurAppl();

  /** Decide whether to handle this event or not based on itsRate and
      theirEventCnt
  */
  bool doHandle() const
    { return theirEventCnt % itsRate == 0; } 

  /// Get the event count.
  static unsigned int getEventCount();

  /// Clear the event count.
  static void clearEventCount();

  /// Set the current application number.
  static void setCurAppl (int applNr);

protected:
  /// Increment theirEventCnt.
  static void incrementEventCount();

  /// Set ID of the StepRep and all its Transports.
  void setID();

  /// Execute process here?
  bool shouldProcess() const;

  /// Connect 2 transports with the given transport prototype.
  static bool connectData (const TransportHolder& prototype,
			   DataHolder& sourceData, DataHolder& targetData);

private:

  /// Connect 2 ParamHolders with the given transport holder prototype.
  bool connectParamHolders (ParamHolder& srcParam, ParamHolder& tgtParam,
			    const TransportHolder& prototype);

  int         itsRefCount;
  WorkHolder* itsWorker;
  // The parent Simul.
  SimulRep*   itsParent;
  // Rank of the current run from MPI::Get_Rank()
  int         itsCurRank;
  // The application number of this run.
  static int          theirCurAppl;
  static unsigned int theirEventCnt;
  // This will give all instances of Step the same event in the
  // Profiling output
  static int          theirProcessProfilerState; 
  static unsigned int theirNextID;
  static unsigned int theirNextConnID; // the ID of the next Param connection  
  int                 itsID;   // the ID of the step
  int                 itsNode; // the node to run this step on
  int                 itsAppl; // the application to run this step in
  // The rate at which process() has to be called.
  int                 itsRate;
  // Add the seqnr as the name suffix?
  bool                itsAddSuffix;
  // Sequence number in the Simul. Used to know the Step order.
  int                 itsSeqNr;
  // Name of the Step object.
  string              itsName;
  CorbaMonitor*       itsMonitor;

};

////////////////////////////////////////////////////////////////////////////

inline void StepRep::incrRefCount()
  { ++itsRefCount; }

inline int StepRep::decrRefCount()
  { return --itsRefCount; }

inline WorkHolder* StepRep::getWorker ()
  { return itsWorker; }

inline DataHolder& StepRep::getInData (int dhIndex)
  { return *itsWorker->getDataManager().getGeneralInHolder(dhIndex); }

inline DataHolder& StepRep::getOutData (int dhIndex)
  { return *itsWorker->getDataManager().getGeneralOutHolder(dhIndex); }

inline Transport& StepRep::getInTransport (int dhIndex) const
  { return itsWorker->getDataManager().getGeneralInHolder(dhIndex)->getTransport(); }

inline Transport& StepRep::getOutTransport (int dhIndex) const
  { return itsWorker->getDataManager().getGeneralOutHolder(dhIndex)->getTransport(); }

inline const string& StepRep::getName() const
  { return itsName; } 

inline void StepRep::setName (const string& name)
  { itsName = name; }

inline int StepRep::getRate() const
  { return itsRate; } 

inline int StepRep::getID() const
  { return itsID; }

inline int StepRep::getNode() const
  { return itsNode; } 

inline int StepRep::getAppl() const
  { return itsAppl; } 

inline int StepRep::getCurAppl()
  { return theirCurAppl; } 

inline void StepRep::setCurAppl (int applNr)
  { theirCurAppl = applNr; } 

inline unsigned int StepRep::getEventCount()
  { return theirEventCnt; }

inline void StepRep::clearEventCount()
  { theirEventCnt = 0; }

inline void StepRep::incrementEventCount()
  { theirEventCnt++; } 

inline bool StepRep::shouldProcess() const
{
  return (itsNode == itsCurRank  ||  itsCurRank < 0)
    && itsAppl == theirCurAppl;
}

inline SimulRep* StepRep::getParent() const
  { return itsParent; }

inline void StepRep::setParent (SimulRep& parent)
  { itsParent = &parent; }

inline int StepRep::getSeqNr() const
  { return itsSeqNr; }

}

#endif
