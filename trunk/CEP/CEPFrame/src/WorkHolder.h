// WorkHolder.h: Abstract base class for the work holders
//
//  Copyright (C) 2000-2002
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
//  Revision 1.22  2002/06/10 09:49:37  diepen
//
//  %[BugId: 37]%
//  Added basePreprocess and basePostprocess
//
//  Revision 1.21  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.20  2002/05/02 12:16:24  schaaf
//  Added method getMonitorValue
//
//  Revision 1.19  2002/03/15 13:28:09  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.18  2002/03/14 14:25:27  wierenga
//  system includes before local includes
//
//  Revision 1.17  2002/03/04 12:54:01  gvd
//  Let WorkHolder copy the name of DataHolders; done by creating baseMake
//
//  Revision 1.16  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.15  2001/12/07 13:58:20  gvd
//  Changes to make connect by name possible
//  Avoid leaks in firewall
//  Replace resolveComm by a new simplifyConnections
//
//  Revision 1.14  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.13  2001/10/05 11:50:37  gvd
//  Added getType function
//
//  Revision 1.12  2001/09/24 14:04:09  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.11  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.10  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.9  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.8  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_WORKHOLDER_H
#define BASESIM_WORKHOLDER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/BaseSim.h"
#include "BaseSim/DataHolder.h"
#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

class ParamBlock;

/**
  Class WorkHolder is the abstract base class for all work holders
  in the BaseSim environment. It main purpose is to offer a common interface
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
				   const ParamBlock& params);

  enum ProcMode {Zeroes,Ones,Infile,Skip,Process};


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
  WorkHolder* baseMake() const;

  /** The dump() method produces output to the user. By default, the
      data in the last output DataHolders are shown to cout, but
      other output sources or targets are valid as well.
  */
  virtual void dump() const;

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
  int getInChannel (const string& name) const;
  int getOutChannel (const string& name) const;

  /** @name DataHolder access functions
      @memo Get the i-th input or output DataHolder of the work holder.
      @doc Get the i-th input or output DataHolder of the work holder.
  */
  //@{
  virtual DataHolder* getInHolder (int channel) = 0;
  virtual DataHolder* getOutHolder (int channel) = 0;
  const DataHolder* getConstInHolder (int channel) const;
  const DataHolder* getConstOutHolder (int channel) const;
  //@}

  /// Get the number of inputs or outputs.
  int getInputs() const;
  int getOutputs() const;

  /**  The ProcMode determines the type of processing performed in the
       process() method. This setting is especially usefull for
       testing. One may for instance set the ProcMode to "ones"; which
       forces the process() method to only fill the relevant output
       data with unit values. Setting the ProcMode to "file" allows
       for storing a well-defined output data set into the
       WorkHolders's DataHolders. 
   */
  void setProcMode (ProcMode aProcMode);

  /** This returns the current ProcMode setting
   */
  ProcMode getProcMode() const;

  // Register a static constructor functions.
  static void registerConstruct (const string& name, WHConstruct*);

  // Get the constructor function for a given class name.
  static WHConstruct* getConstruct (const string& name);

  /// Get monitoring data
  virtual int getMonitorValue(const char* name);

private:
  /** Make a map of all DataHolders names. A separate map is made
      for the input and output DataHolders.
      An error is given if DataHolder names are not unique.
  */
  void fillMaps() const;

  /// Let the derived class make the actual copy.
  virtual WorkHolder* make (const string& name) const = 0;


  int itsNinputs;
  int itsNoutputs;
  int itsIndex;
  string itsName;
  string itsType;
  ProcMode itsProcMode;  
  mutable map<string,int> itsInMap;
  mutable map<string,int> itsOutMap;

  static map<string,WHConstruct*>* itsConstructMap;
};


inline const DataHolder* WorkHolder::getConstInHolder (int channel) const
  { return const_cast<WorkHolder*>(this)->getInHolder (channel); }
inline const DataHolder* WorkHolder::getConstOutHolder (int channel) const
  { return const_cast<WorkHolder*>(this)->getOutHolder (channel); }

inline const string& WorkHolder::getType() const
  { return itsType; }

inline void WorkHolder::setName (const string& name)
  { itsName = name; }
inline const string& WorkHolder::getName() const
  { return itsName; }

inline int WorkHolder::getInputs() const
  { return itsNinputs; }
inline int WorkHolder::getOutputs() const
  { return itsNoutputs; }

inline WorkHolder::ProcMode WorkHolder::getProcMode() const
  { return (itsProcMode); }
inline void WorkHolder::setProcMode (ProcMode aProcMode)
  { itsProcMode = aProcMode; }


#endif
