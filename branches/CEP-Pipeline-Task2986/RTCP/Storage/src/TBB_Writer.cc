//# TBB_Writer.cc: Write TBB data into a HDF5 file
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: TBB_Writer.cc 16951 2012-03-12 11:54:53Z amesfoort $

#include <lofar_config.h>

#include <Storage/TBB_Writer.h>

#include <iostream>
#include <iomanip>
#include <cerrno>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemCallException.h>
#include <Stream/SocketStream.h>
//#include <ApplCommon/AntField.h>

#include <Interface/Stream.h>

namespace LOFAR {
namespace RTCP {

using namespace std;

// TODO: shutdown of thread pairs that receive no data?
// TODO: detect and report data loss if we or the network could not keep up (reporting must be very light)
TBB_Writer::TBB_Writer(map<unsigned, SmartPtr<TBB_StationOut> >& stationOuts, const string& inputDescr,
					const Parset& parset, const string& logPrefix)
: itsStationOutputs(stationOuts),
  itsInputDescriptor(inputDescr),
  itsParset(parset) {

	itsOutputThread = new Thread(this, &TBB_Writer::mainOutputLoop, logPrefix + " OutputThread: ");
	try {
		itsInputThread = new Thread(this, &TBB_Writer::mainInputLoop, logPrefix + " InputThread: ");
	} catch (...) {
		itsOutputThread->cancel();
		throw;
	}
}

TBB_Writer::~TBB_Writer() {
}

#ifdef _DEBUG
string TBB_Writer::stationIdToName(unsigned sid) {
	// TODO: It would be better to read this from some config file (and use this, or a switch stmt version, as a fall-back).
	/* Derived from existing station numbers and names:
	001 - 103: CS + (id+0);
	104 - 120: RS + (id+0);
	121 - 121: CS + (id+80);
	122 - 140: RS + (id+80);
	141 - 142: CS + (id+160);
	143 - 160: RS + (id+160);
	161 - 161: CS + (id+240);
	162 - 180: RS + (id+240);
	181 - 181: CS + (id+320);
	182 - 200: RS + (id+320);
	201 - 205: DE + (id+400);
	206 - 206: FR + (id+400);
	207 - 207: SE + (id+400);
	208 - 208: UK + (id+400);
	209 - 209: FI + (id+400);
	???      : ST + stationId;
	*/
	const unsigned max_known_sid = 209;
	const char* type;
	unsigned stNameNr = (int)(sid - 101) < 0 ? 0 : sid - 101;
	stNameNr = sid + stNameNr / 20 * 80;

	if ( (sid >= 1 && sid <= 103) || sid == 121 || sid == 141 || sid == 142 || sid == 161 || sid == 181) {
		type = "CS"; // Core Station
	} else if (sid >= 104 && sid <= 200) {
		type = "RS"; // Remote Station
	} else if (sid >= 201 && sid <= max_known_sid) {
		static const char* EUStTypes[] = {"DE", "DE", "DE", "DE", "DE", "FR", "SE", "UK", "FI"};
		type = EUStTypes[sid - 201]; // EU Station
	} else {
		type = "ST"; // unknown STation
		stNameNr = sid;
	}

	ostringstream oss;
	oss << type << setw(3) << setfill('0') << stNameNr;
	return oss.str();
}

void TBB_Writer::printHeader(TBB_Header& h) { // debug; TODO: remove or disable
	cout << std::setw(3) << "StId=" << h.stationId << " (" << stationIdToName(h.stationId) << ") rspId=" << h.rspId << " rcuId=" << h.rcuId <<
		" samFreq=" << h.sampleFreq << "Hz seqNr=" << h.seqNr << " time=" << h.time << " #samples=" << h.nOfSamplesPerFrame << " #freqBands=" << h.nOfFreqBands << endl;
	if (h.nOfFreqBands == 0) { // transient data
		cout << "  transient: sampleNr=" << h.sampleNr << endl;
	} else { // spectral data
		cout << "  spectral: bandsliceNr=" << h.bandsliceNr << " (" << "..." << ")" << endl;
		// and bandSel
	}
	cout << "  spare=" << h.spare << " crc=" << h.crc16 << endl;
}
#endif // _DEBUG

uint16_t TBB_Writer::byteSwap(uint16_t v) {
	return __bswap_16(v); // GNU ext
}

uint32_t TBB_Writer::byteSwap(uint32_t v) {
	return __bswap_32(v); // GNU ext
}

#if __BYTE_ORDER == __BIG_ENDIAN
// hton[ls],ntoh[ls] on big endian are no-ops, so we have to provide the byte swaps.
uint16_t TBB_Writer::littleNativeBSwap(uint16_t val) {
	return byteSwap(val);
}
uint32_t TBB_Writer::littleNativeBSwap(uint32_t val) {
	return byteSwap(val);
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
uint16_t TBB_Writer::littleNativeBSwap(uint16_t val) {
	return val;
}
uint32_t TBB_Writer::littleNativeBSwap(uint32_t val) {
	return val;
}
#endif

void TBB_Writer::frameHeaderLittleNativeBSwap(TBB_Header& fh) {
	fh.seqNr = littleNativeBSwap(fh.seqNr); // should be zero or will be cleared to be sure, so useless
	fh.time = littleNativeBSwap(fh.time);
	fh.sampleNr = littleNativeBSwap(fh.sampleNr); // also swaps aliasing member fh.bandsliceNr
	fh.nOfSamplesPerFrame = littleNativeBSwap(fh.nOfSamplesPerFrame);
	fh.nOfFreqBands = littleNativeBSwap(fh.nOfFreqBands);
	fh.spare = littleNativeBSwap(fh.spare); // future use, convert anyway
	fh.crc16 = littleNativeBSwap(fh.crc16);
}

void TBB_Writer::correctTransientSampleNr(TBB_Header& header) {
	/*
	 * We assume header.sampleFreq is either 200 or 160 MHz (another multiple of #samples per frame is also fine).
	 * 
	 * At 200 MHz sample rate with 1024 samples per frame, we have 195213.5 frames per second.
	 * This means that every 2 seconds, a frame overlaps a seconds boundary; every odd frame needs its sampleNr corrected.
	 * At 160 MHz sample rate, an integer number of frames fits in a second (156250), so no correction is needed.
	 */
	if (header.sampleFreq == 200 && header.time & 1) { // time & 1: assumes first frame of obs carried an even time nr.
		header.sampleNr += 512; // in practice, it's 1024 samples/frame TODO?
	}
}

// TODO: doesn't generate the right cksum; ask Andre Gunst (gunst@astron.nl) who to talk to.
uint16_t TBB_Writer::crc16(const unsigned char* data, size_t size) {
	boost::crc_16_type crc16;
	crc16 = for_each(data, data + size, crc16);
cerr << "crc16=" << crc16() << endl; // debug
	return crc16();
}

// TODO: untested
uint32_t TBB_Writer::crc32(const unsigned char* data, size_t size) {
	boost::crc_32_type crc32;
	crc32 = for_each(data, data + size, crc32);
cerr << "crc32=" << crc32() << endl; // debug
	return crc32();
}

/*
 * Process the incoming TBB header.
 * Note that this function may update the header, but not its crc, so you cannot re-verify it.
 */
void TBB_Writer::processHeader(TBB_Header& header, size_t recvPayloadSize/*, uint16_t& lastNSamples*/) {
	frameHeaderLittleNativeBSwap(header); // no-op on little endian

	//uint32_t seqNrTmp = header.seqNr; // seqNr must be (set to) 0 for the header crc, so save it if needed later.
	header.seqNr = 0;
	//const size_t crcHeaderSize = sizeof(TBB_Header) - sizeof(header.crc16);
	//if (crc16(&header, crcHeaderSize) != littleNativeBSwap(header.crc16)) { // TODO: enable checksum and deal with it if check fails
		/*
		 * Spec says each frame has the same fixed length. Even though each frame may hold any numer of samples that fits,
		 * everything about the sampling hardware is regular, so the previous value is a good guess if the header crc fails.
		 */
		//header.nOfSamplesPerFrame = lastNSamples;
	//}
	//header.seqNr = seqNrTmp; // restore

	/*
	 * Act on crazy nOfsamplesPerFrame. We have checksummed, but never read outof bounds, because incoming data indicates so.
	 * Also correct sample numbers for transient data.
	 */
	if (header.nOfFreqBands == 0) { // transient data
		if (recvPayloadSize < sizeof(int16_t) + sizeof(uint32_t)) {
			throw Exception(""); // drop datagrams with payloads under 1 sample + crc32 TODO: subclass and handle it
		}
		uint16_t recvSamples = (recvPayloadSize - sizeof(uint32_t)) / sizeof(int16_t);
		header.nOfSamplesPerFrame = recvSamples; // likely already that amount; don't check, always assign.

		correctTransientSampleNr(header);
	} else { // spectral data (TODO: similar to transient, so merge once spectral is tested)
		if (recvPayloadSize < sizeof(int16_t)) {
			throw Exception(""); // idem as above, but no crc32 for spectral
		}
		uint16_t recvSamples = recvPayloadSize / (2 * sizeof(int16_t));
		header.nOfSamplesPerFrame = recvSamples; // idem as above
	}
}

void TBB_Writer::mainInputLoop() {
	LOG_INFO_STR("Creating connection from " << itsInputDescriptor << "...");
	Stream* inStream(createStream(itsInputDescriptor, true)); // true: server listening socket
	LOG_INFO_STR("Creating connection from " << itsInputDescriptor << ": done");

	//const unsigned maxTargetQueueSize = 100; // throttle if above this; TODO: throttle or something
	//unsigned qSize;

	TBB_Frame* frame = 0;
	//uint16_t lastNSamples = 0; // as a backup if header crc fails
	while (1) {
		try {
			frame = new TBB_Frame;
			size_t datagramSize = inStream->tryRead(frame, sizeof(*frame));
			if (datagramSize < sizeof(TBB_Header)) {
				throw Exception(""); // TODO: subclass and handle it 
			}
			TBB_Header* header = reinterpret_cast<TBB_Header*>(frame);
			processHeader(*header, datagramSize - sizeof(TBB_Header)/*, lastNSamples*/);

			// Throttle if the output thread cannot keep up. No, better recycle buffers, that'll throttle and get rid of the cross-thread mem mgmnt.
			/*qSize = itsFrameQueue.size(); // size() is a locked op, so slows down output thread (and us)
			if (qSize > maxTargetQueueSize) {
				wait((qSize - maxTargetQueueSize) * waitFactor);
			}*/

			// By appending, we transfer the ownership of frame to the output thread that will delete it.
			itsFrameQueue.append(frame);
		} catch (Stream::EndOfStreamException& eos) {
			break;
		} catch (bad_alloc& balloc) {
			// This could be treated sometimes: if queue is large (output thread cannot keep up), wait to see if it lowers. TODO: impl.

			// But if the output thread is malfunctioning, it is over.
			frame = 0; // safe delete below
			break;
		} catch (SystemCallException& sysExc) {
			// consider dealing with errno = EINTR
		
			//else
			break;
		}
	}
	
	delete frame;
	itsFrameQueue.append(0); // signal output thread to terminate
}

void TBB_Writer::setDipoleDataset(DAL::TBB_DipoleDataset& ds, const Parset& parset, unsigned stationId, unsigned rspId, unsigned rcuId) {
	ds.groupType().set("DipoleDataset");
	ds.stationID().set(stationId);
	ds.rspID().set(rspId);
	ds.rcuID().set(rcuId);

	ds.sampleFrequencyValue().set(parset.clockSpeed() / 1e6);
	ds.sampleFrequencyUnit().set("MHz");

/* TODO: These 4 can be filled in when receiving the frame
	ds.time().set(); // in seconds
	ds.sampleNumber().set(); // only relevant for transient mode
	ds.samplesPerFrame().set(); // optional, but set it from the first frame (what if data does not come in frames? hypothetical)
	ds.dataLength().set(); // init TODO: -> unsigned long?
*/

//	ds.nyquistZone().set(parset.nyquistZone()); // TODO: is an unsigned (1, 2 or 3) based on filter band. Is this clear? Shouldn't this be in station or global instead of in dipole dataset?
	//ds.ADC2Voltage().set(); // optional

	// Cable delays (optional) can be implemented from StaticMetaData, but there is no parsing code yet.
	//ds.cableDelay().set();
	//ds.cableDelayUnit().set('s');

	// Needed:
	//ds.dipoleCalibrationDelayValue().set(); // TODO: Pim can compute this from the GainCurve below
	//ds.dipoleCalibrationDelayUnit().set('s');
//	ds.dipoleCalibrationGainCurve().set(); // TODO: where to get this?

	//ds.feed().set(); // optional string

//for antenna, see MeasurementSetFormat.cc. Last resort, look at LCS/Common/ApplCommon/AntField.h , but be careful with HBA/LBA modes etc.
// must be in absolute ITRF (not relative to station)
/*
	ds.antennaPositionValue().set();
	ds.antennaPositionUnit().set(); // TODO: ???
	ds.antennaPositionFrame().set(parset.positionType()); // returns "ITRF" (currently)
	ds.antennaNormalVector().set();
	ds.antennaRotationMatrix().set(); // store 9 vals, per row. (row-minor)
*/
/* optional; but maybe fill these in anyway; tiles/HBA
	ds.tileBeamValue().set();
	ds.tileBeamUnit().set();
	ds.tileBeamFrame().set();
	ds.tileBeamDipoles().set();

	ds.tileCoefUnit().set();
	ds.tileBeamCoefs().set();

	ds.tileDipolePositionValue().set();
	ds.tileDipolePositionUnit().set();
	ds.tileDipolePositionFrame().set();

	ds.dispersionMeasureValue().set(parset.dispersionMeasure()); // passes default parms unsigned beam=0, unsigned pencil=0
	ds.dispersionMeasureUnit().set("s"); // TODO: check if unit is seconds before enabling
*/
}

void TBB_Writer::processPayload(const TBB_Frame& frame) {

	if (frame.header.nOfFreqBands == 0) { // transient data
		// Data crc32 over the unpacked, sign-extended samples (transient data only).
		/*size_t crcDataLen = frame.header.nOfSamplesPerFrame * 2 * sizeof(int16_t);
		if (crc32(&frame.payload.data, crcDataLen) != frame.payload.data[...]) { // TODO: check checksum incl if nSamples == 0; and on big endian
			// Note this crc data error, so we can add all data crc errors to the metadata later. (can this slow us down too much?)
			//badDataCRCs.push_back(DataCRC(idx, frame.payload.crc32));
			// store ranges only. Keeps it compact and compat with future flagging. (Also, log to syslog).
		}*/

	}/* else { // spectral data
		uint32_t bandNr  = frame.header.bandsliceNr & BAND_NR_MASK;
		uint32_t sliceNr = frame.header.bandsliceNr >> SLICE_NR_SHIFT;
	}*/

	map<unsigned, SmartPtr<TBB_StationOut> >::iterator stOutIt(itsStationOutputs.find(frame.header.stationId));
	if (stOutIt == itsStationOutputs.end()) {
		// No station found. Shouldn't happen. Save the data anyway, so we don't loose it, but do not populate the station group.
		// TODO
		LOG_WARN_STR("Unexpected station id in datagram; storing data in data set in empty station group.");
		return;
	}
	SmartPtr<TBB_StationOut> stOut = stOutIt->second;

	ofstream& rawOut(stOut->fileStream(frame.header.rspId, frame.header.rcuId));
	if (!rawOut.is_open()) {
		// Create raw data file and a new DipoleDataset referencing the file.
		string outFilenameRaw(formatString(stOut->itsRawOutFilenameFmt.c_str(), frame.header.rspId, frame.header.rcuId));
		rawOut.open(outFilenameRaw.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if (!rawOut) {
			// TODO: ehhh
			LOG_WARN_STR("Unable to create raw data file; losing data.");
			return;
		}
		/*
		 * Store the base time and sampleNr from the 1st datagram to compute storage offsets.
		 * Note that this assumes that we received the 1st datagram first. (But if not, we can deal with it.)
		 */
		stOut->time0     = frame.header.time;
		stOut->sampleNr0 = frame.header.sampleNr;

		DAL::TBB_Station station(stOut->h5Out.station(frame.header.stationId));
		DAL::TBB_DipoleDataset ds(station.dipole(frame.header.stationId, frame.header.rspId, frame.header.rcuId));
		{
			ScopedLock(stOut->h5OutMutex); // Only 1 output thread may add a HDF5 DipoleDataset to a station group at a time.

			// DipoleDataset shouldn't exist, but check anyway. If it does exist, there is something wrong..., but try to store anyway.
			if (!ds.exists()) {
				// Create 1-dim, unbounded (-1) dataset. We could also resize() it continuously, but now HDF5 just looks at the raw file size.
				// Override endianess. TBB data is always stored little endian and also received as such, so written as-is on any platform.
				vector<ssize_t> dsDims(1, -1);
				ds.create(dsDims, dsDims, outFilenameRaw, ds.LITTLE); // dims, maxDims, ...
				setDipoleDataset(ds, itsParset, frame.header.stationId, frame.header.rspId, frame.header.rcuId);
				// TODO; consider storing time0 and sampleNr0 (and nSamples) per dipole dataset
			}

			stOut->h5Out.flush();
		}
		// TODO: we might want to flush the "previous" raw stream here
	}

	/*
	 * Store the data samples at the offset indicated in the datagram. (Malicious datagrams can still cause totally odd I/O offsets and sizes, or exc.)
	 * Always store incoming little endian TBB data without interpretation or conversion, regardless of platform.
	 */
	off_t offset = ( (frame.header.time - stOut->time0) * (size_t)frame.header.sampleFreq * 1000000
			+ frame.header.sampleNr - stOut->sampleNr0 ) * sizeof(frame.payload.data[0]);
	if (offset >= 0) {
		rawOut.seekp(offset); // if the data arrives in order, this doesn't change anything
		rawOut.write(reinterpret_cast<const char*>(frame.payload.data), frame.header.nOfSamplesPerFrame * sizeof(frame.payload.data[0]));
	} else { // Very unlikely: We didn't receive the 1st datagram first (or malicious datagram).
		// Store anyway, but don't reorder now, there may arrive more. Create a tmp DipoleDataset if not exists and store at -offset.
		//TODO; under mutex!
	}

	//storedRanges.add(frame.header.sampleNr, frame.header.nOfSamplesPerFrame); // TODO: reverse set is for flagging
}

void TBB_Writer::mainOutputLoop() {
	TBB_Frame* frame;
	while ((frame = itsFrameQueue.remove()) != 0) {
// TODO: try/catch
		processPayload(*frame);
		delete frame;
	}

}

} // namespace RTCP
} // namespace LOFAR
