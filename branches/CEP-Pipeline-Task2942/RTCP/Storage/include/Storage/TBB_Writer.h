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
 * $Id: TBB_Writer.h 10353 2012-03-14 15:41:22Z amesfoort $
 */

#ifndef LOFAR_STORAGE_TBB_WRITER_H
#define LOFAR_STORAGE_TBB_WRITER_H 1

#include <cstddef>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <cerrno>

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <boost/crc.hpp>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#ifndef USE_THREADS
#error The TBB writer needs USE_THREADS to operate.
#endif
#include <Common/Thread/Thread.h>
#include <Common/Thread/Queue.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#if defined HAVE_PKVERSION
#include <Storage/Package__Version.h>
#else
#include <Common/Version.h>
#endif

#include <dal/lofar/TBB_File.h>

namespace LOFAR {
namespace RTCP {

/* 
 * Incoming UDP frame format.
 * From 'TBB Design Description.doc', Doc.id: LOFAR-ASTRON-SDD-047, rev. 2.8 (2009-11-30), by Arie Doorduin, Wietse Poiesz
 * available at: http://www.lofar.org/project/lofardoc/document.php
 * Old rev. 2.0 (2006-10-3): http://lus.lofar.org/wiki/lib/exe/fetch.php?media=documents:sdd:lofar-astron-sdd-047_tbb_design_description.pdf
 *
 * In TBB, each frame is 2040 bytes long, consisting of an 88 bytes header, and a 1952 bytes payload (both with their own checksum).
 * There are two types of data that can be transferred: transient data or spectral data.
 * However, incoming frame payloads are somewhat different from frame payloads stored at the TBB: data is transferred unpacked, directly followed by a crc32.
 * Everything is in little-endian byte order.
 */
struct TBB_Header {
	uint8_t stationID;	// Data source station identifier
	uint8_t rspID;		// Data source RSP board identifier
	uint8_t rcuID;		// Data source RCU board identifier
	uint8_t sampleFreq;	// Sample frequency in MHz of the RCU boards

	uint32_t seqNr;		// Used internally by TBB. Set to 0 by RSP (but written again before we receive it)
	uint32_t time;		// Time instance in seconds of the first sample in payload
	// The time field is relative, but if used as UNIX time, uint32_t will wrap at 06:28:15 UTC on 07 Feb 2106 (int32_t wraps at 03:14:08 UTC on 19 Jan 2038).

	union {
		// In transient mode indicates sample number of the first payload sample in current seconds interval.
		uint32_t sampleNr;

		// In spectral mode indicates frequency band and slice (transform block of 1024 samples) of first payload sample.
		uint32_t bandsliceNr; // bandNr[9:0] and sliceNr[31:10].
		// Avoid bit fields, (portable) compilation support is messy. Instead use mask and shift to decode.
#define TBB_BAND_NR_MASK	((1 << 11) - 1) 
#define TBB_SLICE_NR_SHIFT	10
	};

	uint16_t nOfSamplesPerFrame; // Total number of samples in the frame payload
	uint16_t nOfFreqBands;	// Number of frequency bands for each spectrum in spectral mode. Is set to 0 for transient mode.

	uint8_t bandSel[64];	// Each bit in the band selector field indicates whether the band with the bit index is present in the spectrum or not.

	uint16_t spare;		// For future use. Set to 0.
	uint16_t crc16;		// CRC16 over frame header, with seqNr set to 0.


	std::string toString() const;
};

struct TBB_Payload {
	/*
	 * In transient mode, a sample is 12 bit. In spectral, 2*16 bit (real, imag).
	 * In the TBBs, transient samples are packed (2 samples per 3 bytes) with the checksum all the way at the end. This changes on transfer.
	 *
	 * TBB stores a frame in 2040 bytes (actually, 2048 with preamble and gaps). It sends a frame at a time, so derive our max from it.
	 */
#define MAX_TBB_DATA_SIZE		(2040 - sizeof(TBB_Header) - sizeof(uint32_t))	// 1948: TBB frame size without header and payload crc32.

#define MAX_TBB_TRANSIENT_NSAMPLES	(MAX_TBB_DATA_SIZE / 3 * 2)	// 1298 (.666: 1 byte padding when indeed 1298 samples would ever be stored in TBB)
#define MAX_TBB_SPECTRAL_NSAMPLES	(MAX_TBB_DATA_SIZE / (2 * sizeof(int16_t)))	// 487

