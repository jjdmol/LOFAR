//#  GTM_Device.cc: base class for all sockets
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

#include "GTM_Device.h"
#include "GTM_FileHandler.h"
#include <GCF/TM/GCF_DevicePort.h>
#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

GTMDevice::GTMDevice(GCFDevicePort& port) :
  GTMFile(port)
{
}

GTMDevice::~GTMDevice()
{
  close();
}

ssize_t GTMDevice::send(void* buf, size_t count)
{
  if (_fd > -1) 
    return ::write(_fd, buf, count);
  else
    return 0;
}

ssize_t GTMDevice::recv(void* buf, size_t count)
{
  if (_fd > -1) 
    return ::read(_fd, buf, count);
  else
    return 0;
}

bool GTMDevice::open(const string& deviceName)
{
  if (_fd == -1)
  {
    int socketFD;
    socketFD = ::open(deviceName.c_str(), O_RDWR);
    if (socketFD < 0)
    {
      LOG_WARN(formatString (
          "Could not open device '%s' with following reason: %s",
          deviceName.c_str(),
          strerror(errno)));
    }
    setFD(socketFD);
  }
  return (_fd > -1);
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
