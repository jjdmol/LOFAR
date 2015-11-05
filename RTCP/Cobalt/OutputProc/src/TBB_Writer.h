/* TBB_Writer.h
 *
 * Copyright (C) 2012
 * ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: TBB_Writer.h 14188 2012-09-07 15:41:22Z amesfoort $
 */

#ifndef LOFAR_STORAGE_TBB_WRITER_H
#define LOFAR_STORAGE_TBB_WRITER_H 1

#include <stdint.h>

#include <string>
#include <vector>
#include <map>

#include <boost/crc.hpp>

#include <Common/LofarTypes.h>
#ifndef USE_THREADS
#error The TBB writer needs USE_THREADS to operate. You can also define it by hand and link to the right LOFAR lib(s).
#endif
#include <Common/Thread/Thread.h>
#include <Common/Thread/Queue.h>
#include <Stream/FileStream.h>
#include <CoInterface/Parset.h>

/*
 * Package__Version.h is generated by cmake. You also need to link with Package__Version.cc.o, etc.
 * Or hand-compile with -DTBB_WRITER_VERSION=\"x.y.z\". You'll receive the warning below twice and the
 * HDF5 'SYSTEM_VERSION' field is set to "x.y.z". The program options --help/--version show it too.
 */
#ifndef TBB_WRITER_VERSION
#include <OutputProc/Package__Version.h>
#else
#warning TBB_Writer version not derived from the cmake build system, but hard-coded using the TBB_WRITER_VERSION symbol.
#endif

#include <dal/lofar/TBB_File.h>

namespace LOFAR
{
  namespace Cobalt
  {

    /*
     * Incoming UDP frame format.
     * From 'TBB Design Description.doc', Doc.id: LOFAR-ASTRON-SDD-047, rev. 2.8 (2009-11-30), by Arie Doorduin, Wietse Poiesz
     * available at: http://www.lofar.org/project/lofardoc/document.php
     * Old rev. 2.0 (2006-10-3): http://lus.lofar.org/wiki/lib/exe/fetch.php?media=documents:sdd:lofar-astron-sdd-047_tbb_design_description.pdf
     *
     * There are two types of data that can be transferred: transient data and spectral (subband) data. Everything is in little-endian byte order.
     */
    struct TBB_Header {
      uint8_t stationID;        // Data source station identifier
      uint8_t rspID;            // Data source RSP board identifier
      uint8_t rcuID;            // Data source RCU board identifier
      uint8_t sampleFreq;       // Sample frequency in MHz of the RCU boards

      uint32_t seqNr;           // Used internally by TBB. Set to 0 by RSP (but written again before we receive it)
      uint32_t time;            // Time instance in seconds of the first sample in payload
      // The time field is relative, but if used as UNIX time, uint32_t will wrap at 06:28:15 UTC on 07 Feb 2106 (int32_t wraps at 03:14:08 UTC on 19 Jan 2038).

      union {
        // In transient mode indicates sample number of the first payload sample in current seconds interval.
        uint32_t sampleNr;

        // In spectral mode indicates frequency band and slice (transform block of 1024 samples) of first payload sample.
        uint32_t bandSliceNr;         // bandNr[9:0] and sliceNr[31:10].
#define TBB_BAND_NR_MASK        ((1 << 10) - 1)
#define TBB_SLICE_NR_SHIFT      10
      };

      uint16_t nOfSamplesPerFrame;   // Total number of samples in the frame payload
      uint16_t nOfFreqBands;    // Number of frequency bands for each spectrum in spectral mode. Is set to 0 for transient mode.

      uint8_t bandSel[64];      // Each bit in the band selector field indicates whether the band with the bit index is present in the spectrum or not.

      uint16_t spare;           // For future use. Set to 0.
      uint16_t crc16;           // CRC16 over frame header, with seqNr set to 0.
    };

    struct TBB_Payload {
      /*
       * In transient mode, a sample is a signed 12 bit integer. In spectral mode, it is a complex int16_t.
       * In the TBBs, transient samples are packed (2 samples per 3 bytes) with the checksum all the way at the end. This changes on transfer.
       *
       * TBB stores a frame in 2040 bytes (actually, 2048 with preamble and gaps). It sends a frame at a time, so derive our max from it.
       */
#define MAX_TBB_DATA_SIZE               (2040 - sizeof(TBB_Header) - sizeof(uint32_t))  // 1948: TBB frame size without header and payload crc32.

#define MAX_TBB_TRANSIENT_NSAMPLES      (MAX_TBB_DATA_SIZE / 3 * 2)     // 1298 (.666: 1 byte padding when indeed 1298 samples would ever be stored in TBB)
#define MAX_TBB_SPECTRAL_NSAMPLES       (MAX_TBB_DATA_SIZE / (2 * sizeof(int16_t)))     // 487

      // Unpacked, sign-extended (for transient) samples without padding, i.e. as received.
      // Frames might not be full; the doc says the crc32 is always sent right after (no padding), (but this is false for spectral),
      // so we include the crc32 in 'data', but note that the crc32 is a little endian uint32_t, hence ' + 2'.
#ifndef MAX
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#endif
      int16_t data[MAX(MAX_TBB_TRANSIENT_NSAMPLES, 2 * MAX_TBB_SPECTRAL_NSAMPLES) + 2];

