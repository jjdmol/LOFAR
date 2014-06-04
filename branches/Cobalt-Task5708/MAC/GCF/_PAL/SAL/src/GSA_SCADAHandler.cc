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

#include <lofar_config.h>

#include <lofar_config.h>

#define LOFARLOGGER_SUBPACKAGE "SAL"

#include "GSA_SCADAHandler.h"
#include <GSA_Resources.h>
#include <GCF/TM/GCF_Task.h>
#include <Common/ParameterSet.h>

namespace LOFAR {
 namespace GCF {
  namespace PAL {
GSASCADAHandler* GSASCADAHandler::_pInstance = 0;

GSASCADAHandler* GSASCADAHandler::instance()
{
  if (0 == _pInstance) {
    string cmdline;
    for (int i = 0; i < GCFTask::_argc; i++) {
      cmdline += GCFTask::_argv[i];
      cmdline += " ";
    }
    cmdline += "-currentproj ";
    string pvssCmdLineParam = PARAM_DEFAULT_PVSS_CMDLINE;
    char* appName = strrchr(GCFTask::_argv[0], '/');
    if (!globalParameterSet()->isDefined(pvssCmdLineParam)) {            
      pvssCmdLineParam = formatString(PARAM_PVSS_CMDLINE, (appName ? appName + 1 : GCFTask::_argv[0]));
    }
    if (globalParameterSet()->isDefined(pvssCmdLineParam)) {
      cmdline += globalParameterSet()->getString(pvssCmdLineParam);
    }
    // The PVSS API 3.0.1 redirects stdout and stderr output automatically to 
    // a file created by the API
    // We don't want this, so we have to repair this    
    cmdline += " -log +stderr";
    unsigned int offset(0);
    int nrOfWords(0);
    int argc;
    char* argv[20];
    char* words = new char[cmdline.length() + 1];
    argv[0] = words;
    string::size_type indexf(0), indexl(0);
    do {
      indexf = cmdline.find_first_not_of(' ', indexl);
      if (indexf != string::npos) {
        indexl = cmdline.find_first_of(' ', indexf);
        if (indexl == string::npos) {
          indexl = cmdline.length();
        }        
        argv[nrOfWords] = words + offset;
        memcpy(words + offset, cmdline.c_str() + indexf, indexl - indexf);
        offset += indexl - indexf;
        words[offset] = 0;
        offset++;
        nrOfWords++;
      } 
      else {
        indexl = indexf;
      }
    } while (indexl < cmdline.length());
    argc = nrOfWords;
    GSAResources::init(argc, argv);
    
    
    delete [] words;
        
    _pInstance = new GSASCADAHandler();
    ASSERT(!_pInstance->mayDeleted());
  }

  _pInstance->use();
  return _pInstance;
}

void GSASCADAHandler::release()
{
  ASSERT(_pInstance);
  ASSERT(!_pInstance->mayDeleted());
  _pInstance->leave(); 
  if (_pInstance->mayDeleted()) {
    delete _pInstance;
    ASSERT(!_pInstance);
  }
}

GSASCADAHandler::GSASCADAHandler() :
  _running(true)
  
{  
}

void GSASCADAHandler::stop()
{
 
  _pvssApi.stop();

  _running = false;
}
 
void GSASCADAHandler::workProc()
{ 
  if (_running) {
	_pvssApi.workProc();
  }
}

TSAResult GSASCADAHandler::isOperational()
{ 
  TSAResult result(SA_SCADA_NOT_AVAILABLE);
  if (_pvssApi.getManagerState() == Manager::STATE_RUNNING) {
    result = SA_NO_ERROR;
  }
  return result;
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
