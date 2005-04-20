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
#include "SubArray.h"
#include "RemoteStationCalibration.h"

#ifndef CAL_SYSCONF
#define CAL_SYSCONF "."
#endif

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/ParameterSet.h>
#include <fstream>
#include <signal.h>

#include <blitz/array.h>

using namespace LOFAR;
using namespace blitz;
using namespace CAL;
using namespace std;

using namespace CAL_Protocol;

CalServer::CalServer(string name)
  : GCFTask((State)&CalServer::initial, name),
    m_dipolemodel(0), m_acc(0), m_catalog(0)
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
      ParameterSet ps(CAL_SYSCONF "/SpectralWindow.conf");

      //
      // load the spectral window configurations
      //
      m_spws = SPWLoader::loadFromBlitzStrings(ps["SpectralWindow.params"],
					       ps["SpectralWindow.names"]);

      //
      // load the dipole model
      //
      m_dipolemodel = DipoleModelLoader::loadFromFile(CAL_SYSCONF "/DipoleModel.conf");

      //cout << "Dipole model=" << m_dipolemodel->getModel() << endl;

      //
      // load the source catalog
      //
      m_catalog = SourceCatalogLoader::loadFromFile(CAL_SYSCONF "/SourceCatalog.conf");

      //
      // Load antenna arrays
      //
      AntennaArray* lba = AntennaArrayLoader::loadFromFile("LBA", CAL_SYSCONF "/LBAntennas.conf");
      m_arrays.push_back(*lba);
      delete lba;

      AntennaArray* hba = AntennaArrayLoader::loadFromFile("HBA", CAL_SYSCONF "/HBAntennas.conf");
      m_arrays.push_back(*hba);
      delete hba;

      //
      // load the ACC
      //
      m_acc = ACCLoader::loadFromFile(CAL_SYSCONF "/ACC.conf");

      if (!m_acc)
	{
	  LOG_ERROR("Failed to load ACC matrix.");
	  exit(EXIT_FAILURE);
	}

      //cout << "sizeof(ACC)=" << m_acc->getSize() * sizeof(complex<double>) << endl;

#if 0
      ofstream accstream("acc.out");
      if (accstream.is_open())
	{
	  accstream << m_acc->getACC();
	}
#endif
    }
  catch (Exception e) 
    {
      LOG_ERROR_STR("Failed to load configuration files: " << e);
      exit(EXIT_FAILURE);
    }

#if 0
  for (unsigned int i = 0; i < m_spws.size(); i++)
    {
      cout << "SPW[" << i << "]=" << m_spws[i].getName() << endl;
    }
#endif

  //
  // Dimensions of the antenna array
  //
  int nantennas = m_arrays[0].getAntennaPos().extent(firstDim);
  int npol      = m_arrays[0].getAntennaPos().extent(secondDim);

  //
  // Create the FTS-1 subarray, with spectral window 0 (0 - 80 MHz)
  // 
  Array<bool,2> select(nantennas, npol);
  select = true;
  SubArray fts1("FTS-1", m_arrays[0].getAntennaPos(), select, m_spws[0]);

  //
  // Create the calibration algorithm and
  // call the startCalibration routine of the subarray
  // with the ACC.
  //
  RemoteStationCalibration cal(*m_catalog, *m_dipolemodel);
  fts1.startCalibration(&cal, *m_acc);

  //
  // Sanity check on dimensions of the various arrays
  //
  // Number of antennas and polarizations must be equal on all related arrays.
  //
  ASSERT(m_acc->getACC().extent(firstDim) == nantennas
	 && m_acc->getACC().extent(secondDim) == nantennas
	 && m_acc->getACC().extent(thirdDim) == m_spws[0].getNumSubbands()
	 && m_acc->getACC().extent(fourthDim) == npol
	 && m_acc->getACC().extent(fifthDim) == npol);

  //
  // Save the calibration gains and quality matrices
  //
  const CalibrationResult* result = 0;
  if (fts1.getCalibration(result, SubArray::BACK))
    {
      // save gains matrix
      ofstream gains("gains.out");
      if (gains.is_open())
	{
	  gains << result->getGains();
	}
      else
	{
	  LOG_ERROR("Failed to open output file: gains.out");
	  exit(EXIT_FAILURE);
	}

      // save quality matrix
      ofstream quality("quality.out");
      if (quality.is_open())
	{
	  quality << result->getQuality();
	}
      else
	{
	  LOG_ERROR("Failed to open output file: quality.out");
	  exit(EXIT_FAILURE);
	}
    }
  else
    {
      LOG_ERROR("Calibration has not yet completed.");
      exit(EXIT_FAILURE);
    }
}

#if 0
void signalHandler(int sig)
{
  if ( (sig == SIGINT) || (sig == SIGTERM) )
    {
      LOG_ERROR("exiting on signal");
      exit(EXIT_FAILURE);
    }
}                                        
#endif

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

//   signal(SIGINT,  signalHandler);
//   signal(SIGTERM, signalHandler);
//   signal(SIGPIPE, SIG_IGN);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  CalServer cal("CalServer");

  struct timeval start, end;
  gettimeofday(&start, 0);
  cal.calibrate(); // temporary entry point for standalone CalServer
  gettimeofday(&end, 0);

  LOG_INFO_STR("CalServer execution time: " << end.tv_sec - start.tv_sec << " seconds");

#if 0
  // this code is not used since CalServer is currently a stand-alone program

  cal.start(); // make initial transition

  try
  {
    GCFTask::run();
  }
  catch (Exception e)
  {
    LOG_ERROR_STR("Exception: " << e.text());
    exit(EXIT_FAILURE);
  }
#endif

  LOG_INFO("Normal termination of program");

  return 0;
}
