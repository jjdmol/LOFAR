// Simul.h: Class to hold a collection of Step objects
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
//  $Log$
//  Revision 1.25  2002/06/10 09:45:32  diepen
//
//  %[BugId: 38]%
//  Added command dhfile to the parser and added function setOutFile to
//  Simul.
//
//  Revision 1.24  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.23  2002/05/02 12:19:59  schaaf
//  Added CorbaMonitor object to C'tor
//
//  Revision 1.22  2002/03/26 13:29:14  gvd
//  Moved CorbaController object to SimulRep (for better copy ctor semantics)
//  Use fewer pointers and use switch instead of if
//
//  Revision 1.21  2002/03/26 11:37:33  schaaf
//  Added CorbaController and access to VirtualMachine
//
//  Revision 1.20  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.19  2002/01/09 09:56:35  gvd
//  Added getParent to Step (for Robbert)
//
//  Revision 1.18  2001/12/07 13:58:20  gvd
//  Changes to make connect by name possible
//  Avoid leaks in firewall
//  Replace resolveComm by a new simplifyConnections
//
//  Revision 1.17  2001/11/02 14:29:41  gvd
//  Added clone to Step/Simul and use it in SimulRep
//
//  Revision 1.16  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.15  2001/10/19 14:24:42  gvd
//  Added function getSteps and shortcutConnections
//  Removed dataMap from SimulRep
//
//  Revision 1.14  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.13  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.12  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.11  2001/09/18 12:07:28  gvd
//  Changed to resolve Step and Simul memory leaks
//  Introduced ref.counted StepRep and SimulRep classes for that purposes
//  Changed several functions to pass by reference instead of pass by pointer
//
//  Revision 1.10  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.9  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.8  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.7  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_SIMUL_H
#define BASESIM_SIMUL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/Step.h"
#include "BaseSim/SimulRep.h"

class SimulBuilder;
class CorbaController;

/** The Simul class is related to the Step class following a Composit pattern.
    Therefore, a Simul is a collection of Steps and/or Simuls
    In the constructor the actual workholder is defined. 
    The actual simulation work is performed in the process() method
    (which calls the Workholder::process() method)
*/

class Simul: public Step
{
public:

  /** Create the Simul using the given WorkHolder.
      The Simul must get a unique name. To make that process easy,
      by default the suffix _n is added to the name when the Simul is
      added to a Simul (where n is the sequence number starting with 0).
      The controllable argument is used to create a CorbaController object that 
      will be connected to the VirtualMachine object of the SimulRep.
      The monitor argument is used to create a CorbaMonitor object (in Step class)
  */
  explicit Simul (const WorkHolder& worker,
		  const string& name = "aSimul",
		  bool addNameSuffix = true,
		  bool controllable = false, // flag for CorbaControl object
		  bool monitor = false); // flag for Corbamonitor object
  explicit Simul (const WorkHolder* worker,
		  const string& name = "aSimul",
		  bool addNameSuffix = true,
		  bool controllable = false, // flag for CorbaControl object
		  bool monitor = false); // flag for Corbamonitor object

  /// Construct the object using a builder object.
  explicit Simul (const SimulBuilder& builder,
		  const string& name = "aSimul",
		  bool addNameSuffix = true,
		  bool controllable = false,
		  bool monitor = false);

  /** Default constructor is only useful for a Simul class member
      (as used in class Simulator).
      It does not create a useful Simul object.
  */
  Simul();

  /// Copy constructor (reference semantics).
  Simul (const Simul&);

  /// Construct from an existing SimulRep (meant for internal use).
  explicit Simul (SimulRep*);

  virtual ~Simul();
  
  /// Assignment (reference semantics).
  Simul& operator= (const Simul&);

  /// Make a correct copy (reference semantics).
  virtual Simul* clone() const;

