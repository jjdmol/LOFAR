//#  BlueGeneFrontEnd.h : Runs on the FrontEnd of BG/L to provide & dump data
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

#ifndef BLUEGENE_FRONTEND_H
#define BLUEGENE_FRONTEND_H

#include <lofar_config.h>

//# includes
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

//# include defitions for simulation size.
#include <BlueGeneCorrelator/definitions.h>

namespace LOFAR
{

  class BlueGeneFrontEnd: public LOFAR::TinyApplicationHolder {

  public:
    BlueGeneFrontEnd(bool input);
    virtual ~BlueGeneFrontEnd();

    // overload methods form the ApplicationHolder base class
    virtual void define(const KeyValueMap& params = KeyValueMap());
    virtual void init();
    virtual void run(int nsteps);
    virtual void dump() const;
    virtual void quit();

  private:
    WorkHolder* itsWHs;
    int         itsWHcount;
    bool        itsInput;
    int         itsPort;
  };

} // namespace


#endif
