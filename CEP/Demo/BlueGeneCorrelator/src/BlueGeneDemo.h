//#  BlueGeneDemo.h
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

#ifndef BLUEGENEDEMO_H
#define BLUEGENEDEMO_H

#include <lofar_config.h>

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/ApplicationHolder.h>

//# Include definitions.h to check simulation size
#include <tinyOnlineProto/definitions.h>

namespace LOFAR
{

  class BlueGeneDemo: public LOFAR::ApplicationHolder {
    
  public:
    BlueGeneDemo(bool BGl);
    virtual ~BlueGeneDemo();

    // overloaded methods from the ApplicationHolder base class
    virtual void define(const KeyValueMap& params = KeyValueMap());
    virtual void init();
    virtual void run(int nsteps);
    virtual void dump() const;
    virtual void quit();

  private:
    WorkHolder* itsOutsideWHs[ NBEAMLETS + NBEAMLETS * BFBW ]; // Frontend workholders
    WorkHolder* itsInsideWHs [ NBEAMLETS * BFBW ]; // BGL workholders
    int         itsOutsideWHCount;
    int         itsInsideWHCount;
    bool        itsIsBlueGene;
  };



} // namespace LOFAR

#endif
