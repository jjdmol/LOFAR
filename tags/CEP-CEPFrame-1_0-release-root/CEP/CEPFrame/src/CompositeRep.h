//# CompositeRep.h: Class to hold a collection of Step objects
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

#ifndef CEPFRAME_COMPOSITEREP_H
#define CEPFRAME_COMPOSITEREP_H

#include <lofar_config.h>

#include <stdlib.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include "CEPFrame/StepRep.h"
#include "CEPFrame/VirtualMachine.h"

namespace LOFAR
{

class CorbaController;
class Composite;


/** The Composite class is related to the Step class following a Composite pattern.
    Therefore, a Composite is a collection of Steps and/or Composites
    In the constructor the actual workholder is defined. 
    The actual simulation work is performed in the process() method
    (which calls the Workholder::process() method)
*/

class CompositeRep: public StepRep
{
public:
  // Maptype for names
  typedef map<string,StepRep*> nameMapType;

  /** Normally used basic constructor.
      A pointer to a workholder (containing the dataholders) is passed.
  */
  CompositeRep (WorkHolder& worker,
	    const string& name,
	    bool addnameSuffix,
	    bool controllable,
	    bool monitor);

  virtual ~CompositeRep();
  
  
  /** Set Node numbers for both In and OutData
      Set application number as well.
      Do it recursively for all Steps in this Composite.
  */
  virtual void runOnNode (int aNode, int applNr);

  /** The preprocess method is called before process().
      It can be used to initialize WorkHolders, etc.
  */
  virtual void preprocess();

  /** The process method is the basical simulation step.
      It will first read the data for the in DataHolders in the workholder
      Then it will call the process() method for all the Steps in the list
      Finally it will write the data from the DataHolders
      (e.g to another MPI process)
   */
  virtual void process();

  /** The postprocess method is called after process.
      It can be used to clean up WorkHolders, etc.
  */
  virtual void postprocess();

  /// Dump information to the user
  virtual void dump() const;

  /// Add a Step to this Composite
  void addStep (const Step& aStep);

  /// Get all Steps in the Composite.
  const list<Step*>& getSteps() const;

  /// Distinguish between step and simul
  bool isComposite() const;

  /// Mark if this Composite is not part of another Composite
  bool isHighestLevel() const;
  /// lower the highest level flag
  void setNotHighestLevel();

  /** Simplify the connections by using TH_Mem for all connections between
      Steps running on the same node.
  */
  virtual void replaceConnectionsWith(const TransportHolder& newTH,
				      bool blockingComm=true);

  /// Connect source and target DataHolders by name.
  bool connect (const string& sourceName, const string& targetName,
		const TransportHolder& prototype, bool blockingComm=true);

  /** Helper for ConnectInputToArray 
   */
  bool connect_thisIn_In (Step* aStep,          
			  int    thisChannelOffset,
			  int    thatChannelOffset,
			  int    skip,
			  const TransportHolder& prototype,
			  bool blockingComm=true);

  /** Helper for ConnectOutputToArray 
   */
  bool connect_thisOut_Out (Step* aStep,          
			    int    thisChannelOffset,
			    int    thatChannelOffset,
			    int    skip,
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
			    bool blockingComm=true);

//   /**
//      Connect all input DataHolders in the Composite[] array to the input
//      DataHolders of the current simul.
//      This connection is needed in order to let the framework transport
//      the data read by the Composite to the DataHolders of the first Composites
//      in the Composite.
//    */
//   bool connectInputToArray (Composite* aStep[],     // pointer to  array of ptrs to Steps
// 			    int    nrItems,  // n of Steps in aStep[] array
// 			    int    skip,     // skip in inputs in aStep 
// 			    int    offset,  // start with this input nr in aStep
// 			    const TransportHolder& prototype);

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
			     bool blockingComm=true);

  /**
     Get a pointer to the Virtual Machine controlling this CompositeRep. The Virtual 
     Machine is created in the contructor.
   */
  VirtualMachine& getVM();

private:
  /** Split the given name into step and dataholder part (separated by a .).
      Each part can be empty.
      The isSource argument tells if the name is the source or target.
      The step name is looked up and the corresponding StepRep* is filled in.
      An empty step means this Composite.
      Similarly the DataHolder is looked up and its index is filled in.
      (-1 means that no DataHolder part is given).
  */
  bool splitName (bool isSource, const string& name,
		  StepRep*& step, int& dhIndex);

  /// true = this Composite is the top Composite
  bool itsIsHighestLevel;
  /// List of Steps contained in the Composite
  list<Step*> itsSteps;
  /// Map of Step names of Step objects.
  nameMapType itsNameMap;

  /// The VirtualMachine object.
  VirtualMachine itsVM;
  /// pointer to the CorbaController/Monitor objects (can be 0).
  CorbaController* itsController;
};

  
inline bool CompositeRep::isHighestLevel() const
  { return itsIsHighestLevel; }
inline void CompositeRep::setNotHighestLevel()
  { itsIsHighestLevel = false; }
inline const list<Step*>& CompositeRep::getSteps() const
 { return itsSteps; }
inline VirtualMachine& CompositeRep::getVM()
 { return itsVM; }

}

#endif
