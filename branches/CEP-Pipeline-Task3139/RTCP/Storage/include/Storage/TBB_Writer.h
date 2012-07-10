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
 * $Id: TBB_Writer.h 8495 2012-03-14 15:41:22Z amesfoort $
 */

#ifndef LOFAR_STORAGE_TBB_WRITER_H
#define LOFAR_STORAGE_TBB_WRITER_H 1

#include <sys/time.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <boost/array.hpp>

#include <Common/LofarTypes.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#ifndef USE_THREADS
#error The TBB writer needs USE_THREADS to operate.
#endif
#include <Common/Thread/Thread.h>
#include <Common/Thread/Queue.h>
#include <Common/Thread/Condition.h>

#include <Interface/SmartPtr.h>
#include <Interface/Parset.h>		// Writers use a specialized Parset class.

#include <dal/lofar/TBB_File.h>

namespace LOFAR {
namespace RTCP {

struct TBB_StationOut {
	// HDF5 file with one Station Group. Multiple output threads may create HDF5 groups and attributes, so protect with a mutex.
	DAL::TBB_File		h5Out;
	LOFAR::Mutex		h5OutMutex;

	const std::string	rawOutFilenameFmt;	// "L10000_CS001_D20120101T03:16:41.123Z_R%03u_R%03u_tbb.raw" (rsp, rcu)

	// The remaining members are set by the output thread when the first datagram from a (rsp, rcu) is seen.
	// Time and sample numbers of first sample in the first datagram.
	uint32_t			time0;		// sec
	uint32_t			sampleNr0;	// transient only

	// Only one thread deals with an (rsp, rcu) stream, so no mutex is needed for the common case of storing data.
	boost::array<std::ofstream, MAX_RSPBOARDS/*=per station*/ * NR_RCUS_PER_RSPBOARD> rawOuts; // 192 needed for int'l stations


	TBB_StationOut(const std::string& h5Filename, const std::string& rawOutFilenameFmt)
	: h5Out(DAL::TBB_File(h5Filename, DAL::TBB_File::CREATE)),
		rawOutFilenameFmt(rawOutFilenameFmt)
	{
	}

	inline std::ofstream& fileStream(unsigned rsp, unsigned rcu) {
		return rawOuts.at(rsp * NR_RCUS_PER_RSPBOARD + rcu);
	}
};

class TBB_Writer {
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
		uint8_t stationId;	// Data source station identifier
		uint8_t rspId;		// Data source RSP board identifier
		uint8_t rcuId;		// Data source RCU board identifier
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
#define BAND_NR_MASK	((1 << 11) - 1) 
#define SLICE_NR_SHIFT	10
		};

		uint16_t nOfSamplesPerFrame; // Total number of samples in the frame payload
		uint16_t nOfFreqBands;	// Number of frequency bands for each spectrum in spectral mode. Is set to 0 for transient mode.

		uint8_t bandSel[64];	// Each bit in the band selector field indicates whether the band with the bit index is present in the spectrum or not.

		uint16_t spare;		// For future use. Set to 0.
		uint16_t crc16;		// CRC16 over frame header, with seqNr set to 0.
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
		int16_t data[MAX_TBB_TRANSIENT_NSAMPLES + 2];			// 1300*2 bytes; Because only transient data is unpacked, use its max.

#define DEFAULT_TRANSIENT_NSAMPLES	1024						// From the spec and from real data.
#define DEFAULT_SPECTRAL_NSAMPLES	MAX_TBB_SPECTRAL_NSAMPLES	// The spec only states a max, so guess. Spectral mode has never been used or tested.
	};

	struct TBB_Frame {
		TBB_Header header;
		TBB_Payload payload;
	};

/*
 * - The input thread parses incoming TBB frame headers (header CRC, frame boundaries) and puts payloads in its frameQueue.
 * - The output thread CRCs payloads and writes the data to new, dipole-specific .raw file(s).
 *   The first payload for a dipole triggers creation of a new dipoleDataset. Access the HDF5 file under a mutex.
 *   There may be >1 stations sending to the same socket, one TBB_StationOut per station, all in the itsStationOutputs map.
 */
public:
	TBB_Writer(std::map<unsigned, LOFAR::RTCP::SmartPtr<LOFAR::RTCP::TBB_StationOut> >& stationOuts, const std::string& inputDescr,
		const LOFAR::RTCP::Parset& parset, struct timeval& timeoutStamp, const std::string& logPrefix);
	~TBB_Writer();

	void cancel();

private:
	static const size_t nrFrameBuffers = 50000; // 50000 is about 100 MB (from old DAL TBB Writer); bf writer uses 5 (don't know size)...

	// map from stationId to SmartPtr-ed TBB_StationOut
	std::map<unsigned, LOFAR::RTCP::SmartPtr<LOFAR::RTCP::TBB_StationOut> >& itsStationOutputs;
	const std::string			itsInputDescriptor;
	const LOFAR::RTCP::Parset&	itsParset;
	struct timeval&				itsTimeoutStamp;

	LOFAR::Queue<LOFAR::RTCP::SmartPtr<TBB_Frame> > itsFreeQueue;
	LOFAR::Queue<LOFAR::RTCP::SmartPtr<TBB_Frame> > itsReceiveQueue;
	SmartPtr<LOFAR::Thread>		itsOutputThread;
	SmartPtr<LOFAR::Thread>		itsInputThread; // last, so destruction blocks first on joining with the input thread


	// Input thread
	uint16_t byteSwap(uint16_t v);
	uint32_t byteSwap(uint32_t v);
	uint16_t littleNativeBSwap(uint16_t val);
	uint32_t littleNativeBSwap(uint32_t val);
	std::string stationIdToName(unsigned sid);
	std::string stationIdToName_alt(unsigned sid);
	void frameHeaderLittleNativeBSwap(TBB_Header& fh);
	void correctTransientSampleNr(TBB_Header& header);
	uint16_t crc16tbb(const uint16_t* buf, size_t len);
	void processHeader(TBB_Header& header, size_t recvPayloadSize);
	void mainInputLoop();

	// Output thread
	void setDipoleDataset(DAL::TBB_DipoleDataset& ds, const LOFAR::RTCP::Parset& parset,
						unsigned stationId, unsigned rspId, unsigned rcuId);
	uint32_t crc32tbb(const uint16_t* buf, size_t len);
	void processPayload(const TBB_Frame& frame);
	void mainOutputLoop();
};


} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_STORAGE_TBB_WRITER_H