      // For transient, TBB always sends sends 1024 samples per frame (from the spec and from the data).
      // For spectral, it depends on the nr of subbands (max is equal to MAX_TBB_SPECTRAL_NSAMPLES).
      // TBB sends as many samples for all subbands as it can fit; e.g. with 5 subbands, each frame has 485 samples.

#define SPECTRAL_TRANSFORM_SIZE         1024    // RSP FFT block size

#define DEFAULT_TBB_TRANSIENT_NSAMPLES  1024    // for spectral it depends on #subbands
    };

    struct TBB_Frame {
      TBB_Header header;
      TBB_Payload payload;
    };

    // Station meta data from other sources than the parset.
    struct StationMetaData {
      // If we receive data from a station not in the obs, we won't have all the meta data.
      bool available;

      // from the antenna field files
      std::vector<double> antPositions;
      std::vector<double> normalVector;     // [3]
      std::vector<double> rotationMatrix;   // [3, 3] row-major order

      // from the station calibration table files
      //...
    };

    // From station ID to a vector of antenna position coordinate components.
    typedef std::map<unsigned, StationMetaData> StationMetaDataMap;

    struct SubbandInfo {
      std::vector<double>   centralFreqs;     // empty in transient mode
      std::vector<unsigned> storageIndices;   // idem
    };


    class TBB_Dipole
    {
      LOFAR::FileStream*       itsRawOut;
      dal::TBB_Dataset<short>* itsDataset;
      std::vector<dal::Range>  itsFlagOffsets;

      uint32_t itsSampleFreq;   // Hz
      unsigned itsNrSubbands;   // spectral mode only, 0 in transient mode

      uint32_t itsTime;   // seconds
      union {
        uint32_t itsExpSampleNr;         // transient mode
        uint32_t itsExpSliceNr;          // spectral mode
      };
      ssize_t itsDatasetLen;

      // Same truncated polynomial as standard crc32, but with initial_remainder=0, final_xor_value=0, reflected_input=false, reflected_remainder_output=false.
      // The boost::crc_optimal<> declarations precompute lookup tables, so do not declare inside the checking routine. (Still, for every TBB_Dipole...)
      boost::crc_optimal<32, 0x04C11DB7 /*, 0, 0, false, false*/> itsCrc32gen;

      // do not use
      TBB_Dipole& operator=(const TBB_Dipole& rhs);

    public:
      TBB_Dipole();
      TBB_Dipole(const TBB_Dipole& rhs);   // do not use; only for TBB_Station vector<TBB_Dipole>(N) constr
      ~TBB_Dipole();

      // Output threads
      bool isInitialized() const;

      // All TBB_Dipole objects are default constructed in a vector, so have init().
      void init(const TBB_Header& header, const Parset& parset, const StationMetaData& stationMetaData,
                const SubbandInfo& subbandInfo, const std::string& rawFilename, dal::TBB_Station& station,
                Mutex& h5Mutex);

      void processTransientFrameData(const TBB_Frame& frame);
      void processSpectralFrameData(const TBB_Frame& frame, const SubbandInfo& subbandInfo);

    private:
      void appendFlags(size_t offset, size_t len);
      // initTBB_DipoleDataset() must be called with the global h5Mutex held.
      void initTBB_DipoleDataset(const TBB_Header& header, const Parset& parset,
                                 const StationMetaData& stationMetaData, const SubbandInfo& subbandInfo,
                                 const std::string& rawFilename, dal::TBB_Station& station);
      bool hasAllZeroDataSamples(const TBB_Payload& payload, size_t nTrSamples) const;
      bool crc32tbb(const TBB_Payload* payload, size_t nTrSamples);
    };

    class TBB_Station
    {
      dal::TBB_File itsH5File;
      Mutex& itsH5Mutex;
      dal::TBB_Station itsStation;
      std::vector<TBB_Dipole> itsDipoles;
      const Parset& itsParset;
      const StationMetaData& itsStationMetaData;
      const SubbandInfo itsSubbandInfo;   // for spectral mode
      const std::string itsH5Filename;

      double getSubbandCentralFreq(unsigned subbandNr, unsigned nyquistZone, double sampleFreq) const;
      SubbandInfo getSubbandInfo(const Parset& parset) const;
      std::string getRawFilename(unsigned rspID, unsigned rcuID) const;

      // do not use
      TBB_Station();
      TBB_Station(const TBB_Station& station);
      TBB_Station& operator=(const TBB_Station& rhs);

    public:
      // This constructor must be called with the h5Mutex already held.
      // The caller must still unlock after the return, the constructor does not use the passed ref to unlock.
      TBB_Station(const string& stationName, Mutex& h5Mutex, const Parset& parset,
                  const StationMetaData& stationMetaData, const std::string& h5Filename);
      ~TBB_Station();

      // Output threads
      void processPayload(const TBB_Frame& frame);

    private:
      std::string utcTimeStr(double time) const;
      double toMJD(double time) const;

