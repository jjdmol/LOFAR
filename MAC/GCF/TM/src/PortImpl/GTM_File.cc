//#  GTM_File.cc: base class for all sockets
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
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include "GTM_File.h"
#include "GTM_FileHandler.h"
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

GTMFile::GTMFile(GCFRawPort& port) :
  _fd(-1),
  _pHandler(0),
  _port(port)
{
  _pHandler = GTMFileHandler::instance();
  ASSERT(_pHandler);
}

GTMFile::~GTMFile()
{
  close();
  GTMFileHandler::release();
  _pHandler = 0;
}

bool GTMFile::close()
{
  bool result(true);
  
  if (_fd > -1)
  { 
    ASSERT(_pHandler);
    _pHandler->deregisterFile(*this);
    result = (::close(_fd) == 0);
    if (!result)
    {
      LOG_WARN(formatString (
              "::close, error: %s",
              strerror(errno)));
      close();
    }
    
    _fd = -1;
  }
  return result;
}

int GTMFile::setFD(int fd)
{
  if (fd >= 0)
  {
    if (_fd > -1)
    {
      close();
    }
    _fd = fd;
    ASSERT(_pHandler);
    _pHandler->registerFile(*this);
  }
  return (fd);    
}

void GTMFile::workProc()
{
  unsigned long bytesRead = 0;
  
  if (ioctl(_fd, FIONREAD, &bytesRead) > -1)
  {
    if (bytesRead == 0)
    {
      GCFEvent e(F_DISCONNECTED);
      _port.dispatch(e);    
    }
    else 
    {
      GCFEvent e(F_DATAIN);
      _port.dispatch(e);
    }
  }
  else
  {
    ASSERT(_port.getTask());
    LOG_FATAL(LOFAR::formatString (
        "%s(%s): Error in 'ioctl' on socket fd %d: %s",
        _port.getTask()->getName().c_str(), 
        _port.getName().c_str(), 
        _fd, 
        strerror(errno)));        
  }
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
