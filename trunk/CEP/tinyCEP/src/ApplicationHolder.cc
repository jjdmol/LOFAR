//#  ApplicationHolder.cc: Base class for a user application in tinyCEP
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

#include <tinyCEP/ApplicationHolder.h>

namespace LOFAR
{
  ApplicationHolder::ApplicationHolder(int ninput, int noutput, DataHolder* dhptr)
    : itsArgc(0),
      itsArgv(0),
      itsProto(dhptr),
      itsDataManager(0),
      itsNinputs(ninput),
      itsNoutputs(noutput){
    
    itsDataManager = new MiniDataManager(ninput, noutput);

    for (int i=0; i < itsNinputs; i++) {
      itsDataManager->addInDataHolder(i, dhptr);
    }
    for (int i=0; i < itsNoutputs; i++){
      itsDataManager->addOutDataHolder(i, dhptr);
    }
  }

  ApplicationHolder::~ApplicationHolder() {
    delete itsDataManager;
    delete itsProto;
  }
    
  void ApplicationHolder::define(const KeyValueMap& map) {
  }
  
  void ApplicationHolder::init() {
    // connect the DataHolders 1-1

    // tinyCEP has as requirements:
    // * number of inputs is equal to number of outputs
    // * inputs and outputs map 1 on 1
    // * DataHolder prototype is equal for input and output

    for (int i = 0; i < itsNinputs; i++) {
      Transporter& TR1 = itsDataManager->getInHolder(i)->getTransporter();
      Transporter& TR2 = itsDataManager->getOutHolder(i)->getTransporter();

      TR1.setItsID(i);
      TR2.setItsID(itsNinputs+i);
      
      TR1.setSourceAddr(itsDataManager->getOutHolder(i));
      TR2.setSourceAddr(itsDataManager->getInHolder(i));

      TR2.connectTo(&TR1, *itsProto->getTransporter().getTransportHolder());

      TR1.init();
      TR2.init();
    }
  }
  
  void ApplicationHolder::run(int nsteps) {
  }
  
  void ApplicationHolder::run_once() {
  }

  void ApplicationHolder::quit() {
  }

} // namespace LOFAR
