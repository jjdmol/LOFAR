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
 * $Id: TBB_Writer_main.cc 17261 2012-09-07 18:58:53Z amesfoort $
 */

/* @author Alexander S. van Amesfoort
 * Parts derived from the BF writer written by Jan David Mol, and from
 * TBB writers written by Lars Baehren, Andreas Horneffer, and Joseph Masters.
 */

#include <lofar_config.h>			// before any other include

#define _FILE_OFFSET_BITS 64
#include <cstddef>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <libgen.h>
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
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/AntField.h>
#include <Interface/Exceptions.h>
#include <Storage/IOPriority.h>

#include <dal/lofar/StationNames.h>

#define TBB_DEFAULT_BASE_PORT		0x7bb0	// i.e. tbb0
#define TBB_DEFAULT_LAST_PORT		0x7bbb	// 0x7bbf for NL, 0x7bbb for int'l stations

#define STDLOG_BUFFER_SIZE			1024

using namespace std;

struct progArgs {
	string parsetFilename;
	string antFieldDir;
	string outputDir;
	string input;
	uint16_t port;
	struct timeval timeoutVal;
	bool keepRunning;
};

static char stdoutbuf[STDLOG_BUFFER_SIZE];
static char stderrbuf[STDLOG_BUFFER_SIZE];

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

static vector<string> getTBB_InputStreamNames(const string& input, uint16_t portsBase) {
	int nTbbBoards;
	try {
		LOFAR::StationConfig stConf;
		nTbbBoards = stConf.nrTBBs;
	} catch (LOFAR::AssertError& ) { // config file not found
		LOG_DEBUG_STR("Falling back to up to " << TBB_DEFAULT_LAST_PORT - TBB_DEFAULT_BASE_PORT + 1 << " streams (1 per board)");
		nTbbBoards = TBB_DEFAULT_LAST_PORT - TBB_DEFAULT_BASE_PORT + 1; // fallback
	}

	vector<string> allInputStreamNames;
	if (input == "udp" || input == "tcp") {
		for (uint16_t port = portsBase; port <= portsBase + nTbbBoards; ++port) {
			// 0.0.0.0: could restrict to station IPs/network, but need netmask lookup and allow localhost. Not critical: we are on a separate VLAN.
			string streamName(input + ":0.0.0.0:" + LOFAR::formatString("%hu", port));
			allInputStreamNames.push_back(streamName);
		}
	} else { // file or named pipe input
		size_t placeholderPos = input.find_last_of('%');
		if (placeholderPos == string::npos) { // single input, no expansion needed
			allInputStreamNames.push_back(input);
		} else { // expand: replace e.g. file:x%y-%.raw by file:x%y-0.raw, file:x%y-1.raw, ..., file:x%y-11.raw
			for (int i = 0; i < nTbbBoards; ++i) {
				string streamName(input);
				streamName.replace(placeholderPos, 1, LOFAR::formatString("%u", i));
				allInputStreamNames.push_back(streamName);
			}
		}
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
	} catch (LOFAR::AssertError& exc) {
		// Throwing AssertError already sends a message to the logger.
	} catch (dal::DALValueError& exc) {
		throw LOFAR::RTCP::StorageException(exc.what());
	}

	return stMdMap;
}

