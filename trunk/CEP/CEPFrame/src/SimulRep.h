// SimulRep.h: Class to hold a collection of Step objects
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

#ifndef CEPFRAME_SIMULREP_H
#define CEPFRAME_SIMULREP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include "CEPFrame/StepRep.h"
#include "CEPFrame/Transport.h"
#include "CEPFrame/VirtualMachine.h"

class CorbaController;
class Simul;


/** The Simul class is related to the Step class following a Composit pattern.
    Therefore, a Simul is a collection of Steps and/or Simuls
    In the constructor the actual workholder is defined. 
    The actual simulation work is performed in the process() method
    (which calls the Workholder::process() method)
*/

class SimulRep: public StepRep
{
public:
  // Maptype for names
  typedef map<string,StepRep*> nameMapType;

  /** Normally used basic constructor.
      A pointer to a workholder (containing the dataholders) is passed.
  */
  SimulRep (WorkHolder& worker,
	    const string& name,
	    bool addnameSuffix,
	    bool controllable,
	    bool monitor);

  virtual ~SimulRep();
  
  
  /** Set Node numbers for both In and OutData
      Set application number as well.
      Do it recursively for all Steps in this Simul.
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

  /// Add a Step to this Simul
  void addStep (const Step& aStep);

  /// Get all Steps in the Simul.
  const list<Step*>& getSteps() const;

  /// Distinguish between step and simul
  bool isSimul() const;

  /// Mark if this Simul is not part of another Simul
  bool isHighestLevel() const;
  /// lower the highest level flag
  void setNotHighestLevel();

  /** Check the connections.
      It checks if all DataHolders are connected properly.
  */
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
  virtual void optimizeConnectionsWith(const TransportHolder& newTH);

  /// Connect source and target DataHolders by name.
  bool connect (const string& sourceName, const string& targetName,
		const TransportHolder& prototype);

  /** Helper for ConnectInputToArray 
   */
  bool connect_thisIn_In (Step* aStep,          
			  int    thisChannelOffset,
			  int    thatChannelOffset,
			  int    skip,
			  const TransportHolder& prototype);

  /** Helper for ConnectOutputToArray 
   */
  bool connect_thisOut_Out (Step* aStep,          
			    int    thisChannelOffset,
			    int    thatChannelOffset,
			    int    skip,
			    const TransportHolder& prototype);

  /**
     Connect all input DataHolders in the aStep[] array to the input
     DataHolders of the current simul.
     This connection is needed in order to let the framework transport
     the data read by the Simul to the DataHolders of the first Steps
     in the Simul.
  */
  bool connectInputToArray (Step* aStep[],  // pointer to  array of ptrs to Steps
			    int    nrItems, // nr of Steps in aStep[] array
			    int    skip,     // skip in inputs in aStep 
			    int    offset,  // start with this input nr in aStep
			    const TransportHolder& prototype);

//   /**
//      Connect all input DataHolders in the Simul[] array to the input
//      DataHolders of the current simul.
//      This connection is needed in order to let the framework transport
//      the data read by the Simul to the DataHolders of the first Simuls
//      in the Simul.
//    */
//   bool connectInputToArray (Simul* aStep[],     // pointer to  array of ptrs to Steps
// 			    int    nrItems,  // n of Steps in aStep[] array
// 			    int    skip,     // skip in inputs in aStep 
// 			    int    offset,  // start with this input nr in aStep
// 			    const TransportHolder& prototype);

  /**
     Connect all output DataHolders in the aStep[] array to the output
     DataHolders of the current simul.
     This connection is needed in order to let the framework transport
     the data read by the Simul to the DataHolders of the first Steps
     in the Simul.
   */
  bool connectOutputToArray (Step* aStep[],  // pointer to  array of ptrs to Steps
			     int    nrItems, // nr of Steps in aStep[] array
			     int    skip,     // skip in inputs in aStep 
			     int    offset,  // start with this input nr in aStep
			     const TransportHolder& prototype);

  /**
     Get a pointer to the Virtual Machine controlling this SimulRep. The Virtual 
     Machine is created in the contructor.
   */
  VirtualMachine& getVM();

  // Set the output file for the given data holder.
  // As in connect, the dhName has to be given as "step.dhname".
  // If the file name is empty, the output file is closed.
  bool setDHFile (const string& dhName, const string& fileName);

private:
  /// Do the possible shortcut of Simul connections.
  void doShortcut (Transport& tp);

  /** Split the given name into step and dataholder part (separated by a .).
      Each part can be empty.
      The isSource argument tells if the name is the source or target.
      The step name is looked up and the corresponding StepRep* is filled in.
      An empty step means this Simul.
      Similarly the DataHolder is looked up and its index is filled in.
      (-1 means that no DataHolder part is given).
  */
  bool splitName (bool isSource, const string& name,
		  StepRep*& step, int& dhIndex);

  /// true = this Simul is the top Simul
  bool itsIsHighestLevel;
  /// List of Steps contained in the Simul
  list<Step*> itsSteps;
  /// Map of Step names of Step objects.
  nameMapType itsNameMap;

  /// Profiling States
  static int          theirProcessProfilerState; 
  static int          theirInReadProfilerState; 
  static int          theirInWriteProfilerState; 
  static int          theirOutReadProfilerState; 
  static int          theirOutWriteProfilerState; 

  /// The VirtualMachine object.
  VirtualMachine itsVM;
  /// pointer to the CorbaController/Monitor objects (can be 0).
  CorbaController* itsController;
};

  
inline bool SimulRep::isHighestLevel() const
  { return itsIsHighestLevel; }
inline void SimulRep::setNotHighestLevel()
  { itsIsHighestLevel = false; }
inline const list<Step*>& SimulRep::getSteps() const
 { return itsSteps; }
inline VirtualMachine& SimulRep::getVM()
 { return itsVM; }

#endif
