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

#include <cstddef>
#include <unistd.h>

#include <iostream>

#include <Common/LofarConstants.h>
#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#include <Common/StringUtil.h>
#include <Stream/SocketStream.h>
//#include <ApplCommon/AntField.h>

#include <Interface/Stream.h>

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

	SmartPtr<char, SmartPtrDeleteArray<char> > date_str = new char[output_size];
	size_t nwritten = strftime(date_str, output_size, output_format, &tm);
	if (nwritten == 0) {
		data_str[0] = '\0';
	}
	/*int nprinted = */snprintf(date_str + nwritten, sizeof(output_format) - nwritten, output_format_secs, secs);

	string date(date_str);
	return date;
}

//////////////////////////////////////////////////////////////////////////////

TBB_Dipole::TBB_Dipole() : itsDataset(NULL), itsDatasetLen(0) {
}

TBB_Dipole::~TBB_Dipole() {
	/*
	 * Set dataset len (if ext raw) and DATA_LENGTH and FLAG_OFFSETS attributes at the end.
	 * Don't lock the station file: this is done by the main thread after joining with all workers.
	 */
	if (usesExternalDataFile()) {
		try {
			vector<ssize_t> dims(1, itsDatasetLen); // TODO: get rid of this in DAL
			itsDataset->resize(dims);
		} catch (DAL::DALException& exc) {
			LOG_WARN("TBB: failed to resize HDF5 dipole dataset to external data size");
		}
	}
	try {
		itsDataset->dataLength().value = itsDatasetLen; //TODO: -> unsigned long?
	} catch (DAL::DALException& exc) {
		LOG_WARN("TBB: failed to set dipole DATA_LENGTH attribute");
	}
	try {
		itsDataset->flagOffsets().value = itsFlagOffsets;
	} catch (DAL::DALException& exc) {
		LOG_WARN("TBB: failed to set dipole FLAG_OFFSETS attribute");
	}

	delete itsDataset;
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

void TBB_Dipole::writeFrameData(const TBB_Frame& frame, Mutex& h5Mutex) {
	off_t offset = (frame.header.time - itsTime0) * itsSampleFreq + frame.header.sampleNr - itsSampleNr0;
	if (offset >= itsDatasetLen) {
		/*
		 * If writing around HDF5, there is no need to lock. Set the HDF5 dataset size at the end (destr).
		 * If writing through HDF5, we have to lock and the HDF5 dataset size is updated by HDF5.
		 */
		if (usesExternalDataFile()) {
			if (offset > itsDatasetLen) {
				rawOut.seekp(offset * sizeof(data[0])); // skip space of lost frame
			}
			rawOut.write(reinterpret_cast<const char*>(data), static_cast<size_t>(frame.header.nOfSamplesPerFrame) * sizeof(data[0]));
		} else {
			vector<size_t> pos(1, offset); // TODO: get rid of this vector stuff in DAL
			vector<ssize_t> dsNewDims(1, offset + frame.header.nOfSamplesPerFrame);

			ScopedLock h5Lock(h5Mutex);
			itsDataset.resize(dsNewDims);
			itsDataset.set1D(pos, frame.header.nOfSamplesPerFrame, data);
		}

		/*
		 * Flag lost frame(s) (assume no out-of-order, see below). Assumes all frames have the same nr of samples.
		 * Note: this cannot detect lost frames at the end of a dataset.
		 */
		for (unsigned lostOffset = itsDatasetLen; lostOffset < offset; lostOffset += frame.header.nOfSamplesPerFrame) {
			itsFlagOffsets.push_back(static_cast<unsigned>(lostOffset));
		}

		itsDatasetLen = offset + frame.header.nOfSamplesPerFrame;
	} else { // Out-of-order or duplicate frames are very unlikely in the LOFAR TBB setup.
		// Let us know if it ever happens, then we will do something.
		LOG_WARN_STR("TBB: Dropped out-of-order or duplicate TBB frame at " << header.stationID <<
				" " << header.rspID << " " << header.rcuID << " " << offset);
	}
}

// TODO: cross-check units for StationGroup and DipoleDataset with casacore (see Ger's e-mail)
void TBB_Dipole::initTBB_DipoleDataset(const TBB_Header& header, const Parset& parset, const string& rawFilename,
			DAL::TBB_Station& station, Mutex& h5Mutex) {
	itsDataset = new DAL::TBB_DipoleDataset(station.dipole(header.stationID, header.rspID, header.rcuID));

	ScopedLock h5OutLock(h5Mutex);

	itsDataset->groupType().value = "DipoleDataset";
	itsDataset->stationID().value = header.stationID;
	itsDataset->rspID().value = header.rspID;
	itsDataset->rcuID().value = header.rcuID;

	itsDataset->sampleFrequencyValue().value = itsSampleFreq;
	itsDataset->sampleFrequencyUnit().value = "MHz";

	itsDataset->time().value = header.time; // in seconds. Note: may have been corrected earlier
	itsDataset->sampleNumber().value = header.sampleNr; // only relevant for transient mode
	itsDataset->samplesPerFrame().value = header.nOfSamplesPerFrame;
	//itsDataset->dataLength().value is set at the end
	//itsDataset->flagOffsets().value is set at the end
	itsDataset->nyquistZone().value = parset.nyquistZone(); // unsigned nr indicating filter band
	//itsDataset->ADC2Voltage().value = ???; // optional; TODO???

	// Cable delays (optional) can be implemented from StaticMetaData, but there is no parsing code yet. TODO
	//itsDataset->cableDelay().value = ???;
	//itsDataset->cableDelayUnit().value = 's';

	//itsDataset->dipoleCalibrationDelayValue().value = ???; // TODO: Pim can compute this from the GainCurve below
	//itsDataset->dipoleCalibrationDelayUnit().value = 's';
	//itsDataset->dipoleCalibrationGainCurve().value = ???; // TODO: where to get this?

	//itsDataset->feed().value(); // optional string

/*	//for antenna, see MeasurementSetFormat.cc. Last resort, look at LCS/Common/ApplCommon/AntField.h , but be careful with HBA/LBA modes etc.
	// must be in absolute ITRF (not relative to station)
	itsDataset->antennaPositionValue().value = ???;
	itsDataset->antennaPositionUnit().value = ???; // TODO: ???
	itsDataset->antennaPositionFrame().value = parset.positionType(); // returns "ITRF" (currently)
	itsDataset->antennaNormalVector().value = ???;
	itsDataset->antennaRotationMatrix().value = ???; // store 9 vals, per row. (row-minor)
*/
/* optional; but maybe fill these in anyway; tiles/HBA
	itsDataset->tileBeamValue().value = ???;
	itsDataset->tileBeamUnit().value = ???;
	itsDataset->tileBeamFrame().value = ???;
	itsDataset->tileBeamDipoles().value = ???;

	itsDataset->tileCoefUnit().value = ???;
	itsDataset->tileBeamCoefs().value = ???;

	itsDataset->tileDipolePositionValue().value = ???;
	itsDataset->tileDipolePositionUnit().value = ???;
	itsDataset->tileDipolePositionFrame().value = ???;

	itsDataset->dispersionMeasureValue().value = parset.dispersionMeasure(); // passes default parms unsigned beam=0, unsigned pencil=0
	itsDataset->dispersionMeasureUnit().value = "s"; // TODO: check if unit is seconds before enabling
*/

	// Create 1-dim, unbounded (-1) dataset. 
	// Override endianess. TBB data is always stored little endian and also received as such, so written as-is on any platform.
	vector<ssize_t> dsDims(1, 0);
	vector<ssize_t> dsMaxDims(1, -1);
	itsDataset->create(dsDims, dsMaxDims, outFilenameRaw, ds->LITTLE);
}

//////////////////////////////////////////////////////////////////////////////

TBB_Station::TBB_Station(const string& stationName, const Parset& parset, const string& h5Filename)
: itsH5File(DAL::TBB_File(h5Filename, DAL::TBB_File::CREATE))
, itsStation(itsH5File, itsH5File.station(stationName))
, itsDipoles(vector<TBB_Station>(MAX_RSPBOARDS/* = per station*/ * NR_RCUS_PER_RSPBOARD)) // = 192 for int'l stations
, itsParset(parset)
, itsH5Filename(h5Filename)
{
	initCommonLofarAttributes(h5Filename);
	initTBB_RootAttributesAndGroups();
}

TBB_Station::~TBB_Station() {
	// Executed by the main thread after joined with all workers, so no need to lock.
	try {
		itsStation.nofDipoles.value = itsStation.dipoles.size();
	} catch (DAL::DALException& exc) {
		LOG_WARN("TBB: failed to set station NOF_DIPOLES attribute");
	}
}

TBB_Dipole& TBB_Station::getDipole(const TBB_Header& header) {
	// Guard against bogus incoming rsp/rcu IDs with at().
	return itsDipoles.at(header.rspID * NR_RCUS_PER_RSPBOARD + header.rcuID);
}

/*
 * Returns last mod date/time of filename or current time of day if stat()ing
 * filename fails, in "YYYY-MM-DDThh:mm:ss.s" UTC format.
 * For FILEDATE attribute.
 */
string TBB_Station::getFileModDate(const string& filename) {
	struct timeval tv;
	struct stat st;
	int err;

	err = stat(filename.c_str(), &st);
	if (err != 0) {
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
	if (strftime( utc_str, sizeof(utc_str), "%Y-%m-%dT%H:%M:%S", &tm ) == 0) {
		return "";
	}

	return formatString("%s.%09luZ", utc_str, timeNSec);
}

double TBB_Station::toMJD(double time) {
	// January 1st, 1970, 00:00:00 (GMT) equals 40587.0 Modify Julian Day number
	return 40587.0 + time / (24*60*60);
}

void TBB_Station::initCommonLofarAttributes(const string& filename) {
	DAL::TBB_File& file = itsH5File;
	Parset& parset = itsParset;

	file.groupType().value = "Root";
	const string baseFilename(basename(filename));
	file.fileName().value = baseFilename;
	file.fileDate().value = getFileModDate(baseFilename);
	file.fileType().value = "tbb"; // TODO: this is obviously tbb specific; the rest of this function is not. -> DAL; also the next one

	file.telescope().value = "LOFAR";
	file.observer() .value = "unknown"; // TODO: name(s) of the observer(s); check new bf writer

	file.projectID()     .value = parset.getString("Observation.Campaign.name");
	file.projectTitle()  .value = parset.getString("Observation.Campaign.title");
	file.projectPI()     .value = parset.getString("Observation.Campaign.PI");
	ostringstream oss;
	// Use ';' instead of ',' to pretty print, because ',' already occurs in names (e.g. Smith, J.).
	writeVector(oss, parset.getStringVector("Observation.Campaign.CO_I"), "; ", "", "");
	file.projectCOI()    .value = oss.str();
	file.projectContact().value = parset.getString("Observation.Campaign.contact");

/*	// TODO: add getString() and getStringVector() routines to RTCP Parset (sub)class
	file.projectID()     .value = parset.observationName(); // TODO: check if Name -> ID
	file.projectTitle()  .value = parset.observationTitle();
	file.projectPI()     .value = parset.observationPI();
	file.projectCOI()    .value = parset.observationCOI(); // TODO: see above
	file.projectContact().value = parset.observationContact();
*/
	//file.observationID() .value = str(format("%s") % parset.observationID());
	file.observationID() .value = formatString("%u", parset.observationID());

	file.observationStartUTC().value = utcTimeStr(parset.startTime());
	file.observationStartMJD().value = toMJD(parset.startTime());

	// The stop time can be a bit further than the one actually specified, because we process in blocks. TODO: check bf writer
	const size_t nBlocks = ceil((parset.stopTime() - parset.startTime()) / parset.CNintegrationTime());
	const double stopTime = parset.startTime() + nBlocks * parset.CNintegrationTime();
	file.observationEndUTC().value = timeStr(stopTime);
	file.observationEndMJD().value = toMJD(stopTime);

	file.observationNofStations().value = parset.nrStations(); // TODO: SS beamformer?

	// For the observation attribs, dump all stations participating in the observation (i.e. allStationNames(), not mergedStatioNames()).
	// This may not correspond to which station HDF5 groups will be written, but that is true anyway, regardless of any merging (e.g. w/ piggy-backed TBB).
	file.observationStationsList().value = parset.allStationNames(); // TODO: SS beamformer?

	const vector<double> subbandCenterFrequencies(parset.subbandToFrequencyMapping());
	double min_centerfrequency = *min_element(subbandCenterFrequencies.begin(), subbandCenterFrequencies.end());
	double max_centerfrequency = *max_element(subbandCenterFrequencies.begin(), subbandCenterFrequencies.end());
	double sum_centerfrequencies = accumulate(subbandCenterFrequencies.begin(), subbandCenterFrequencies.end(), 0.0);
	double subbandBandwidth = parset.sampleRate();

	file.observationFrequencyMin()   .value = (min_centerfrequency - subbandBandwidth / 2) / 1e6;
	file.observationFrequencyCenter().value = sum_centerfrequencies / subbandCenterFrequencies.size();
	file.observationFrequencyMax()   .value = (max_centerfrequency + subbandBandwidth / 2) / 1e6;
	file.observationFrequencyUnit()  .value = "MHz";

	file.observationNofBitsPerSample().value = parset.nrBitsPerSample();

	file.clockFrequency()    .value = parset.clockSpeed() / 1e6;
	file.clockFrequencyUnit().value = "MHz";

	file.antennaSet().value = parset.antennaSet(); // TODO: does this provide the strings from the ICD const tables?!?

	file.filterSelection().value = parset.getString("Observation.bandFilter");

	//file.target().value = ???; // TODO: not in bf h5 writer; as array of strings?
	// Code from bf MS Writer:
	//vector<string> targets(parset.getStringVector("Observation.Beam[" + toString(subarray) + "].target"));

	/*unexp
	 * Set release string, e.g. "LOFAR-Release-1.0".
	 * Only works if writer is rebuilt and reinstalled on every release, so force cmake deps. (Means on build failure, everyone will know soon!)
	 */
/*
#if defined HAVE_PKVERSION
	const string releaseStr(TBB_WriterVersion::getVersion());
#else // TODO: test both cases
	ostringstream vss;
	string type("brief");
	Version::show<TBB_WriterVersion>(vss, "TBB_Writer",  type);
	const string releaseStr(vss.str());
#endif
	file.systemVersion().value = versionStr;
*/
	//file.pipelineName().value = ???; // TODO: remove from DAL; for offline/LTA only
	//file.pipelineVersion().value = ???; // idem

	//file.ICDNumber().value = "1"; //TODO: this is wrong if we provide a spec doc, and redundant if we already have 'tbb'; LOFAR-USG-ICD-001: TBB Time-Series Data"); // make it 1 or 001, or drop this, it's in file type (data product type)
	//file.ICDVersion().value = "2.02.15"; // patch number is useless; and this is wrong if we provide a spec doc
	//file.notes().value = ""; // TODO: needed?
	// TODO: No TBB_SysLog group anymore, add it back to CommonLofarAttributes instead of Notes?
	/*if (parset.fakeInputData()) { // ignore checkFakeInputData(), always annotate the data product if data is fake
		syslog.append("input data is fake!");
	}*/
}

void TBB_Station::initTBB_RootAttributesAndGroups() {
	// The writer creates one HDF5 file per station, so create only one Station Group here.
	//itsH5File.operatingMode().value = "transient"; // TODO: add string mode field: "transient" or "spectral". Maybe it's Observation.TBB.TBBsetting.operatingMode = 1
	itsH5File.nofStations.value = 1;

	// Find the station name we're looking for ("CS001" == "CS001HBA0") and retrieve it's pos using the found idx.
	vector<double> stPos;

	vector<string> obsStationNames(parset.allStationNames()); // can also contain "CS001HBA0"
	vector<string>::const_iterator nameIt(obsStationNames.begin());

	vector<double> stationPositions(parset.positions()); // len must be (is generated as) 3x #stations
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
	station.create();
	setStationGroup(station, stPos);

	// Trigger Group
	DAL::TBB_Trigger tg(file.triggerData()); // TODO: triggerData() -> trigger()
	tg.create();
	setTriggerGroup(tg);
}

// Populates the Station group. Note: Does _not_ set the dipole datasets.
void TBB_Station::setStationGroup(DAL::TBB_Station& st, const vector<double>& stPosition) {
	//st.groupType().value = "StationGroup"; // TODO: should have been set by DAL class
	//st.stationName().value = stName; // TODO: should have been set by DAL class

	st.stationPositionValue().value = stPosition;
	//st.stationPositionUnit().value = ???; // TODO: ??? array of strings? (also at beamDirection, but not at antennaPosition)
	st.stationPositionFrame().value = parset.positionType(); // returns "ITRF"

	// TODO: Also see bf writer MS makeBeamTable() kind of function. In the future, also indicates broken tiles/dipoles w/ timestamp.
	// TODO: ITRF -> ITRF2008...?
	if (parset.haveAnaBeam()) { // HBA // TODO: AnaBeam vs Beam?
		st.beamDirectionValue().value = parset.getAnaBeamDirection(); // always for beam 0
		st.beamDirectionFrame().value = parset.getAnaBeamDirectionType(); // idem
	} else {
		unsigned nBeams = parset.nrBeams(); // TODO: What if >1 station beams? Now, only write beam0. Probably irrel for now, because of AnaBeam (HBA).
		if (nBeams > 0) {
			st.beamDirectionValue().value = parset.getBeamDirection(0);
			st.beamDirectionFrame().value = parset.getBeamDirectionType(0); // TODO: fix getBeamDirectionType() sprintf -> snprintf (check all parset funcs)
		} else { // No beam (known or at all), so set null vector
			vector<double> noBeamDirVal(2, 0.0);
			st.beamDirectionValue().value = noBeamDirVal;
			st.beamDirectionFrame().value = "ITRF";
		}
	}
	//st.beamDirectionUnit().value = ???; // TODO: ??? beam dir = 2 angles. array of string??? "radians"

	st.clockOffsetValue().value = parset.clockCorrectionTime(stName); // TODO: check if stName is as expected; returns 0.0 if not avail
	st.clockOffsetUnit().value = "s";
	//st.triggerOffset().value = ???; // TODO: remove from DAL
	//st.nofDipoles.value is set at the end (destr)

	// Create dipole datasets when the first datagram for that dipole has been received.
}

void TBB_Station::setTriggerGroup(DAL::TBB_Trigger& tg) {
	tg.groupType().value = "TriggerGroup";
//	tg.triggerType().value = "Unknown"; // We don't get this or any other trigger data yet, so do the minimum.
//	tg.triggerVersion().value = 0; // There is no trigger alg impl yet.

	// TODO: put these into DAL, because we can set them
	// Trigger parameters (how to decide if there is a trigger) (per obs)
//	tg.paramCoincidenceChannels().value = parset.tbbNumberOfCoincidenceChannels();	// int(->unsigned); from Observation.ObservationControl.StationControl.TBBControl.NoCoincChann
//	tg.paramCoincidenceTime()    .value = parset.tbbCoincidenceTime();				// double; from Observation.ObservationControl.StationControl.TBBControl.CoincidenceTime
//	tg.paramDirectionFit()       .value = parset.tbbDoDirectionFit();				// string; from Observation.ObservationControl.StationControl.TBBControl.DoDirectionFit
//	tg.paramElevationMin()       .value = parset.tbbMinElevation();					// double; from Observation.ObservationControl.StationControl.TBBControl.MinElevation
//	tg.paramFitVarianceMax()     .value = parset.tbbMaxFitVariance();				// double; Observation.ObservationControl.StationControl.TBBControl.MaxFitVariance
	// unused: Observation.ObservationControl.StationControl.TBBControl.ParamExtension = []

	// Trigger data (per trigger)
	// N/A atm

	// TODO: (this is already done for any attribute in any group?)
	/* From an e-mail by Pim Schellart:
	 * To actually implement this, given the likely need for many
	 * changes, it is probably easiest to provide only one method for the
	 * TriggerGroup that allows setting (keyword, value) pairs and one to
	 * retrieve an attribute. That way we can just get/set whatever we need
	 * in that group without having to bother you with it.
	 */
}

//////////////////////////////////////////////////////////////////////////////

TBB_Writer::TBB_StreamWriter::TBB_StreamWriter(const string& inputStreamName,
		const string& logPrefix)
: itsInputStreamName(inputStreamName)
, itsLogPrefix(logPrefix)
{
	itsFrameBuffers = new TBB_Frame[nrFrameBuffers];
	itsReceiveQueue.reserve(nrFrameBuffers);
	for (unsigned i = nrFrameBuffers; i > 0; ) {
		itsFreeQueue.append(&itsFrameBuffers[--i]);
	}

	gettimeofday(&itsTimeoutStamp.tv, NULL);

	itsOutputThread = NULL;
	try {
		itsOutputThread = new Thread(this, /*&TBB_StreamWriter::*/mainOutputLoop, logPrefix + "OutputThread: ");
		itsInputThread  = new Thread(this, /*&TBB_StreamWriter::*/mainInputLoop,  logPrefix + "InputThread: ");
	} catch (...) {
		if (itsOutputThread != NULL) {
			itsReceiveQueue.append(NULL); // tell output thread to stop
			delete itsOutputThread;
		}
		delete[] itsFrameBuffers;
		throw;
	}
}

TBB_Writer::TBB_StreamWriter::~TBB_StreamWriter() {
	// Only cancel input thread. It will tell the output thread.
	itsInputThread->cancel();

	delete itsInputThread;
	delete itsOutputThread;
	delete[] itsFrameBuffers;
}

time_t TBB_StreamWriter::getTimeoutStampSec() {
	return itsTimeoutStamp.tv_sec; // racy read (and no access once guarantee)
}

#if __BYTE_ORDER == __BIG_ENDIAN
// hton[ls],ntoh[ls] on big endian are no-ops, so we have to provide the byte swaps.
uint16_t TBB_StreamWriter::littleNativeBSwap(uint16_t val) {
	return __bswap_16(val); // GNU ext
}
uint32_t TBB_StreamWriter::littleNativeBSwap(uint32_t val) {
	return __bswap_32(val); // GNU ext
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
uint16_t TBB_StreamWriter::littleNativeBSwap(uint16_t val) {
	return val;
}
uint32_t TBB_StreamWriter::littleNativeBSwap(uint32_t val) {
	return val;
}
#endif

void TBB_StreamWriter::frameHeaderLittleNativeBSwap(TBB_Header& header) {
	//header.seqNr              = littleNativeBSwap(header.seqNr); // neither intended nor useful for us
	header.time               = littleNativeBSwap(header.time);
	header.sampleNr           = littleNativeBSwap(header.sampleNr); // also swaps header.bandsliceNr
	header.nOfSamplesPerFrame = littleNativeBSwap(header.nOfSamplesPerFrame);
	header.nOfFreqBands       = littleNativeBSwap(header.nOfFreqBands);
	//header.spare              = littleNativeBSwap(header.spare); // unused
	header.crc16              = littleNativeBSwap(header.crc16);
}

void TBB_StreamWriter::correctTransientSampleNr(TBB_Header& header) {
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
uint16_t TBB_StreamWriter::crc16tbb(const uint16_t* buf, size_t len) {
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
uint32_t TBB_StreamWriter::crc32tbb(const uint16_t* buf, size_t len) {
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
void TBB_StreamWriter::processHeader(TBB_Header& header, size_t recvPayloadSize) {
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

	struct TbbInStream { // mainly for the destructor
		Stream* itsStream;
		TbbInStream() :
			inStream(createStream(itsInputStreamName, true)) {
		}
		~TbbInStream() {
			delete inStream;
			try {
				itsReceiveQueue.append(NULL); // always notify output thread at exit of no more data
			} catch (...) {
			}
		}
	} stream;

	while (1) {
		bool droppedSmall = false;
		TBB_Frame* frame;

		try {
			frame = itsFreeQueue.remove();

			size_t datagramSize = stream.itsStream->tryRead(frame, sizeof(*frame));

			// Notify master that we are still busy. (Racy, see TS decl)
			gettimeofday(&itsTimeoutStamp.tv, NULL);

			if (datagramSize < sizeof(TBB_Header)) {
				throw TBB_MalformedFrameException("dropping too small TBB frame");
			}
			processHeader(frame.header, datagramSize - sizeof(TBB_Header));

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
		}/* catch (...) { // Cancellation exc happens at exit. Nothing to do, so disabled. Otherwise, must rethrow.
			throw;
		}*/
	}
}

void TBB_StreamWriter::processPayload(const TBB_Frame& frame) {
	SmartPtr<TBB_Station> station(getStation(frame.stationID, frame.header));

	// Each dipole stream is sent to a single port (thread), so no need to grab a mutex here to avoid double init.
	// Do pass a ref to the h5 mutex for when writing into the HDF5 file.
	TBB_Dipole& dipole(station.getDipole(header));
	if (!dipole.isInitialized()) {
		string rawFilename("");
		if (itsDumpRaw) {
			rawFilename = itsH5Filename;
			string rsprcuStr(formatString("_%03u%03u", frame.header.rspID, frame.header.rcuID));
			size_t pos = rawFilename.find('_', rawFilename.find('_'));
			rawFilename.insert(pos, rsprcuStr); // insert _rsp/rcu IDs after station name (2nd '_')
			rawFilename.resize(rawFilename.size() - (sizeof(".h5") - 1);
			rawFilename.append(".raw");
		}
		dipole.initDipole(frame.header, itsParset, rawFilename, itsStation, itsH5Mutex);
	}

	// Verify data checksum.
	if (frame.header.nOfFreqBands == 0) { // transient mode
		uint32_t csum = crc32tbb(reinterpret_cast<const uint16_t*>(frame.payload.data), frame.header.nOfSamplesPerFrame + 2/*=crc32*/);
		if (csum != 0) {
			/*
			 * On a data checksum error 'flag' this offset, but still store the data.
			 * Lost frame vs crc error can be seen from the data: lost means a zero block.
			 */
			dipole.itsFlagOffsets.push_back(offset);

			uint32_t crc32 = *reinterpret_cast<const uint32_t*>(&frame.payload.data[frame.header.nOfSamplesPerFrame]); // TODO: 2 lines tmp
			LOG_INFO_STR(itsLogPrefix << "crc32: " << frame.header.toString() << " " << crc32);
		}
	} else { // spectral mode
		//uint32_t bandNr  = frame.header.bandsliceNr & TBB_BAND_NR_MASK;
		//uint32_t sliceNr = frame.header.bandsliceNr >> TBB_SLICE_NR_SHIFT;
		// TODO: prepare to store, but spectral output format unclear.
	}

	dipole.writeFrameData(frame, itsH5Mutex);
}

void TBB_StreamWriter::mainOutputLoop() {
	const unsigned maxErrors = 16;
	unsigned nrErrors = 0; // i.e. per output thread per dump

	while (1) {
		TBB_Frame* frame;
		try {
			frame = itsReceived.remove()
			if (frame == NULL) {
				break;
			}

#ifdef PRINT_QUEUE_LEN
			LOG_INFO_STR(itsLogPrefix << "recvqsz=" << itsReceiveQueue.size());
#endif

			processPayload(*frame);
		} catch (exception& exc) {
			// e.g. std::out_of_range for at() on bad rsp/rcu ID,
			// DALException, or SystemCallException on failed to open file.
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
	for (unsigned i = 0; i < inputStreamNames.size(); i++) {
		string streamLogPrefix(logPrefix + "stream " + inputStreamNames[i].c_str() + ": ");
		itsStreamWriters.push_back(new StreamWriter(inputStreamNames[i], streamLogPrefix);
	}
}

TBB_Writer::~TBB_Writer() {
}

time_t TBB_Writer::getTimeoutStampSec(unsigned streamWriterNr) {
	return itsStreamWriters[streamWriterNr].getTimeoutStampSec();
}

SmartPtr<TBB_Station> TBB_Writer::getStation(unsigned stationID, const TBB_header& header) {
	ScopedLock sc(itsStationsMutex);
	map<unsigned, SmartPtr<TBB_Station> >::iterator stIt(itsStations.find(stationID));
	if (stIt == itsStations.end()) {
		// Create new station with HDF5 file and station HDF5 group.
		string h5Filename(createNewTBB_H5Filename(header));
		TBB_Station* station = new TBB_Station(stationName, itsParset, h5Filename); // TODO: leaks
		stIt = itsStations.insert(make_pair(stationID, station)).first;
	}

	return stIt->second;
}

string TBB_Writer::createNewTBB_H5Filename(const TBB_header& header) {
	const string typeExt("tbb.h5");
	string obsIDStr(formatString("%u", itsParset.observationID()));
	string stationName(DAL::stationIDToName(stationID));		

	// Use the recording time of the first (received) frame as timestamp.
	struct timeval tv;
	tv.tv_sec = header.time;
	if (header.nOfFreqBands == 0) { // transient mode
		tv.tv_usec = static_cast<unsigned long>(round( static_cast<double>(header.sampleNr) / header.sampleFreq ));
	} else { // spectral mode
		tv.tv_usec = (header.bandsliceNr & TBB_BAND_NR_MASK) * 1000;
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
	string runNrStr("");
	if (itsRunNr > 0) {
		runNrStr.append(formatString("R%03u_", itsRunNr));
	}
	size_t pos = h5Filename.size() - typeExt.size();
	while (access(h5Filename.c_str(), F_OK) == 0) {
		size_t replacedLen = runNrStr.size();
		itsRunNr += 1;
		string runNrStr(formatString("R%03u_", itsRunNr));
		h5Filename.replace(pos, replacedLen, runNrStr);
	}

	return h5Filename;
}

} // namespace RTCP
} // namespace LOFAR
