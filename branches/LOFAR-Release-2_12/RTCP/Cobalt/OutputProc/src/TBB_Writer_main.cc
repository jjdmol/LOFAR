//# TBB_Writer_main.cc: LOFAR Transient Buffer Boards (TBB) Data Writer
//# Copyright (C) 2012-2015  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands.
//#
//# This program is free software: you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite.  If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>               // before any other include

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
#include <sstream>
#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/AntField.h>
#include <CoInterface/Exceptions.h>

#ifdef HAVE_DAL
#  include <dal/lofar/StationNames.h>
#else
#  warning The TBB writer may be built without DAL, but will not write any output
#endif

#include "TBB_Writer.h"
#include "IOPriority.h"

#define TBB_DEFAULT_BASE_PORT   0x7bb0  // i.e. tbb0
#define TBB_DEFAULT_LAST_PORT   0x7bbb  // 0x7bbf for NL, 0x7bbb for int'l stations

using namespace std;

struct progArgs {
  string parsetFilename;
  string stCalTablesDir;
  string staticMetaDataDir;
  string outputDir;
  string input;
  vector<uint16_t> ports;
  struct timeval timeoutVal;
  bool keepRunning;
};

static volatile sig_atomic_t sigint_seen;

static void termSigsHandler(int sig_nr)
{
  if (sig_nr == SIGINT) {
    /*
     * For graceful user abort. Signal might be missed, but timeout
     * catches it later, so don't bother with cascaded signals.
     */
    sigint_seen = 1;
  }
}

/*
 * Register signal handlers for SIGINT and SIGTERM to gracefully terminate early,
 * so we can break out of blocking system calls and exit without corruption of already written output.
 * Leave SIGQUIT (Ctrl-\) untouched, so users can still easily quit immediately.
 */
static void setTermSigsHandler()
{
  struct sigaction sa;

  sa.sa_handler = termSigsHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  int err = sigaction(SIGINT,  &sa, NULL); // keyb INT (typically Ctrl-C)
  err |= sigaction(SIGTERM, &sa, NULL);
  err |= sigaction(SIGALRM, &sa, NULL);    // for setitimer(); don't use sleep(3) and friends
  if (err != 0) {
    LOG_WARN("TBB: Failed to register SIGINT/SIGTERM handler to allow manual, early, graceful program termination.");
  }
}

static vector<string> getTBB_InputStreamNames(const string& input, vector<uint16_t>& ports)
{
  vector<string> allInputStreamNames;
  if (input == "udp" || input == "tcp") {
    for (vector<uint16_t>::iterator it = ports.begin(); it != ports.end(); ++it) {
      // 0.0.0.0: could restrict to station IPs/network, but need netmask lookup
      // and allow localhost. Not critical: data arrives via a separate VLAN.
      string streamName(input + ":0.0.0.0:" + LOFAR::formatString("%hu", *it));
      allInputStreamNames.push_back(streamName);
    }
  } else { // file or named pipe input
    size_t colonPos = input.find(':');
    if (colonPos == string::npos) {
      return allInputStreamNames;
    }
    size_t placeholderPos = input.find_last_of('%');
    if (placeholderPos == string::npos) { // single input, no expansion needed
      if (access(input.c_str() + colonPos + 1, R_OK) == 0) {
        allInputStreamNames.push_back(input);
      }
    } else { // expand e.g. file:x%y-%.raw into {file:x%y-0.raw, file:x%y-1.raw, ..., file:x%y-11.raw}
      for (unsigned i = 0; i < ports.size(); ++i) {
        string streamName(input);
        streamName.replace(placeholderPos, 1, LOFAR::formatString("%u", i));
        if (access(streamName.c_str() + colonPos + 1, R_OK) == 0) {
          allInputStreamNames.push_back(streamName);
        }
      }
    }
  }

  return allInputStreamNames;
}

static void retrieveStationCalTables(string& /*stCalTablesDir*/)
{
  // TODO: implement via the qpid messaging middleware
}

