/* TBB_Writer_main.cc
 * 
 * LOFAR Transient Buffer Boards (TBB) Data Writer  Copyright (C) 2012
 * ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * $Id: TBB_Writer_main.cc 14523 2012-03-14 18:58:53Z amesfoort $
 *
 * @author Alexander S. van Amesfoort
 * Parts derived from the BF writer written by Jan David Mol, and from
 * TBB writers written by Lars Baehren, Andreas Horneffer, and Joseph Masters.
 */

/*
 * NOTE: Only transient data mode has been implemented and tested.
 * TODO: Some code for spectral mode is implemented, but it could never be tested and the TBB HDF5 format considers transient data only.
 */

#define _POSIX_C_SOURCE			1	// sigaction(2)
#define _GNU_SOURCE			1	// getopt_long(3)

#include <lofar_config.h>			// before any other include

#include <cstddef>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>

#include <iostream>

#include <boost/lexical_cast.hpp>

#include <Storage/TBB_Writer.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/NewHandler.h>
#include <ApplCommon/AntField.h>
#include <Interface/Exceptions.h>
#include <Storage/IOPriority.h>

#include <dal/lofar/StationNames.h>

#define TBB_DEFAULT_BASE_PORT		0x7bb0	// i.e. tbb0
#define TBB_DEFAULT_LAST_PORT		0x7bbb	// 0x7bbf for NL, 0x7bbb for int'l stations

using namespace std;

struct progArgs {
	string parsetFilename;
	string antFieldDir;
	string outputDir;
	string proto;
	uint16_t port;
	struct timeval timeoutVal;
	bool keepRunning;
	bool rawDataFiles;
};

// Install a new handler to produce backtraces for std::bad_alloc.
LOFAR::NewHandler badAllocExcHandler(LOFAR::BadAllocException::newHandler);

static bool sigint_seen;

static void termSigsHandler(int sig_nr) {
	if (sig_nr == SIGINT) {
		/*
		 * For graceful user abort. Signal might be missed, but timeout
		 * catches it later, so don't bother with cascaded signals.
		 */
		sigint_seen = true;
	}
}

/*
 * Register signal handlers for SIGINT and SIGTERM to gracefully terminate early,
 * so we can break out of blocking system calls and exit without corruption of already written output.
 * Leave SIGQUIT (Ctrl-\) untouched, so users can still easily quit immediately.
 */
static void setTermSigsHandler() {
	struct sigaction sa;

	sa.sa_handler = termSigsHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	int err = sigaction(SIGINT,  &sa, NULL); // keyb INT (typically Ctrl-C)
	err    |= sigaction(SIGTERM, &sa, NULL);
	err    |= sigaction(SIGALRM, &sa, NULL); // for setitimer(); don't use sleep(3) and friends
	if (err != 0) {
		LOG_WARN("TBB: Failed to register SIGINT/SIGTERM handler to allow manual, early, graceful program termination.");
	}
}

static vector<string> getTBB_InputStreamNames(const string& proto, uint16_t portsBase) {
	/*
	 * Proto: TBB always arrives over UDP (but command-line override).
	 * 
	 * Ports: Each board sends data to port TBB_DEFAULT_BASE_PORT + itsBoardNr, so that's where put a r/w thread pair each to listen.
	 * The number of TBB boards can be retrieved using LCS/ApplCommon/src/StationConfig.cc: nrTBBs = StationInfo.getInt("RS.N_TBBOARDS");
	 * but we know that stations have 6 (NL stations) or 12 (EU stations) TBB boards, so simply use defines.
	 */
	vector<string> allInputStreamNames;

	if (proto == "udp" || proto == "tcp") {
		uint16_t portsEnd = portsBase + TBB_DEFAULT_LAST_PORT - TBB_DEFAULT_BASE_PORT;
		for (uint16_t port = portsBase; port <= portsEnd; port++) {
			// 0.0.0.0: It would be better to restrict to station IPs/network, but need netmask lookup and need to allow localhost.
			string streamName(proto + ":0.0.0.0:" + LOFAR::formatString("%hu", port));
			allInputStreamNames.push_back(streamName);
		}
	} else { // e.g. "file:data/rw_20110719_110541_1110.dat" or "pipe:data/rw_20110719_110541_1110.pipe"
		allInputStreamNames.push_back(proto);
	}

	return allInputStreamNames;
}

