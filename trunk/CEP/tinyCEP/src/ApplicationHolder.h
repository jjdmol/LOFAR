//#  ApplicationHolder.h: Base class for a user application in tinyCEP
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef TINYCEP_APPLICATIONHOLDER_H
#define TINYCEP_APPLICATIONHOLDER_H

#include <lofar_config.h>

//# Includes
#include <Transport/DataHolder.h>
#include <Transport/Transporter.h>
#include <tinyCEP/MiniDataManager.h>

#include <Common/KeyValueMap.h>

namespace LOFAR
{

  // Description of class.
  class ApplicationHolder
  {
  public:
    //    ApplicationHolder(int ninput, int noutput, DataHolder* dhptr);
    // default constructor does nothing.
    ApplicationHolder();
    virtual ~ApplicationHolder();
    
/*     virtual ApplicationHolder* clone() const = 0; */

    void setarg (int argc, const char* argv[]);
    void getarg (int* argc, const char** argv[]);
 
    void baseDefine (const KeyValueMap& params = KeyValueMap());
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
    virtual void define(const KeyValueMap&) = 0;
    virtual void init();
    virtual void run(int nsteps);
    virtual void run_once();
    virtual void dump() const;
    virtual void quit();

    // Forbid copy constructor
    ApplicationHolder (const ApplicationHolder&);
    // Forbid assignment
    ApplicationHolder& operator= (const ApplicationHolder&);

  private:
    int    itsArgc;
    const char** itsArgv;

    // DataHolder prototype for the application
    DataHolder* itsProto;
    // pointer to the application dataManager
    MiniDataManager* itsDataManager;

  };

} // namespace LOFAR

#endif
