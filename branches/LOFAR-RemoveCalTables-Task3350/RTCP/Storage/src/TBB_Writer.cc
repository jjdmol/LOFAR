/* TBB_Writer.cc: Write TBB data into a HDF5 file
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
 * $Id: TBB_Writer.cc 16951 2012-03-12 11:54:53Z amesfoort $
 */

#include <lofar_config.h>

#include <Storage/TBB_Writer.h>

#include <climits>

#include <iostream>
#include <iomanip>
#include <cerrno>

#include <boost/crc.hpp>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Stream/SocketStream.h>
//#include <ApplCommon/AntField.h>

#include <Interface/Stream.h>

namespace LOFAR {
namespace RTCP {

using namespace std;

// TODO: shutdown of thread pairs that receive no data?
// TODO: detect and report data loss if we or the network could not keep up (reporting must be very light)
TBB_Writer::TBB_Writer(map<unsigned, SmartPtr<TBB_StationOut> >& stationOuts, const string& inputDescr,
					const Parset& parset, struct timeval& timeoutStamp, const string& logPrefix)
: itsStationOutputs(stationOuts),
  itsInputDescriptor(inputDescr),
  itsParset(parset),
  itsTimeoutStamp(timeoutStamp) {

	for (size_t i = 0; i < nrFrameBuffers; i++) {
		itsFreeQueue.append(new TBB_Frame);
	}

	itsOutputThread = new Thread(this, &TBB_Writer::mainOutputLoop, logPrefix + " OutputThread: ");
	try {
		itsInputThread = new Thread(this, &TBB_Writer::mainInputLoop, logPrefix + " InputThread: ");
	} catch (...) {
		itsOutputThread->cancel();
		throw;
	}
}

TBB_Writer::~TBB_Writer() {
	cancel();
}

void TBB_Writer::cancel() {
	/*
	 * Only cancel input thread who will signal the output thread
	 * to stop after processing already forwarded data.
	 */
	itsInputThread->cancel();
}

string TBB_Writer::stationIdToName(unsigned sid) {
	string name;

	switch (sid) {
		case   1: name = "CS001"; break;
		case   2: name = "CS002"; break;
		case   3: name = "CS003"; break;
		case   4: name = "CS004"; break;
		case   5: name = "CS005"; break;
		case   6: name = "CS006"; break;
		case   7: name = "CS007"; break;
		case   8: name = "CS008"; break;
		case   9: name = "CS009"; break;
		case  10: name = "CS010"; break;
		case  11: name = "CS011"; break;
		case  12: name = "CS012"; break;
		case  13: name = "CS013"; break;
		case  14: name = "CS014"; break;
		case  15: name = "CS015"; break;
		case  16: name = "CS016"; break;
		case  17: name = "CS017"; break;
		case  18: name = "CS018"; break;
		case  19: name = "CS019"; break;
		case  20: name = "CS020"; break;
		case  21: name = "CS021"; break;
		case  22: name = "CS022"; break;
		case  23: name = "CS023"; break;
		case  24: name = "CS024"; break;
		case  25: name = "CS025"; break;
		case  26: name = "CS026"; break;
		case  27: name = "CS027"; break;
		case  28: name = "CS028"; break;
		case  29: name = "CS029"; break;
		case  30: name = "CS030"; break;
		case  31: name = "CS031"; break;
		case  32: name = "CS032"; break;

		case 101: name = "CS101"; break;
		case 102: name = "CS102"; break;
		case 103: name = "CS103"; break;

		case 104: name = "RS104"; break;
		case 105: name = "RS105"; break;
		case 106: name = "RS106"; break;
		case 107: name = "RS107"; break;
		case 108: name = "RS108"; break;
		case 109: name = "RS109"; break;

		case 121: name = "CS201"; break;

		case 122: name = "RS202"; break;
		case 123: name = "RS203"; break;
		case 124: name = "RS204"; break;
		case 125: name = "RS205"; break;
		case 126: name = "RS206"; break;
		case 127: name = "RS207"; break;
		case 128: name = "RS208"; break;
		case 129: name = "RS209"; break;
		case 130: name = "RS210"; break;

		case 141: name = "CS301"; break;
		case 142: name = "CS302"; break;

		case 143: name = "RS303"; break;
		case 144: name = "RS304"; break;
		case 145: name = "RS305"; break;
		case 146: name = "RS306"; break;
		case 147: name = "RS307"; break;
		case 148: name = "RS308"; break;
		case 149: name = "RS309"; break;
		case 150: name = "RS310"; break;

		case 161: name = "CS401"; break;

		case 162: name = "RS402"; break;
		case 163: name = "RS403"; break;
		case 164: name = "RS404"; break;
		case 165: name = "RS405"; break;
		case 166: name = "RS406"; break;
		case 167: name = "RS407"; break;
		case 168: name = "RS408"; break;
		case 169: name = "RS409"; break;
		case 170: name = "RS410"; break;
		case 171: name = "RS411"; break;
		case 172: name = "RS412"; break;
		case 173: name = "RS413"; break;

		case 181: name = "CS501"; break;

		case 182: name = "RS502"; break;
		case 183: name = "RS503"; break;
		case 184: name = "RS504"; break;
		case 185: name = "RS505"; break;
		case 186: name = "RS506"; break;
		case 187: name = "RS507"; break;
		case 188: name = "RS508"; break;
		case 189: name = "RS509"; break;

		case 201: name = "DE601"; break;
		case 202: name = "DE602"; break;
		case 203: name = "DE603"; break;
		case 204: name = "DE604"; break;
		case 205: name = "DE605"; break;
		case 206: name = "FR606"; break;
		case 207: name = "SE607"; break;
		case 208: name = "UK608"; break;
		case 209: name = "FI609"; break;

		// Unknown station number, try the magic alt function. If it doesn't know either, use ST<sid> as a placeholder.
		default:  name = stationIdToName_alt(sid);
   }

   return name;
}

// This conversion routine is more complicated, but more likely to already support future NL stations.
string TBB_Writer::stationIdToName_alt(unsigned sid) {
	/* "Relation" derived from existing station numbers and names:
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
	//fh.seqNr              = littleNativeBSwap(fh.seqNr); // neither intended nor useful for us
	fh.time               = littleNativeBSwap(fh.time);
	fh.sampleNr           = littleNativeBSwap(fh.sampleNr); // also swaps aliasing union co-member fh.bandsliceNr
	fh.nOfSamplesPerFrame = littleNativeBSwap(fh.nOfSamplesPerFrame);
	fh.nOfFreqBands       = littleNativeBSwap(fh.nOfFreqBands);
	//fh.spare              = littleNativeBSwap(fh.spare); // currently unused
	fh.crc16              = littleNativeBSwap(fh.crc16);
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

/*
 * This code is based on the Python ref/test code from Gijs Schoonderbeek. It does not do a std crc16 (AFAICS).
 * It assumes that the seqNr field (buf[1]) has been zeroed.
 * Do not call this function with len < 1; reject the frame earlier.
 */
uint16_t TBB_Writer::crc16tbb(const uint16_t* buf, size_t len) {
	uint16_t CRC            = 0;
	const uint32_t CRC_poly = 0x18005;
	const uint16_t bits     = 16;
	uint32_t data           = 0;
	const uint32_t CRCDIV   = (CRC_poly & 0x7fffffff) << 15;

	data = (buf[0] & 0x7fffffff) << 16;
	for (uint32_t i = 1; i < len; i++) {
		data += buf[i];
		for (uint16_t j = 0; j < bits; j++) {
			if ((data & 0x80000000) != 0) {
				data = data ^ CRCDIV;
			}
			data = data & 0x7fffffff;
			data = data << 1;
		}
	}
	CRC = data >> 16;
	return CRC;
}

/*
 * This code is based on the Python ref/test code from Gijs Schoonderbeek. It does not do a std crc32 (AFAICS).
 * It computes a 32 bit result, but the buf arg is of uint16_t*.
 * Do not call this function with len < 2; reject the frame earlier.
 */
uint32_t TBB_Writer::crc32tbb(const uint16_t* buf, size_t len) {
	uint32_t CRC            = 0;
	const uint64_t CRC_poly = 0x104C11DB7ULL;
	const uint16_t bits     = 16;
	uint64_t data           = 0;
	const uint64_t CRCDIV   = (CRC_poly & 0x7fffffffffffULL) << 15;

	// Added '& 0x0fff' to mask sign-extended unpacking of the samples. (vs Gijs' Python code)
	data = buf[0] & 0x0fff;
	data = data & 0x7fffffffffffULL;
	data = data << 16;
	data = data + (buf[1] & 0x0fff);
	data = data & 0x7fffffffffffULL;
	data = data << 16;
	uint32_t i = 2;
	for ( ; i < len-2; i++) {
		data = data + (buf[i] & 0x0fff);
		for (uint32_t j = 0; j < bits; j++) {
			if (data & 0x800000000000ULL) {
				data = data ^ CRCDIV;
			}
			data = data & 0x7fffffffffffULL;
			data = data << 1;
		}
	}

	// Do the 32 bit checksum separately, without the '& 0xfff' masking.
	// Process the two 16 bit halves in reverse order (no endian swap!), but keep the i < len cond.
	// TODO: verify that this swapping is needed/different on big endian
	for (buf += 1; i < len; i++, buf -= 2) {
		data = data + buf[i];
		for (uint32_t j = 0; j < bits; j++) {
			if (data & 0x800000000000ULL) {
				data = data ^ CRCDIV;
			}
			data = data & 0x7fffffffffffULL;
			data = data << 1;
		}
	}

	CRC = (uint32_t)(data >> 16);
	return CRC;
}

/*
 * Process the incoming TBB header.
 * Note that this function may update the header, but not its crc, so you cannot re-verify it.
 */
void TBB_Writer::processHeader(TBB_Header& header, size_t recvPayloadSize/*, uint16_t& lastNSamples*/) {
	frameHeaderLittleNativeBSwap(header); // no-op on little endian

	//uint32_t seqNrTmp = header.seqNr; // seqNr must be 0 for the header crc; don't need seqNr later.
	header.seqNr = 0;
	uint16_t csum = crc16tbb(reinterpret_cast<uint16_t*>(&header), sizeof(header) / sizeof(uint16_t));
	if (csum != 0) {
		/*
		 * Spec says each frame has the same fixed length. Even though each frame may hold any numer of samples that fits,
		 * everything about the sampling hardware is regular, so the previous value is a good guess if the header crc fails.
		 */
		//header.nOfSamplesPerFrame = lastNSamples; // TODO

		cerr << "Header checksum failed of frame with time=" << header.time << " sampleNr=" << header.sampleNr <<
				" crc16=" << header.crc16 << endl;
	}
	//header.seqNr = seqNrTmp; // restore

	if (header.nOfFreqBands == 0) { // transient data
		// Use received size instead of received nOfSamplesPerFrame header field to access data.
		if (recvPayloadSize < sizeof(int16_t) + sizeof(uint32_t)) {
			// Drop bad frame; also, the data crc routine cannot handle this.
			throw Exception("dropping bad tbb transient frame"); // TODO: sub-class exc.
		}
		uint16_t recvSamples = (recvPayloadSize - sizeof(uint32_t)) / sizeof(int16_t);
		header.nOfSamplesPerFrame = recvSamples; // most likely already 1024

		correctTransientSampleNr(header);
	} else { // spectral data
		if (recvPayloadSize < sizeof(int16_t)) {
			throw Exception("dropping bad tbb spectral frame");
		}
		uint16_t recvSamples = recvPayloadSize / (2 * sizeof(int16_t));
		header.nOfSamplesPerFrame = recvSamples;
	}
}

void TBB_Writer::mainInputLoop() {
	//LOG_INFO_STR("Reading incoming data from " << itsInputDescriptor);
	LOG_INFO_STR(string("Reading incoming data from ") << itsInputDescriptor);
	Stream* inStream(createStream(itsInputDescriptor, true)); // true: server listening socket

	while (1) { // we use thread cancellation
		try {
			SmartPtr<TBB_Frame> frame(itsFreeQueue.remove());
			size_t datagramSize = inStream->tryRead(frame, sizeof(*frame));
//			LOG_INFO_STR("Received datagram");
			if (datagramSize < sizeof(TBB_Header)) {
				throw Exception("dropping too small tbb frame"); // TODO: subclass and handle it: Dropped frame smaller than header size
			}

			gettimeofday(&itsTimeoutStamp, NULL); // notifies master that we are still busy.

			TBB_Header* header = &frame->header;
			processHeader(*header, datagramSize - sizeof(TBB_Header));

			// Throttle if the output thread cannot keep up. No, better recycle buffers, that'll throttle and get rid of the cross-thread mem mgmnt.
			/*qSize = itsReceiveQueue.size(); // size() is a locked op, so slows down output thread (and us)
			if (qSize > maxTargetQueueSize) {
				wait((qSize - maxTargetQueueSize) * waitFactor);
			}*/

			// By appending, we transfer the ownership of frame to the output thread that will delete it.
			itsReceiveQueue.append(frame.release());
		} catch (Stream::EndOfStreamException& eos) {
			LOG_INFO_STR("EndOfStreamException");
			break;
		} catch (bad_alloc& balloc) {
			// This could be treated sometimes: if queue is large (output thread cannot keep up), wait to see if it lowers. TODO: impl.

			// But if the output thread is malfunctioning, it is over.
			LOG_INFO_STR("bad_alloc exception");
			break;
		} catch (SystemCallException& sysExc) {
			LOG_INFO_STR(sysExc.text());
			if (sysExc.error != EINTR) { // EINTR on Linux but other OS may also restart the read, even if not SA_RESTART.
				break;
			}

		} catch (...) {
			// Thread.h already logs on a Cancellation exc.
			delete inStream;
			itsReceiveQueue.append(0); // no more data; hope this doesn't throw...
			throw; // Cancellation exceptions MUST be rethrown. Any other we cannot handle, so rethrow too.
		}
	}

	delete inStream;
	itsReceiveQueue.append(0); // no more data
}

void TBB_Writer::setDipoleDataset(DAL::TBB_DipoleDataset& ds, const Parset& parset, unsigned stationId, unsigned rspId, unsigned rcuId) {
	ds.groupType().value = "DipoleDataset";
	ds.stationID().value = stationId;
	ds.rspID().value = rspId;
	ds.rcuID().value = rcuId;

	ds.sampleFrequencyValue().value = parset.clockSpeed() / 1e6;
	ds.sampleFrequencyUnit().value = "MHz";

/* TODO: These 4 can be filled in when receiving the frame
	ds.time().value = ???; // in seconds
	ds.sampleNumber().value = ???; // only relevant for transient mode
	ds.samplesPerFrame().value = ???;
	ds.dataLength().value = ???; // init TODO: -> unsigned long?
*/

//	ds.nyquistZone().value = parset.nyquistZone(); // TODO: is an unsigned (1, 2 or 3) based on filter band.
	//ds.ADC2Voltage().value = ???; // optional

	// Cable delays (optional) can be implemented from StaticMetaData, but there is no parsing code yet.
	//ds.cableDelay().value = ???;
	//ds.cableDelayUnit().value = 's';

	// Needed:
	//ds.dipoleCalibrationDelayValue().value = ???; // TODO: Pim can compute this from the GainCurve below
	//ds.dipoleCalibrationDelayUnit().value = 's';
//	ds.dipoleCalibrationGainCurve().value = ???; // TODO: where to get this?

	//ds.feed().set(); // optional string

//for antenna, see MeasurementSetFormat.cc. Last resort, look at LCS/Common/ApplCommon/AntField.h , but be careful with HBA/LBA modes etc.
// must be in absolute ITRF (not relative to station)
/*
	ds.antennaPositionValue().value = ???;
	ds.antennaPositionUnit().value = ???; // TODO: ???
	ds.antennaPositionFrame().value = parset.positionType(); // returns "ITRF" (currently)
	ds.antennaNormalVector().value = ???;
	ds.antennaRotationMatrix().value = ???; // store 9 vals, per row. (row-minor)
*/
/* optional; but maybe fill these in anyway; tiles/HBA
	ds.tileBeamValue().value = ???;
	ds.tileBeamUnit().value = ???;
	ds.tileBeamFrame().value = ???;
	ds.tileBeamDipoles().value = ???;

	ds.tileCoefUnit().value = ???;
	ds.tileBeamCoefs().value = ???;

	ds.tileDipolePositionValue().value = ???;
	ds.tileDipolePositionUnit().value = ???;
	ds.tileDipolePositionFrame().value = ???;

	ds.dispersionMeasureValue().value = parset.dispersionMeasure(); // passes default parms unsigned beam=0, unsigned pencil=0
	ds.dispersionMeasureUnit().value = "s"; // TODO: check if unit is seconds before enabling
*/
}

void TBB_Writer::processPayload(const TBB_Frame& frame) {
	if (frame.header.nOfFreqBands == 0) { // transient data
		uint32_t csum = crc32tbb(reinterpret_cast<const uint16_t*>(frame.payload.data), frame.header.nOfSamplesPerFrame + 2); // +2: the crc32 in terms of 16 bit data vals
		if (csum != 0) {
			// TODO: Keep track of "broken" data (flag it), but still store it.
			//badDataCRCs.push_back(DataCRC(idx, frame.payload.crc32));
			static unsigned nseen = 0;

			if (nseen++ < 4) // racy, but tmp
			cerr << "Data checksum failed of frame with time=" << frame.header.time << " sampleNr=" << frame.header.sampleNr <<
					" crc32=" << *reinterpret_cast<const uint32_t*>(&frame.payload.data[frame.header.nOfSamplesPerFrame]) << endl;
		}
	} else { // spectral data
		//uint32_t bandNr  = frame.header.bandsliceNr & BAND_NR_MASK;
		//uint32_t sliceNr = frame.header.bandsliceNr >> SLICE_NR_SHIFT;
		// TODO: prepare to store, but spectral output format unclear.
	}

	map<unsigned, SmartPtr<TBB_StationOut> >::iterator stOutIt(itsStationOutputs.find(frame.header.stationId));
	if (stOutIt == itsStationOutputs.end()) {
		// No station found. Shouldn't happen. Save the data anyway, so we don't lose it, but do not populate the station group.
		// TODO: fix this for sure; any static conf mistake triggers this
		LOG_WARN_STR("Unexpected station id in datagram; storing data in data set in empty station group.");
		printf("StationId: %hhu\n", frame.header.stationId);
		return;
	}
	TBB_StationOut* stOut(stOutIt->second.get());

	ofstream& rawOut(stOut->fileStream(frame.header.rspId, frame.header.rcuId));
	if (!rawOut.is_open()) { // TODO: can cause too many files to be opened?
		// Create raw data file and a new DipoleDataset referencing the file.
		string outFilenameRaw(formatString(stOut->rawOutFilenameFmt.c_str(), frame.header.rspId, frame.header.rcuId));
		rawOut.open(outFilenameRaw.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if (!rawOut) {
			// TODO: ehhh
			LOG_WARN_STR("Unable to create raw data file; losing data.");
			return;
		}
		/*
		 * Store the base time and sampleNr from the first datagram to compute storage offsets.
		 * Note that this assumes that we received the first datagram first. If not, we compute a negative offset and store tmp to restore later.
		 */
		stOut->time0     = frame.header.time;
		stOut->sampleNr0 = frame.header.sampleNr;

		string stationName(stationIdToName(frame.header.stationId));
		DAL::TBB_Station station(stOut->h5Out.station(stationName));
		string dipoleName(formatString("%03hhu%03hhu%03hhu", frame.header.stationId, frame.header.rspId, frame.header.rcuId));
		DAL::TBB_DipoleDataset ds(station.dipole(dipoleName));
		{
			ScopedLock h5OutLock(stOut->h5OutMutex); // Only 1 output thread may add a HDF5 DipoleDataset to a station group at a time.

			// DipoleDataset shouldn't exist, but check anyway. If it does exist, there is something wrong..., but try to store anyway.
			if (!ds.exists()) {
				// Create 1-dim, unbounded (-1) dataset. 
//We could also resize() it continuously, but now HDF5 just looks at the raw file size.
				// Override endianess. TBB data is always stored little endian and also received as such, so written as-is on any platform.
				vector<ssize_t> dsDims(1, 0);
				vector<ssize_t> dsMaxDims(1, -1);
				ds.create(dsDims, dsMaxDims, outFilenameRaw, ds.LITTLE);
				setDipoleDataset(ds, itsParset, frame.header.stationId, frame.header.rspId, frame.header.rcuId);
				// TODO; consider storing time0 and sampleNr0 (and nSamples) per dipole dataset

				stOut->h5Out.flush();
			}

		}
		// TODO: we might want to flush the "previous" raw stream here
	}

	/*
	 * Store the data samples at the offset indicated in the datagram. Go to extreme length not to drop data.
	 * Always store incoming little endian TBB data without interpretation or conversion, regardless of platform.
	 */
	off_t offset = ( (frame.header.time - stOut->time0) * (size_t)frame.header.sampleFreq * 1000000
			+ frame.header.sampleNr - stOut->sampleNr0 ) * sizeof(frame.payload.data[0]);
	if (offset >= 0) {
		rawOut.seekp(offset); // If the data arrives in order, this doesn't change anything.
		rawOut.write(reinterpret_cast<const char*>(frame.payload.data), frame.header.nOfSamplesPerFrame * sizeof(frame.payload.data[0]));

		// Update the .h5 file. As long as the last frame per stream arrives last, this works out. (Else, no data is lost.)
		vector<ssize_t> dsNewDims(1, offset / sizeof(frame.payload.data[0]) + frame.header.nOfSamplesPerFrame);
		
		string stationName(stationIdToName(frame.header.stationId)); // TODO: dupl code
		DAL::TBB_Station station(stOut->h5Out.station(stationName));
		string dipoleName(formatString("%03hhu%03hhu%03hhu", frame.header.stationId, frame.header.rspId, frame.header.rcuId));
		DAL::TBB_DipoleDataset ds(station.dipole(dipoleName));

		ScopedLock h5OutLock(stOut->h5OutMutex); // TODO: slow?, rework
		ds.resize(dsNewDims);
	}/* else { // Very unlikely: We didn't receive the 1st datagram first (or malicious datagram).
		LOG_WARN_STR("Negative offset; storing data at -offset in extra, empty dataset.");
		const off_t nonNegatableInt = sizeof(off_t) == 8 ? LONG_MIN : INT_MIN;
		if (offset != nonNegatableInt) {
			offset = -offset;
		} else { // very unlikely * very unlikely...
			offset = 0;
		}
		// Store anyway, but don't reorder now, there may be more to come. Create a tmp DipoleDataset if not exists and store at -offset.
		//TODO; under mutex! can we compute offset earlier?
	}*/

	//storedRanges.add(frame.header.sampleNr, frame.header.nOfSamplesPerFrame); // TODO: reverse set is for flagging
}

void TBB_Writer::mainOutputLoop() {
	for (SmartPtr<TBB_Frame> frame; (frame = itsReceiveQueue.remove()) != 0; itsFreeQueue.append(frame.release())) {
		try {
			processPayload(*frame);
		} catch (Exception& ex) { // TODO: specialize
			LOG_INFO_STR(ex.text());
		}
	}
}

} // namespace RTCP
} // namespace LOFAR
