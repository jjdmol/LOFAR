//#  -*- mode: c++ -*-
//#  CalServer.h: class definition for the Beam Server task.
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

#ifndef CALSERVER_H_
#define CALSERVER_H_

#include "SpectralWindow.h"
#include "AntennaArray.h"
#include "SourceCatalog.h"
#include "DipoleModel.h"
#include "ACC.h"

#include <GCF/TM/GCF_Control.h>

namespace CAL
{
  class CalServer : public GCFTask
  {
  public:
    /**
     * The constructor of the CalServer task.
     * @param name The name of the task. The name is used for looking
     * up connection establishment information using the GTMNameService and
     * GTMTopologyService classes.
     */
    CalServer(string name);
    virtual ~CalServer();

    /**
     * Calibrate function. This method is the temporary entry-point of the
     * calibration server to call the calibrate method of the RemoteStationCalibration
     * class.
     * It loads all relevant configuration files and calls the calibration routine.
     */
    void calibrate();

    /**
     * Are all ports connected and are we ready to go to the
     * enabled state?
     */
    bool isEnabled();

    /**
     * The initial state.
     */
    GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

    /**
     * The undertaker method deletes dead clients on the m_dead_clients list.
     */
    void undertaker();

    /**
     * The enabled state.
     */
    GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface &p);

  private:
    /**
     * List of defined spectral windows.
     */
    std::vector<SpectralWindow> m_spws;   // vector of spectral windows (read from config file)
    std::vector<AntennaArray>   m_arrays; // vector of antenna arrays (read from config file)
    /*const*/ DipoleModel*          m_dipolemodel;   // dipole model
    const ACC*                  m_acc;           // ACC matrix

    /**
     * Client/Server management member variables.
     */
    GCFTCPPort                   m_acceptor;     // connect point for clients
    std::list<GCFPortInterface*> m_clients;      // list of clients
    std::list<GCFPortInterface*> m_dead_clients; // list of disconnected clients
    GCFPort                      m_acmserver;    // connection to the ACM server
  };
};
     
#endif /* CALSERVER_H_ */
