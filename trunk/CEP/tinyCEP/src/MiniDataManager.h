//#  MiniDataManager.h: a DataManager implementation for tinyCEP
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

#ifndef TINYDEP_MINIDATAMANAGER_H
#define TINYDEP_MINIDATAMANAGER_H

#include <lofar_config.h>

//# Includes
#include <tinyCEP/DataManager.h>
#include <libTransport/Transporter.h>

namespace LOFAR
{

  //# Forward Declarations
  class forward;


  // Description of class.
  class MiniDataManager : public DataManager
  {
  public:
    MiniDataManager(int inputs, int outputs);

    virtual ~MiniDataManager();

    Transporter* getInTransporter(int dholder);
    Transporter* getOutTransporter(int dholder);

    void addInTransporter(int channel, Transporter* tptr);
    void addOutTransporter(int channel, Transporter* tptr);

  private:
    int itsNinputs;
    int itsNoutputs;
    
    Transporter** itsInTRs;
    Transporter** itsOutTRs;
  };

} // namespace LOFAR

#endif
