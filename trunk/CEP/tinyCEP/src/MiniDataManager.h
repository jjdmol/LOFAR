//#  MiniDataManager.h: a BaseDataManager implementation for tinyCEP
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
#include <tinyCEP/BaseDataManager.h>
#include <libTransport/DataHolder.h>

namespace LOFAR
{

  //# Forward Declarations
  class forward;


  // Description of class.
  class MiniDataManager : public BaseDataManager
  {
  public:
    MiniDataManager(int inputs, int outputs);

    virtual ~MiniDataManager();

    DataHolder* getInHolder(int dholder);
    DataHolder* getOutHolder(int dholder);

    void addInDataHolder(int channel, DataHolder* dhptr);
    void addOutDataHolder(int channel, DataHolder* dhptr);

  private:
    int itsNinputs;
    int itsNoutputs;
    
    DataHolder** itsInTRs;
    DataHolder** itsOutTRs;
  };

} // namespace LOFAR

#endif