      void initCommonLofarAttributes();
      void initTBB_RootAttributesAndGroups(const std::string& stName);
      void initStationGroup(dal::TBB_Station& st, const std::string& stName,
                            const std::string& stFullName, const std::vector<double>& stPosition);
      void initTriggerGroup(dal::TBB_Trigger& tg);
    };

    class TBB_Writer;

    class TBB_StreamWriter
    {
      /*
       * - The input thread receives incoming TBB frame headers, checks the header CRC, and puts them in a frameQueue.
       * - The output thread checks the data CRC, creates an HDF5 file per station, creates groups and datasets,
       *   writes the data, and returns empty frame pointers through the emptyQueue back to the input thread.
       *
       * On timeouts for all input threads, the main thread sends C++ thread cancellations. Input appends a NULL msg to notify output.
       * This isolates (soft) real-time input from HDF5/disk latencies, and the HDF5 C library from C++ cancellation exceptions.
       */

      /*
       * Queue size: With PRINT_QUEUE_LEN defined, the max used buffer size observed was 343.
       * This was for 1 udp stream (instead of 6 or 12) from 1 station. Having 1024 buffers per thread seems reasonable.
       */
      static const unsigned nrFrameBuffers = 1024;

      TBB_Frame* itsFrameBuffers;

      // Queue pointers point into itsFrameBuffers.
      Queue<TBB_Frame*> itsReceiveQueue;   // input  -> output thread
      Queue<TBB_Frame*> itsFreeQueue;      // output -> input  thread

      TBB_Writer& itsWriter;
      const std::string& itsInputStreamName;
      const unsigned itsExpFrameSize;
      const std::string& itsLogPrefix;
      int& itsInExitStatus;
      int& itsOutExitStatus;

      // See TBB_Writer_main.cc::doTBB_Run() why this is used racily for now.
      // Inflate struct timeval to 64 bytes (typical LEVEL1_DCACHE_LINESIZE). Unnecessary...
      struct timeval itsTimeoutStamp __attribute__((aligned(64)));

      boost::crc_optimal<16, 0x8005 /*, 0, 0, false, false*/> itsCrc16gen;

#ifdef DUMP_RAW_STATION_FRAMES
      LOFAR::FileStream* itsRawStationData;
#endif

      // Thread objects must be last in TBB_StreamWriter for safe destruction.
      Thread* itsOutputThread;
      Thread* itsInputThread;

      // do not use
      TBB_StreamWriter();
      TBB_StreamWriter(const TBB_StreamWriter& rhs);
      TBB_StreamWriter& operator=(const TBB_StreamWriter& rhs);

    public:
      TBB_StreamWriter(TBB_Writer& writer, const std::string& inputStreamName,
                       size_t expNTrSamples, const std::string& logPrefix,
                       int& inExitStatus, int& outExitStatus);
      ~TBB_StreamWriter();

      // Main thread
      time_t getTimeoutStampSec() const;

    private:
      // Input threads
      void frameHeaderLittleToHost(TBB_Header& fh) const;
      void correctSampleNr(TBB_Header& header) const;
      bool crc16tbb(const TBB_Header* header);
      void processHeader(TBB_Header& header, size_t recvPayloadSize);
      void mainInputLoop();

      // Output threads
      void mainOutputLoop();
    };

    class TBB_Writer
    {
      // Usually, we handle only 1 station, but users have request to support multiple concurrently.
      // The LOFAR system could better use different input streams (udp ports), but we/they are busy.
      // map from stationID to a TBB_Station*
      std::map<unsigned, TBB_Station*> itsStations;
      Mutex itsStationsMutex;

      // Global H5 mutex. All HDF5 operations go under a single mutex, incl file creation:
      // don't depend on the HDF5 lib being compiled with --thread-safe.
      Mutex itsH5Mutex;

      const Parset& itsParset;
      const StationMetaDataMap& itsStationMetaDataMap;
      StationMetaData itsUnknownStationMetaData;   // referred to for data from unknown stations (fallback)
      const std::string& itsOutDir;

      unsigned itsRunNr;

      std::vector<TBB_StreamWriter*> itsStreamWriters;
      // NOTE: do not add vars here; leave itsStreamWriters last for safe thread destruction!

      // do not use
      TBB_Writer();
      TBB_Writer(const TBB_Writer& writer);
      TBB_Writer& operator=(const TBB_Writer& rhs);

    public:
      TBB_Writer(const std::vector<std::string>& inputStreamNames, const Parset& parset,
                 const StationMetaDataMap& stationMetaDataMap, const std::string& outDir,
                 const std::string& logPrefix, vector<int>& thrExitStatus);
      ~TBB_Writer();

      // Output threads
      TBB_Station* getStation(const TBB_Header& header);
      // Must be called holding itsStationsMutex.
      std::string createNewTBB_H5Filename(const TBB_Header& header, const std::string& stationName);

      // Main thread
      time_t getTimeoutStampSec(unsigned streamWriterNr) const;
    };

  } // namespace Cobalt
} // namespace LOFAR

#endif // LOFAR_STORAGE_TBB_WRITER_H

