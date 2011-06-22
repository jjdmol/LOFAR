//# MSWriterNull: a null MSWriter
//#
//#  Copyright (C) 2001
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
//#  $Id: $

#include <lofar_config.h>

#include <AMCBase/Epoch.h>
#include <Common/LofarLogger.h>
#include <Storage/MSWriter.h>
#include <Storage/MSWriterFile.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

template<typename T> class FreePtr {
public:
  FreePtr(T ptr): ptr(ptr) {}
  ~FreePtr() { free(ptr); }
  T get() const { return ptr; }
  T ptr;
};

const unsigned ALIGN = 512;

namespace LOFAR 
{

  namespace RTCP
  {

    MSWriterFile::MSWriterFile (const char*msName)
    :
#if 1
         itsFile             (msName,O_SYNC | O_RDWR | O_CREAT | O_TRUNC | O_DIRECT,
				     S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH),
#else
         itsFile             (msName,O_RDWR | O_CREAT | O_TRUNC,
				     S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH),
#endif
         itsBuffer(0),
         itsBufferSize(0),
         itsRemainder(0)
    {
    }

    MSWriterFile::~MSWriterFile()
    {
      if (itsBuffer && itsRemainder) {
        // pad with zeroes
        memset(itsBuffer + itsRemainder, 0, ALIGN - itsRemainder);
        itsFile.write(itsBuffer, ALIGN);
      }

      free( itsBuffer );
    }

    void MSWriterFile::write(StreamableData *data)
    {
      char *dataPtr = (char*)data->dataPtr();

      if (!dataPtr) {
        // CorrelatedData
        LOG_DEBUG_STR( "Writing directly to disk");
        data->write( &itsFile, true, ALIGN );
      } else {
        // SampleData
        size_t headerSize;
        size_t dataSize = data->requiredSize();

        FreePtr<void *> headerPtr = data->makeHeader(ALIGN, headerSize);

	// header should already be aligned
        ASSERT( headerSize == ALIGN );
        ASSERT( (reinterpret_cast<size_t>(headerPtr.get()) & (ALIGN-1)) == 0 );

        if (!itsBuffer) {
          itsBufferSize = headerSize + dataSize + ALIGN;
          if (posix_memalign( (void**)&itsBuffer, ALIGN, itsBufferSize ) != 0)
            THROW( StorageException, "Not enough memory to allocate " << itsBufferSize << " bytes for O_DIRECT writing");
        }

        if (!itsRemainder && (reinterpret_cast<size_t>(dataPtr) & (ALIGN-1)) == 0) {
          // pointer is aligned and we can write from it immediately

          // collect remainder
          itsRemainder = dataSize & (ALIGN-1);
          memcpy(itsBuffer, dataPtr + dataSize - itsRemainder, itsRemainder);

          // write bulk
          LOG_DEBUG_STR( "Writing directly to disk, with remainder of " << itsRemainder );
          itsFile.write(headerPtr.get(), headerSize);
          itsFile.write(dataPtr, dataSize - itsRemainder);
        } else {
          // not everything is aligned or there is a remainder -- use the buffer
          ASSERT( itsRemainder + headerSize + dataSize <= itsBufferSize );

          // move data to our buffer, and recompute new sizes
          memcpy( itsBuffer + itsRemainder, headerPtr.get(), headerSize );
          memcpy( itsBuffer + itsRemainder + headerSize, dataPtr, dataSize );
          dataSize = itsRemainder + headerSize + dataSize;
          itsRemainder = dataSize & (ALIGN-1);

          // write bulk
          LOG_DEBUG_STR( "Writing through buffer to disk, with remainder of " << itsRemainder );
          itsFile.write(itsBuffer, dataSize - itsRemainder);

          // move remainder to the front
          memmove(itsBuffer, itsBuffer + dataSize - itsRemainder, itsRemainder);
        }
      }
    }

  } // namespace RTCP
} // namespace LOFAR