	// Unpacked, sign-extended (for transient) samples without padding, i.e. as received.
	// Frames might not be full; the crc32 is always sent right after (no padding unlike as stored by TBB),
	// so we include it in 'data', but note that the crc32 is a little endian uint32_t, hence + 2. The crc32 is computed for transient data only.
	int16_t data[MAX_TBB_TRANSIENT_NSAMPLES + 2];				// 1300*2 bytes; Because only transient data is unpacked, use its max.

#define DEFAULT_TRANSIENT_NSAMPLES	1024						// From the spec and from real data.
#define DEFAULT_SPECTRAL_NSAMPLES	MAX_TBB_SPECTRAL_NSAMPLES	// The spec only states a max, so guess. Spectral mode has never been used or tested.
};

struct TBB_Frame {
	TBB_Header  header;
	TBB_Payload payload;
};

class TBB_Dipole {
private:
	DAL::TBB_DipoleDataset* itsDataset;
	std::ofstream itsRawOut; // if raw out requested
	std::vector<unsigned> itsFlagOffsets;

	ssize_t itsDatasetLen;

	uint32_t itsSampleFreq; // Hz
	uint32_t itsTime0; // seconds
	uint32_t itsSampleNr0; // for transient data only

	// Same truncated polynomials as standard crc32, but with initial_remainder=0, final_xor_value=0, reflected_input=false, reflected_remainder_output=false.
	// The boost::crc_optimal<> declarations precompute lookup tables, so do not declare inside the checking routine.
	boost::crc_optimal<32, 0x04C11DB7/*, 0, 0, false, false*/> itsCrc32gen; // instead of crc_32_type

	// do not use
	TBB_Dipole& operator=(const TBB_Dipole& rhs);

public:
	TBB_Dipole();
	TBB_Dipole(const TBB_Dipole& rhs); // do not use; only for TBB_Station vector<TBB_Dipole>(N) constr
	~TBB_Dipole();

	// Output threads
	bool isInitialized() const;
	bool usesExternalDataFile() const;

	// All TBB_Dipole objects are default constructed in a vector, so provide an init procedure.
	void initDipole(const TBB_Header& header, const Parset& parset, const std::string& rawFilename,
			DAL::TBB_Station& station, Mutex& h5Mutex);

	void processFrameData(const TBB_Frame& frame, Mutex& h5Mutex);

private:
	void initTBB_DipoleDataset(const TBB_Header& header, const Parset& parset, const std::string& rawFilename,
			DAL::TBB_Station& station, Mutex& h5Mutex);
	bool hasAllZeroDataSamples(const TBB_Frame& frame) const;
	bool crc32tbb(const TBB_Payload* payload, size_t nsamples);
};

class TBB_Station {
	DAL::TBB_File itsH5File;
	Mutex itsH5Mutex;
	DAL::TBB_Station itsStation;
	std::vector<TBB_Dipole> itsDipoles;
	const Parset& itsParset;
	const std::string itsH5Filename;
	const bool itsDumpRaw;

	std::string getRawFilename(unsigned rspID, unsigned rcuID);

	// do not use
	TBB_Station();
	TBB_Station(const TBB_Station& station);
	TBB_Station& operator=(const TBB_Station& rhs);

public:
	TBB_Station(const string& stationName, const Parset& parset, const std::string& h5Filename, bool dumpRaw);
	~TBB_Station();

	// Output threads
	void processPayload(const TBB_Frame& frame);

private:
	std::string getFileModDate(const std::string& filename) const;
	std::string utcTimeStr(double time) const;
	double toMJD(double time) const;

