//#  GSA_SCADAHandler.h: describes the SCADA handler for connection with the 
//#                      PVSS system
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

#ifndef GSA_SCADAHANDLER_H
#define GSA_SCADAHANDLER_H

#include <GCF/TM/GCF_Handler.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GSA_PvssApi.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

/**
 * This class implements the main loop part of message exchange handling, which 
 * uses the PVSS API. It calls the dispatch of the PVSS API to handle incoming 
 * and outgoing messages from/to PVSS.
 */
class GSASCADAHandler : GCFHandler
{
public:
    static GSASCADAHandler* instance ();
    static void release();
    
    void workProc ();
    void stop ();
    PVSSresult isOperational ();

private:
    GSASCADAHandler ();
    virtual ~GSASCADAHandler () { _pInstance = 0; }
    /**
     * Don't allow copying of the GSASCADAHandler object.
     */
    GSASCADAHandler (const GSASCADAHandler&);
    GSASCADAHandler& operator= (const GSASCADAHandler&);
    static GSASCADAHandler* _pInstance;
    
    GSAPvssApi      _pvssApi;
    bool            _running;
};
  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
#endif