static int antSetName2AntFieldIndex(const string& antSetName) {
	int idx;

	if (strncmp(antSetName.c_str(), "LBA", sizeof("LBA")-1) == 0) {
		idx = LOFAR::AntField::LBA_IDX;
	} else if (strncmp(antSetName.c_str(), "HBA_ZERO", sizeof("HBA_ZERO")-1) == 0) {
		idx = LOFAR::AntField::HBA0_IDX;
	} else if (strncmp(antSetName.c_str(), "HBA_ONE", sizeof("HBA_ONE")-1) == 0) {
		idx = LOFAR::AntField::HBA1_IDX;
	} else if (strncmp(antSetName.c_str(), "HBA", sizeof("HBA")-1) == 0) {
		idx = LOFAR::AntField::HBA_IDX;
	} else {
		throw LOFAR::RTCP::StorageException("unknown antenna set name");
	}

	return idx;
}

static LOFAR::RTCP::StationMetaDataMap getExternalStationMetaData(const LOFAR::RTCP::Parset& parset, const string& antFieldDir) {
	LOFAR::RTCP::StationMetaDataMap stMdMap;

	try {
		// Find path to antenna field files. If not a prog arg, try via $LOFARROOT, else via parset.
		// LOFAR repos location: MAC/Deployment/data/StaticMetaData/AntennaFields/
		string antFieldPath(antFieldDir);
		if (antFieldPath.empty()) {
			char* lrpath = getenv("LOFARROOT");
			if (lrpath != NULL) {
				antFieldPath = string(lrpath) + "/etc/StaticMetaData/";
			} else { // parset typically gives "/data/home/lofarsys/production/lofar/etc/StaticMetaData"
				antFieldPath = parset.AntennaFieldsDir(); // doesn't quite do what its name suggests, so append a component
				if (!antFieldPath.empty()) {
					antFieldPath.push_back('/');
				}
			}
			antFieldPath.append("AntennaFields/");
		}

		int fieldIdx = antSetName2AntFieldIndex(parset.antennaSet());

		vector<string> stationNames(parset.allStationNames());
		for (vector<string>::const_iterator it(stationNames.begin());
                     it != stationNames.end(); ++it) {

			string stName(it->substr(0, sizeof("CS001")-1)); // drop any "HBA0"-like suffix
			string antFieldFilename(antFieldPath + stName + "-AntennaField.conf");

			// Tries to locate the filename if no abs path is given, else throws AssertError exc.
			LOFAR::AntField antField(antFieldFilename);

			// Compute absolute antenna positions from centre + relative.
			// See AntField.h in ApplCommon for the AFArray typedef and contents (first is shape, second is values).
			LOFAR::RTCP::StationMetaData stMetaData;
			stMetaData.available = true;
			stMetaData.antPositions = antField.AntPos(fieldIdx).second;
			for (size_t i = 0; i < stMetaData.antPositions.size(); i += 3) {
				stMetaData.antPositions.at(i+2) += antField.Centre(fieldIdx).second.at(2);
				stMetaData.antPositions[i+1]    += antField.Centre(fieldIdx).second[1];
				stMetaData.antPositions[i]      += antField.Centre(fieldIdx).second[0];
			}

			stMetaData.normalVector   = antField.normVector(fieldIdx).second;
			stMetaData.rotationMatrix = antField.rotationMatrix(fieldIdx).second;

			stMdMap.insert(make_pair(dal::stationNameToID(stName), stMetaData));
		}
	} catch (exception& exc) { // LOFAR::AssertError or dal::DALValueError (rare)
		// AssertError already sends a message to the logger.
		throw LOFAR::RTCP::StorageException(exc.what());
	}

	return stMdMap;
}

