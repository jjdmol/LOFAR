

#ifndef LOFAR_STORAGE_RTSTORAGE_H
#define LOFAR_STORAGE_RTSTORAGE_H

#include <Common/Timer.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>

#include <Stream/Stream.h>
#include <Stream/FileStream.h>

#include <Interface/Parset.h>
#include <Interface/Exceptions.h>
#include <Interface/PipelineOutput.h>
#include <Interface/MultiDimArray.h>

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
      

      // stores indexes of subbands that have packets waiting to be written
      Queue<unsigned> *myToWriteQueue;

   private:
      const Parset *itsPS;
      unsigned     itsRank;
      unsigned     itsSize;

      PipelineOutputSet itsPipelineOutputSet;

      unsigned     itsNrOutputs;
      unsigned     itsNrSubbands;
      unsigned     itsNrSubbandsPerStorage;
      unsigned     itsMyNrSubbands;

      NSTimer itsWriteTimer;

      std::vector<Stream *>      itsInputStreams;
      std::vector<bool>          itsIsNullStream;
      Matrix<FileStream *>       myFDs;
      std::vector<InputThread *> itsInputThreads;

      Format *myFormat;

      void createInputStream(unsigned subband);
      void createInputThread();
      bool processSubband(unsigned subband);
      void writeLogMessage();

    };

  } // namespace RTCP
} // namespace LOFAR



#endif // define LOFAR_STORAGE_RTSTORAGE_H
