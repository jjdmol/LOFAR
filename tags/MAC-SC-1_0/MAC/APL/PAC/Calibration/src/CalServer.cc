//#
//#  CalServer.cc: implementation of CalServer class
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES
#include "CAL_Protocol.ph"

#include "CalServer.h"
#include "SpectralWindow.h"

#ifndef CAL_SYSCONF
#define CAL_SYSCONF "."
#endif

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/ParameterSet.h>

#include <blitz/array.h>

using namespace LOFAR;
using namespace blitz;
using namespace CAL;
using namespace std;

using namespace CAL_Protocol;

CalServer::CalServer(string name)
    : GCFTask((State)&CalServer::initial, name)
{
#if 0
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, CAL_PROTOCOL);
  //m_acmserver.init(*this, "acmserver", GCFPortInterface::SAP, ACM_PROTOCOL);
#endif

}

CalServer::~CalServer()
{}

bool CalServer::isEnabled()
{
  return false; //m_acmserver.isConnected();
}

GCFEvent::TResult CalServer::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
  {
    case F_INIT:
    {
    }
    break;

    case F_ENTRY:
    {
      //if (!m_acmserver.isConnected()) m_acmserver.open();
    }
    break;

    case F_CONNECTED:
    {
      if (isEnabled())
      {
	TRAN(CalServer::enabled);
      }
    }
    break;

    case F_DISCONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      port.close();
    }
    break;

    case F_TIMER:
    {
      if (!port.isConnected())
      {
	LOG_DEBUG(formatString("port '%s' retry of open...", port.getName().c_str()));
	port.open();
      }
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void CalServer::undertaker()
{
  for (list<GCFPortInterface*>::iterator it = m_dead_clients.begin();
       it != m_dead_clients.end();
       it++)
  {
    delete (*it);
  }
  m_dead_clients.clear();
}

GCFEvent::TResult CalServer::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  undertaker(); // destroy dead clients

  switch (e.signal)
  {
    case F_ENTRY:
    {
      if (!m_acceptor.isConnected()) m_acceptor.open();
    }
    break;

    case F_ACCEPT_REQ:
    {
      GCFTCPPort* client = new GCFTCPPort();
      client->init(*this, "client", GCFPortInterface::SPP, CAL_PROTOCOL);
      m_acceptor.accept(*client);
      m_clients.push_back(client);

      LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", m_clients.size()));
    }
    break;
      
    case F_CONNECTED:
    {
      LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
    }
    break;

    case F_TIMER:
    {
      GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

      LOG_INFO(formatString("timer=(%d,%d)", timer->sec, timer->usec));

    }
    break;

    case F_DISCONNECTED:
    {
      LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
      port.close();

      if (&m_acmserver == &port || &m_acceptor == &port)
      {
	m_acceptor.close();
	TRAN(CalServer::initial);
      }
      else
      {
	// TODO: clean up

	m_clients.remove(&port);
	m_dead_clients.push_back(&port);
      }
    }
    break;

    case F_EXIT:
    {
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void CalServer::calibrate()
{
  //
  // Load configuration files.
  //
  try 
    {
      ParameterSet ps(string(CAL_SYSCONF) + "/" + string("SpectralWindow.conf"));

      //
      // load the spectral window configurations
      //
      m_spws = SPWLoader::loadFromBlitzStrings(ps["SpectralWindow.params"],
					       ps["SpectralWindow.names"]);

      //
      // load the dipole model
      //
      ps = ParameterSet(string(CAL_SYSCONF) + "/" + string("DipoleModel.conf"));
      m_dipolemodel = DipoleModelLoader::loadFromBlitzString("Primary DipoleModel", ps["DipoleModel.sens"]);

      cout << "Dipole model=" << m_dipolemodel->getModel() << endl;

      //
      // Load antenna arrays
      //
      ps = ParameterSet(string(CAL_SYSCONF) + "/" + string("RemoteStation.conf"));
      AntennaArray* lba = AntennaArrayLoader::loadFromBlitzString("LBA", ps["RS.LBA_POSITIONS"]);
      m_arrays.push_back(*lba);
      AntennaArray* hba = AntennaArrayLoader::loadFromBlitzString("HBA", ps["RS.HBA_POSITIONS"]);
      m_arrays.push_back(*hba);

      //
      // load the ACC
      //
      m_acc = ACCLoader::loadFromFile(string(CAL_SYSCONF) + "/" + string("ACC.conf"));

      if (!m_acc)
	{
	  LOG_ERROR("Failed to load ACC matrix.");
	  exit(EXIT_FAILURE);
	}

      cout << "sizeof(ACC)=" << m_acc->getSize() * sizeof(complex<double>) << endl;
    }
  catch (Exception e) 
    {
      cout << "Failed to load configuration files: " << e << endl;
      exit(EXIT_FAILURE);
    }

  for (unsigned int i = 0; i < m_spws.size(); i++)
    {
      cout << "SPW[" << i << "]=" << m_spws[i].getName() << endl;
    }

  //
  // Call the calibration routine.
  //
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try 
  {
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration file: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  CalServer cal("CalServer");

  cal.calibrate(); // temporary entry point for standalone CalServer

#if 0
  cal.start(); // make initial transition

  try
  {
    GCFTask::run();
  }
  catch (Exception e)
  {
    cerr << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }
#endif

  LOG_INFO("Normal termination of program");

  return 0;
}