static int doTBB_Run(const vector<string>& inputStreamNames, const LOFAR::RTCP::Parset& parset,
                     const LOFAR::RTCP::StationMetaDataMap& stMdMap, struct progArgs& args) {
	string logPrefix("TBB obs " + LOFAR::formatString("%u", parset.observationID()) + ": ");

	int err = 1;
	try {
		// When this obj goes out of scope, worker threads are cancelled and joined with.
		LOFAR::RTCP::TBB_Writer writer(inputStreamNames, parset, stMdMap,
                                       args.outputDir, args.rawDataFiles, logPrefix);

		/*
		 * We don't know how much data comes in, so cancel workers when all are idle for a while (timeoutVal).
		 * In some situations, threads can become active again after idling a bit, so periodically monitor thread timeout stamps.
		 * Poor man's sync, but per-thread timers to break read() to notify us of idleness does not work.
		 * This (sucks and :)) could be improved once the LOFAR system tells us how much data will be dumped, or when done.
		 */
		struct itimerval timer = {args.timeoutVal, args.timeoutVal};
		if (setitimer(ITIMER_REAL, &timer, NULL) != 0) {
			throw LOFAR::SystemCallException("setitimer failed");
		}

		bool anyFrameReceived = false; // don't quit if there is no data immediately after starting
		size_t nrWorkersDone;
		do {
			pause();
			if (sigint_seen) { // typically Ctrl-C
				args.keepRunning = false;
				break;
			}

			nrWorkersDone = 0;
			for (size_t i = 0; i < inputStreamNames.size(); i++) {
				struct timeval now;
				gettimeofday(&now, NULL);
				time_t lastActive_sec = writer.getTimeoutStampSec(i);
				if (lastActive_sec != 0) {
					anyFrameReceived = true;
				}
				if (anyFrameReceived && lastActive_sec <= now.tv_sec - args.timeoutVal.tv_sec) {
					nrWorkersDone += 1;
				}
			}
		} while (nrWorkersDone < inputStreamNames.size());

		err = 0;

	} catch (LOFAR::Exception& exc) {
		LOG_FATAL_STR(logPrefix << "LOFAR::Exception: " << exc.text());
	} catch (exception& exc) {
		LOG_FATAL_STR(logPrefix << "std::exception: " << exc.what());
	}

	return err;
}

static int ensureOutputDirExists(string outputDir) {
	struct stat st;

	if (outputDir == "") {
		outputDir = ".";
	}

	if (stat(outputDir.c_str(), &st) != 0) {
		return errno;
	}

	// Check if the last component is a dir too (stat() did the rest).
	if (!S_ISDIR(st.st_mode)) {
		return ENOTDIR;
	}

	return 0;
}

static void printUsage(const char* progname) {
	cout << "LOFAR TBB_Writer version: ";
#ifndef TBB_WRITER_VERSION
	cout << LOFAR::StorageVersion::getVersion();
#else
	cout << TBB_WRITER_VERSION;
#endif
	cout << endl;
	cout << "Write incoming LOFAR TBB data with meta data to disk in HDF5 format." << endl;
	cout << "Usage: " << progname << " --parsetfile=parsets/L12345.parset [OPTION]..." << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "  -s, --parsetfile=L12345.parset      parset file (observation settings) (mandatory)" << endl;
	cout << endl;
	cout << "  -a, --antfielddir=/d/AntennaFields  override $LOFARROOT and parset path for antenna field files (like CS001-AntennaField.conf)" << endl;
	cout << "  -o, --outputdir=tbbout              output directory" << endl;
	cout << "  -p, --proto=tcp|udp                 input stream type (default: udp)" << endl;
	cout << "              file:raw.dat" << endl;
	cout << "              pipe:named.pipe" << endl;
	cout << "  -b, --portbase=31665                start of range of 12 consecutive udp/tcp ports to receive from" << endl;
	cout << "  -t, --timeout=10                    seconds of input inactivity until dump is considered completed" << endl;
	cout << endl;
	cout << "  -r, --rawdatafiles[=true|false]     output separate .raw data files (default: true; do not set to false atm);" << endl;
	cout << "                                      .raw files is strongly recommended, esp. when receiving from multiple stations" << endl;
	cout << "  -k, --keeprunning[=true|false]      accept new input after a dump completed (default: true)" << endl;
	cout << endl;
	cout << "  -h, --help                          print program name, version number and this info, then exit" << endl;
	cout << "  -v, --version                       same as --help" << endl;
}

