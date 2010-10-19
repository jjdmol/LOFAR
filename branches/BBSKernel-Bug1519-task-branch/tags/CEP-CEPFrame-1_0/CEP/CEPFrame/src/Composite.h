//# Composite.h: Class to hold a collection of Step objects
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

#ifndef CEPFRAME_COMPOSITE_H
#define CEPFRAME_COMPOSITE_H

#include <lofar_config.h>

#include <CEPFrame/Step.h>
#include <CEPFrame/CompositeRep.h>

namespace LOFAR
{

class NetworkBuilder;
class CorbaController;

/** The Composite class is related to the Step class following a Composite pattern.
    Therefore, a Composite is a collection of Steps and/or Composites
    In the constructor the actual workholder is defined. 
    The actual simulation work is performed in the process() method
    (which calls the Workholder::process() method)
*/

class Composite: public Step
{
public:

  /** Create the Composite using the given WorkHolder.
      The Composite must get a unique name. To make that process easy,
      by default the suffix _n is added to the name when the Composite is
      added to a Composite (where n is the sequence number starting with 0).
      The controllable argument is used to create a CorbaController object that 
      will be connected to the VirtualMachine object of the CompositeRep.
      The monitor argument is used to create a CorbaMonitor object (in Step class)
  */
  explicit Composite (WorkHolder& worker,
		  const string& name = "aComposite",
		  bool addNameSuffix = true,
		  bool controllable = false, // flag for CorbaControl object
		  bool monitor = false); // flag for Corbamonitor object
  explicit Composite (WorkHolder* worker,
		  const string& name = "aComposite",
		  bool addNameSuffix = true,
		  bool controllable = false, // flag for CorbaControl object
		  bool monitor = false); // flag for Corbamonitor object

  /// Construct the object using a builder object.
  explicit Composite (NetworkBuilder& builder,
		  const string& name = "aComposite",
		  bool addNameSuffix = true,
		  bool controllable = false,
		  bool monitor = false);

  /** Default constructor is only useful for a Composite class member
      (as used in class ApplicationHolder).
      It does not create a useful Composite object.
  */
  Composite();

  /// Copy constructor (reference semantics).
  Composite (const Composite&);

  /// Construct from an existing CompositeRep (meant for internal use).
  explicit Composite (CompositeRep*);

  virtual ~Composite();
  
  /// Assignment (reference semantics).
  Composite& operator= (const Composite&);

  /// Make a correct copy (reference semantics).
  virtual Composite* clone() const;

  /** Add a Step to this Composite
      An exception is thrown if the name of the step (possibly after
      adding the suffix (see constructor)) is not unique.
      Also an exception is thrown if the step has already been added to
      another simul.
  */
  void addStep (const Step* aStep)
    { itsComposite->addStep (*aStep); }
  void addStep (const Step& aStep)
    { itsComposite->addStep (aStep); }

  /// Get all Steps in the Composite.
  const list<Step*>& getSteps() const
    { return itsComposite->getSteps(); }

  /// Is this Composite is not part of another Composite?
  bool isHighestLevel() const
    { return itsComposite->isHighestLevel(); }
  /// Clear the highest level flag (thus it is part of another Composite).
  void setNotHighestLevel()
    { itsComposite->setNotHighestLevel(); }

  /** Connect source and target DataHolders which must be
      dataHolders of this Composite or of a Step inside this Composite.
      The connection is done by means of names. A name can be given in 4 ways:
      - step.dataholder  to specify a dataholder in a step.
      - step             to specify a step
      - .dataholder      to specify a dataholder of the simul
      - .                to specify the simul itself
      A connection can be specified for a single dataholder or for all
      dataholders of the simul or a step.
      E.g.
        connect (".", "step1");
           connects all input dataholders of the simul to the input
	   dataholders of step1.
        connect ("step2.dhout1", "step3.dhin1");
           connects output dataholder dhout1 of step2 to input dataholder
	   dhin1 of step3.
  */
  bool connect (const string& sourceName, const string& targetName,
		const TransportHolder& prototype,
		bool blockingComm=true)
    { return itsComposite->connect (sourceName, targetName, prototype,
				    blockingComm); }
		

  /// Connect this Composite to an array of Composites.
  bool connectInputArray (Composite* aComposite[],   // pointer to  array of ptrs to Steps
			  int    nrItems, // nr of Steps in aStep[] array
			  const TransportHolder& prototype,
			  bool blockingComm=true);


  /**
     Connect all input DataHolders in the aStep[] array to the input
     DataHolders of the current simul.
     This connection is needed in order to let the framework transport
     the data read by the Composite to the DataHolders of the first Steps
     in the Composite.
  */
  bool connectInputToArray (Step* aStep[],  // pointer to  array of ptrs to Steps
			    int    nrItems, // nr of Steps in aStep[] array
			    int    skip,     // skip in inputs in aStep 
			    int    offset,  // start with this input nr in aStep
			    const TransportHolder& prototype,
			    bool blockingComm=true)
    { return itsComposite->connectInputToArray (aStep, nrItems, skip,
						offset, prototype, blockingComm); }

  /**
     Connect all output DataHolders in the aStep[] array to the output
     DataHolders of the current simul.
     This connection is needed in order to let the framework transport
     the data read by the Composite to the DataHolders of the first Steps
     in the Composite.
   */
  bool connectOutputToArray (Step* aStep[],  // pointer to  array of ptrs to Steps
			     int    nrItems, // nr of Steps in aStep[] array
			     int    skip,     // skip in inputs in aStep 
			     int    offset,  // start with this input nr in aStep
			     const TransportHolder& prototype,
			     bool blockingComm=true)
    { return itsComposite->connectOutputToArray (aStep, nrItems, skip,
						 offset, prototype, blockingComm); }

  /// Obtain a pointer to the VirtualMachine object
  VirtualMachine& getVM()
    { return itsComposite->getVM(); }

private:
  /// Helper for ConnectInputToArray 
  bool connect_thisIn_In (Step* aStep,          
			  int    thisChannelOffset,
			  int    thatChannelOffset,
			  int    skip,
			  const TransportHolder& prototype ,
			  bool blockingComm=true)
    { return itsComposite->connect_thisIn_In (aStep, thisChannelOffset,
					  thatChannelOffset, skip,
					  prototype, blockingComm); }

  /// Helper for ConnectOutputToArray 
  bool connect_thisOut_Out (Step* aStep,          
			    int    thisChannelOffset,
			    int    thatChannelOffset,
			    int    skip,
			    const TransportHolder& prototype ,
			    bool blockingComm=true)
    { return itsComposite->connect_thisOut_Out (aStep, thisChannelOffset,
						thatChannelOffset, skip,
						prototype, blockingComm); }


  // Pointer to the actual simul object.
  CompositeRep* itsComposite;
};

}
  
#endif
