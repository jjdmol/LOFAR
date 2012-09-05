/* TBB_Writer.cc: Write TBB data into an HDF5 file
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
 * $Id: TBB_Writer.cc 36610 2012-03-12 11:54:53Z amesfoort $
 */

#include <lofar_config.h>

#include <cstddef>
#include <csignal>
#include <ctime>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <endian.h>
#if __BYTE_ORDER != __BIG_ENDIAN && __BYTE_ORDER != __LITTLE_ENDIAN
#error Byte order is neither big endian nor little endian: not supported
#endif

#include <iostream>
#include <sstream>

#include <Storage/TBB_Writer.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLogger.h>
#ifdef basename // some glibc have this as a macro
#undef basename
#endif
#include <Common/SystemUtil.h>
#include <Common/SystemCallException.h>
#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/AntField.h>
#include <Stream/SocketStream.h>
#include <Interface/Exceptions.h>
#include <Interface/Stream.h>

#include <dal/lofar/StationNames.h>

namespace LOFAR {
namespace RTCP {

using namespace std;

EXCEPTION_CLASS(TBB_MalformedFrameException, StorageException);

/*
 * The output_format is without seconds. The output_size is including the terminating NUL char.
 * Helper for in filenames and for the FILEDATE attribute.
 */
static string formatFilenameTimestamp(const struct timeval& tv, const char* output_format,
                                      const char* output_format_secs, size_t output_size) {
	struct tm tm;
	gmtime_r(&tv.tv_sec, &tm);
	double secs = tm.tm_sec + tv.tv_usec / 1000000.0;

	vector<char> date(output_size);

	size_t nwritten = strftime(&date[0], output_size, output_format, &tm);
	if (nwritten == 0) {
		date[0] = '\0';
	}
	(void)snprintf(&date[0] + nwritten, output_size - nwritten, output_format_secs, secs);

	return string(&date[0]);
}

string TBB_Header::toString() const {
	std::ostringstream oss;
	oss << (uint32_t)stationID << " " << (uint32_t)rspID << " " << (uint32_t)rcuID << " " << (uint32_t)sampleFreq <<
			" " << seqNr << " " << time << " " << sampleNr << " " << nOfSamplesPerFrame << " " << nOfFreqBands <<
			" " /*<< bandSel << " "*/ << spare << " " << crc16;
	return oss.str();
}

//////////////////////////////////////////////////////////////////////////////

TBB_Dipole::TBB_Dipole()
: itsDataset(NULL) // needed, setting the others is superfluous
, itsDatasetLen(0)
, itsSampleFreq(0)
, itsTime0(0)
, itsSampleNr0(0)
{
}

// Do not use. Only needed for vector<TBB_Dipole>(N).
TBB_Dipole::TBB_Dipole(const TBB_Dipole& rhs)
: itsDataset(rhs.itsDataset) // needed, setting the others is superfluous
//, itsRawOut(rhs.itsRawOut) // ofstream has no copy constr and is unnecessary (this whole func is), so disabled
, itsFlagOffsets(rhs.itsFlagOffsets)
, itsDatasetLen(rhs.itsDatasetLen)
, itsSampleFreq(rhs.itsSampleFreq)
, itsTime0(rhs.itsTime0)
, itsSampleNr0(rhs.itsSampleNr0)
{
}

TBB_Dipole::~TBB_Dipole() {
	/*
	 * Executed by the main thread after joined with all workers, so no need to lock or delay cancellation.
	 *
	 * Set dataset len (if ext raw) and DATA_LENGTH and FLAG_OFFSETS attributes at the end.
	 * Executed by the main thread after joined with all workers, so no need to lock or delay cancellation.
	 * Skip on uninitialized (default constructed) objects.
	 */
	if (itsDataset != NULL) {
		if (usesExternalDataFile()) {
			try {
				itsDataset->resize1D(itsDatasetLen);
			} catch (dal::DALException& exc) {
				LOG_WARN_STR("TBB: failed to resize HDF5 dipole dataset to external data size: " << exc.what());
			}
		}
		try {
			itsDataset->dataLength().value = static_cast<unsigned long long>(itsDatasetLen);
		} catch (dal::DALException& exc) {
			LOG_WARN_STR("TBB: failed to set dipole DATA_LENGTH attribute: " << exc.what());
		}
		try {
			itsDataset->flagOffsets().value = itsFlagOffsets;
		} catch (dal::DALException& exc) {
			LOG_WARN_STR("TBB: failed to set dipole FLAG_OFFSETS attribute: " << exc.what());
		}

		delete itsDataset;
	}
}

void TBB_Dipole::initDipole(const TBB_Header& header, const Parset& parset, const StationMetaData& stationMetaData,
		const string& rawFilename, dal::TBB_Station& station, Mutex& h5Mutex) {
	if (header.sampleFreq == 200 || header.sampleFreq == 160) {
		itsSampleFreq = static_cast<uint32_t>(header.sampleFreq) * 1000000;
	} else { // might happen if header of first frame is corrupt
		itsSampleFreq = parset.clockSpeed(); // Hz
		LOG_WARN("TBB: Unknown sample rate in TBB frame header; using sample rate from the parset");
	}

	{
		ScopedLock h5OutLock(h5Mutex);
		initTBB_DipoleDataset(header, parset, stationMetaData, rawFilename, station);
	}

	if (!rawFilename.empty()) {
		itsRawOut.open(rawFilename.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if (!itsRawOut.good()) {
			throw SystemCallException("Failed to open external raw file, dropping frame");
		}
	}

	itsTime0 = header.time;
	itsSampleNr0 = header.sampleNr;
}

bool TBB_Dipole::isInitialized() const {
	return itsDataset != NULL;
}

bool TBB_Dipole::usesExternalDataFile() const {
	return itsRawOut.is_open();
}

void TBB_Dipole::addFlags(size_t offset, size_t len) {
	// Add a new flag range or extend the last stored flag range. 'len' cannot be 0.
	if (itsFlagOffsets.empty() || offset > itsFlagOffsets.back().end) {
		itsFlagOffsets.push_back(dal::Range(offset, offset + len));
	} else { // extend flag range
		itsFlagOffsets.back().end += len;
	}
}

void TBB_Dipole::processFrameData(const TBB_Frame& frame, Mutex& h5Mutex) {
	off_t offset = (frame.header.time - itsTime0) * itsSampleFreq + frame.header.sampleNr - itsSampleNr0;

	if (frame.header.nOfFreqBands == 0) { // transient mode

#ifndef DISABLE_CRCS
		// Verify data checksum.
		if (!crc32tbb(&frame.payload, frame.header.nOfSamplesPerFrame)) {
			/*
			 * On a data checksum error 'flag' this offset, but still store the data.
			 * Lost frame vs crc error can be seen from the data: a block of zeros indicates lost.
			 */
			addFlags(offset, frame.header.nOfSamplesPerFrame);

			const uint32_t* crc32 = reinterpret_cast<const uint32_t*>(&frame.payload.data[frame.header.nOfSamplesPerFrame]);
			LOG_INFO_STR("TBB: crc32: " << frame.header.toString() << " " << *crc32);
		}
		else
#endif
		if (hasAllZeroDataSamples(frame)) {
			/* Because of the crc32tbb variant, payloads with only zeros validate as correct.
			 * Given the used frame size (1024 samples for transient), this is extremenly unlikely
			 * to be real data. Rather, such zero blocks are from RCUs that are disabled or broken.
			 * Still store the zeros to be able to distinguish from lost frames.
			 */
			addFlags(offset, frame.header.nOfSamplesPerFrame);
		}

	} else { // spectral mode
		//uint32_t bandNr  = frame.header.bandsliceNr & TBB_BAND_NR_MASK;
		//uint32_t sliceNr = frame.header.bandsliceNr >> TBB_SLICE_NR_SHIFT;
	}

	if (offset >= itsDatasetLen) {
		/*
		 * If writing around HDF5, there is no need to lock. Set the HDF5 dataset size at the end (destr).
		 * If writing through HDF5, we have to lock and the HDF5 dataset size is updated by HDF5.
		 */
		if (usesExternalDataFile()) {
			if (offset > itsDatasetLen) {
				itsRawOut.seekp(offset * sizeof(frame.payload.data[0])); // skip space of lost frame
			}
			itsRawOut.write(reinterpret_cast<const char*>(frame.payload.data), static_cast<size_t>(frame.header.nOfSamplesPerFrame) * sizeof(frame.payload.data[0]));
		} else {
			ScopedLock h5Lock(h5Mutex);
			itsDataset->resize1D(offset + frame.header.nOfSamplesPerFrame);
			itsDataset->set1D(offset, frame.payload.data, frame.header.nOfSamplesPerFrame);
		}

		/*
		 * Flag lost frame(s) (assume no out-of-order, see below). Assumes all frames have the same nr of samples.
		 * Note: this cannot detect lost frames at the end of a dataset.
		 */
		size_t nflags = offset - itsDatasetLen;
		if (nflags > 0) {
			addFlags(itsDatasetLen, nflags);
		}

		itsDatasetLen = offset + frame.header.nOfSamplesPerFrame;
	} else { // Out-of-order or duplicate frames are very unlikely in the LOFAR TBB setup.
		// Let us know if it ever happens, then we will do something. (here and in addFlags())
		LOG_WARN_STR("TBB: Dropped out-of-order or duplicate TBB frame at " << frame.header.stationID <<
				" " << frame.header.rspID << " " << frame.header.rcuID << " " << offset);
	}
}

void TBB_Dipole::initTBB_DipoleDataset(const TBB_Header& header, const Parset& parset, const StationMetaData& stationMetaData,
                                       const string& rawFilename, dal::TBB_Station& station) {
	itsDataset = new dal::TBB_DipoleDataset(station.dipole(header.stationID, header.rspID, header.rcuID));

	// Create 1-dim, unbounded (-1) dataset. 
	// Override endianess. TBB data is always stored little endian and also received as such, so written as-is on any platform.
	itsDataset->create1D(0, -1, LOFAR::basename(rawFilename), itsDataset->LITTLE);

	itsDataset->groupType().value = "DipoleDataset";
	itsDataset->stationID().value = header.stationID;
	itsDataset->rspID()    .value = header.rspID;
	itsDataset->rcuID()    .value = header.rcuID;

	itsDataset->sampleFrequency()    .value = itsSampleFreq / 1000000;
	itsDataset->sampleFrequencyUnit().value = "MHz";

	itsDataset->time().value = header.time; // in seconds. Note: may have been corrected in correctTransientSampleNr()
	if (header.nOfFreqBands == 0) { // transient mode
		itsDataset->sampleNumber().value = header.sampleNr;
	}
	itsDataset->samplesPerFrame().value = header.nOfSamplesPerFrame;
	//itsDataset->dataLength().value is set at the end (destr)
	//itsDataset->flagOffsets().value is set at the end (destr)
	itsDataset->nyquistZone().value = parset.nyquistZone();

//#include "MAC/APL/PIC/RSP_Driver/src/CableSettings.h" or "RCUCables.h"
	// Cable delays (optional) from static meta data.
	//itsDataset->cableDelay().value = ???; // TODO
	//itsDataset->cableDelayUnit().value = "ns";

/*
> No DIPOLE_CALIBRATION_DELAY_VALUE
> No DIPOLE_CALIBRATION_DELAY_UNIT
These can be calculated from the values in the LOFAR calibration
tables, but we can do that ourselves as long as the calibration table
values for each dipole are written to the new keyword. Sander: please put them in; see the code ref below.
DIPOLE_CALIBRATION_GAIN_CURVE.

// Use StaticMetaData/CalTables

calibration delay value en unit zijn nuttiger
en is het beste om die er gelijk in te schrijven
momenteel
In /opt/cep/lus/daily/Mon/src/code/src/PyCRTools/modules/metadata.py
heb ik code om de calibratie tabellen uit te lezen
De functie: getStationPhaseCalibration
elke .dat file bevat 96*512*2 doubles
voor 96 rcus, 512 frequenties, een complexe waarde
maar nu vraag ik me wel weer af of de frequenties of de rcus eerst komen
*/
	//itsDataset->dipoleCalibrationDelay().value = ???; // Pim can compute this from the GainCurve below
	//itsDataset->dipoleCalibrationDelayUnit().value = 's';
	//itsDataset->dipoleCalibrationGainCurve().value = ???;


	// Skip if station is not participating in the observation (should not happen).
	if (stationMetaData.available && 2u * 3u * header.rcuID + 2u < stationMetaData.antPositions.size()) {
		/*
		 * Selecting the right positions depends on the antenna set. Checking vs the tables in
		 * lhn001:/home/veen/lus/src/code/data/lofar/antennapositions/ can help, but their repos may be outdated.
		 */
		vector<double> antPos(3);
		antPos[0] = stationMetaData.antPositions[2u * 3u * header.rcuID];
		antPos[1] = stationMetaData.antPositions[2u * 3u * header.rcuID + 1u];
		antPos[2] = stationMetaData.antPositions[2u * 3u * header.rcuID + 2u];
		itsDataset->antennaPosition()     .value = antPos; // absolute position

		itsDataset->antennaPositionUnit() .value = "m";
		itsDataset->antennaPositionFrame().value = parset.positionType(); // "ITRF"

		/*
		 * The normal vector and rotation matrix are actually per antenna field,
		 * but given the HBA0/HBA1 "ears" depending on antenna set, it was
		 * decided to store them per antenna.
		 */
		itsDataset->antennaNormalVector()  .value = stationMetaData.normalVector;   // 3 doubles
		itsDataset->antennaRotationMatrix().value = stationMetaData.rotationMatrix; // 9 doubles, 3x3, row-minor
	}

	// Tile beam is the analog beam. Only HBA can have one analog beam; optional.
	if (parset.haveAnaBeam()) {
		itsDataset->tileBeam()     .value = parset.getAnaBeamDirection(); // always for beam 0
		itsDataset->tileBeamUnit() .value = "m";
		itsDataset->tileBeamFrame().value = parset.getAnaBeamDirectionType(); // idem

		//itsDataset->tileBeamDipoles().value = ???;

		//itsDataset->tileCoefUnit().value = ???;
		//itsDataset->tileBeamCoefs().value = ???;

		// Relative position within the tile.
		//itsDataset->tileDipolePosition().value = ???;
		//itsDataset->tileDipolePositionUnit().value = ???;
		//itsDataset->tileDipolePositionFrame().value = ???;
	}

	itsDataset->dispersionMeasure()    .value = parset.dispersionMeasure(0, 0); // 0.0 if no dedispersion was done
	itsDataset->dispersionMeasureUnit().value = "pc/cm^3";
}

bool TBB_Dipole::hasAllZeroDataSamples(const TBB_Frame& frame) const {
	/*
	 * Real data only has a few consecutive zero values, so this loop terminates
	 * quickly, unless the antenna is broken or disabled, which happens sometimes.
	 */
	for (size_t i = 0; i < frame.header.nOfSamplesPerFrame; i++) {
		if (frame.payload.data[i] != 0) {
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

TBB_Station::TBB_Station(const string& stationName, Mutex& h5Mutex, const Parset& parset,
                         const StationMetaData& stationMetaData, const string& h5Filename,
                         bool dumpRaw)
: itsH5File(dal::TBB_File(h5Filename, dal::TBB_File::CREATE))
, itsH5Mutex(h5Mutex)
, itsStation(itsH5File.station(stationName))
, itsDipoles(MAX_RSPBOARDS/* = per station*/ * NR_RCUS_PER_RSPBOARD) // = 192 for int'l stations
, itsParset(parset)
, itsStationMetaData(stationMetaData)
, itsH5Filename(h5Filename)
, itsDumpRaw(dumpRaw)
{
	initCommonLofarAttributes();
	initTBB_RootAttributesAndGroups(stationName);
}

TBB_Station::~TBB_Station() {
	// Executed by the main thread after joined with all workers, so no need to lock or delay cancellation.
	try {
		itsStation.nofDipoles().value = itsStation.dipoles().size();
	} catch (dal::DALException& exc) {
		LOG_WARN_STR("TBB: failed to set station NOF_DIPOLES attribute: " << exc.what());
	}
}

string TBB_Station::getRawFilename(unsigned rspID, unsigned rcuID) {
	string rawFilename = itsH5Filename;
	string rsprcuStr(formatString("_%03u%03u", rspID, rcuID));
	size_t pos = rawFilename.find('_', rawFilename.find('_') + 1);
	rawFilename.insert(pos, rsprcuStr); // insert _rsp/rcu IDs after station name (2nd '_')
	rawFilename.resize(rawFilename.size() - (sizeof(".h5") - 1));
	rawFilename.append(".raw");
	return rawFilename;
}

void TBB_Station::processPayload(const TBB_Frame& frame) {
	// Guard against bogus incoming rsp/rcu IDs with at().
	TBB_Dipole& dipole(itsDipoles.at(frame.header.rspID * NR_RCUS_PER_RSPBOARD + frame.header.rcuID));

	// Each dipole stream is sent to a single port (thread), so no need to grab a mutex here to avoid double init.
	// Do pass a ref to the h5 mutex for when writing into the HDF5 file.
	if (!dipole.isInitialized()) {
		string rawFilename("");
		if (itsDumpRaw) {
			rawFilename = getRawFilename(frame.header.rspID, frame.header.rcuID);
		}
		dipole.initDipole(frame.header, itsParset, itsStationMetaData, rawFilename, itsStation, itsH5Mutex);
	}

	dipole.processFrameData(frame, itsH5Mutex);
}

// For timestamp attributes in UTC.
string TBB_Station::utcTimeStr(double time) const {
	time_t timeSec = static_cast<time_t>(floor(time));
	unsigned long timeNSec = static_cast<unsigned long>(round( (time-floor(time))*1e9 ));

	char utc_str[50];
	struct tm tm;
	gmtime_r(&timeSec, &tm);
	if (strftime(utc_str, sizeof(utc_str), "%Y-%m-%dT%H:%M:%S", &tm) == 0) {
		return "";
	}

	return formatString("%s.%09luZ", utc_str, timeNSec);
}

double TBB_Station::toMJD(double time) const {
	// January 1st, 1970, 00:00:00 (GMT) equals 40587.0 Modify Julian Day number
	return 40587.0 + time / (24*60*60);
}

void TBB_Station::initCommonLofarAttributes() {
	itsH5File.groupType().value = "Root";

	//itsH5File.fileName() is set by DAL
	//itsH5File.fileDate() is set by DAL
	//itsH5File.fileType() is set by DAL
	//itsH5File.telescope() is set by DAL

	itsH5File.projectID()   .value = itsParset.getString("Observation.Campaign.name", "");
	itsH5File.projectTitle().value = itsParset.getString("Observation.Scheduler.taskName", "");
	itsH5File.projectPI()   .value = itsParset.getString("Observation.Campaign.PI", "");
	ostringstream oss;
	// Use ';' instead of ',' to pretty print, because ',' already occurs in names (e.g. Smith, J.).
	writeVector(oss, itsParset.getStringVector("Observation.Campaign.CO_I", ""), "; ", "", "");
	itsH5File.projectCOI()    .value = oss.str();
	itsH5File.projectContact().value = itsParset.getString("Observation.Campaign.contact", "");

	itsH5File.observationID() .value = formatString("%u", itsParset.observationID());

	itsH5File.observationStartUTC().value = utcTimeStr(itsParset.startTime());
	itsH5File.observationStartMJD().value = toMJD(itsParset.startTime());

	// The stop time can be a bit further than the one actually specified, because we process in blocks.
	unsigned nrBlocks = floor((itsParset.stopTime() - itsParset.startTime()) / itsParset.CNintegrationTime());
	double stopTime = itsParset.startTime() + nrBlocks * itsParset.CNintegrationTime();

	itsH5File.observationEndUTC().value = utcTimeStr(stopTime);
	itsH5File.observationEndMJD().value = toMJD(stopTime);

	itsH5File.observationNofStations().value = itsParset.nrStations(); // TODO: SS beamformer?
	// For the observation attribs, dump all stations participating in the observation (i.e. allStationNames(), not mergedStationNames()).
	// This may not correspond to which station HDF5 groups will be written for TBB, but that is true anyway, regardless of any merging.
	itsH5File.observationStationsList().value = itsParset.allStationNames(); // TODO: SS beamformer?

	const vector<double> subbandCenterFrequencies(itsParset.subbandToFrequencyMapping());
	double min_centerfrequency = *min_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
	double max_centerfrequency = *max_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
	double sum_centerfrequencies = accumulate( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end(), 0.0 );

	double subbandBandwidth = itsParset.sampleRate();

	itsH5File.observationFrequencyMax()   .value = (max_centerfrequency + subbandBandwidth / 2) / 1e6;
	itsH5File.observationFrequencyMin()   .value = (min_centerfrequency - subbandBandwidth / 2) / 1e6;
	itsH5File.observationFrequencyCenter().value = sum_centerfrequencies / subbandCenterFrequencies.size();
	itsH5File.observationFrequencyUnit()  .value = "MHz";

	itsH5File.observationNofBitsPerSample().value = itsParset.nrBitsPerSample();
	itsH5File.clockFrequency()             .value = itsParset.clockSpeed() / 1e6;
	itsH5File.clockFrequencyUnit()         .value = "MHz";

	itsH5File.antennaSet()     .value = itsParset.antennaSet();
	itsH5File.filterSelection().value = itsParset.getString("Observation.bandFilter", "");

	unsigned nrSAPs = itsParset.nrBeams();
	vector<string> targets(nrSAPs);

	for (unsigned sap = 0; sap < nrSAPs; sap++) {
		targets[sap] = itsParset.beamTarget(sap);
	}

	itsH5File.targets().value = targets;

#ifndef TBB_WRITER_VERSION
	itsH5File.systemVersion().value = LOFAR::StorageVersion::getVersion();
#else
	itsH5File.systemVersion().value = TBB_WRITER_VERSION;
#endif

	//itsH5File.docName() is set by DAL
	//itsH5File.docVersion() is set by DAL

	itsH5File.notes().value = "";
}

// The writer creates one HDF5 file per station, so create only one Station Group here.
void TBB_Station::initTBB_RootAttributesAndGroups(const string& stName) {
	int operatingMode = itsParset.getInt("Observation.TBB.TBBsetting.operatingMode", 0);
	if (operatingMode == 1) {
		itsH5File.operatingMode().value = "transient";
	} else if (operatingMode == 2) {
		itsH5File.operatingMode().value = "spectral";
	} else { // should not happen, parset assumed to be ok
		LOG_WARN("TBB: Failed to get operating mode from parset");
	}

	itsH5File.nofStations().value = 1u;

	// Find the station name we are looking for ("CS001" == "CS001HBA0") and retrieve its pos using the found idx.
	vector<double> stPos;

	vector<string> obsStationNames(itsParset.allStationNames());
	vector<string>::const_iterator nameIt(obsStationNames.begin());

	vector<double> stationPositions(itsParset.positions()); // len must be (is generated as) 3x #stations
	vector<double>::const_iterator posIt(stationPositions.begin());

	for ( ; nameIt != obsStationNames.end(); ++nameIt, posIt += 3) {
		if (*nameIt == stName) { // both include "HBA0" or similar suffix
			break;
		}
	}
	if (nameIt != obsStationNames.end() && posIt < stationPositions.end()) { // found?
		stPos.assign(posIt, posIt + 3);
	}
	itsStation.create();
	initStationGroup(itsStation, stName, stPos);

	// Trigger Group
	dal::TBB_Trigger tg(itsH5File.trigger());
	tg.create();
	initTriggerGroup(tg);
}

void TBB_Station::initStationGroup(dal::TBB_Station& st, const string& stName, const vector<double>& stPosition) {
	st.groupType()  .value = "StationGroup";
	st.stationName().value = stName;

	if (!stPosition.empty()) {
		st.stationPosition()     .value = stPosition;
		st.stationPositionUnit() .value = "m";
		st.stationPositionFrame().value = itsParset.positionType(); // "ITRF"
	}

	// digital beam(s)
	if (itsParset.nrBeams() > 0) { // TODO: What if >1 station beams? For now, only write beam 0.
		st.beamDirection()     .value = itsParset.getBeamDirection(0);
		st.beamDirectionFrame().value = itsParset.getBeamDirectionType(0);
		st.beamDirectionUnit() .value = "m";
	}

	// clockCorrectionTime() returns 0.0 if stName is unknown, while 0.0 is valid for some stations...
	st.clockOffset()    .value = itsParset.clockCorrectionTime(stName);
	st.clockOffsetUnit().value = "s";

	//st.nofDipoles.value is set at the end (destr)
}

void TBB_Station::initTriggerGroup(dal::TBB_Trigger& tg) {
	tg.groupType()     .value = "TriggerGroup";
	tg.triggerType()   .value = "Unknown";
	tg.triggerVersion().value = 0; // There is no trigger algorithm info available to us yet.

	// Trigger parameters (how to decide if there is a trigger; per obs)
	try {
		tg.paramCoincidenceChannels().value = itsParset.getInt   ("Observation.ObservationControl.StationControl.TBBControl.NoCoincChann");
		tg.paramCoincidenceTime()    .value = itsParset.getDouble("Observation.ObservationControl.StationControl.TBBControl.CoincidenceTime");
		tg.paramDirectionFit()       .value = itsParset.getString("Observation.ObservationControl.StationControl.TBBControl.DoDirectionFit");
		tg.paramElevationMin()       .value = itsParset.getDouble("Observation.ObservationControl.StationControl.TBBControl.MinElevation");
		tg.paramFitVarianceMax()     .value = itsParset.getDouble("Observation.ObservationControl.StationControl.TBBControl.MaxFitVariance");
	} catch (APSException& exc) {
		LOG_WARN("TBB: Failed to write all trigger parameters: missing from parset");
	}

	// Trigger data (per trigger)
	// N/A atm

	/*
	 * It is very likely that the remaining (optional) attributes and the trigger alg
	 * will undergo many changes. TBB user/science applications will have to retrieve and
	 * set the remaining fields "by hand" for a while using e.g. DAL by checking and
	 * specifying each attribute name presumed available.
	 * Until it is clear what is needed and available, this cannot be standardized.
	 *
	 * If you add fields using getTYPE(), catch the possible APSException as above.
	 */
}

//////////////////////////////////////////////////////////////////////////////

TBB_StreamWriter::TBB_StreamWriter(TBB_Writer& writer, const string& inputStreamName, const string& logPrefix)
: itsWriter(writer)
, itsInputStreamName(inputStreamName)
, itsLogPrefix(logPrefix)
{
	itsFrameBuffers = new TBB_Frame[nrFrameBuffers];
	//itsReceiveQueue.reserve(nrFrameBuffers); // Queue does not support this...
	for (unsigned i = nrFrameBuffers; i > 0; ) {
		itsFreeQueue.append(&itsFrameBuffers[--i]);
	}

	itsTimeoutStamp.tv_sec = 0;
	itsTimeoutStamp.tv_usec = 0;

	itsOutputThread = NULL;
	try {
		itsOutputThread = new Thread(this, &TBB_StreamWriter::mainOutputLoop, logPrefix + "OutputThread: ");
		itsInputThread  = new Thread(this, &TBB_StreamWriter::mainInputLoop,  logPrefix + "InputThread: ");
	} catch (exception& exc) {
		if (itsOutputThread != NULL) {
			try {
				itsReceiveQueue.append(NULL); // tell output thread to stop
			} catch (exception& exc) {
				LOG_WARN_STR("TBB: failed to notify output thread to terminate: " << exc.what());
			}
			delete itsOutputThread;
		}
		delete[] itsFrameBuffers;
		throw;
	}
}

TBB_StreamWriter::~TBB_StreamWriter() {
	// Only cancel input thread. It will notify the output thread.
	itsInputThread->cancel();

	delete itsInputThread;
	delete itsOutputThread;
	delete[] itsFrameBuffers;
}

time_t TBB_StreamWriter::getTimeoutStampSec() const {
	return itsTimeoutStamp.tv_sec; // racy read (and no access once guarantee)
}

void TBB_StreamWriter::frameHeaderLittleToHost(TBB_Header& header) const {
	//header.seqNr              = le32toh(header.seqNr); // (must be) zeroed, but otherwise not useful for us
	header.time               = le32toh(header.time);
	header.sampleNr           = le32toh(header.sampleNr); // also swaps header.bandsliceNr
	header.nOfSamplesPerFrame = le16toh(header.nOfSamplesPerFrame);
	header.nOfFreqBands       = le16toh(header.nOfFreqBands);
	//header.spare              = le16toh(header.spare); // unused
	header.crc16              = le16toh(header.crc16);
}

void TBB_StreamWriter::correctTransientSampleNr(TBB_Header& header) const {
	/*
	 * We assume header.sampleFreq is either 200 or 160 MHz (another multiple of #samples per frame is also fine).
	 * 
	 * At 200 MHz sample rate with 1024 samples per frame, we have 195213.5 frames per second.
	 * This means that every 2 seconds, a frame overlaps a seconds boundary; every odd frame needs its sampleNr corrected.
	 * At 160 MHz sample rate, an integer number of frames fits in a second (156250), so no correction is needed.
	 *
	 * This fixup assumes no other sample freq than 200 MHz that needs a correction is used (checked in initDipole()),
	 * and that the hw time nr starts even (it is 0) (cannot be checked, because dumps can start at any frame).
	 */
	if (header.sampleFreq == 200 && header.time & 1) {
		header.sampleNr += DEFAULT_TRANSIENT_NSAMPLES / 2;
	}
}

/*
 * Assumes that the seqNr field in the TBB_Frame at buf has been zeroed.
 * Takes a ptr to a complete header. (Drop too small frames earlier.)
 */
bool TBB_StreamWriter::crc16tbb(const TBB_Header* header) {
	itsCrc16gen.reset();

	/*
	 * The header checksum is done like the data, i.e. on 16 bit little endian blocks at a time.
	 * As with the data, both big and little endian CPUs need to byte swap.
	 */
	const int16_t* ptr = reinterpret_cast<const int16_t*>(header);
	size_t i;
	for (i = 0; i < (sizeof(*header) - sizeof(header->crc16)) / sizeof(int16_t); i++) {
		int16_t val = __bswap_16(ptr[i]);
		itsCrc16gen.process_bytes(&val, sizeof(int16_t));
	}

	// Byte swap the little endian checksum on big endian only.
	// It is also possible to process header->crc16 and see if checksum() equals 0.
	uint16_t crc16val = header->crc16;
#if __BYTE_ORDER == __BIG_ENDIAN
	crc16val = __bswap_16(crc16val);
#endif
	return itsCrc16gen.checksum() == crc16val;
}

/*
 * Note: The nsamples arg is without the space taken by the crc32 in payload. (Drop too small frames earlier.)
 */
bool TBB_Dipole::crc32tbb(const TBB_Payload* payload, size_t nsamples) {
	itsCrc32gen.reset();

	/*
	 * Both little and big endian CPUs need to byte swap, because the data always arrives
	 * in little and the boost routines treat it as uint8_t[] (big).
	 */
	const int16_t* ptr = reinterpret_cast<const int16_t*>(payload->data);
	size_t i;
	for (i = 0; i < nsamples; i++) {
		int16_t val = __bswap_16(ptr[i]);
		itsCrc32gen.process_bytes(&val, sizeof(int16_t));
	}

	// Byte swap the little endian checksum on big endian only.
	// It is also possible to process crc32val and see if checksum() equals 0.
	uint32_t crc32val = *reinterpret_cast<const uint32_t*>(&ptr[nsamples]);
#if __BYTE_ORDER == __BIG_ENDIAN
	crc32val = __bswap_32(crc32val);
#endif
	return itsCrc32gen.checksum() == crc32val;
}

/*
 * Process the incoming TBB header.
 * Note that this function may update the header, but not its crc, so you cannot re-verify it.
 */
void TBB_StreamWriter::processHeader(TBB_Header& header, size_t recvPayloadSize) {
#ifndef DISABLE_CRCS
	header.seqNr = 0; // For the header crc. Don't save/restore it as we don't need this field.
	if (!crc16tbb(&header)) {
		/*
		 * Spec says each frame has the same fixed length, so the previous values are a good base guess if the header crc fails.
		 * But it is not clear if it is worth the effort. For now, drop the frame.
		 */
		throw TBB_MalformedFrameException("crc16: " + header.toString());
	}
#endif

	frameHeaderLittleToHost(header); // no-op on little endian

	if (header.nOfFreqBands == 0) { // transient mode
		// Use received size instead of received nOfSamplesPerFrame header field to access data.
		if (recvPayloadSize < 2 * sizeof(int16_t) + sizeof(uint32_t)) {
			// Drop it. The data crc routine only works for at least 2 data elements + a crc32.
			throw TBB_MalformedFrameException("dropping too small TBB transient frame");
		}
		uint16_t recvSamples = (recvPayloadSize - sizeof(uint32_t)) / sizeof(int16_t);
		header.nOfSamplesPerFrame = recvSamples; // most likely already DEFAULT_TRANSIENT_NSAMPLES

		correctTransientSampleNr(header);
	} else { // spectral mode
		if (recvPayloadSize < sizeof(int16_t)) {
			throw TBB_MalformedFrameException("dropping too small TBB spectral frame");
		}
		uint16_t recvSamples = recvPayloadSize / (2 * sizeof(int16_t));
		header.nOfSamplesPerFrame = recvSamples;
	}
}

void TBB_StreamWriter::mainInputLoop() {
	LOG_INFO_STR(itsLogPrefix << "Waiting for incoming data at " << itsInputStreamName);
	Stream* stream = createStream(itsInputStreamName, true); // allocates, but wrapped in an obj proved a hassle

	while (1) {
		TBB_Frame* frame;

		try {
			frame = NULL;
			frame = itsFreeQueue.remove();

			size_t datagramSize = stream->tryRead(frame, sizeof(*frame));

			// Notify master that we are still busy. (Racy, see TS decl)
			gettimeofday(&itsTimeoutStamp, NULL);

			if (datagramSize < sizeof(TBB_Header)) {
				throw TBB_MalformedFrameException("dropping too small TBB frame");
			}
			processHeader(frame->header, datagramSize - sizeof(TBB_Header));

			itsReceiveQueue.append(frame);
		} catch (TBB_MalformedFrameException& mffExc) {
			LOG_WARN_STR(itsLogPrefix << mffExc.what());
			if (frame != NULL) {
				itsFreeQueue.append(frame);
			}
		} catch (Stream::EndOfStreamException& ) { // we use thread cancellation, but just in case
			LOG_INFO_STR(itsLogPrefix << "EndOfStreamException");
			break;
		} catch (exception& exc) {
			LOG_FATAL_STR(itsLogPrefix << exc.what());
			break;
		} catch (...) { // Cancellation exc happens at exit. Nothing to do, so disabled. Otherwise, must rethrow.
			try {
				itsReceiveQueue.append(NULL); // always notify output thread at exit of no more data
			} catch (exception& exc) {
				LOG_WARN_STR(itsLogPrefix << "may have failed to notify output thread to terminate: " << exc.what());
			}
			delete stream;
			throw;
		}
	}

	itsReceiveQueue.append(NULL);
	delete stream;
}

void TBB_StreamWriter::mainOutputLoop() {
	const unsigned maxErrors = 16;
	unsigned nrErrors = 0; // i.e. per output thread per dump

	while (1) {
		TBB_Frame* frame;
		try {
			frame = NULL;
			frame = itsReceiveQueue.remove();
			if (frame == NULL) {
				break;
			}

#ifdef PRINT_QUEUE_LEN
			LOG_INFO_STR(itsLogPrefix << "recvqsz=" << itsReceiveQueue.size());
#endif

			TBB_Station* station = itsWriter.getStation(frame->header);
			station->processPayload(*frame);
		} catch (exception& exc) {
			// e.g. std::out_of_range for at() on bad rsp/rcu ID,
			// DALException, or StorageException/SystemCallException on failed to create file.
			nrErrors += 1;
			if (nrErrors < maxErrors) {
				LOG_WARN_STR(itsLogPrefix << exc.what());
			} else {
				LOG_FATAL_STR(itsLogPrefix << exc.what());
				break;
			}
		}

		if (frame != NULL) {
			itsFreeQueue.append(frame);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

TBB_Writer::TBB_Writer(const vector<string>& inputStreamNames, const Parset& parset,
                       const StationMetaDataMap& stationMetaDataMap, const string& outDir, bool dumpRaw,
                       const string& logPrefix)
: itsParset(parset)
, itsStationMetaDataMap(stationMetaDataMap)
, itsOutDir(outDir)
, itsDumpRaw(dumpRaw)
, itsRunNr(0)
{
	// Mask all signals to inherit for workers. This forces signals to be delivered to the main thread.
	// Wrap to make sure we always (try to) restore the signal mask.
	struct SigMask {
		sigset_t sigset_old;

		SigMask() {
			sigset_t sigset_all_masked;
			sigfillset(&sigset_all_masked);
			if (pthread_sigmask(SIG_SETMASK, &sigset_all_masked, &sigset_old) != 0) {
				// The LOFAR sys uses another way to control us, so do not make this fatal.
				LOG_WARN_STR("TBB: pthread_sigmask() failed to mask signals to inherit for worker threads.");
			}
		}

		~SigMask() {
			if (pthread_sigmask(SIG_SETMASK, &sigset_old, NULL) != 0) {
				// No exc in destr. If restoring fails and --keeprunning, we remain deaf.
				LOG_WARN_STR("TBB: pthread_sigmask() failed to restore signals. We may be deaf to signals.");
			}
		}
	} sigm;

	itsUnknownStationMetaData.available = false;

	for (unsigned i = 0; i < inputStreamNames.size(); i++) {
		itsStreamWriters.push_back(new TBB_StreamWriter(*this, inputStreamNames[i], logPrefix)); // TODO: leaks just created obj if push_back() fails
	}
}

TBB_Writer::~TBB_Writer() {
	map<unsigned, TBB_Station* >::iterator it(itsStations.begin());
	for ( ; it != itsStations.end(); ++it) {
		delete it->second;
	}

	for (unsigned i = itsStreamWriters.size(); i > 0; ) {
		delete itsStreamWriters[--i];
	}
}

TBB_Station* TBB_Writer::getStation(const TBB_Header& header) {
	ScopedLock sl(itsStationsMutex); // protect against insert below
	map<unsigned, TBB_Station*>::iterator stIt(itsStations.find(header.stationID));
	if (stIt != itsStations.end()) {
		return stIt->second; // common case
	}

	// Create new station with HDF5 file and station HDF5 group.
	string stationName(dal::stationIDToName(header.stationID));
	string h5Filename(createNewTBB_H5Filename(header, stationName));
	StationMetaDataMap::const_iterator stMdIt(itsStationMetaDataMap.find(header.stationID));
	// If not found, station is not participating in the observation. Should not happen, but don't panic.
	const StationMetaData& stMetaData = stMdIt == itsStationMetaDataMap.end() ? itsUnknownStationMetaData : stMdIt->second;

	TBB_Station* station;
	{
		ScopedLock slH5(itsH5Mutex);
		station = new TBB_Station(stationName, itsH5Mutex, itsParset, stMetaData, h5Filename, itsDumpRaw); // TODO: mem leak if insert() fails. Also, really need global h5lock: cannot create 2 different h5 files at once safely.
	}

	return itsStations.insert(make_pair(header.stationID, station)).first->second;
}

// Must be called holding itsStationsMutex.
string TBB_Writer::createNewTBB_H5Filename(const TBB_Header& header, const string& stationName) {
	const string typeExt("tbb.h5");
	string obsIDStr(formatString("%u", itsParset.observationID()));

	// Use the recording time of the first (received) frame as timestamp.
	struct timeval tv;
	tv.tv_sec = header.time;
	if (header.nOfFreqBands == 0) { // transient mode
		tv.tv_usec = static_cast<unsigned long>(round( static_cast<double>(header.sampleNr) / header.sampleFreq ));
	} else { // spectral mode
		tv.tv_usec = 0;
	}

	// Generate the output filename, because for TBB it is not in the parset.
	// From LOFAR-USG-ICD005 spec named "LOFAR Data Format ICD File Naming Conventions", by A. Alexov et al.
	const char output_format[] = "D%Y%m%dT%H%M"; // without secs
	const char output_format_secs[] = "%06.3fZ"; // total width of ss.sss is 6
	const char output_format_example[] = "DYYYYMMDDTHHMMSS.SSSZ";
	string triggerDateTime(formatFilenameTimestamp(tv, output_format, output_format_secs, sizeof(output_format_example)));
	string h5Filename(itsOutDir + "L" + obsIDStr + "_" + stationName + "_" + triggerDateTime + "_" + typeExt);

	// If the file already exists, add a run nr and retry. (seq race with DAL's open and doesn't check .raw, but good enough)
	// If >1 stations per node, start at the prev run nr if any (hence itsRunNr).
	if (itsRunNr == 0) {
		if (access(h5Filename.c_str(), F_OK) != 0 && errno == ENOENT) {
			// Does not exist (or broken dir after all, or dangling sym link...). Try this one.
			return h5Filename;
		} else { // Exists, inc run number. 
			itsRunNr = 1;
		}
	}

	size_t pos = h5Filename.size() - typeExt.size();
	string runNrStr(formatString("R%03u_", itsRunNr));
	h5Filename.insert(pos, runNrStr);
	while (itsRunNr < 1000 && ( access(h5Filename.c_str(), F_OK) == 0 || errno != ENOENT )) {
		itsRunNr += 1;
		runNrStr = formatString("R%03u_", itsRunNr);
		h5Filename.replace(pos, runNrStr.size(), runNrStr);
	}
	if (itsRunNr == 1000) { // run number is supposed to fit in 3 digits
		throw StorageException("failed to generate new .h5 filename after trying 1000 filenames.");
	}

	return h5Filename;
}

time_t TBB_Writer::getTimeoutStampSec(unsigned streamWriterNr) const {
	return itsStreamWriters[streamWriterNr]->getTimeoutStampSec();
}

} // namespace RTCP
} // namespace LOFAR