static int parseArgs(int argc, char *argv[], struct progArgs* args) {
	int rv = 0;

	// Default values
	args->parsetFilename = "";	// there is no default parset filename, so not passing it is fatal
	args->antFieldDir = "";		// idem

	args->outputDir = "";
	args->proto = "udp";
	args->port = TBB_DEFAULT_BASE_PORT;
	args->timeoutVal.tv_sec = 10; // after this default of inactivity cancel all input threads and close output files
	args->timeoutVal.tv_usec = 0;
	args->rawDataFiles = true;
	args->keepRunning = true;

	static const struct option long_opts[] = {
		// {const char *name, int has_arg, int *flag, int val}
		{"parsetfile",   required_argument, NULL, 's'}, // observation (s)ettings
		{"antfielddir",  required_argument, NULL, 'a'},
		{"outputdir",    required_argument, NULL, 'o'},
		{"proto",        required_argument, NULL, 'p'},
		{"portbase",     required_argument, NULL, 'b'}, // port (b)ase
		{"timeout",      required_argument, NULL, 't'},

		{"rawdatafiles", optional_argument, NULL, 'r'},
		{"keeprunning",  optional_argument, NULL, 'k'},

		{"help",         no_argument,       NULL, 'h'},
		{"version",      no_argument,       NULL, 'v'},

		{NULL, 0, NULL, 0}
	};

	opterr = 0; // prevent error printing to stderr by getopt_long()
	int opt;
	while ((opt = getopt_long(argc, argv, "hvs:a:o:p:b:t:r::k::", long_opts, NULL)) != -1) {
		switch (opt) {
		case 's':
			args->parsetFilename = optarg;
			break;
		case 'a':
			args->antFieldDir = optarg;
			if (args->antFieldDir[args->antFieldDir.size() - 1] != '/') {
				args->antFieldDir.push_back('/');
			}
			break;
		case 'o':
			args->outputDir = optarg;
			if (args->outputDir[args->outputDir.size() - 1] != '/') {
				args->outputDir.push_back('/');
			}
			break;
		case 'p':
			if (strcmp(optarg, "tcp") == 0 || strcmp(optarg, "udp") == 0 ||
				strncmp(optarg, "file", sizeof("file")-1) == 0 || strncmp(optarg, "pipe", sizeof("pipe")-1) == 0) {
				args->proto = optarg;
			} else {
				cerr << "Invalid protocol option: " << optarg << endl;
				rv = 1;
			}
			break;
		case 'b':
			try {
				args->port = boost::lexical_cast<uint16_t>(optarg);
				if (args->port > 65536 - (TBB_DEFAULT_LAST_PORT - TBB_DEFAULT_BASE_PORT)) {
					throw boost::bad_lexical_cast(); // abuse exc type to have single catch
				}
			} catch (boost::bad_lexical_cast& /*exc*/) {
				cerr << "Invalid port option: " << optarg << endl;
				rv = 1;
			}
			break;
		case 't':
			try {
				args->timeoutVal.tv_sec = boost::lexical_cast<unsigned long>(optarg);
			} catch (boost::bad_lexical_cast& /*exc*/) {
				cerr << "Invalid timeout option: " << optarg << endl;
				rv = 1;
			}
			break;
		case 'r':
			if (optarg == NULL || optarg[0] == '\0') {
				args->rawDataFiles = true;
				break;
			}
			try {
				args->rawDataFiles = boost::lexical_cast<bool>(optarg);
			} catch (boost::bad_lexical_cast& /*exc*/) {
				cerr << "Invalid rawdata option: " << optarg << endl;
				rv = 1;
			}
			break;
		case 'k':
			if (optarg == NULL || optarg[0] == '\0') {
				args->keepRunning = true;
				break;
			}
			try {
				args->keepRunning = boost::lexical_cast<bool>(optarg);
			} catch (boost::bad_lexical_cast& /*exc*/) {
				cerr << "Invalid keeprunning option: " << optarg << endl;
				rv = 1;
			}
			break;
		case 'h':
		case 'v':
			if (rv == 0) {
				rv = 2;
			}
			break;
		default: // '?'
			cerr << "Invalid program argument: " << argv[optind-1] << endl;
			rv = 1;
		}
	}

	if (optind < argc) {
		cerr << "Failed to recognize options:";
		while (optind < argc) {
			cerr << " " << argv[optind++];
		}
		cerr << endl;
		rv = 1;
	}

	return rv;
}

