//#  GSA_PvssApi.h: describes the API with the PVSS system
//#
//#  Copyright (C) 2002-2003
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

#ifndef  GSA_PVSSAPI_H
#define  GSA_PVSSAPI_H

// PVSS includes
#include <Manager.hxx>        

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

class GSASCADAHandler;

class GSAPvssApi : public Manager
{
private:
    friend class GSASCADAHandler;
	GSAPvssApi();
    virtual ~GSAPvssApi() {};

    void doWork();
    void stop();
    void init();
};
  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
#endif
