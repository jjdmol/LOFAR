//  StepRep.h: Class representing a basic simulation block
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
//  Revision 1.16  2002/05/08 14:16:38  wierenga
//  Added optimizeConnectionsWith method
//
//  Revision 1.15  2002/05/07 12:03:21  gvd
//  Made itsMonitor private
//
//  Revision 1.14  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.13  2002/05/02 12:17:55  schaaf
//  Added CorbaMonitor object
//
//  Revision 1.12  2002/03/15 13:28:08  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.11  2002/03/14 14:21:00  wierenga
//  system include before local includes
//
//  Revision 1.10  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.9  2001/12/07 13:58:20  gvd
//  Changes to make connect by name possible
//  Avoid leaks in firewall
//  Replace resolveComm by a new simplifyConnections
//
//  Revision 1.8  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.7  2001/10/19 14:24:42  gvd
//  Added function getSteps and shortcutConnections
//  Removed dataMap from SimulRep
//
//  Revision 1.6  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.5  2001/09/24 14:36:11  gvd
//  Also use applnr in test on process to run
//
//  Revision 1.4  2001/09/24 14:04:09  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.3  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.2  2001/09/20 06:33:43  gvd
//  Added getCurRank() to StepRep.h to be used in SimulRep.cc
//
//  Revision 1.1  2001/09/18 12:07:28  gvd
//  Changed to resolve Step and Simul memory leaks
//  Introduced ref.counted StepRep and SimulRep classes for that purposes
//  Changed several functions to pass by reference instead of pass by pointer
//
//  Revision 1.12  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.11  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to StepRep
//
//  Revision 1.10  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.9  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_STEPREP_H
#define BASESIM_STEPREP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/BaseSim.h"
#include "BaseSim/WorkHolder.h"
#include "BaseSim/TransportHolder.h"
#include <stdlib.h>
#include <Common/lofar_string.h>

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
  StepRep (const WorkHolder& worker, 
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
  { return *itsWorker->getInHolder(dhIndex); }

inline DataHolder& StepRep::getOutData (int dhIndex)
  { return *itsWorker->getOutHolder(dhIndex); }

inline Transport& StepRep::getInTransport (int dhIndex) const
  { return itsWorker->getInHolder(dhIndex)->getTransport(); }

inline Transport& StepRep::getOutTransport (int dhIndex) const
  { return itsWorker->getOutHolder(dhIndex)->getTransport(); }

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


#endif
