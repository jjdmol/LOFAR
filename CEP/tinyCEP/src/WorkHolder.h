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

#include <Transport/BaseSim.h>
#include <Transport/DataHolder.h>
#include <tinyCEP/MiniDataManager.h>
//#include <CEPFrame/ParamManager.h>
#include <Common/lofar_iostream.h>
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
      The default implementation calls basePreprocess for all DataHolders.
  */
  void basePreprocess();
  virtual void preprocess();

  /** This is the main function of the WorkHolder. It is called from
      the Step::Process method after storing the input data in the
      input DataHolders. The purpose of the Process() method is to
      transform the data in the input DataHolders into the output
      DataHolders.     
   */
  void baseProcess();
  virtual void process() = 0;

  /** The postprocess method is called after process.
      It can be used to clean up the WorkHolder.
      The default implementation calls basePostprocess for all DataHolders.
  */
  void basePostprocess();
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
  MiniDataManager& getDataManager();
  void setDataManager(MiniDataManager* dmptr);

  // Get its ParamManager
/*   ParamManager& getParamManager(); */


  // Register a static constructor functions.
  static void registerConstruct (const string& name, WHConstruct*);

  // Get the constructor function for a given class name.
  static WHConstruct* getConstruct (const string& name);

  /// Get monitoring data
  virtual int getMonitorValue(const char* name);

protected:
  MiniDataManager* itsDataManager;

private:
  /** Make a map of all DataHolders names. A separate map is made
      for the input and output DataHolders.
      An error is given if DataHolder names are not unique.
  */
  void fillMaps();

  /// Let the derived class make the actual copy.
  virtual WorkHolder* make (const string& name) = 0;

  int itsNinputs;
  int itsNoutputs;
  int itsIndex;
  string itsName;
  string itsType;
  mutable map<string,int> itsInMap;
  mutable map<string,int> itsOutMap;
/*   ParamManager itsParamManager; */
  bool itsFirstProcessCall;

  static map<string,WHConstruct*>* itsConstructMap;
};


inline MiniDataManager& WorkHolder::getDataManager()
  { return *itsDataManager; }

inline void WorkHolder::setDataManager(MiniDataManager* dmptr)
  { itsDataManager = dmptr; }

/* inline ParamManager& WorkHolder::getParamManager() */
/*   { return itsParamManager; } */

inline const string& WorkHolder::getType() const
  { return itsType; }

inline void WorkHolder::setName (const string& name)
  { itsName = name; }
inline const string& WorkHolder::getName() const
  { return itsName; }


}

#endif
