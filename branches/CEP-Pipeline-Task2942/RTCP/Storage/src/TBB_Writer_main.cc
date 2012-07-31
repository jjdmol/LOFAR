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

#define _POSIX_C_SOURCE				1	// sigaction(2), gmtime_r(3)
#define _GNU_SOURCE					1	// getopt_long(3)

#include <lofar_config.h>				// before any other include

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <ctime>						// strftime()
#include <climits>
#include <getopt.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <boost/lexical_cast.hpp>

#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <Common/Thread/Thread.h>
#include <Common/NewHandler.h>
#include <Storage/TBB_Writer.h>
#include <Storage/IOPriority.h>

#define TBB_DEFAULT_BASE_PORT		0x7bb0	// i.e. tbb0
#define TBB_DEFAULT_LAST_PORT		0x7bbb	// 0x7bbf for NL, 0x7bbb for int'l stations

using namespace std;

struct progArgs {
	string outDir;
	string parsetFilename;
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

#if 0
/*
 * Return station names that will send data to this host.
 * 
 * This function will be redundant once the mapping info is available through the parset.
 * Multiple stations may be sending to the same locus node (i.e. TBB writer).
 * This mapping is indicated in MAC/Deployment/data/StaticMetaData/TBBConnections.dat
 */
static vector<string> getTBB_StationNames(const string& tbbMappingFilename) {
	vector<string> stationNames;

	LOFAR::TBB_StaticMapping tbbMapping(tbbMappingFilename);
	if (tbbMapping.empty()) {
		throw LOFAR::IOException("Failed to derive any node-to-station mappings from the static TBB mapping file");
	}

	string myHname(LOFAR::myHostname(true));
	stationNames = tbbMapping.getStationNames(myHname);
	if (stationNames.empty()) {
		// Likely, it only knows e.g. 'tbbsinknode123' instead of 'tbbsinknode123.example.com', so retry.
		myHname = LOFAR::myHostname(false);
		stationNames = tbbMapping.getStationNames(myHname);
		if (stationNames.empty()) {
			throw LOFAR::IOException("Failed to retrieve station names that will send TBB data to this node");
		}
	}

	return stationNames;
}
#endif

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

static int doTBB_Run(const vector<string>& inputStreamNames, LOFAR::RTCP::Parset& parset, struct progArgs& args) {
	string logPrefix("TBB obs " + LOFAR::formatString("%u", parset.observationID()) + " ");

	int err = 1;
	try {
		// When this obj goes out of scope, worker threads are cancelled and joined with.
		LOFAR::RTCP::TBB_Writer writer(inputStreamNames, parset, args.outDir, args.rawDataFiles, logPrefix);

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

static int ensureOutDirExists(string outputDir) {
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
	cout << "Usage: " << progname << " [--help] [--version] [--outputdir=/data/tbboutdir] [--parsetfile=/home/user/tbb/obsxxx.parset]"
		" [--proto=udp] [--portbase=0x7bb0] [--timeout=10] [--rawdata=1] [--keeprunning=1]" << endl;
}

static int parseArgs(int argc, char *argv[], struct progArgs* args) {
	int rv = 0;

	// Default values
	args->outDir = "";
	args->parsetFilename = "";	// there is no default parset filename, so not passing it is fatal
	args->proto = "udp";
	args->port = TBB_DEFAULT_BASE_PORT;
	args->timeoutVal.tv_sec = 10; // after this default of inactivity cancel all input threads and close output files
	args->timeoutVal.tv_usec = 0;
	args->rawDataFiles = true;
	args->keepRunning = true;

	static struct option long_opts[] = {
		// {const char *name, int has_arg, int *flag, int val}
		{"help",        no_argument,       NULL, 'h'},
		{"version",     no_argument,       NULL, 'v'},

		{"outputdir",   required_argument, NULL, 'd'},
		{"parsetfile",  required_argument, NULL, 'c'}, // 'c'onfig
		{"proto",       required_argument, NULL, 'p'},
		{"portbase",    required_argument, NULL, 'b'}, // 'b'ase
		{"timeout",     required_argument, NULL, 't'},

		{"rawdata",     optional_argument, NULL, 'r'},
		{"keeprunning", optional_argument, NULL, 'k'},

		{NULL, 0, NULL, 0}
	};

	opterr = 0; // prevent error printing to stderr by getopt_long()
	int opt;
	while ((opt = getopt_long(argc, argv, "hvd:c:p:b:t:r::k::", long_opts, NULL)) != -1) {
		switch (opt) {
		case 'h':
		case 'v':
			cout << "LOFAR TBB_Writer version: ";
#ifdef HAVE_PKVERSION
			cout << StorageVersion::getVersion();
#else
#warning TBB_Writer version cannot be printed correctly with help and version program options
			cout << "0.909";
#endif
			cout << endl;
			if (rv == 0) {
				rv = 2;
			}
			break;
		case 'd':
			args->outDir = optarg;
			if (args->outDir[args->outDir.size() - 1] != '/') {
				args->outDir.push_back('/');
			}
			break;
		case 'c':
			args->parsetFilename = optarg;
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

	if ((err = ensureOutDirExists(args.outDir)) != 0) {
		LOG_FATAL_STR("TBB: output directory: " << strerror(err));
#if defined HAVE_LOG4CPLUS
		LOGGER_EXIT_THREAD();
#endif
		return 1;
	}

#if 0
	/*
	 * Retrieve the station name that will send data to this host (input thread will check this at least once).
	 * Getting this from static meta data is inflexible.
	 */
	string tbbMappingFilename(args.conMapFilename); // no arg anymore
	if (tbbMappingFilename.empty()) {
		const string defaultTbbMappingFilename("TBBConnections.dat");
		char* lrpath = getenv("LOFARROOT");
		if (lrpath != NULL) {
			tbbMappingFilename = string(lrpath) + "/etc/StaticMetaData/";
		}
		tbbMappingFilename.append(defaultTbbMappingFilename);
	}
	vector<string> stationNames(getTBB_StationNames(tbbMappingFilename));
#endif

	const vector<string> inputStreamNames(getTBB_InputStreamNames(args.proto, args.port));

	// We don't run alone, so increase the QoS we get from the OS to decrease the chance of data loss.
	setIOpriority();
	setRTpriority();
	lockInMemory();

	err = 0;
	try {
		do {
			LOFAR::RTCP::Parset parset(args.parsetFilename); // reload config per run

			err += doTBB_Run(inputStreamNames, parset, args);

		} while (args.keepRunning && err < 1000);
		if (err == 1000) { // Nr of dumps per obs was estimated to fit in 3 digits.
			LOG_FATAL("TBB: Reached max nr of errors seen. Shutting down to avoid filling up storage with logging crap.");
		}

	// parset exceptions are fatal
	} catch (LOFAR::RTCP::InterfaceException& exc) {
		LOG_FATAL_STR("TBB: LOFAR::InterfaceException: parset: " << exc.text());
		err = 1;
	} catch (LOFAR::APSException& exc) {
		LOG_FATAL_STR("TBB: LOFAR::APSException: parameterset: " << exc.text());
		err = 1;
	}

#if defined HAVE_LOG4CPLUS
	LOGGER_EXIT_THREAD();
#endif

	return err == 0 ? 0 : 1;
}

