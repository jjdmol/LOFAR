//# Simulator.h: Abstract base class for simulator programs
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

#ifndef CEPFRAME_SIMULATOR_H
#define CEPFRAME_SIMULATOR_H

#include <lofar_config.h>

#include "CEPFrame/Simulator.h"
#include "CEPFrame/Simul.h"
#include <Common/KeyValueMap.h>

namespace LOFAR
{

/**
   Simulator is the abstract base class for simulation application programs.
   It defines a few methods that need to be implemented in a derived class
   doing the simulation.

   The main functions that have to be implemented are define and run.
   Define should set up the simulation. Note that the base class already
   does the initialization and finalization of the transporter.
*/

class Simulator
{
public:
  /// Default constructor does nothing.
  Simulator();

  virtual ~Simulator();

  /// Set the command line argument pointers (to be used by MPI).
  void setarg (int argc, const char* argv[]);

  /// Get the command line argument pointers (to be used by MPI).
  void getarg (int* argc, const char** argv[]);

  /** Define the simulation.
      It starts the transport and calls the define function.
      At the end it resolves the communication.
  */
  void baseDefine (const KeyValueMap& params = KeyValueMap());

  /** Check the the simulation.
      The connections are checked and thereafter the user check()
      function is called.
  */
  void baseCheck();

  /// Do a prerun on the simulation.
  void basePrerun();

  /// Run the simulation by calling the run function.
  void baseRun (int nsteps = -1);

  /// Dump the simulation data by calling the dump function.
  void baseDump();

  /// Set the output file of a data holder.
  void baseDHFile (const string& dh, const string& name);

  /// Do a postrun on the simulation.
  void basePostrun();

  /** Quit the simulation.
      It calls the quit function and closes the transport.
  */
  void baseQuit();

protected:
  /**@name Virtual functions
     @memo Functions to be implemented in derived class.
     @doc The following functions can or have to be implemented in
     a derived class. The default implementations of the non-pure virtual
     functions do nothing.
     These virtual functions are called by their base... counterparts.
  */
  //@{
  virtual void define (const KeyValueMap&) = 0;
  virtual void check();
  virtual void prerun();
  virtual void run (int nsteps) = 0;
  virtual void dump() const;
  virtual void postrun();
  virtual void quit();
  //@}

  /// Fill the pointer to the simulation.
  void setSimul (const Simul&);

  /// Get the pointer to the simulation.
  Simul& getSimul();
  const Simul& getSimul() const;

private:
  // Forbid copy constructor.
  Simulator (const Simulator&);

  // Forbid assignment.
  Simulator& operator= (const Simulator&);


  /** A pointer to the simulation.
      Note that the base destructor does not delete the pointer.
  */
  Simul itsSimul;

  int    itsArgc;
  const char** itsArgv;
  bool   itsPreDone;
  bool   itsPostDone;
};


inline void Simulator::setSimul (const Simul& simul)
  { itsSimul = simul; }

inline Simul& Simulator::getSimul()
  { return itsSimul; }

inline const Simul& Simulator::getSimul() const
  { return itsSimul; }


}


#endif
