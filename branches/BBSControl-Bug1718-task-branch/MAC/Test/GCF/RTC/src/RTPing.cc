//
//  RTPing.cc: Implementation of the Ping task class.
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//

#include <lofar_config.h>

#include "RTPing.h"
#include "RTDefines.h"
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/CmdLine.h>
#include "Echo_Protocol.ph"

namespace LOFAR
{
 namespace GCF
 {
using namespace Common;
using namespace TM;
using namespace RTCPMLlight;
  namespace Test
  {
/**
 * Function to calculate the elapsed time between two tiemval's.
 */
static double time_elapsed(timeval* start, timeval* stop) 
{
  return stop->tv_sec-start->tv_sec 
    + (stop->tv_usec-start->tv_usec)/(double)1e6; 
}

Ping::Ping(string name, string scope, string type, TPSCategory category)
  : GCFTask((State)&Ping::initial, name), 
  _pingTimer(-1), 
  _answerHandler(*this),
  _echoPingPSET(scope.c_str(), type.c_str(), category, &_answerHandler)
{
  // register the port for debug tracing
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_signalnames);

  /**
   * Initialize the "client" port
   * - Pass the this pointer, because this port belongs to
   *   this task.
   * - Give it the name "client".
   * - This is a Service Access Port which uses the
   *   ECHO_PROTOCOL 
   */
  if (type == "TTypeF")
  {
    _echoPingPSET.initProperties(propertiesSF1);
  }
  else
  {
    _echoPingPSET.initProperties(propertiesSG1);
  }
  _client.init(*this, "client", GCFPortInterface::SAP, ECHO_PROTOCOL);
}

GCFEvent::TResult Ping::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      _echoPingPSET.enable();
      _client.open();
      break;

    case F_CONNECTED:
    case F_MYPS_ENABLED:
      if (_client.isConnected() && _echoPingPSET.isEnabled())
      {
        TRAN(Ping::connected);
      }
      break;

    case F_DISCONNECTED:
      _client.setTimer(1.0); // try connect again after 1 second
      break;

    case F_TIMER:
      _client.open();
      break;


    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = (GCFPropValueEvent*)(&e);
      assert(pResponse);
      if ((strncmp(pResponse->pPropName, "B_A_BRD", 7) == 0) && 
          (strncmp(pResponse->pPropName + 8, ".max", 4) == 0))
      {
        GCFPVInteger* pMaxSeqProp = (GCFPVInteger*)(pResponse->pValue);
        assert(pMaxSeqProp);
        if (pResponse->pPropName[7] == '1')
        {
          _echoPingPSET["sn"].setValue(*pMaxSeqProp);
        }
        else
        {
          _echoPingPSET["sn000"].setValue(*pMaxSeqProp);
        }
      }
      break;
    }

    case F_EXIT:
      _pingTimer = _client.setTimer(1.0, 0.3);
      break;
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult Ping::connected(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned int seqnr = 0;
  
  switch (e.signal)
  {

    case F_ENTRY:
    {
      GCFPVInteger* pMaxSeqProp = (GCFPVInteger*)(_echoPingPSET["max"].getValue());
      assert(pMaxSeqProp);
      _maxSeqNr = pMaxSeqProp->getValue();
      delete pMaxSeqProp; // was created by the first getValue() - clone of current value
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      if ((strncmp(pResponse->pPropName, "B_A_BRD", 7) == 0) && 
          (strncmp(pResponse->pPropName + 8, ".max", 4) == 0))
      {
        GCFPVInteger* pMaxSeqProp = (GCFPVInteger*)(pResponse->pValue);
        assert(pMaxSeqProp);
        _maxSeqNr = pMaxSeqProp->getValue();
      }
      break;
    }
    case F_TIMER:
    {
      
      GCFTimerEvent* pTimer = (GCFTimerEvent*) &e;
      
      char timeString[9];
      strftime(timeString, 9, "%T", localtime(&pTimer->sec));
      printf("Timer event received on %s.%ld\n", timeString, pTimer->usec);     
      timeval pingTime;

      // create PingEvent
      gettimeofday(&pingTime, 0);
      EchoPingEvent ping;
      ping.seqnr = seqnr++,
      ping.pingTime = pingTime;

      // send the event
      _client.send(ping);
      if (seqnr >= _maxSeqNr) seqnr = 0;
      if (_echoPingPSET.getScope()[7] == '1')
      {
        _echoPingPSET["sn"].setValue(GCFPVInteger(ping.seqnr));
      }
      else
      {
        char seqNrName[6];
        for (unsigned int i = 0; i < 256; i++)
        {
          sprintf(seqNrName, "sn%03d", i);
          _echoPingPSET[seqNrName].setValue(GCFPVInteger(ping.seqnr));
        }
      }
      printf("PING sent (seqnr=%d)\n", ping.seqnr);

      TRAN(Ping::awaiting_echo); // wait for the echo
      break;
    }
    case F_DISCONNECTED:
      (void)_client.cancelTimer(_pingTimer);

      seqnr = 0;
      TRAN(Ping::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult Ping::awaiting_echo(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_TIMER:
      printf("Missed echo dead-line.\n");
      break;

    case ECHO_ECHO:
    {
      timeval echoTime;
      gettimeofday(&echoTime, 0);

      EchoEchoEvent echo(e);

      printf ("ECHO received (seqnr=%d): elapsed = %f sec.\n", echo.seqnr,
           time_elapsed(&(echo.pingTime), &echoTime));

      TRAN(Ping::connected);
      break;
    }
    case F_DISCONNECTED:
      (void)_client.cancelTimer(_pingTimer);
      TRAN(Ping::initial);
      break;
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }
  
  return status;
}

  } // namespace Test
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF;

int main(int argc, char** argv)
{
  TM::GCFTask::init(argc, argv);
  
  LOG_INFO("MACProcessScope: GCF.TEST.RTC.RTPing");

  string brdnr("1");
  if (argv != 0)
  {
    Common::CCmdLine cmdLine;

    // parse argc,argv 
    if (cmdLine.SplitLine(argc, argv) > 0)
    {
      brdnr = cmdLine.GetSafeArgument("-brdnr", 0, "1");
    }            
  }

  string type;
  Common::TPSCategory category;
  if (brdnr == "1")
  {
    type = "TTypeF";
    category = Common::PS_CAT_PERMANENT;
  }
  else
  {
    type = "TTypeG";
    category = Common::PS_CAT_TEMPORARY;
  }
  Test::Ping pingTask(string("RTPING") + brdnr, string("B_A_BRD") + brdnr, type, category);

  pingTask.start(); // make initial transition

  TM::GCFTask::run();
  
  return 0;  
}