static int doTBB_Run(const vector<string>& inputStreamNames, const LOFAR::RTCP::Parset& parset,
                     const LOFAR::RTCP::StationMetaDataMap& stMdMap, struct progArgs& args) {
	string logPrefix("TBB obs " + LOFAR::formatString("%u", parset.observationID()) + ": ");

	vector<int> thrExitStatus(2 * inputStreamNames.size(), 0);
	int err = 1;
	try {
		// When this obj goes out of scope, worker threads are cancelled and joined with.
		LOFAR::RTCP::TBB_Writer writer(inputStreamNames, parset, stMdMap, args.outputDir, logPrefix, thrExitStatus);

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
				args.keepRunning = false; // for main(), not for worker threads
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
		LOG_FATAL_STR(logPrefix << "LOFAR::Exception: " << exc);
	} catch (exception& exc) {
		LOG_FATAL_STR(logPrefix << "std::exception: " << exc.what());
	}

	// Propagate exit status != 0 from any input or output worker thread.
	for (unsigned i = 0; i < thrExitStatus.size(); ++i) {
		if (thrExitStatus[i] != 0) {
			err = 1;
			break;
		}
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
	cout << "  -i, --input=tcp|udp                 input stream(s) or type (default: udp)" << endl;
	cout << "              file:raw.dat                if file or pipe name has a '%'," << endl;
	cout << "              pipe:named-%.pipe           then the last '%' is replaced by 0, 1, ..., 11" << endl;
	cout << "  -b, --portbase=31665                start of range of 12 consecutive udp/tcp ports to receive from" << endl;
	cout << "  -t, --timeout=10                    seconds of input inactivity until dump is considered completed" << endl;
	cout << endl;
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
	args->input = "udp";
	args->port = TBB_DEFAULT_BASE_PORT;
	args->timeoutVal.tv_sec = 10; // after this default of inactivity cancel all input threads and close output files
	args->timeoutVal.tv_usec = 0;
	args->keepRunning = true;

	static const struct option long_opts[] = {
		// {const char *name, int has_arg, int *flag, int val}
		{"parsetfile",   required_argument, NULL, 's'}, // observation (s)ettings
		{"antfielddir",  required_argument, NULL, 'a'},
		{"outputdir",    required_argument, NULL, 'o'},
		{"input",        required_argument, NULL, 'i'},
		{"portbase",     required_argument, NULL, 'b'}, // port (b)ase
		{"timeout",      required_argument, NULL, 't'},

		{"keeprunning",  optional_argument, NULL, 'k'},

		{"help",         no_argument,       NULL, 'h'},
		{"version",      no_argument,       NULL, 'v'},

		{NULL, 0, NULL, 0}
	};

	opterr = 0; // prevent error printing to stderr by getopt_long()
	int opt;
	while ((opt = getopt_long(argc, argv, "hvs:a:o:p:b:t:k::", long_opts, NULL)) != -1) {
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
		case 'i':
			if (strcmp(optarg, "tcp") == 0 || strcmp(optarg, "udp") == 0 ||
				strncmp(optarg, "file:", sizeof("file:")-1) == 0 || strncmp(optarg, "pipe:", sizeof("pipe:")-1) == 0) {
				args->input = optarg;
			} else {
				LOG_FATAL_STR("TBB: Invalid input option: " << optarg);
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
				LOG_FATAL_STR("TBB: Invalid port option: " << optarg);
				rv = 1;
			}
			break;
		case 't':
			try {
				args->timeoutVal.tv_sec = boost::lexical_cast<unsigned long>(optarg);
			} catch (boost::bad_lexical_cast& /*exc*/) {
				LOG_FATAL_STR("TBB: Invalid timeout option: " << optarg);
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
				LOG_FATAL_STR("TBB: Invalid keeprunning option: " << optarg);
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
			LOG_FATAL_STR("TBB: Invalid program argument: " << argv[optind-1]);
			rv = 1;
		}
	}

	if (optind < argc) {
		LOG_FATAL("TBB: Failed to recognize options:");
		while (optind < argc) {
			LOG_FATAL_STR(" " << argv[optind++]);
		}
		rv = 1;
	}

	return rv;
}

int main(int argc, char* argv[]) {
	struct progArgs args;
	int err;

#if defined HAVE_LOG4CPLUS || defined HAVE_LOG4CXX
	struct Log {
		Log(const char* argv0) {
			char *dirc = strdup(argv0); // dirname() may clobber its arg
			if (dirc != NULL) {
				INIT_LOGGER(string(getenv("LOFARROOT") ? : dirname(dirc)) + "/../etc/Storage_main.log_prop");
				free(dirc);
			}
		}

		~Log() {
			LOGGER_EXIT_THREAD(); // destroys NDC created by INIT_LOGGER()
		}
	} logger(argv[0]);
#endif

	err  = setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
	err |= setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);
	if (err != 0) {
		LOG_WARN("TBB: failed to change stdout and/or stderr output buffers");
	}

	if ((err = parseArgs(argc, argv, &args)) != 0) {
		if (err == 2) err = 0;
		printUsage(argv[0]);
		return err;
	}

	setTermSigsHandler();

	if ((err = ensureOutputDirExists(args.outputDir)) != 0) {
		LOG_FATAL_STR("TBB: output directory: " << strerror(err));
		return 1;
	}

	const vector<string> inputStreamNames(getTBB_InputStreamNames(args.input, args.port));

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
		LOG_FATAL_STR("TBB: LOFAR::InterfaceException: parset: " << exc);
	} catch (LOFAR::APSException& exc) {
		LOG_FATAL_STR("TBB: LOFAR::APSException: parameterset: " << exc);
	} catch (LOFAR::RTCP::StorageException& exc) {
		LOG_FATAL_STR("TBB: LOFAR::StorageException: antenna field files: " << exc);
	}

	return err == 0 ? 0 : 1;
}