  /** Add a Step to this Simul
      An exception is thrown if the name of the step (possibly after
      adding the suffix (see constructor)) is not unique.
      Also an exception is thrown if the step has already been added to
      another simul.
  */
  void addStep (const Step* aStep)
    { itsSimul->addStep (*aStep); }
  void addStep (const Step& aStep)
    { itsSimul->addStep (aStep); }

  /// Get all Steps in the Simul.
  const list<Step*>& getSteps() const
    { return itsSimul->getSteps(); }

  /// Is this Simul is not part of another Simul?
  bool isHighestLevel() const
    { return itsSimul->isHighestLevel(); }
  /// Clear the highest level flag (thus it is part of another Simul).
  void setNotHighestLevel()
    { itsSimul->setNotHighestLevel(); }

  /** Connect source and target DataHolders which must be
      dataHolders of this Simul or of a Step inside this Simul.
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
		const TransportHolder& prototype = TRANSPORTER())
    { return itsSimul->connect (sourceName, targetName, prototype); }
		

  /// Connect this Simul to an array of Simuls.
  bool connectInputArray (Simul* aSimul[],   // pointer to  array of ptrs to Steps
			  int    nrItems=1, // nr of Steps in aStep[] array
			  const TransportHolder& prototype = TRANSPORTER());


  /**
     Connect all input DataHolders in the aStep[] array to the input
     DataHolders of the current simul.
     This connection is needed in order to let the framework transport
     the data read by the Simul to the DataHolders of the first Steps
     in the Simul.
  */
  bool connectInputToArray (Step* aStep[],  // pointer to  array of ptrs to Steps
			    int    nrItems=1, // nr of Steps in aStep[] array
			    int    skip=0,     // skip in inputs in aStep 
			    int    offset=0,  // start with this input nr in aStep
			    const TransportHolder& prototype = TRANSPORTER())
    { return itsSimul->connectInputToArray (aStep, nrItems, skip,
					    offset, prototype); }

  /**
     Connect all output DataHolders in the aStep[] array to the output
     DataHolders of the current simul.
     This connection is needed in order to let the framework transport
     the data read by the Simul to the DataHolders of the first Steps
     in the Simul.
   */
  bool connectOutputToArray (Step* aStep[],  // pointer to  array of ptrs to Steps
			     int    nrItems=1, // nr of Steps in aStep[] array
			     int    skip=0,     // skip in inputs in aStep 
			     int    offset=0,  // start with this input nr in aStep
			     const TransportHolder& prototype = TRANSPORTER())
    { return itsSimul->connectOutputToArray (aStep, nrItems, skip,
					     offset, prototype); }

  /// Obtain a pointer to the VirtualMachine object
  VirtualMachine& getVM()
    { return itsSimul->getVM(); }

  /// Set the output file for the given data holder.
  /// As in connect, the dhName has to be given as "step.dhname".
  /// If the file name is empty, the output file is closed.
  bool setDHFile (const string& dhName, const string& fileName)
    { return itsSimul->setDHFile (dhName, fileName); }

private:
  /// Helper for ConnectInputToArray 
  bool connect_thisIn_In (Step* aStep,          
			  int    thisChannelOffset,
			  int    thatChannelOffset,
			  int    skip=0,
			  const TransportHolder& prototype = TRANSPORTER())
    { return itsSimul->connect_thisIn_In (aStep, thisChannelOffset,
					  thatChannelOffset, skip,
					  prototype); }

  /// Helper for ConnectOutputToArray 
  bool connect_thisOut_Out (Step* aStep,          
			    int    thisChannelOffset=0,
			    int    thatChannelOffset=0,
			    int    skip=0,
			    const TransportHolder& prototype = TRANSPORTER())
    { return itsSimul->connect_thisOut_Out (aStep, thisChannelOffset,
					    thatChannelOffset, skip,
					    prototype); }


  // Pointer to the actual simul object.
  SimulRep* itsSimul;
};

  
#endif
