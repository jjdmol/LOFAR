//# WorkHolder.h: Abstract base class for the work holders
//#
//# Copyright (C) 2000-2002
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

#ifndef TINYFRAME_WORKHOLDER_H
#define TINYFRAME_WORKHOLDER_H

#include <lofar_config.h>

#include <tinyCEP/TinyDataManager.h>
/* #include <Common/lofar_iostream.h> */

#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

namespace LOFAR
{

class KeyValueMap;
 
/**
  Class WorkHolder is the abstract base class for all work holders
  in the CEPFrame environment. It main purpose is to offer a common interface
  to a class like Step. Apart from that it also offers some common
  functionality to the classes derived from it.
*/

class WorkHolder
{
public:
  // Define the signature of each WH maker function.
  // They can be registered in a static map.
  typedef WorkHolder* WHConstruct (const string& name,
				   int ninput, int noutput,
				   const KeyValueMap& params);

  /** The constructor with the number of input and output
      DataHolders as arguments.
      The constructors of the subclasses of WorkHolder will actually
      create the (typed!) DataHolders.
  */
  explicit WorkHolder (int inputs=0, int outputs=0,
		       const string& name="aWorkHolder",
		       const string& type="WH");

  /// Copy constructor (copy semantics).
  WorkHolder (const WorkHolder&);

  virtual ~WorkHolder();

  /// Assignment (copy semantics).
  WorkHolder& operator= (const WorkHolder&);

  /** Make a fresh copy of the derived WorkHolder object.
      Its DataHolders get the names from the given WorkHolder.
      Making the actual copy is done by the virtual function make.
  */
  WorkHolder* baseMake();

  /** The dump() method produces output to the user. By default, the
      data in the last output DataHolders are shown to cout, but
      other output sources or targets are valid as well.
  */
  virtual void dump();

  /** The preprocess method is called before process.
      It can be used to initialize the WorkHolder.
      The default implementation initialises all DataHolders.
  */
  void basePreprocess();
  virtual void preprocess();

  /** This is the main function of the WorkHolder. It is called from
      the Step::Process method after storing the input data in the
      input DataHolders. The purpose of the Process() method is to
      transform the data in the input DataHolders into the output
      DataHolders.     
   */
  virtual void baseProcess();
  virtual void process() = 0;

  /** The postprocess method is called after process.
      It can be used to clean up the WorkHolder.
      The default implementation calls basePostprocess for all DataHolders.
  */
  virtual void basePostprocess();
  virtual void postprocess();

  /// Get the type of the work holder.
  virtual const string& getType() const;

  /// Set the name of the work holder.
  void setName (const string& name);

  /// Get the name of the work holder.
  const string& getName() const;

  /** Find the input or output channel for the DataHolder with the given name.
      -1 is returned if the name is not found.
  */
  int getInChannel (const string& name);
  int getOutChannel (const string& name);

  // Get/set its DataManager
  TinyDataManager& getDataManager();
  void setDataManager(TinyDataManager* dmptr);

  // Register a static constructor functions.
  static void registerConstruct (const string& name, WHConstruct*);

  // Get the constructor function for a given class name.
  static WHConstruct* getConstruct (const string& name);

  /// Get monitoring data
  virtual int getMonitorValue(const char* name);

  /// Determine if the processing step needs to be called
  bool doHandle();

  /// Set node and application number
  void runOnNode(int aNode, int applNr=0);

  /// Get the node number for this WorkHolder.
  int getNode() const; 

  /// Get the application number for this WorkHolder.
  int getAppl() const; 

  /// Execute process here?
  bool shouldProcess() const;

  /// Get the application number of the current run.
  static int getCurAppl();

  /// Set the current application number.
  static void setCurAppl (int applNr);


protected:
  TinyDataManager* itsDataManager;

  int itsCurRank;   // Rank of the current run. 
  int itsProcessStep;

  int itsNinputs;
  int itsNoutputs;
  bool itsFirstProcessCall;
private:
  /** Make a map of all DataHolders names. A separate map is made
      for the input and output DataHolders.
      An error is given if DataHolder names are not unique.
  */
  void fillMaps();

  /// Let the derived class make the actual copy.
  virtual WorkHolder* make (const string& name) = 0;

  int itsIndex;
  string itsName;
  string itsType;
  mutable map<string,int> itsInMap;
  mutable map<string,int> itsOutMap;
  
  static int theirCurAppl; // The application number of this run.

  int itsNode;  // The node to run this WorkHolder on.
  int itsAppl;  // The application to run this WorkHolder in.

  static map<string,WHConstruct*>* itsConstructMap;
  static int          theirReadProfilerState; 
  static int          theirProcessProfilerState; 
  static int          theirWriteProfilerState; 

};


inline TinyDataManager& WorkHolder::getDataManager()
  { return *itsDataManager; }

inline const string& WorkHolder::getType() const
  { return itsType; }

inline void WorkHolder::setName (const string& name)
  { itsName = name; }
inline const string& WorkHolder::getName() const
  { return itsName; }

inline bool WorkHolder::shouldProcess() const
{
  return (itsNode == itsCurRank  ||  itsCurRank < 0)
    && itsAppl == theirCurAppl;
}

inline int WorkHolder::getNode() const
  { return itsNode; } 

inline int WorkHolder::getAppl() const
  { return itsAppl; } 

inline int WorkHolder::getCurAppl()
  { return theirCurAppl; } 

inline void WorkHolder::setCurAppl (int applNr)
  { theirCurAppl = applNr; } 

}

#endif
