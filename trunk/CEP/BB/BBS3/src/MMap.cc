//#  MMap.cc: implements a mmap operation
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

#include <lofar_config.h>

#include <BBS3/MMap.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <Common/LofarLogger.h>
#include <errno.h>

namespace LOFAR
{
  
MMap::MMap(const string& fileName, protection prot)
  : itsFileName  (fileName),
    itsNrBytes   (0),
    itsPageStart (0),
    itsPtr       (0),
    itsProtection(prot)
{
  // Open file
  int pr;
  switch (itsProtection) {
  case Read:
    pr = O_RDONLY;
    break;
  case Write:
    pr = O_WRONLY;
    break;
  case ReWr:
    pr = O_RDWR;
    break;
  default:
    pr = O_RDONLY;
    LOG_WARN("Invalid protection argument in MMap construction. Using Read protection instead...");
  }
  itsFd = open( fileName.c_str(), pr);
  ASSERTSTR (itsFd >= 0, "Opening of file " << fileName << " failed." );
}

MMap::~MMap()
{
  unmapFile();
  close(itsFd);
}

void MMap::mapFile(long long startOffset, size_t nrBytes)
{                                        // the actual mapped area will be a multiple of the page size
  if (itsPtr != 0)
  {
    LOG_WARN("Previous region still mapped! Unmapping...");
    unmapFile();
  }

  int sz = getpagesize();
  // Calculate start of page on which startOffset is located.
  long long pageStartOffset = (startOffset/sz) * sz;
  // Add the difference to the nr of bytes to map.
  itsNrBytes = nrBytes + startOffset-pageStartOffset;

  int protect;
  switch (itsProtection) {
  case Read:
    protect = PROT_READ;
    break;
  case Write:
    protect = PROT_WRITE;
    break;
  case ReWr:
    protect = PROT_READ | PROT_WRITE;
    break;
  default:
    protect = PROT_READ;
    LOG_WARN("Invalid value of protection member variable. Using Read protection instead...");
  }
    
  // Do mmap
  itsPageStart = mmap ( 0, itsNrBytes, protect, MAP_SHARED, itsFd,
			pageStartOffset);
  ASSERTSTR (itsPageStart != MAP_FAILED, "mmap failed: " << strerror(errno) ); 
  // Get requested pointer
  itsPtr = (char*)itsPageStart + startOffset-pageStartOffset;
}

void MMap::unmapFile()
{
  if (itsPtr != 0)
  {
    int res = munmap(itsPageStart, itsNrBytes);
    ASSERTSTR (res == 0, "munmap failed: " << strerror(errno));
    itsPageStart = 0;
    itsPtr = 0;
    itsNrBytes = 0;
    unlockMappedMemory();
  }
}

void MMap::lockMappedMemory()
{
  ASSERTSTR(itsPtr > 0, "No area has been mapped. ");
  int res;
  res = mlock(itsPageStart, itsNrBytes);
  ASSERTSTR(res != 0, "mlock failed: " << strerror(errno) ); 
}

void MMap::unlockMappedMemory()
{
  if (itsPageStart != 0)
  {
    int res = munlock(itsPageStart, itsNrBytes);
    ASSERTSTR (res == 0, "munlock failed: " << strerror(errno));
  }
}


} // namespace LOFAR
