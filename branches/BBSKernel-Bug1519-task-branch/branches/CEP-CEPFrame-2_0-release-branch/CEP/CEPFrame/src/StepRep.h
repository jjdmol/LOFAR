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

#ifndef LOFAR_CEPFRAME_STEPREP_H
#define LOFAR_CEPFRAME_STEPREP_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//#include <Transport/BaseSim.h>
#include <CEPFrame/BlockRep.h>
#include <tinyCEP/WorkHolder.h>
#include <Transport/TransportHolder.h>
#include <CEPFrame/DataManager.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

class Step;
class CompositeRep;

/** The Step class is the basic building block for simulations.
    In the constructor the actial worker and dataholders are defined. 
    The actual simulation work is performed in the process() method
    (which calls the Workholder::baseProcess() method)
*/

class StepRep : public BlockRep
{
public:
  /** Normally used basic constructor
      a pointer to a workholder (containing the dataholders) is passed 
  */
  StepRep (WorkHolder& worker, 
	   const string& name,
	   bool addNameSuffix);

  virtual ~StepRep();

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

  /// This methods distinguishes between StepRep and CompositeRep class.
  virtual bool isComposite() const;

  /// Get DataManager handling a specific in/output channel
  DataManager& getInDataManager(int channel);
  DataManager& getOutDataManager(int channel);

  /// Get the channel number in the actual DataManager of an input/output channel.
  /// For a Step these will be identical.
  int getInChannelNumber(int channel);
  int getOutChannelNumber(int channel);

  /// returns pointer to the WorkHolder.
  WorkHolder* getWorker();

  /// Get number of inputs/outputs
  int getNrInputs() const;
  int getNrOutputs() const;

/*   /// Get i-th input DataHolder. */
/*   DataHolder& getInData (int dhIndex); */
/*   /// Get i-th output DataHolder. */
/*   DataHolder& getOutData (int dhIndex); */

  /// Get the application number for this StepRep.
  int getAppl() const; 

  /** Set Node numbers for both In and OutData
      Set application number as well.
  */
  void runOnNode(int aNode, int applNr);

  /// Get the node number for this Step.
  int getNode() const; 

 /** Optimize connections by replacing a TransportHolder with
      a possibly more efficient TransportHolder (newTH)
  */
/*   virtual void replaceConnectionsWith(const TransportHolder& newTH, */
/* 				      bool blockingComm); */
 
  /** SetRate methods:
      These methods set the rate at which input/output dataholders are
      read/written.
      The rate is the fraction of the events that is to be read/written.
      The dhIndex argument selects only the given input/output
      DataHolder; dhIndex=-1 sets all input/output DataHolders.
      setRate calls both setInRate and setOutRate and also sets the
      Step::itsRate.
  */
  void setProcessRate (int rate);
  void setInRate (int rate, int dhIndex);
  void setOutRate (int rate, int dhIndex);

  /// Get the application number of the current run.
  static int getCurAppl();

  /// Set the current application number.
  static void setCurAppl (int applNr);

private:

  int         itsRefCount;

  WorkHolder* itsWorker;
  DataManager* itsDataManager;
  // The parent Composite.
  CompositeRep*   itsParent;

  // Add the seqnr as the name suffix?
  bool                itsAddSuffix;
  // Sequence number in the Composite. Used to know the Step order.
  int                 itsSeqNr;
  // Name of the Step object.
  string              itsName;

};

////////////////////////////////////////////////////////////////////////////

inline WorkHolder* StepRep::getWorker ()
  { return itsWorker; }

inline DataManager& StepRep::getInDataManager(int)
  { return (DataManager&)(itsWorker->getDataManager()); }

inline DataManager& StepRep::getOutDataManager(int)
  { return (DataManager&)(itsWorker->getDataManager()); }

inline int StepRep::getInChannelNumber(int channel)
  { return channel; }

inline int StepRep::getOutChannelNumber(int channel)
  { return channel; }

inline int StepRep::getNrInputs() const
  { return itsWorker->getDataManager().getInputs(); }

inline int StepRep::getNrOutputs() const
  { return itsWorker->getDataManager().getOutputs(); }

/* inline DataHolder& StepRep::getInData (int dhIndex) */
/*   { return *itsWorker->getDataManager().getGeneralInHolder(dhIndex); } */

/* inline DataHolder& StepRep::getOutData (int dhIndex) */
/*   { return *itsWorker->getDataManager().getGeneralOutHolder(dhIndex); } */

inline int StepRep::getAppl() const
  { return itsWorker->getAppl(); } 

inline int StepRep::getCurAppl()
  { return WorkHolder::getCurAppl(); } 

inline void StepRep::setCurAppl (int applNr)
  { WorkHolder::setCurAppl(applNr); } 

inline void StepRep::runOnNode (int aNode, int applNr)
  { itsWorker->runOnNode(aNode, applNr); }

inline int StepRep::getNode() const
  { return itsWorker->getNode(); } 

}

#endif