	void initCommonLofarAttributes(const std::string& filename);
	void initTBB_RootAttributesAndGroups(const std::string& stName);
	void initStationGroup(DAL::TBB_Station& st, const std::string& stName,
			const std::vector<double>& stPosition);
	void initTriggerGroup(DAL::TBB_Trigger& tg);
};

class TBB_Writer;

class TBB_StreamWriter {
	/*
	 * - The input thread receives incoming TBB frame headers, checks the header CRC, and puts them in a frameQueue.
	 * - The output thread checks the data CRC, creates an HDF5 file per station, creates groups and datasets,
	 *   writes the data, and returns empty frame pointers through the emptyQueue back to the input thread.
	 *
	 * On timeouts for all input threads, the main thread sends C++ thread cancellations. Input appends a NULL msg to notify output.
	 * This isolates (soft) real-time input from HDF5/disk latencies, and the HDF5 C library from C++ cancellation exceptions.
	 */

	// Two times max receive queue length (PRINT_QUEUE_LEN defined) as observed on locus node (July 2012).
	static const unsigned nrFrameBuffers = 1024;

	TBB_Frame* itsFrameBuffers;

	// Queue pointers point into itsFrameBuffers.
	Queue<TBB_Frame*> itsReceiveQueue; // input  thread -> output thread
	Queue<TBB_Frame*> itsFreeQueue;    // output thread -> input  thread

	TBB_Writer& itsWriter;
	const std::string& itsInputStreamName;
	const std::string& itsLogPrefix;

	// See TBB_Writer_main.cc::doTBB_Run() why this is used racily and thus tmp.
	// Inflate struct timeval to 64 bytes (typical LEVEL1_DCACHE_LINESIZE).
	struct timeval itsTimeoutStamp __attribute__((aligned(64)));

	boost::crc_optimal<16, 0x8005    /*, 0, 0, false, false*/> itsCrc16gen; // instead of crc_16_type

	Thread* itsOutputThread;
	Thread* itsInputThread; // last in TBB_StreamWriter

	// do not use
	TBB_StreamWriter();
	TBB_StreamWriter(const TBB_StreamWriter& rhs);
	TBB_StreamWriter& operator=(const TBB_StreamWriter& rhs);

public:
	TBB_StreamWriter(TBB_Writer& writer, const std::string& inputStreamName, const std::string& logPrefix);
	~TBB_StreamWriter();

	// Main thread
	time_t getTimeoutStampSec() const;

private:
	// Input threads
	void frameHeaderLittleToHost(TBB_Header& fh) const;
	void correctTransientSampleNr(TBB_Header& header) const;
	bool crc16tbb(const TBB_Header* header);
	void processHeader(TBB_Header& header, size_t recvPayloadSize);
	void mainInputLoop();

	// Output threads
	void mainOutputLoop();
};

class TBB_Writer {
	// Usually, we handle only 1 station, but we can handle multiple at a time.
	// map from stationID to a TBB_Station*
	std::map<unsigned, TBB_Station* > itsStations;
	Mutex itsStationsMutex;

	const Parset& itsParset;
	const std::string& itsOutDir;
	const bool itsDumpRaw;

	unsigned itsRunNr;

	std::vector<TBB_StreamWriter* > itsStreamWriters; // last in TBB_Writer

	// do not use
	TBB_Writer();
	TBB_Writer(const TBB_Writer& writer);
	TBB_Writer& operator=(const TBB_Writer& rhs);

public:
	TBB_Writer(const std::vector<std::string>& inputStreamNames, const Parset& parset,
			const std::string& outDir, bool dumpRaw, const std::string& logPrefix);
	~TBB_Writer();

	// Output threads
	TBB_Station* getStation(const TBB_Header& header);
	std::string createNewTBB_H5Filename(const TBB_Header& header, const std::string& stationName);

	// Main thread
	time_t getTimeoutStampSec(unsigned streamWriterNr) const;
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_STORAGE_TBB_WRITER_H

