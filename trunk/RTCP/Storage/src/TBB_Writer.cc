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

#include <Storage/TBB_Writer.h>

#include <unistd.h>

#include <iostream>

#include <Common/LofarConstants.h>
#include <Common/SystemUtil.h>
#include <Common/SystemCallException.h>
#include <Common/StringUtil.h>
#include <ApplCommon/AntField.h>
#include <Stream/SocketStream.h>
#include <Interface/Stream.h>
#include <Interface/SmartPtr.h>

#include <dal/lofar/Station.h>

namespace LOFAR {
namespace RTCP {

using namespace std;

EXCEPTION_CLASS(TBB_MalformedFrameException, Exception);

/*
 * The output_format is without seconds; the output_size is incl the '\0'.
 * Helper for in filenames and for the FILEDATE attribute.
 */
static string formatFilenameTimestamp(const struct timeval& tv, const char* output_format,
			const char* output_format_secs, size_t output_size) {
	struct tm tm = {0};
	gmtime_r(&tv.tv_sec, &tm);
	double secs = tm.tm_sec + tv.tv_usec / 1000000.0;

	SmartPtr<char, SmartPtrDeleteArray<char> > date_str(new char[output_size]);
	size_t nwritten = strftime(date_str, output_size, output_format, &tm);
	if (nwritten == 0) {
		date_str[0] = '\0';
	}
	/*int nprinted = */snprintf(date_str + nwritten, sizeof(output_format) - nwritten, output_format_secs, secs);

	string date(date_str);
	return date;
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

// Do not use; only needed for vector<TBB_Dipole>(N)
TBB_Dipole::TBB_Dipole(const TBB_Dipole& rhs)
: itsDataset(rhs.itsDataset) // needed, setting the others is superfluous
//, itsRawOut(rhs.itsRawOut) // ofstream has no copy constr and unnecessary (this whole func), so disabled
, itsFlagOffsets(rhs.itsFlagOffsets)
, itsDatasetLen(rhs.itsDatasetLen)
, itsSampleFreq(rhs.itsSampleFreq)
, itsTime0(rhs.itsTime0)
, itsSampleNr0(rhs.itsSampleNr0)
{
}

TBB_Dipole::~TBB_Dipole() {
	/*
	 * Set dataset len (if ext raw) and DATA_LENGTH and FLAG_OFFSETS attributes at the end.
	 * Executed by the main thread after joined with all workers, so no need to lock or delay cancellation.
	 * Skip on uninitialized (default constructed) objects.
	 */
	if (itsDataset != NULL) {
		if (usesExternalDataFile()) {
			try {
				vector<ssize_t> dims(1, itsDatasetLen); // TODO: get rid of this in DAL
				itsDataset->resize(dims);
			} catch (DAL::DALException& exc) {
				LOG_WARN_STR("TBB: failed to resize HDF5 dipole dataset to external data size: " << exc.what());
			}
		}
		try {
			itsDataset->dataLength().value = itsDatasetLen; //TODO: -> unsigned long?
		} catch (DAL::DALException& exc) {
			LOG_WARN_STR("TBB: failed to set dipole DATA_LENGTH attribute: " << exc.what());
		}
		try {
			itsDataset->flagOffsets().value = itsFlagOffsets;
		} catch (DAL::DALException& exc) {
			LOG_WARN_STR("TBB: failed to set dipole FLAG_OFFSETS attribute: " << exc.what());
		}

		delete itsDataset;
	}
}

void TBB_Dipole::initDipole(const TBB_Header& header, const Parset& parset, const string& rawFilename,
			DAL::TBB_Station& station, Mutex& h5Mutex) {
	if (header.sampleFreq == 200 || header.sampleFreq == 160) {
		itsSampleFreq = static_cast<uint32_t>(header.sampleFreq) * 1000000;
	} else { // might happen if header of first frame is corrupt
		itsSampleFreq = parset.clockSpeed();
		LOG_WARN("TBB: Unknown sample rate in TBB frame header; using sample rate from the parset");
	}

	initTBB_DipoleDataset(header, parset, rawFilename, station, h5Mutex);

	if (!rawFilename.empty()) {
		itsRawOut.open(rawFilename.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if (!itsRawOut.good()) {
			throw SystemCallException("Failed to open external raw file, dropping frame");
		}
	}

	itsTime0 = header.time;
	itsSampleNr0 = header.sampleNr;
}

bool TBB_Dipole::isInitialized() {
	return itsDataset != NULL;
}

bool TBB_Dipole::usesExternalDataFile() {
	return itsRawOut.is_open();
}

void TBB_Dipole::processFrameData(const TBB_Frame& frame, Mutex& h5Mutex) {
	off_t offset = (frame.header.time - itsTime0) * itsSampleFreq + frame.header.sampleNr - itsSampleNr0;

	if (frame.header.nOfFreqBands == 0) { // transient mode
		// Verify data checksum.
		uint32_t csum = crc32tbb(reinterpret_cast<const uint16_t*>(frame.payload.data), frame.header.nOfSamplesPerFrame + 2/*=crc32*/);
		if (csum != 0) {
			/*
			 * On a data checksum error 'flag' this offset, but still store the data.
			 * Lost frame vs crc error can be seen from the data: a block of zeros indicates lost.
			 */
			itsFlagOffsets.push_back(offset);

			uint32_t crc32 = *reinterpret_cast<const uint32_t*>(&frame.payload.data[frame.header.nOfSamplesPerFrame]);
			LOG_INFO_STR("TBB: crc32: " << frame.header.toString() << " " << crc32);
		}
	} else { // spectral mode
		//uint32_t bandNr  = frame.header.bandsliceNr & TBB_BAND_NR_MASK;
		//uint32_t sliceNr = frame.header.bandsliceNr >> TBB_SLICE_NR_SHIFT;
		// TODO: prepare to store, but spectral output format unclear.
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
			vector<size_t> pos(1, offset); // TODO: get rid of this vector stuff in DAL
			vector<ssize_t> dsNewDims(1, offset + frame.header.nOfSamplesPerFrame);

			ScopedLock h5Lock(h5Mutex);
			itsDataset->resize(dsNewDims);
			itsDataset->set1D(pos, frame.header.nOfSamplesPerFrame, frame.payload.data);
		}

		/*
		 * Flag lost frame(s) (assume no out-of-order, see below). Assumes all frames have the same nr of samples.
		 * Note: this cannot detect lost frames at the end of a dataset. Also, this cannot re-flag a crc32 error.
		 */
		for (unsigned lostOffset = itsDatasetLen; lostOffset < offset; lostOffset += frame.header.nOfSamplesPerFrame) {
			itsFlagOffsets.push_back(static_cast<unsigned>(lostOffset));
		}

		itsDatasetLen = offset + frame.header.nOfSamplesPerFrame;
	} else { // Out-of-order or duplicate frames are very unlikely in the LOFAR TBB setup.
		// Let us know if it ever happens, then we will do something.
		LOG_WARN_STR("TBB: Dropped out-of-order or duplicate TBB frame at " << frame.header.stationID <<
				" " << frame.header.rspID << " " << frame.header.rcuID << " " << offset);
	}
}

void TBB_Dipole::initTBB_DipoleDataset(const TBB_Header& header, const Parset& parset, const string& rawFilename,
			DAL::TBB_Station& station, Mutex& h5Mutex) {
	itsDataset = new DAL::TBB_DipoleDataset(station.dipole(header.stationID, header.rspID, header.rcuID));

	ScopedLock h5OutLock(h5Mutex);

	// Create 1-dim, unbounded (-1) dataset. 
	// Override endianess. TBB data is always stored little endian and also received as such, so written as-is on any platform.
	vector<ssize_t> dsDims(1, 0);
	vector<ssize_t> dsMaxDims(1, -1);
	itsDataset->create(dsDims, dsMaxDims, LOFAR::basename(rawFilename), itsDataset->LITTLE);

	itsDataset->groupType().value = "DipoleDataset";
	itsDataset->stationID().value = header.stationID;
	itsDataset->rspID().value = header.rspID;
	itsDataset->rcuID().value = header.rcuID;

	itsDataset->sampleFrequency().value = itsSampleFreq;
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
	itsDataset->cableDelayUnit().value = "ns";

	//itsDataset->dipoleCalibrationDelay().value = ???; // TODO: Pim can compute this from the GainCurve below
/*
> No DIPOLE_CALIBRATION_DELAY_VALUE
> No DIPOLE_CALIBRATION_DELAY_UNIT
These can be calculated from the values in the LOFAR calibration
tables, but we can do that ourselves as long as the calibration table
values for each dipole are written to the new keyword
DIPOLE_CALIBRATION_GAIN_CURVE.
*/
	//itsDataset->dipoleCalibrationDelayUnit().value = 's';
	//itsDataset->dipoleCalibrationGainCurve().value = ???; // TODO: where to get this?

#if 0 // tmp
int antSetName2AntFieldIndex(const string& antSetName) {
	int idx;

	if (strcmp(antSetName.c_str(), "LBA") == 0) {
		idx = LOFAR::LBA_IDX;
	} else if (strcmp(antSetName.c_str(), "HBA_ZERO") == 0) {
		idx = LOFAR::HBA0_IDX;
	} else if (strcmp(antSetName.c_str(), "HBA_ONE") == 0) {
		idx = LOFAR::HBA1_IDX;
	} else if (strcmp(antSetName.c_str(), "HBA") == 0) {
		idx = LOFAR::HBA_IDX;
	} else {
		idx = -1;
	}

	return idx;
}
#endif
#if 0
	// e.g. LOFAR/MAC/Deployment/data/StaticMetaData/AntennaFields/CS001-AntennaField.conf
	string antFieldFilename(antFieldPath + stationName + "-AntennaField.conf");
	AntField antField(antFieldFilename); // will locate the filename if no abs path is given
	/*} catch (::LOFAR::AssertError& exc) {
		// A message has already been sent to the logger.
	}*/
	int fieldIdx = antSetName2AntFieldIndex(parset.antennaSet());

	// See AntField.h in ApplCommon for the AFArray typedef and contents.
	// Relative position wrt to field center. (TODO: but Pim's mail says absolute ITRF)
	AFArray& antPos(antField.AntPos(fieldIdx));
	itsDataset->antennaPosition().value = AntField::getData(antPos); // TODO: select dipole pos
	itsDataset->antennaPositionUnit().value = "m";
	itsDataset->antennaPositionFrame().value = parset.positionType(); // "ITRF"

// Rot matrix and normal vector are per field (3 avail), not per dipole
/*
These specify one rotation matrix and normal vector per station (one
for LBA, HBA ear 0 and ear 1 each) but because the one to use depends
on the antenna set (for instance for HBA one would need to store two
or figure out a common one which is complicated) we chose to just
store it per antenna which adds some overhead but is more flexible.
The antenna positions should be in absolute ITRF (e.g. not relative to
the station, whose absolute position is stored in the same frame in
the StationGroup) and the frame should reflect the target date but you
are free to pick the format or just use "ITRF".
*/
	AFArray& normVec(antField.normVector(fieldIdx));
	itsDataset->antennaNormalVector().value = AntField::getData(normVec); // 3 doubles

	AFArray& rotMat(antField.rotationMatrix(fieldIdx));
	itsDataset->antennaRotationMatrix().value = AntField::getData(rotMat); // 9 doubles, 3x3, row-minor
#endif

	// Tile beam is the analog beam. HBA can have 1 analog beam, thus optional.
	if (parset.haveAnaBeam()) {
		itsDataset->tileBeam().value = parset.getAnaBeamDirection(); // always for beam 0
		itsDataset->tileBeamUnit().value = "m";
		itsDataset->tileBeamFrame().value = parset.getAnaBeamDirectionType(); // idem
		//itsDataset->tileBeamDipoles().value = ???;

		//itsDataset->tileCoefUnit().value = ???;
		//itsDataset->tileBeamCoefs().value = ???;

		// Relative position within the tile.
		//itsDataset->tileDipolePosition().value = ???;
		//itsDataset->tileDipolePositionUnit().value = ???;
		//itsDataset->tileDipolePositionFrame().value = ???;
	}


	itsDataset->dispersionMeasure().value = parset.dispersionMeasure(0, 0); // 0.0 if no dedispersion was done
	itsDataset->dispersionMeasureUnit().value = "pc/cm^3";
}

//////////////////////////////////////////////////////////////////////////////

TBB_Station::TBB_Station(const string& stationName, const Parset& parset, const string& h5Filename, bool dumpRaw)
: itsH5File(DAL::TBB_File(h5Filename, DAL::TBB_File::CREATE))
, itsStation(itsH5File.station(stationName))
, itsDipoles(MAX_RSPBOARDS/* = per station*/ * NR_RCUS_PER_RSPBOARD) // = 192 for int'l stations
, itsParset(parset)
, itsH5Filename(h5Filename)
, itsDumpRaw(dumpRaw)
{
	initCommonLofarAttributes(h5Filename);
	initTBB_RootAttributesAndGroups(stationName);
}

TBB_Station::~TBB_Station() {
	// Executed by the main thread after joined with all workers, so no need to lock or delay cancellation.
	try {
		itsStation.nofDipoles().value = itsStation.dipoles().size();
	} catch (DAL::DALException& exc) {
		LOG_WARN_STR("TBB: failed to set station NOF_DIPOLES attribute: " << exc.what());
	}
}

string TBB_Station::getRawFilename(unsigned rspID, unsigned rcuID) {
	string rawFilename = itsH5Filename;
	string rsprcuStr(formatString("_%03u%03u", rspID, rcuID));
	size_t pos = rawFilename.find('_', rawFilename.find('_'));
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
		dipole.initDipole(frame.header, itsParset, rawFilename, itsStation, itsH5Mutex);
	}

	dipole.processFrameData(frame, itsH5Mutex);
}

/*
 * Returns last mod date/time of filename, or current time of day if stat()ing fails,
 * in "YYYY-MM-DDThh:mm:ss.s" UTC format.
 * For FILEDATE attribute.
 */
string TBB_Station::getFileModDate(const string& filename) {
	struct timeval tv;
	struct stat st;
	int err;

	if ((err = stat(filename.c_str(), &st)) != 0) {
		gettimeofday(&tv, NULL); // If stat() fails, this is close enough to file mod date.
	} else {
		tv.tv_sec = st.st_mtime;
		tv.tv_usec = st.st_mtim.tv_nsec / 1000;
	}

	const char output_format[] = "%Y-%m-%dT%H:%M:";
	const char output_format_secs[] = "%04.1f"; // _total_ width of 4 of "ss.s"
	const char output_format_example[] = "YYYY-MM-DDThh:mm:ss.s";
	return formatFilenameTimestamp(tv, output_format, output_format_secs, sizeof(output_format_example));
}

// For timestamp attributes in UTC.
string TBB_Station::utcTimeStr(double time) {
	time_t timeSec = static_cast<time_t>(floor(time));
	unsigned long timeNSec = static_cast<unsigned long>(round( (time-floor(time))*1e9 ));

	char utc_str[50];
	struct tm tm = {0};
	gmtime_r(&timeSec, &tm);
	if (strftime(utc_str, sizeof(utc_str), "%Y-%m-%dT%H:%M:%S", &tm) == 0) {
		return "";
	}

	return formatString("%s.%09luZ", utc_str, timeNSec);
}

double TBB_Station::toMJD(double time) {
	// January 1st, 1970, 00:00:00 (GMT) equals 40587.0 Modify Julian Day number
	return 40587.0 + time / (24*60*60);
}

void TBB_Station::initCommonLofarAttributes(const string& filename) {
	itsH5File.groupType().value = "Root"; // TODO: set basic group fields that DAL checks automatically in DAL
	const string baseFilename(LOFAR::basename(filename));
	itsH5File.fileName() .value = baseFilename;
	itsH5File.fileDate() .value = getFileModDate(baseFilename);

	itsH5File.fileType() .value = "tbb";
	itsH5File.telescope().value = "LOFAR";

	itsH5File.projectID()   .value = itsParset.getString("Observation.Campaign.name");
	itsH5File.projectTitle().value = itsParset.getString("Observation.Campaign.title");
	itsH5File.projectPI()   .value = itsParset.getString("Observation.Campaign.PI");
	ostringstream oss;
	// Use ';' instead of ',' to pretty print, because ',' already occurs in names (e.g. Smith, J.).
	writeVector(oss, itsParset.getStringVector("Observation.Campaign.CO_I"), "; ", "", "");
	itsH5File.projectCOI()    .value = oss.str();
	itsH5File.projectContact().value = itsParset.getString("Observation.Campaign.contact");

	itsH5File.observationID() .value = formatString("%u", itsParset.observationID());

	itsH5File.observationStartUTC().value = utcTimeStr(itsParset.startTime());
	itsH5File.observationStartMJD().value = toMJD(itsParset.startTime());

	// The stop time can be a bit further than the one actually specified, because we process in blocks.
	unsigned nrBlocks = floor((itsParset.stopTime() - itsParset.startTime()) / itsParset.CNintegrationTime());
	double stopTime = itsParset.startTime() + nrBlocks * itsParset.CNintegrationTime();

	itsH5File.observationEndUTC().value = utcTimeStr(stopTime);
	itsH5File.observationEndMJD().value = toMJD(stopTime);

	itsH5File.observationNofStations().value = itsParset.nrStations(); // TODO: SS beamformer?
	// For the observation attribs, dump all stations participating in the observation (i.e. allStationNames(), not mergedStatioNames()).
	// This may not correspond to which station HDF5 groups will be written, but that is true anyway, regardless of any merging (e.g. w/ piggy-backed TBB).
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
	itsH5File.clockFrequency()    .value = itsParset.clockSpeed() / 1e6;
	itsH5File.clockFrequencyUnit().value = "MHz";

	itsH5File.antennaSet().value = itsParset.antennaSet();
	itsH5File.filterSelection().value = itsParset.getString("Observation.bandFilter");

	unsigned nrSAPs = itsParset.nrBeams();
	vector<string> targets(nrSAPs);

	for (unsigned sap = 0; sap < nrSAPs; sap++) {
		targets[sap] = itsParset.beamTarget(sap);
	}

	itsH5File.targets().value = targets;

#ifdef HAVE_PKVERSION
	itsH5File.systemVersion().value = StorageVersion::getVersion();
#else
#warning SYSTEM_VERSION attribute cannot be written correctly into HDF5 output file
	itsH5File.systemVersion().value = "0.909";
#endif

	itsH5File.docName()   .value = "ICD 1: TBB Time-Series Data";
	itsH5File.docVersion().value = "2.02.25";

	itsH5File.notes().value = "";
}

// The writer creates one HDF5 file per station, so create only one Station Group here.
void TBB_Station::initTBB_RootAttributesAndGroups(const string& stName) {
	int operatingMode = itsParset.getInt("Observation.TBB.TBBsetting.operatingMode");
	if (operatingMode == 1) {
		itsH5File.operatingMode().value = "transient";
	} else if (operatingMode == 2) {
		itsH5File.operatingMode().value = "spectral";
	} else {
		itsH5File.operatingMode().value = "unknown"; // should not happen, parset assumed to be ok
	}

	itsH5File.nofStations().value = 1u;

	// Find the station name we are looking for ("CS001" == "CS001HBA0") and retrieve its pos using the found idx.
// TODO: maybe this is wrong: we need the position of CS001HBA0, not of CS001. Also for the dipole pos which is wrt this pos.
	vector<double> stPos;

	vector<string> obsStationNames(itsParset.allStationNames()); // can also contain "CS001HBA0"
	vector<string>::const_iterator nameIt(obsStationNames.begin());

	vector<double> stationPositions(itsParset.positions()); // len must be (is generated as) 3x #stations
	vector<double>::const_iterator posIt(stationPositions.begin());

	const size_t stNameLen = 5;
	for ( ; nameIt != obsStationNames.end(); ++nameIt, posIt += 3) {
		if (nameIt->substr(0, stNameLen) == stName) {
			break;
		}
	}
	if (nameIt != obsStationNames.end() // found?
				&& posIt < stationPositions.end()) { // only fails if Parset provided broken vectors
		stPos.assign(posIt, posIt + 3);
	} else { // not found or something wrong; create anyway or we lose data
		stPos.assign(3, 0.0);
	}
	itsStation.create();
	initStationGroup(itsStation, stName, stPos);

	// Trigger Group
	DAL::TBB_Trigger tg(itsH5File.trigger());
	tg.create();
	initTriggerGroup(tg);
}

void TBB_Station::initStationGroup(DAL::TBB_Station& st, const string& stName, const vector<double>& stPosition) {
	st.groupType().value = "StationGroup";
	st.stationName().value = stName;

	st.stationPosition().value = stPosition;
	st.stationPositionUnit().value = "m";
	st.stationPositionFrame().value = itsParset.positionType(); // "ITRF"

	// digital beam(s)
	unsigned nBeams = itsParset.nrBeams();
	if (nBeams > 0) { // What if >1 station beams? For now, only write beam 0.
		st.beamDirection().value = itsParset.getBeamDirection(0);
		st.beamDirectionFrame().value = itsParset.getBeamDirectionType(0);
	} else { // No beam (known or at all), so set null vector
		st.beamDirection().value = vector<double>(2, 0.0);
		st.beamDirectionFrame().value = "ITRF";
	}

	st.beamDirectionUnit().value = "m";

	st.clockOffset().value = itsParset.clockCorrectionTime(stName); // TODO: check if stName is as expected (it must be incl HBA0, HBA1, LBA etc, e.g. "CS002HBA0"); returns 0.0 if not avail
	st.clockOffsetUnit().value = "s";

	//st.nofDipoles.value is set at the end (destr)
}

void TBB_Station::initTriggerGroup(DAL::TBB_Trigger& tg) {
	tg.groupType().value = "TriggerGroup";
	tg.triggerType().value = "Unknown";
	tg.triggerVersion().value = 0; // There is no trigger algorithm info available to us yet.

	// Trigger parameters (how to decide if there is a trigger; per obs)
// TODO: if the parset doesn't have these, it'll throw APSException and we will not create the station group and not process the data
	tg.paramCoincidenceChannels().value = itsParset.getInt   ("Observation.ObservationControl.StationControl.TBBControl.NoCoincChann");
	tg.paramCoincidenceTime()    .value = itsParset.getDouble("Observation.ObservationControl.StationControl.TBBControl.CoincidenceTime");
	tg.paramDirectionFit()       .value = itsParset.getString("Observation.ObservationControl.StationControl.TBBControl.DoDirectionFit");
	tg.paramElevationMin()       .value = itsParset.getDouble("Observation.ObservationControl.StationControl.TBBControl.MinElevation");
	tg.paramFitVarianceMax()     .value = itsParset.getDouble("Observation.ObservationControl.StationControl.TBBControl.MaxFitVariance");

	// Trigger data (per trigger)
	// N/A atm

	/*
	 * It is very likely that the remaining (optional) attributes and the trigger alg
	 * will undergo many changes. TBB user/science applications will have to retrieve and
	 * set the remaining fields "by hand" for a while using e.g. DAL by checking and
	 * specifying each attribute name presumed available.
	 * Until it is clear what is needed and available, this cannot be standardized.
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

#if __BYTE_ORDER == __BIG_ENDIAN
// hton[ls],ntoh[ls] on big endian are no-ops, so we have to provide the byte swaps.
uint16_t TBB_StreamWriter::littleNativeBSwap(uint16_t val) const {
	return __bswap_16(val); // GNU ext
}
uint32_t TBB_StreamWriter::littleNativeBSwap(uint32_t val) const {
	return __bswap_32(val); // GNU ext
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
uint16_t TBB_StreamWriter::littleNativeBSwap(uint16_t val) const {
	return val;
}
uint32_t TBB_StreamWriter::littleNativeBSwap(uint32_t val) const {
	return val;
}
#endif

void TBB_StreamWriter::frameHeaderLittleNativeBSwap(TBB_Header& header) const {
	//header.seqNr              = littleNativeBSwap(header.seqNr); // neither intended nor useful for us
	header.time               = littleNativeBSwap(header.time);
	header.sampleNr           = littleNativeBSwap(header.sampleNr); // also swaps header.bandsliceNr
	header.nOfSamplesPerFrame = littleNativeBSwap(header.nOfSamplesPerFrame);
	header.nOfFreqBands       = littleNativeBSwap(header.nOfFreqBands);
	//header.spare              = littleNativeBSwap(header.spare); // unused
	header.crc16              = littleNativeBSwap(header.crc16);
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
 * This code is based on the Python ref/test code from Gijs Schoonderbeek. It does not do a std crc16 (AFAICS). (VHDL hw code from generator at www.easics.com)
 * It assumes that the seqNr field, ((uint32_t*)buf)[1], has been zeroed.
 * Do not call this function with len < 1; reject the frame earlier.
 */
uint16_t TBB_StreamWriter::crc16tbb(const uint16_t* buf, size_t len) const {
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
 * This code is based on the Python ref/test code from Gijs Schoonderbeek. It does not do a std crc32 (AFAICS). (VHDL hw code from generator at www.easics.com)
 * It computes a 32 bit result, but the buf arg is of uint16_t*.
 * Do not call this function with len < 2; reject the frame earlier.
 */
uint32_t TBB_Dipole::crc32tbb(const uint16_t* buf, size_t len) const {
	uint32_t CRC            = 0;
	const uint64_t CRC_poly = 0x104C11DB7ULL;
	const uint16_t bits     = 16;
	uint64_t data           = 0;
	const uint64_t CRCDIV   = (CRC_poly & 0x7fffffffffffULL) << 15;

	data = buf[0];
	data = data & 0x7fffffffffffULL;
	data = data << 16;
	data = data + buf[1];
	data = data & 0x7fffffffffffULL;
	data = data << 16;
	uint32_t i = 2;
	for ( ; i < len-2; i++) {
		data = data + buf[i];
		for (uint32_t j = 0; j < bits; j++) {
			if (data & 0x800000000000ULL) {
				data = data ^ CRCDIV;
			}
			data = data & 0x7fffffffffffULL;
			data = data << 1;
		}
	}

	/*
	 * Do the 32 bit checksum separately, without the '& 0xfff' masking.
	 * Process the two 16 bit halves in reverse order (no endian swap!), but keep the i < len cond.
	 */
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
void TBB_StreamWriter::processHeader(TBB_Header& header, size_t recvPayloadSize) const {
	frameHeaderLittleNativeBSwap(header); // no-op on little endian

	header.seqNr = 0; // for the crc; don't save/restore it as we don't need this field
	uint16_t csum = crc16tbb(reinterpret_cast<uint16_t*>(&header), sizeof(header) / sizeof(uint16_t));
	if (csum != 0) {
		/*
		 * Spec says each frame has the same fixed length, so the previous values are a good base guess if the header crc fails.
		 * But it is not clear if it is worth the effort. For now, drop the frame.
		 */
		throw TBB_MalformedFrameException("crc16: " + header.toString());
	}

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
/*		if (recvPayloadSize < sizeof(int16_t)) {
			throw TBB_MalformedFrameException("dropping too small TBB spectral frame");
		}*/
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
			itsFreeQueue.append(frame);
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

		itsFreeQueue.append(frame);
	}
}

//////////////////////////////////////////////////////////////////////////////

TBB_Writer::TBB_Writer(const vector<string>& inputStreamNames, const Parset& parset,
		const string& outDir, bool dumpRaw, const string& logPrefix)
: itsParset(parset)
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

	for (unsigned i = 0; i < inputStreamNames.size(); i++) {
		itsStreamWriters.push_back(new TBB_StreamWriter(*this, inputStreamNames[i], logPrefix));
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
	ScopedLock sl(itsStationsMutex);
	map<unsigned, TBB_Station* >::iterator stIt(itsStations.find(header.stationID));
	if (stIt == itsStations.end()) {
		// Create new station with HDF5 file and station HDF5 group.
		string stationName(DAL::stationIDToName(header.stationID));
		string h5Filename(createNewTBB_H5Filename(header, stationName));
		TBB_Station* station = new TBB_Station(stationName, itsParset, h5Filename, itsDumpRaw);
		return itsStations.insert(make_pair(header.stationID, station)).first->second;
	}

	return stIt->second;
}

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
	while ((access(h5Filename.c_str(), F_OK) == 0 || errno != ENOENT) && itsRunNr < 1000) {
		itsRunNr += 1;
		runNrStr = formatString("R%03u_", itsRunNr);
		h5Filename.replace(pos, runNrStr.size(), runNrStr);
	}
	if (itsRunNr == 1000) { // run number is supposed to fit in 3 digits
		throw StorageException("failed to generate new .h5 filename after 1000 filenames tried.");
	}

	return h5Filename;
}

time_t TBB_Writer::getTimeoutStampSec(unsigned streamWriterNr) {
	return itsStreamWriters[streamWriterNr]->getTimeoutStampSec();
}

} // namespace RTCP
} // namespace LOFAR

