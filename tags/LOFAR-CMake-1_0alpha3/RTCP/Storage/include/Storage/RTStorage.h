

#ifndef LOFAR_STORAGE_RTSTORAGE_H
#define LOFAR_STORAGE_RTSTORAGE_H

#include <Common/Timer.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>

#include <Stream/Stream.h>
#include <Stream/FileStream.h>

// signal CorrelatedData.h that we know that we're going to write big
// edian on little endian hardware
#define WRITE_BIG_ON_LITTLE_ENDIAN

#include <Interface/Parset.h>
#include <Interface/Exceptions.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_ProcessingPlan.h>
#include <Interface/MultiDimArray.h>
#include <Interface/RSPTimeStamp.h>

#include <Storage/Format.h>
#include <Storage/InputThread.h>

namespace LOFAR {
  namespace RTCP { 

    class RTStorage 
    {
    public:
      RTStorage(const Parset *, unsigned rank, unsigned size);
      ~RTStorage();

      void preprocess();
      void process();
      void postprocess();

   private:
      const Parset *itsPS;
      unsigned     itsRank;
      unsigned     itsSize;

      CN_Configuration itsConfiguration;
      CN_ProcessingPlan<> itsPlan;

      unsigned     itsNrOutputs;
      unsigned     itsNrSubbands;
      unsigned     itsNrSubbandsPerStorage;
      unsigned     itsMyNrSubbands;
      uint32       itsAlignment;       // alignment of userbuffers and
				       // writes, for O_DIRECT
      Matrix<int>  itsPreviousSequenceNumbers;
      
      TimeStamp    itsStartStamp;

      NSTimer itsWriteTimer;
      double  bytesWritten;

      std::vector<Stream *>      itsInputStreams;
      std::vector<bool>          itsIsNullStream;
      Matrix<FileStream *>       myFDs;
      std::vector<InputThread *> itsInputThreads;

      Format *myFormat;

      void createInputStream(unsigned subband);
      void createInputThread();
      bool processSubband(unsigned subband);
      void writeLogMessage();
      void checkForDroppedData(StreamableData *data, unsigned sb, unsigned output);
      
    };

  } // namespace RTCP
} // namespace LOFAR



#endif // define LOFAR_STORAGE_RTSTORAGE_H
