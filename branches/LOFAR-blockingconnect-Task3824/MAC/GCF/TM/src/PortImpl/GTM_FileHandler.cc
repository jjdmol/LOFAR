//#  GTM_FileHandler.cc: the specific handler for file message production
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

#include "GTM_FileHandler.h"
#include "GTM_File.h"
#include <GCF/TM/GCF_Task.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {
GTMFileHandler* GTMFileHandler::_pInstance = 0;

GTMFileHandler* GTMFileHandler::instance()
{
	if (0 == _pInstance) {
		_pInstance = new GTMFileHandler();
		ASSERT(!_pInstance->mayDeleted());
	}
	_pInstance->use();

	return _pInstance;
}

void GTMFileHandler::release()
{
	ASSERT(_pInstance);
	ASSERT(!_pInstance->mayDeleted());
	_pInstance->leave(); 
	if (_pInstance->mayDeleted()) {
		delete _pInstance;
		ASSERT(!_pInstance);
	}
}

GTMFileHandler::GTMFileHandler() : 
	_running(true)
{
	FD_ZERO(&_readFDs);
}

void GTMFileHandler::registerFile(GTMFile& file)
{
	LOG_TRACE_OBJ_STR("Adding filedescriptor " << file.getFD());
	FD_SET(file.getFD(), &_readFDs);
	_files[file.getFD()] = &file;
}

void GTMFileHandler::deregisterFile(GTMFile& file)
{
	LOG_TRACE_OBJ_STR("Removing filedescriptor " << file.getFD());
	FD_CLR(file.getFD(), &_readFDs);
	_files.erase(file.getFD());  
}

void GTMFileHandler::workProc()
{
	int				result;
	int				fd;
	TFiles 			testFiles;
	struct timeval	select_timeout;

 	//
	// because select call changes the timeout value to
	// contain the remaining time we need to set it to 10ms
	// on every call to workProc
	// 
	select_timeout.tv_sec  = 0;
	select_timeout.tv_usec = 10000;

	_running = true;
	fd_set testFDs = _readFDs;
	testFiles = _files;

  // map keys are sorted low-to-high, so last key is highest
  int maxfd = testFiles.empty() ? 0 : testFiles.rbegin()->first;

  // wait for any file to be readable, or for our timeout
	result = ::select(maxfd + 1, &testFDs, (fd_set *) 0, (fd_set *) 0, &select_timeout);

	if (result >= 0) {
    for (TFiles::iterator i = testFiles.begin(); i != testFiles.end() && _running; ++i) {
      int &fd = i->first;
      GTMFile &file = i->second;

			if (FD_ISSET(fd, &testFDs)) {
				file->doWork();
			}
    }
	}
}

void GTMFileHandler::stop()
{
	_running = false;
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
