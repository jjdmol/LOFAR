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
    ApplicationHolder(int ninput, int noutput, DataHolder* dhptr);
    ApplicationHolder();
    virtual ~ApplicationHolder();
    
/*     virtual ApplicationHolder* clone() const = 0; */

    void setarg (int argc, const char* argv[]);
    void getarg (int* argc, const char** argv[]);
 
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

    int itsNinputs;
    int itsNoutputs;
  };

} // namespace LOFAR

#endif