static int antSetName2AntFieldIndex(const string& antSetName)
{
  int idx;

  if (strncmp(antSetName.c_str(), "LBA", sizeof("LBA") - 1) == 0) {
    idx = LOFAR::AntField::LBA_IDX;
  } else if (strncmp(antSetName.c_str(), "HBA_ZERO", sizeof("HBA_ZERO") - 1) == 0) {
    idx = LOFAR::AntField::HBA0_IDX;
  } else if (strncmp(antSetName.c_str(), "HBA_ONE", sizeof("HBA_ONE") - 1) == 0) {
    idx = LOFAR::AntField::HBA1_IDX;
  } else if (strncmp(antSetName.c_str(), "HBA", sizeof("HBA") - 1) == 0) {
    idx = LOFAR::AntField::HBA_IDX;
  } else {
    throw LOFAR::Cobalt::StorageException("unknown antenna set name");
  }

  return idx;
}

static LOFAR::Cobalt::StationMetaDataMap getExternalStationMetaData(const LOFAR::Cobalt::Parset& parset, const string& staticMetaDataDir)
{
  LOFAR::Cobalt::StationMetaDataMap stMdMap;

  try {
    // Find path to antenna field files. If not a prog arg, try via $LOFARROOT, else via parset.
    // LOFAR repos location: MAC/Deployment/data/StaticMetaData
    string staticMetaDataPath(staticMetaDataDir);
    if (staticMetaDataPath.empty()) {
      char* lrPath = getenv("LOFARROOT");
      if (lrPath == NULL) {
        throw LOFAR::APSException("StaticMetaData dir unknown: LOFARROOT not set and command line option not used");
      }
      staticMetaDataPath = lrPath;
      if (!staticMetaDataPath[0] != '\0' && staticMetaDataPath[staticMetaDataDir.size() - 1] != '/') {
        staticMetaDataPath.push_back('/');
      }
      staticMetaDataPath.append("etc/StaticMetaData/");
    }

    int fieldIdx = antSetName2AntFieldIndex(parset.settings.antennaSet);

    vector<string> stationNames(parset.allStationNames());
    for (vector<string>::const_iterator it(stationNames.begin());
         it != stationNames.end(); ++it) {

      string stName(it->substr(0, sizeof("CS001") - 1)); // drop any "HBA0"-like suffix
      string antFieldFilename(staticMetaDataPath + stName + "-AntennaField.conf");

      // Tries to locate the filename if no abs path is given, else throws AssertError exc.
      LOFAR::AntField antField(antFieldFilename);

      // Compute absolute antenna positions from centre + relative.
      // See AntField.h in ApplCommon for the AFArray typedef and contents (first is shape, second is values).
      LOFAR::Cobalt::StationMetaData stMetaData;
      stMetaData.available = true;
      stMetaData.antPositions = antField.AntPos(fieldIdx).second;
      for (size_t i = 0; i < stMetaData.antPositions.size(); i += 3) {
        stMetaData.antPositions.at(i + 2) += antField.Centre(fieldIdx).second.at(2);
        stMetaData.antPositions[i + 1] += antField.Centre(fieldIdx).second[1];
        stMetaData.antPositions[i] += antField.Centre(fieldIdx).second[0];
      }

      stMetaData.normalVector = antField.normVector(fieldIdx).second;
      stMetaData.rotationMatrix = antField.rotationMatrix(fieldIdx).second;

#ifdef HAVE_DAL
      stMdMap.insert(make_pair(dal::stationNameToID(stName), stMetaData));
#endif
    }
  } catch (LOFAR::AssertError& exc) {
    // Throwing AssertError already sends a message to the logger.
#ifdef HAVE_DAL
  } catch (dal::DALValueError& exc) {
    throw LOFAR::Cobalt::StorageException(exc.what());
#endif
  }

  return stMdMap;
}

