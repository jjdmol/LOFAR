//# Step.h: Class representing a basic simulation block
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

#ifndef CEPFRAME_STEP_H
#define CEPFRAME_STEP_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <CEPFrame/StepRep.h>

namespace LOFAR
{

//# Forward Declarations
class Composite;


/** The Step class is the basic building block for simulations.
    In the constructor the actual worker and dataholders are defined. 
    The actual simulation work is performed in the process() method
    (which calls the WorkHolder::baseProcess() method).

    Note that the actual Step data is contained in the reference counted
    class StepRep. In this way a copy of a Step is a cheap operation.
*/

class Step
{
public:
  /** Build the Step using the given WorkHolder.
      The Step must get a unique name. To make that process easy,
      by default the suffix _n is added to the name when the Step is
      added to a Composite (where n is the sequence number starting with 0).
  */
  explicit Step (WorkHolder& worker,
		 const string& name = "aStep",
		 bool addNameSuffix = true,
		 bool monitor       = false); // flag for Corbamonitor object
  explicit Step (WorkHolder* worker,
		 const string& name = "aStep",
		 bool addNameSuffix = true,
		 bool monitor       = false); // flag for Corbamonitor object

  /// Copy constructor (reference semantics).
  Step (const Step&);

  /// Construct a Step from a StepRep (meant for internal use).
  explicit Step (StepRep*);

  virtual ~Step();

  /// Assignment (reference semantics).
  Step& operator= (const Step&);

  /// Make a correct copy (reference semantics).
  virtual Step* clone() const;

  /** The preprocess method is called before process.
      It can be used to initialize WorkHolders, etc.
  */
  void preprocess()
    { itsRep->preprocess(); }

  /** The process method is the basical simulation step.
      It will first read the data for the input DataHolders in the workholder.
      Then it will call Workholder->process().
      Finally it will write the data from the output DataHolders
      (e.g to another MPI process).
  */
  void process()
    { itsRep->process(); }

  /** The postprocess method is called after process.
      It can be used to clean up WorkHolders, etc.
  */
  void postprocess()
    { itsRep->postprocess(); }

  /// Dump information to user
  void dump() const
    { itsRep->dump(); }

  /// Is the Step a Composite?
  bool isComposite() const
    { return itsRep->isComposite(); }

  /// get WorkHolder.
  WorkHolder* getWorker()
    { return itsRep->getWorker(); }

  /// Get i-th input DataHolder.
  DataHolder& getInData (int dhIndex)
    { return itsRep->getInData (dhIndex); }
  /// Get i-th output DataHolder.
  DataHolder& getOutData (int dhIndex)
    { return itsRep->getOutData (dhIndex); }

  /// Get ID of the Step.
  //  It will be obtained from the zeroth InData object in the Workholder
  int getID() const
    { return itsRep->getID(); }

  /// Get the node number for this Step.
  int getNode() const
    { return itsRep->getNode(); }

  /// Get the application number for this Step.
  int getAppl() const
    { return itsRep->getAppl(); }

  /**
     Set Node numbers for both In and OutData.
     Set application number as well.
     This is done recursively, thus executed for all Steps in a Composite.
  */
  void runOnNode (int aNode, int applNr=0)
    { itsRep->runOnNode (aNode, applNr); }

  /// return this Step's name
  const string& getName() const
    { return itsRep->getName(); }

  /// set the Step's name
  void setName (const string& name)
    { itsRep->setName(name); }

  /// Get the parent Composite object.
  Composite getParent() const;

  /** Basic connect function.
      Connect nrDH output DataHolders of aStep to input DataHolders of
      this Step. Start at the given indices.
      If nrDH=-1 is given, it will be set to the mininum number of
      DataHolders in input and output.
  */
  bool connect (Step* aStep, 
		int thisDHIndex,
		int thatDHIndex,
		int nrDH,
		const TransportHolder& prototype,
		bool blockingComm = true)
    { return itsRep->connectRep (aStep->itsRep, thisDHIndex,
				 thatDHIndex, nrDH, prototype,
				 blockingComm); }
  
