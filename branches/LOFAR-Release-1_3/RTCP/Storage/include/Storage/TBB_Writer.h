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

#include <sys/time.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#ifndef USE_THREADS
#error The TBB writer needs USE_THREADS to operate.
#endif
#include <Common/Thread/Thread.h>
#include <Common/Thread/Queue.h>

#include <Interface/SmartPtr.h>
#include <Interface/Parset.h>		// Writers use a specialized Parset class.

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

	inline std::string toString() {
		return std::string(stationID) << " " << rspID << " " << rcuID << " " << sampleFreq << " " << seqNr << " " << time << " " <<
				sampleNr << " " << nOfSamplesPerFrame << " " << nOfFreqBands << " " /*<< bandSel << " "*/ << spare << " " << crc16;
	}
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
	// so we include it in 'data', but note that the crc32 is an uint32_t (hence '+ 2'). The crc32 is computed for transient data only.
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

	// do not use:
	TBB_Dipole(const TBB_Dipole& rhs);
	TBB_Dipole& operator=(const TBB_Dipole& rhs);

public:
	TBB_Dipole();
	~TBB_Dipole();

	bool isInitialized();
	bool usesExternalDataFile();

	// All TBB_Dipole objects are default constructed in a vector, so provide init().
	void initDipole(const TBB_Header& header, const std::string& rawFilename,
			DAL::TBB_Station& station, Mutex& h5Mutex);

	void writeFrameData(const TBB_Frame& frame, Mutex& h5Mutex);

private:
	void initTBB_DipoleDataset(const TBB_Header& header, const std::string& rawFilename,
			DAL::TBB_Station& station, Mutex& h5Mutex);
};

class TBB_Station {
	DAL::TBB_File itsH5File;
	Mutex itsH5Mutex;
	DAL::TBB_Station itsStation;
	std::vector<TBB_Dipole> itsDipoles;
	const Parset& itsParset;
	const std::string& h5Filename;

	// do not use:
	TBB_Station();
	TBB_Station(const TBB_Station& station);
	TBB_Station& operator=(const TBB_Station& rhs);

public:
	TBB_Station(unsigned stationID, const Parset& parset, const std::string& h5Filename);
	~TBB_Station();

	// Returns a writable dipole reference. If not found, insert a new TBB_Dipole (creates a Dipole dataset in the HDF5 file).
	TBB_Dipole& getDipole(const TBB_Header& header);

private:
	std::string getFileModDate(const std::string& filename);
	std::string timeStr(double time);
	double toMJD(double time);

	void initCommonLofarAttributes(const std::string& filename);
	void initTBB_RootAttributesAndGroups();
	void initStationGroup();
	void initTriggerGroup();
};

class TBB_Writer {
	// Usually, we handle only 1 station, but we can handle multiple at a time.
	// map from stationID to SmartPtr-ed TBB_Station
	std::map<unsigned, SmartPtr<TBB_Station> > itsStations;
	Mutex itsStationsMutex;

	const Parset& itsParset;
	const std::string& itsOutDir;
	const bool itsDumpRaw;

	unsigned itsRunNr;


	class TBB_StreamWriter {
		/*
		 * - The input thread receives incoming TBB frame headers, checks the header CRC, and puts them in a frameQueue.
		 * - The output thread checks the data CRC, creates an HDF5 file per station, creates groups and datasets,
		 *   writes the data, and returns an empty frame through the emptyQueue to the input thread.
		 */

		// Two times max receive queue length (PRINT_QUEUE_LEN defined) as observed on locus node (July 2012).
		static const unsigned nrFrameBuffers = 1024;

		TBB_Frame* itsFrameBuffers;

		// Queue pointers point into itsFrameBuffers.
		Queue<TBB_Frame*> itsReceiveQueue; // input  thread -> output thread
		Queue<TBB_Frame*> itsFreeQueue;    // output thread -> input  thread

		const std::string& itsInputStreamName;
		const std::string& itsLogPrefix;

		// See TBB_Writer_main.cc::doTBB_Run() why this is used racily and thus tmp.
		// Inflate struct timeval to 64 bytes (typical LEVEL1_DCACHE_LINESIZE).
		struct timeval itsTimeoutStamp __attribute__((aligned(64)));

		Thread* itsOutputThread;
		Thread* itsInputThread; // last in TBB_StreamWriter and thus in TBB_Writer

		// do not use:
		TBB_StreamWriter();
		TBB_StreamWriter(const TBB_StreamWriter& rhs);
		TBB_StreamWriter& operator=(const TBB_StreamWriter& rhs);

	public:
		TBB_StreamWriter(const std::string& inputStreamName, const std::string& logPrefix);
		~TBB_StreamWriter();

		time_t getTimeoutStampSec();

	private:
		// Input thread
		uint16_t littleNativeBSwap(uint16_t val);
		uint32_t littleNativeBSwap(uint32_t val);
		void frameHeaderLittleNativeBSwap(TBB_Header& fh);
		void correctTransientSampleNr(TBB_Header& header);
		uint16_t crc16tbb(const uint16_t* buf, size_t len);
		void processHeader(TBB_Header& header, size_t recvPayloadSize);
		void mainInputLoop();

		// Output thread
		uint32_t crc32tbb(const uint16_t* buf, size_t len);
		std::string createNewTBB_H5Filename(const TBB_header& header);
		SmartPtr<TBB_Station> getStation(unsigned stationID, const TBB_header& header);
		void processPayload(const TBB_Frame& frame);
		void mainOutputLoop();
	};
	std::vector<SmartPtr<TBB_StreamWriter> > itsStreamWriters; // last in TBB_Writer


	// do not use:
	TBB_Writer();
	TBB_Writer(const TBB_Writer& writer);
	TBB_Writer& operator=(const TBB_Writer& rhs);

public:
	TBB_Writer(const std::vector<std::string>& inputStreamNames, const Parset& parset,
			const std::string& outDir, bool dumpRaw, const std::string& logPrefix);
	~TBB_Writer();

	time_t getTimeoutStampSec(unsigned streamWriterNr);
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_STORAGE_TBB_WRITER_H