static int doTBB_Run(const vector<string>& inputStreamNames, const LOFAR::Cobalt::Parset& parset,
                     const LOFAR::Cobalt::StationMetaDataMap& stMdMap, struct progArgs& args)
{
  string logPrefix("TBB obs " + LOFAR::formatString("%u", parset.settings.observationID) + ": ");

  vector<int> thrExitStatus(2 * inputStreamNames.size(), 1);
  int status = 1;
  try {
#ifndef HAVE_DAL
    // Allow building without DAL (some users don't need TBB_Writer), but bail if run.
    (void)stMdMap; // silence compiler warning
    (void)args; // idem
    throw LOFAR::APSException("TBB_Writer needs but was not built with DAL");
#else
    // When this obj goes out of scope, worker threads are cancelled and joined with.
    LOFAR::Cobalt::TBB_Writer writer(inputStreamNames, parset, stMdMap, args.outputDir, logPrefix, thrExitStatus);

    /*
     * We don't know how much data comes in, so cancel workers when all are idle for a while (timeoutVal).
     * In some situations, threads can become active again after idling a bit, so periodically monitor thread timeout stamps.
     * Poor man's sync, but per-thread timers to break read() to notify us of idleness does not work.
     * This (sucks and :)) could be improved once the LOFAR system tells us how much data will be dumped, or when done.
     */
    struct itimerval timer = {args.timeoutVal, args.timeoutVal};
    if (setitimer(ITIMER_REAL, &timer, NULL) != 0) {
      THROW_SYSCALL("setitimer");
    }

    bool anyFrameReceived = false; // don't quit if there is no data immediately after starting
    size_t nrWorkersDone;
    do {
      pause();
      if (sigint_seen == 1) { // typically Ctrl-C
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

    status = 0;
#endif
  } catch (LOFAR::Exception& exc) {
    LOG_FATAL_STR(logPrefix << "LOFAR::Exception: " << exc);
  } catch (exception& exc) {
    LOG_FATAL_STR(logPrefix << "std::exception: " << exc.what());
  }

  // Propagate exit status != 0 from any input or output worker thread.
  for (unsigned i = 0; i < thrExitStatus.size(); ++i) {
    if (thrExitStatus[i] != 0) {
      status = 1;
      break;
    }
  }

  return status;
}

static int isAccessibleDirname(const string& dirname)
{
  struct stat st;

  if (stat(dirname.c_str(), &st) != 0) {
    return errno;
  }

  // Check if the last component is a dir too (stat() did the rest).
  if (!S_ISDIR(st.st_mode)) {
    return ENOTDIR;
  }

  return 0;
}

static void printUsage(const char* progname)
{
  cout << "LOFAR TBB_Writer version: " << LOFAR::OutputProcVersion::getVersion() << endl;
  cout << "Write incoming LOFAR TBB data with meta data to storage in HDF5 format." << endl;
  cout << "Usage: " << progname << " -p parsets/L12345.parset [OPTION]..." << endl;
  cout << endl;
  cout << "Options:" << endl;
  cout << "  -s, --parset=L12345.parset          path to file with observation settings (mandatory)" << endl;
  cout << endl;
  cout << "  -c, --stcaltablesdir=/c/CalTables   path to override retrieval of station calibration tables (like CS001/CalTable_001_mode1.dat) (NOTE: st cal tables retrieval currently not performed, even with this option!)" << endl; // TODO: remove NOTE when implemented
  cout << "  -m, --staticmetadatadir=/a/StaticMetaData  path to override $LOFARROOT for antenna field files (like CS001-AntennaField.conf)" << endl;
  cout << "  -o, --outputdir=tbbout              output directory" << endl;
  cout << "  -i, --input=tcp|udp|                input stream(s) or type (default: udp)" << endl;
  cout << "              file:raw.dat|               if file or pipe name has a '%'," << endl;
  cout << "              pipe:named-%.pipe           then the last '%' is replaced by numbers of available stream files/pipes (i.e. 0, 1, ..., 11)" << endl;
  cout << "  -p, --ports=31664,31665,31666       comma separated list of udp/tcp ports without duplicates to receive from (default: 12 ports starting at 31664)" << endl;
  cout << "  -t, --timeout=10                    seconds of input inactivity until dump is considered complete" << endl;
  cout << endl;
  cout << "  -k, --keeprunning=1|0               accept new input after a dump completed (default: 1)" << endl;
  cout << endl;
  cout << "  -h, --help                          print program name, version number and this info, then exit" << endl;
  cout << "  -v, --version                       same as --help" << endl;
}

static int parseArgs(int argc, char *argv[], struct progArgs* args)
{
  int status = 0;

  bool parsetFilenameSpecified = false; // there is no default parset filename, so not passing it is fatal 
  // Default values
  args->parsetFilename = "";
  args->stCalTablesDir = "";
  args->staticMetaDataDir = "";

  args->outputDir = "";
  args->input = "udp";
  args->ports.resize(TBB_DEFAULT_LAST_PORT - TBB_DEFAULT_BASE_PORT + 1);
  for (unsigned p = 0; p <= TBB_DEFAULT_LAST_PORT - TBB_DEFAULT_BASE_PORT; p++) {
    args->ports[p] = TBB_DEFAULT_BASE_PORT + p;
  }
  args->timeoutVal.tv_sec = 10;
  args->timeoutVal.tv_usec = 0;
  args->keepRunning = true;

  static const struct option long_opts[] = {
    // NOTE: If you change this, then also change the code below AND the printUsage() code above!
    // {const char *name, int has_arg, int *flag, int val}
    {"parset",            required_argument, NULL, 's'},
    {"stcaltablesdir",    required_argument, NULL, 'c'}, // station calibration tables
    {"staticmetadatadir", required_argument, NULL, 'm'}, // for antenna field info
    {"outputdir",         required_argument, NULL, 'o'},
    {"input",             required_argument, NULL, 'i'},
    {"ports",             required_argument, NULL, 'p'},
    {"timeout",           required_argument, NULL, 't'},

    {"keeprunning",       required_argument, NULL, 'k'},

    {"help",              no_argument,       NULL, 'h'},
    {"version",           no_argument,       NULL, 'v'},

    {NULL, 0, NULL, 0}}; // terminating NULL entry

  opterr = 0; // prevent error printing to stderr by getopt_long()
  int opt, err;
  while ((opt = getopt_long(argc, argv, "s:c:m:o:i:p:t:k:hv", long_opts, NULL)) != -1) {
    switch (opt) {
    case 's':
      args->parsetFilename = optarg;
      parsetFilenameSpecified = true;
      break;
    case 'c':
      args->stCalTablesDir = optarg;
      if (args->stCalTablesDir[0] != '\0' && args->stCalTablesDir[args->stCalTablesDir.size() - 1] != '/') {
        args->stCalTablesDir.push_back('/');
      }
      if ((err = isAccessibleDirname(args->stCalTablesDir)) != 0) {
        cerr << "TBB: station cal tab dir argument value " << optarg << ": " << strerror(err) << endl;
        status = 1;
      }
      break;
    case 'm':
      args->staticMetaDataDir = optarg;
      if (args->staticMetaDataDir[0] != '\0' && args->staticMetaDataDir[args->staticMetaDataDir.size() - 1] != '/') {
        args->staticMetaDataDir.push_back('/');
      }
      if ((err = isAccessibleDirname(args->staticMetaDataDir)) != 0) {
        cerr << "TBB: antenna field dir argument value " << optarg << ": " << strerror(err) << endl;
        status = 1;
      }
      break;
    case 'o':
      args->outputDir = optarg;
      if (args->outputDir[0] != '\0' && args->outputDir[args->outputDir.size() - 1] != '/') {
        args->outputDir.push_back('/');
      }
      if ((err = isAccessibleDirname(args->outputDir)) != 0) {
        cerr << "TBB: output dir argument value " << optarg << ": " << strerror(err) << endl;
        status = 1;
      }
      break;
    case 'i':
      if (strcmp(optarg, "tcp") == 0 || strcmp(optarg, "udp") == 0 ||
          strncmp(optarg, "file:", sizeof("file:") - 1) == 0 ||
          strncmp(optarg, "pipe:", sizeof("pipe:") - 1) == 0) {
        args->input = optarg;
      } else {
        cerr << "TBB: Invalid input argument value: " << optarg << endl;
        status = 1;
      }
      break;
    case 'p':
      try {
        args->ports.clear();
        vector<string> portStrings = LOFAR::StringUtil::split(optarg, ',');
        for (vector<string>::iterator it = portStrings.begin();
             it != portStrings.end(); ++it) {
          int port = boost::lexical_cast<int>(*it); // <int> to reject < 0
          if (port <= 0 || port >= 65536) {
            throw boost::bad_lexical_cast(); // abuse to have single catch
          }
          args->ports.push_back(static_cast<uint16_t>(port));
        }

        sort(args->ports.begin(), args->ports.end());
        if (unique(args->ports.begin(), args->ports.end()) != args->ports.end()) {
          cerr << "TBB: Warning: duplicates in port argument value" << endl;
        }
      } catch (boost::bad_lexical_cast& exc) {
        cerr << "TBB: Invalid port argument value: " << optarg << endl;
        status = 1;
      }
      break;
    case 't':
      try {
        args->timeoutVal.tv_sec = boost::lexical_cast<unsigned long>(optarg);
      } catch (boost::bad_lexical_cast& /*exc*/) {
        cerr << "TBB: Invalid timeout argument value: " << optarg << endl;
        status = 1;
      }
      break;
    case 'k':
      try {
        args->keepRunning = boost::lexical_cast<bool>(optarg);
      } catch (boost::bad_lexical_cast& /*exc*/) {
        cerr << "TBB: Invalid keeprunning argument value: " << optarg << endl;
        status = 1;
      }
      break;
    case 'h':
    case 'v':
      return 2;
    default: // '?'
      cerr << "TBB: Invalid program argument or missing argument value: " << argv[optind - 1] << endl;
      status = 1;
      break;
    }
  }

  if (optind < argc) {
    ostringstream oss;
    oss << "TBB: Failed to recognize arguments:";
    while (optind < argc) {
      oss << " " << argv[optind++]; // good enough
    }
    cerr << oss.str() << endl;
    status = 1;
  }

  if (!parsetFilenameSpecified) {
    cerr << "TBB: parameter set file must be specified on command-line" << endl;
    status = 1;
  }

  return status;
}

int main(int argc, char* argv[])
{
  LOFAR::Exception::TerminateHandler termHandler(LOFAR::Exception::terminate);

  struct progArgs args;
  int err;

  if ((err = parseArgs(argc, argv, &args)) != 0) {
    if (err == 2) {
      err = 0;
    }
    printUsage(argv[0]);
    return err;
  }

  INIT_LOGGER("TBB_Writer");

  setTermSigsHandler();

  const vector<string> inputStreamNames(getTBB_InputStreamNames(args.input, args.ports));
  if (inputStreamNames.empty()) {
    LOG_FATAL("TBB: none of the input streams is accessible to read from");
    return 1;
  }

  err = 1;
  try {
    LOFAR::Cobalt::Parset parset(args.parsetFilename);

    // We don't run alone, so try to increase the QoS we get from the OS to decrease the chance of data loss.
    if (parset.settings.realTime) {
      setIOpriority(); // reqs CAP_SYS_NICE
      setRTpriority(); // reqs CAP_SYS_ADMIN
      lockInMemory();  // reqs CAP_IPC_LOCK
    }

    LOFAR::Cobalt::StationMetaDataMap stMdMap(getExternalStationMetaData(parset, args.staticMetaDataDir));

    retrieveStationCalTables(args.stCalTablesDir);

    err = 0;
    do {
      err += doTBB_Run(inputStreamNames, parset, stMdMap, args);
    } while (args.keepRunning && err < 100);
    if (err == 100) { // Nr of dumps per obs was estimated to fit in 3 digits.
      LOG_FATAL("TBB: Reached max nr of errors seen. Shutting down to avoid filling up storage with logging crap.");
    }

    // Config exceptions (opening or parsing) are fatal. Too bad we cannot have it in one type.
  } catch (LOFAR::Cobalt::CoInterfaceException& exc) {
    LOG_FATAL_STR("TBB: Required parset key/values missing: " << exc);
  } catch (LOFAR::APSException& exc) {
    LOG_FATAL_STR("TBB: Parameterset error: " << exc);
  } catch (LOFAR::AssertError& exc) {
    LOG_FATAL_STR("TBB: Assert error: " << exc);
  } catch (LOFAR::Cobalt::StorageException& exc) {
    LOG_FATAL_STR("TBB: Antenna field files: " << exc);
  }

  return err != 0;
}