  /**
     Connect all output DataHolders of aStep to the input DataHolders of
     this Step. The steps must have the same number of DataHolders.
     This is the normal way of connecting two Steps to each other.
  */
  bool connectInput (Step* aStep,
		     const TransportHolder& prototype,
		     bool blockingComm = true)
    { return itsRep->connectInput (aStep, prototype, blockingComm); }

  /**
     Connect all output DataHolders in the array of Steps (aStep[]) to
     the input DataHolders of the current step. This is the normal way
     of connecting the output of multiple Steps to a Composite object.
     If the nrSteps argument is -1, it is assumed that
     each of the Steps in the array has only one input DataHolder
     which will be connected to the output DataHolders of the Composite.
  */
  bool connectInputArray (Step* aStep[],   // pointer to array of ptrs to Steps
			  int   nrSteps,   // nr of Steps in aStep[] array
			  const TransportHolder& prototype,
			  bool blockingComm = true)
    { return itsRep->connectInputArray (aStep, nrSteps, prototype,
					blockingComm); }

  /**
     Connect all input DataHolders in the array of Steps (aStep[]) to
     the output DataHolders of the current step. This is the normal
     way of connecting the output of a Composite object to Steps.
     If the nrSteps argument is -1, it is assumed that
     each of the Steps in the array has only one output DataHolder
     which will be connected to the input DataHolders of the Composite.
  */
  bool connectOutputArray (Step* aStep[],   // pointer to array of ptrs to Steps
			   int   nrSteps,   // nr of Steps in aStep[] array
			   const TransportHolder& prototype,
			   bool blockingComm=true)
    { return itsRep->connectOutputArray (aStep, nrSteps, prototype,
					 blockingComm); }

  /** Replace the connections with using a specified TransportHolder.
  */
  void replaceConnectionsWith(const TransportHolder& newTH, 
			      bool blockingComm=true)
    { return itsRep->replaceConnectionsWith(newTH, blockingComm); }

  /** Set properties of a communication channel: synchronisity and sharing of DataHolders
      by input and output.
  */
  void setInBufferingProperties(int channel, bool synchronous, 
                                bool shareDHs=false)
    { return itsRep->setInBufferingProperties(channel, synchronous, shareDHs);}

  void setOutBufferingProperties(int channel, bool synchronous, 
                                 bool shareDHs=false)
    { return itsRep->setOutBufferingProperties(channel, synchronous, shareDHs);}

  /** SetRate methods:
      These methods set the rate at which input/output dataholders are
      read/written.
      The rate is the fraction of the events that is to be read/written.
      The dhIndex argument selects only the given input/output
      DataHolder; dhIndex=-1 sets all input/output DataHolders.
      setRate calls both setInRate and setOutRate and also sets the
      Step::itsRate.
      The monitor argument is used to create a CorbaMonitor object
  */
  bool setProcessRate (int rate=1)
    { return itsRep->setProcessRate (rate); }
  bool setInRate (int rate=1, int dhIndex=-1)
    { return itsRep->setInRate (rate, dhIndex); }
  bool setOutRate (int rate=1, int dhIndex=-1)
    { return itsRep->setOutRate (rate, dhIndex); }

   /// Get the event count.
  static unsigned int getEventCount()
    { return StepRep::getEventCount(); }

  /// Clear the event count.
  static void clearEventCount()
    { StepRep::clearEventCount(); }

  /// Get the current application number.
  static int getCurAppl()
    { return StepRep::getCurAppl(); } 

  /// Set the current application number.
  static void setCurAppl (int applNr)
    { StepRep::setCurAppl (applNr); } 

protected:
  /// Default constructor for derived class.
  Step()
    : itsRep(0) {}

  /// Get the internal rep object (to be used by StepRep and CompositeRep).
  friend class StepRep;
  friend class CompositeRep;
  StepRep* getRep()
    { return itsRep; }
  const StepRep* getRep() const
    { return itsRep; }

  StepRep* itsRep;

};

}

#endif
