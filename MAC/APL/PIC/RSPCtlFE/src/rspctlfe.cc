//#
//#  rspctlfe.cc: Front end for the command line interface to the RSPDriver
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
//#

//#
//# Usage:
//#
//# rspctlfe --id=<number>       # set the id of the frontend. Used in the name of the propertyset
//#
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <getopt.h>
#include <unistd.h>

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>

#include "rspctlfe.h"
#include <RSP_Protocol/RSPFE_Protocol.ph>

using namespace std;
using namespace LOFAR;
using namespace GCF::PAL;
using namespace GCF::TM;
using namespace GCF::Common;
using namespace RSPFE_Protocol;
using namespace rspctl;

// local funtions
static void usage();

const string RSPCtlFE::FE_PROPSET_NAME("PIC_CaptureStats%d");
const string RSPCtlFE::FE_PROPSET_TYPENAME("TLcuPicCaptureStats");
const string RSPCtlFE::FE_PROPERTY_COMMAND("command");
const string RSPCtlFE::FE_PROPERTY_STATUS("status");

INIT_TRACER_CONTEXT(RSPCtlFE, LOFARLOGGER_PACKAGE);

RSPCtlFEAnswer::RSPCtlFEAnswer(RSPCtlFE& rspctlfe) :
  _rspctlfe(rspctlfe)
{
}

void RSPCtlFEAnswer::handleAnswer (GCFEvent& answer)
{
  switch (answer.signal)
  {
    case F_VCHANGEMSG:
    case F_VGETRESP:
    {
      GCFPropValueEvent& vcEvent = (GCFPropValueEvent&) answer;
      string propName(vcEvent.pPropName);
      string val = ((GCFPVString*)(vcEvent.pValue))->getValue();
      _rspctlfe.valueChanged(propName, val);
      break;
    }
    default:
      break;
  }
}

RSPCtlFE::RSPCtlFE(string name, int argc, char** argv)
  : GCFTask((State)&RSPCtlFE::initial, name), 
    m_propertySetAnswer(*this),
    m_id(-1),
    m_myPropertySet(),
    m_server(),
    m_rspctlOutputTimer(0),
    m_rspctlOutputPosition(0),
    m_rspctlOutputFilename()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());

  registerProtocol(RSPFE_PROTOCOL, RSPFE_PROTOCOL_signalnames);

  parse_options(argc, argv);
  
  if(m_id == -1)
  {
    string msg("Error: argument --id is required");
    LOG_FATAL(msg.c_str());
    cerr << msg << endl;
    exit(EXIT_FAILURE);
  }

  string propertySetName(formatString(FE_PROPSET_NAME.c_str(),m_id));
  m_myPropertySet = boost::shared_ptr<GCFMyPropertySet>(new GCFMyPropertySet(
      propertySetName.c_str(),
      FE_PROPSET_TYPENAME.c_str(),
      PS_CAT_TEMPORARY,
      &m_propertySetAnswer));
  m_myPropertySet->enable();
  
  string servername(formatString("server%d",m_id));
  m_server.init(*this, servername, GCFPortInterface::SPP, RSPFE_PROTOCOL),
  
  m_rspctlOutputFilename = string(formatString("rspctlfe%doutput.log",m_id));
}

RSPCtlFE::~RSPCtlFE()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());

}

void RSPCtlFE::valueChanged(const string& propName, const string& value)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  
  if(propName.find(FE_PROPERTY_COMMAND) != string::npos)
  {
    if(value == string("stop"))
    {
      if(m_server.isConnected())
      {
        RSPFEStopRspctlEvent stopEvent;
        m_server.send(stopEvent);
      }
    }
    else
    {
      // only allow the FE to start rspctl when there is no connection with rspctl
      if(!m_server.isConnected())
      {
        // start the rspctl executable with the options specified.
        string commandLine("rspctl ");
        commandLine += value;
        
        string hostname(m_server.getHostName());
        if(hostname.length() == 0)
        {
          char localHostName[200];
          gethostname(localHostName,200);
          hostname = string(localHostName);
        }
        commandLine += string(formatString(" --feport=%s:%d",hostname.c_str(),m_server.getPortNumber()));
    
        commandLine += string(" &> ") + m_rspctlOutputFilename + string(" &");
        
        // remove the current logfile
        m_rspctlOutputPosition=0;
        remove(m_rspctlOutputFilename.c_str());
        
        int i=system(commandLine.c_str());
        if(i!=0)
        {
          LOG_FATAL(formatString("unable to start the rspctl executable using: %s",commandLine.c_str()));
          m_myPropertySet->setValue(FE_PROPERTY_STATUS,"unable to start the rspctl executable");
        }
        else
        {
          LOG_INFO(formatString("started rspctl using: %s",commandLine.c_str()));
          m_myPropertySet->setValue(FE_PROPERTY_STATUS,"rspctl started");
          
          m_rspctlOutputTimer = m_server.setTimer((long)1);
        }
      }
    }
  }
}

