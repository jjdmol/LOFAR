//  Step.h: Class representing a basic simulation block
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

#ifndef CEPFRAME_STEP_H
#define CEPFRAME_STEP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//# Includes
#include "CEPFrame/StepRep.h"
#include TRANSPORTERINCLUDE

namespace LOFAR
{

//# Forward Declarations
class Simul;


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
      added to a Simul (where n is the sequence number starting with 0).
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

  /// Is the Step a Simul?
  bool isSimul() const
    { return itsRep->isSimul(); }

  /// get WorkHolder.
  WorkHolder* getWorker()
    { return itsRep->getWorker(); }

  /// Get i-th input DataHolder.
  DataHolder& getInData (int dhIndex)
    { return itsRep->getInData (dhIndex); }
  /// Get i-th output DataHolder.
  DataHolder& getOutData (int dhIndex)
    { return itsRep->getOutData (dhIndex); }

  /// Get Transport object for i-th input DataHolder.
  Transport& getInTransport (int dhIndex) const
    { return itsRep->getInTransport (dhIndex); }
  /// Get Tranport object for i-th output DataHolder.
  Transport& getOutTransport (int dhIndex) const
    { return itsRep->getOutTransport (dhIndex); }

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
     This is done recursively, thus executed for all Steps in a Simul.
  */
  void runOnNode (int aNode, int applNr=0)
    { itsRep->runOnNode (aNode, applNr); }

  /// return this Step's name
  const string& getName() const
    { return itsRep->getName(); }

  /// set the Step's name
  void setName (const string& name)
    { itsRep->setName(name); }

  /// Get the parent simul object.
  Simul getParent() const;

  /** Basic connect function.
      Connect nrDH output DataHolders of aStep to input DataHolders of
      this Step. Start at the given indices.
      If nrDH=-1 is given, it will be set to the mininum number of
      DataHolders in input and output.
  */
  bool connect (Step* aStep, 
		int thisDHIndex=0,
		int thatDHIndex=0,
		int nrDH=-1,
		const TransportHolder& prototype = TRANSPORTER())
    { return itsRep->connectRep (aStep->itsRep, thisDHIndex,
				 thatDHIndex, nrDH, prototype); }
  
  /**
     Connect all output DataHolders of aStep to the input DataHolders of
     this Step. The steps must have the same number of DataHolders.
     This is the normal way of connecting two Steps to each other.
  */
  bool connectInput (Step* aStep,
		     const TransportHolder& prototype = TRANSPORTER())
    { return itsRep->connectInput (aStep, prototype); }

  /**
     Connect all output DataHolders in the array of Steps (aStep[]) to
     the input DataHolders of the current step. This is the normal way
     of connecting the output of multiple Steps to a Simul object.
     If the nrSteps argument is -1, it is assumed that
     each of the Steps in the array has only one input DataHolder
     which will be connected to the output DataHolders of the Simul.
  */
  bool connectInputArray (Step* aStep[],   // pointer to array of ptrs to Steps
			  int   nrSteps=-1, // nr of Steps in aStep[] array
			  const TransportHolder& prototype = TRANSPORTER())
    { return itsRep->connectInputArray (aStep, nrSteps, prototype); }

  /**
     Connect all input DataHolders in the array of Steps (aStep[]) to
     the output DataHolders of the current step. This is the normal
     way of connecting the output of a Simul object to Steps.
     If the nrSteps argument is -1, it is assumed that
     each of the Steps in the array has only one output DataHolder
     which will be connected to the input DataHolders of the Simul.
  */
  bool connectOutputArray (Step* aStep[],   // pointer to array of ptrs to Steps
			   int   nrSteps=-1, // nr of Steps in aStep[] array
			   const TransportHolder& prototype = TRANSPORTER())
    { return itsRep->connectOutputArray (aStep, nrSteps, prototype); }

  /** 
      Connect named parameter of step1 with that of step2 with given prototype
  */ 
  bool connectParam(const string& name, Step* aStep,
                    const TransportHolder& prototype)
    { return itsRep->connectParam(name, aStep, prototype); }

  /** Check if all connections are correct and if everything is connected.
      By default messages are written to cerr.
  */
  bool checkConnections (ostream& os=cerr)
    { return checkConnections (os, 0); }

  /** Shortcut the connections by removing all possible Simul
      connections. In this way the steps in different simuls communicate
      directly.
      Note that hereafter checkConnections() will give more messages.
  */
  void shortcutConnections()
    { return itsRep->shortcutConnections(); }

  /** Simplify the connections by using TH_Mem for all connections between
      Steps running on the same node.
  */
  void simplifyConnections()
    { return itsRep->simplifyConnections(); }
  void optimizeConnectionsWith(const TransportHolder& newTH)
    { return itsRep->optimizeConnectionsWith(newTH); }

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
  bool setRate (int rate=1, int dhIndex=-1)
    { return itsRep->setRate (rate, dhIndex); }
  bool setInRate (int rate=1, int dhIndex=-1)
    { return itsRep->setInRate (rate, dhIndex); }
  bool setOutRate (int rate=1, int dhIndex=-1)
    { return itsRep->setOutRate (rate, dhIndex); }

  /** Decide whether to handle this event or not based on rate and
      event count.
  */
  bool doHandle() const
    { return itsRep->doHandle(); }

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

  /// Check if all connections are correct.
  bool checkConnections (ostream& os, const StepRep* parent)
    { return itsRep->checkConnections (os, parent); }

  /// Get the internal rep object (to be used by StepRep and SimulRep).
  friend class StepRep;
  friend class SimulRep;
  StepRep* getRep()
    { return itsRep; }
  const StepRep* getRep() const
    { return itsRep; }

  StepRep* itsRep;

};

}

#endif