static char stdoutbuf[1024], stderrbuf[1024];

int main(int argc, char* argv[]) {
	struct progArgs args;
	int err;

	setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
	setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);

#if defined HAVE_LOG4CPLUS
	INIT_LOGGER(string(getenv("LOFARROOT") ? : ".") + "/etc/Storage_main.log_prop");
#endif

	if ((err = parseArgs(argc, argv, &args)) != 0) {
		if (err == 2) err = 0;
		printUsage(argv[0]);
#if defined HAVE_LOG4CPLUS
		LOGGER_EXIT_THREAD();
#endif
		return err;
	}

	setTermSigsHandler();

	if ((err = ensureOutputDirExists(args.outputDir)) != 0) {
		LOG_FATAL_STR("TBB: output directory: " << strerror(err));
#if defined HAVE_LOG4CPLUS
		LOGGER_EXIT_THREAD();
#endif
		return 1;
	}

	const vector<string> inputStreamNames(getTBB_InputStreamNames(args.proto, args.port));

	// We don't run alone, so try to increase the QoS we get from the OS to decrease the chance of data loss.
	setIOpriority(); // reqs CAP_SYS_NICE or CAP_SYS_ADMIN
	setRTpriority(); // reqs CAP_SYS_NICE
	lockInMemory();  // reqs CAP_IPC_LOCK

	err = 1;
	try {
		LOFAR::RTCP::Parset parset(args.parsetFilename);
		LOFAR::RTCP::StationMetaDataMap stMdMap(getExternalStationMetaData(parset, args.antFieldDir));

		err = 0;
		do {
			err += doTBB_Run(inputStreamNames, parset, stMdMap, args);
		} while (args.keepRunning && err < 1000);
		if (err == 1000) { // Nr of dumps per obs was estimated to fit in 3 digits.
			LOG_FATAL("TBB: Reached max nr of errors seen. Shutting down to avoid filling up storage with logging crap.");
		}

	// Config exceptions (opening or parsing) are fatal. Too bad we cannot have it in one type.
	} catch (LOFAR::RTCP::InterfaceException& exc) {
		LOG_FATAL_STR("TBB: LOFAR::InterfaceException: parset: " << exc.text());
	} catch (LOFAR::APSException& exc) {
		LOG_FATAL_STR("TBB: LOFAR::APSException: parameterset: " << exc.text());
	} catch (LOFAR::RTCP::StorageException& exc) {
		LOG_FATAL_STR("TBB: LOFAR::StorageException: antenna field files: " << exc.text());
	}

#if defined HAVE_LOG4CPLUS
	LOGGER_EXIT_THREAD();
#endif

	return err == 0 ? 0 : 1;
}