GCFEvent::TResult RSPCtlFE::initial(GCFEvent& e, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());

  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
    case F_INIT:
    break;

    case F_ENTRY:
    {
      // open the server port
      if (!m_server.isConnected())
      {
        m_server.open();
      }
    }
    break;

    case F_CONNECTED:
    {
      if (m_server.isConnected())
      {
        TRAN(RSPCtlFE::operational);
      }
    }
    break;

    case F_DISCONNECTED:
    {
      m_myPropertySet->setValue(FE_PROPERTY_STATUS,"rspctl stopped");
      port.setTimer((long)5);
      port.close();
    }
    break;

    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(e);
      if(timerEvent.id == m_rspctlOutputTimer)
      {
        pollRspCtlOutput();
        m_rspctlOutputTimer = m_server.setTimer((long)1);
      }
      else
      {
        // reopen the server port
        m_server.open();
      }
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult RSPCtlFE::operational(GCFEvent& e, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());

  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY:
    {
    }
    break;

    case RSPFE_STATUS_UPDATE:
    {
      RSPFEStatusUpdateEvent statusUpdateEvent(e);
      m_myPropertySet->setValue(FE_PROPERTY_STATUS,statusUpdateEvent.status);
    }
    break;
    
    case F_DISCONNECTED:
    {
      m_myPropertySet->setValue(FE_PROPERTY_STATUS,"rspctl stopped");
      port.close();
      TRAN(RSPCtlFE::initial);
    }
    break;

    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(e);
      if(timerEvent.id == m_rspctlOutputTimer)
      {
        pollRspCtlOutput();
        m_rspctlOutputTimer = m_server.setTimer((long)1);
      }
    }
    break;

    default:
      cerr << "Error: unhandled event." << endl;
      break;
  }

  return status;
}

void RSPCtlFE::pollRspCtlOutput()
{
  // check new additions to the rspctl output file and log them
  char line[300];
  FILE* pFile = fopen(m_rspctlOutputFilename.c_str(),"r");
  if(pFile != 0)
  {
    fseek(pFile,m_rspctlOutputPosition,SEEK_SET);
    while(!feof(pFile))
    {
      if(fgets (line, 300 , pFile) != 0)
      {
        LOG_INFO(formatString("rspctl log:%s",line));
      }
    }
    m_rspctlOutputPosition = ftell(pFile);
    fclose(pFile);
  }
}

void RSPCtlFE::mainloop()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());

  start(); // make initial transition
  GCFTask::run();
}

static void usage()
{
  cout << "rspctlfe usage:" << endl;
  cout << endl;
  cout << "rspctlfe --id=<number>   # set the id of the front end." << endl;
}

void RSPCtlFE::parse_options(int argc, char** argv)
{
  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
  while (1)
  {
    static struct option long_options[] =
      {
        { "id",         required_argument, 0, 'i' },
        { "help",       no_argument,       0, 'h' },

        { 0, 0, 0, 0 },
      };

    int option_index = 0;
    int c = getopt_long(argc, argv,
                        "i:h", long_options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
      case 'i':
        if (optarg)
        {
          m_id = atoi(optarg);
          if(m_id < 0)
          {
            string msg("Error: argument to --id out of range, value must be >= 0");
            LOG_FATAL(msg.c_str());
            cerr << msg << endl;
            exit(EXIT_FAILURE);
          }
        }
        else
        {
          string msg("Error: option '--id' requires an argument.");
          LOG_FATAL(msg.c_str());
          cerr << msg << endl;
          exit(EXIT_FAILURE);
        }
        break;

      case 'h':
        usage();
        break;

      case '?':
      default:
        exit(EXIT_FAILURE);
        break;
    }
  }
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  RSPCtlFE c("RSPCtlFE", argc, argv);

  try
  {
    c.mainloop();
  }
  catch (Exception e)
  {
    LOG_FATAL(formatString("Exception: %s", e.text().c_str()));
    cout << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Normal termination of program");

  return 0;
}
