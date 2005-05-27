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
#include "Source.h"
#include "DipoleModel.h"
#include "ACC.h"
#include "SubArray.h"

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace CAL {

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
       * After completing the calibration it writes two result files:
       *   gains.out and quality.out.
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
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &port);

      /*@{*/
      /**
       * Handle the CAL_Protocol requests
       */
      GCFEvent::TResult handle_cal_start      (GCFEvent& e, GCFPortInterface &port);
      GCFEvent::TResult handle_cal_stop       (GCFEvent& e, GCFPortInterface &port);
      GCFEvent::TResult handle_cal_subscribe  (GCFEvent& e, GCFPortInterface &port);
      GCFEvent::TResult handle_cal_unsubscribe(GCFEvent& e, GCFPortInterface &port);
      /*@}*/

      /**
       * The undertaker method deletes dead clients on the m_dead_clients list.
       */
      void undertaker();

      /**
       * Remove a client and the associated subarray.
       */
      void remove_client(GCFPortInterface* port);

      /**
       * The enabled state.
       */
      GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface &port);

    private:
      /**
       * List of defined spectral windows.
       */
      std::vector<SpectralWindow> m_spws;         // vector of spectral windows (read from config file)
      AntennaArrays               m_arrays;       // antenna arrays (read from config file)
      SubArrays                   m_subarrays;    // the subarrays
      DipoleModels                m_dipolemodels; // dipole model (read from file)
      const ACC*                  m_acc;          // ACC matrix (read from file)
      Sources                     m_sources;      // source catalog (read from file)

      /**
       * Client/Server management member variables.
       */
      GCFTCPPort                               m_acceptor;     // connect point for clients
      std::map<GCFPortInterface*, std::string> m_clients;      // list of clients with related subarray name
      std::list<GCFPortInterface*>             m_dead_clients; // list of disconnected clients
      GCFPort                                  m_acmserver;    // connection to the ACM server
    };

  }; // namespace CAL
}; // namespace LOFAR
     
#endif /* CALSERVER_H_ */
