//# TH_ZoidClient.cc: In-memory transport mechanism
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#if defined HAVE_ZOID

#include <Transport/DataHolder.h>
#include <CS1_BGLProc/TH_ZoidClient.h>

extern "C" {
#include <lofar.h>
}

namespace LOFAR
{

bool TH_ZoidClient::recvBlocking(void *buf, int nbytes, int, int, DataHolder *)
{
  static size_t maxBytes = ~ (size_t) 0;

  for (size_t bytesRead = 0; bytesRead < (size_t) nbytes;) {
    size_t count = std::min(nbytes - bytesRead, maxBytes);
    lofar_ion_to_cn((char *) buf + bytesRead, &count);

    switch (__zoid_error()) {
      case 0	  : bytesRead += count;
      		    break;

      case E2BIG  : maxBytes = nbytes - __zoid_excessive_size();
		    break;

      default	  : return false;
    }
  }

  return true;
}


bool TH_ZoidClient::sendBlocking(void *buf, int nbytes, int, DataHolder *)
{
  static size_t maxBytes = ~ (size_t) 0;

  for (size_t bytesWritten = 0; bytesWritten < (size_t) nbytes;) {
    size_t count = std::min(nbytes - bytesWritten, maxBytes);
    lofar_cn_to_ion((char *) buf + bytesWritten, count);

    switch (__zoid_error()) {
      case 0	  : bytesWritten += count;
		    break;

      case E2BIG  : maxBytes = nbytes - __zoid_excessive_size();
		    break;

      default	  : return false;
    }
  }

  return true;
}

}

#endif
