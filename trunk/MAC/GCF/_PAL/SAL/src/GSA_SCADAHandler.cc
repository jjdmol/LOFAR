//#  GSA_SCADAHandler.cc: describes the SCADA handler for connection with the 
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

#include "GSA_SCADAHandler.h"
#include <GSA_Resources.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/ParameterSet.h>

using namespace GCF;

GSASCADAHandler* GSASCADAHandler::_pInstance = 0;

GSASCADAHandler* GSASCADAHandler::instance()
{
  if (0 == _pInstance)
  {
    string cmdline;
    string pvssCmdLineParam = PARAM_DEFAULT_PVSS_CMDLINE;
    if (!ParameterSet::instance()->isDefined(pvssCmdLineParam))
    {
      char* appName = strrchr(GCFTask::_argv[0], '/');      
      pvssCmdLineParam = formatString(PARAM_PVSS_CMDLINE, (appName ? appName + 1 : GCFTask::_argv[0]));
    }
    try 
    {
      cmdline = ParameterSet::instance()->getString(pvssCmdLineParam);
    }
    catch (...)
    {
      cmdline = "-num 1 -proj LCU";
      LOG_WARN(formatString (
          "Specify the requested (see above) key in your '%s.conf.in' file. Used default",
          strrchr(GCFTask::_argv[0], '/') + 1));
    }
    // The PVSS API 3.0.1 redirects stdout and stderr output automatically to 
    // a file created by the API
    // We don't want this, so we have to repair this    
    cmdline += " -log +stderr";
    unsigned int offset(0);
    int nrOfWords(1);
    int argc;
    char* argv[20];
    char* words = new char[strlen(GCFTask::_argv[0]) + 1 + cmdline.length() + 1];
    memcpy(words, GCFTask::_argv[0], strlen(GCFTask::_argv[0]));
    offset = strlen(GCFTask::_argv[0]); 
    words[offset] = 0;
    offset++;
    argv[0] = words;
    string::size_type indexf(0), indexl(0);
    do 
    {
      indexf = cmdline.find_first_not_of(' ', indexl);
      if (indexf < string::npos)
      {
        indexl = cmdline.find_first_of(' ', indexf);
        if (indexl == string::npos)
        {
          indexl = cmdline.length();
        }
        //char* word = new char[indexl - indexf + 1];
        argv[nrOfWords] = words + offset;
        memcpy(words + offset, cmdline.c_str() + indexf, indexl - indexf);
        offset += indexl - indexf;
        words[offset] = 0;
        offset++;
        nrOfWords++;
      } 
      else
      {
        indexl = indexf;
      }
    } while (indexl < cmdline.length());
    argc = nrOfWords;
    GSAResources::init(argc, argv);
    
    
    delete [] words;
        
    _pInstance = new GSASCADAHandler();
    assert(!_pInstance->mayDeleted());
  }

  _pInstance->use();
  return _pInstance;
}

void GSASCADAHandler::release()
{
  assert(_pInstance);
  assert(!_pInstance->mayDeleted());
  _pInstance->leave(); 
  if (_pInstance->mayDeleted())
  {
    delete _pInstance;
    assert(!_pInstance);
  }
}

GSASCADAHandler::GSASCADAHandler() :
  _running(true)
  
{  
  GCFTask::registerHandler(*this);
}

void GSASCADAHandler::stop()
{
 
  _pvssApi.stop();

  _running = false;
}
 
void GSASCADAHandler::workProc()
{ 
  if (_running) _pvssApi.workProc();
}

TSAResult GSASCADAHandler::isOperational()
{ 
  TSAResult result(SA_SCADA_NOT_AVAILABLE);
  if (_pvssApi.getManagerState() == Manager::STATE_RUNNING && _running)
    result = SA_NO_ERROR;
  return result;
}
