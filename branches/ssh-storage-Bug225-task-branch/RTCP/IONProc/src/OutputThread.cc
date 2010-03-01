//#  OutputThread.cc:
//#
//#  Copyright (C) 2008
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Semaphore.h>
#include <Interface/Mutex.h>
#include <IONProc/OutputThread.h>
#include <IONProc/ION_Allocator.h>
#include <Stream/SystemCallException.h>
#include <Scheduling.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#include <memory>

#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/format.hpp>


namespace LOFAR {
namespace RTCP {


std::stack<unsigned, std::vector<unsigned> > OutputThread::theFreePorts;

Mutex OutputThread::theFreePortsMutex;
Mutex OutputThread::theCheckPasswordFileMutex;


OutputThread::OutputThread(const Parset &ps, const unsigned subband, const unsigned output, StreamableData *dataTemplate)
:
  itsConnecting(true), // avoid race condition when checking this at thread start
  itsParset(ps),
  itsSubband(subband),
  itsOutput(output),
  //itsDescription(std::string("OutputThread: ObsID = ") + boost::lexical_cast<unsigned>(ps.observationID()) + ", subband = " + boost::lexical_cast<unsigned>(subband) + ", output = " + boost::lexical_cast<unsigned>(output)),
  itsDescription(boost::str(boost::format("OutputThread: ObsID = %u, subband = %u, output = %u") % ps.observationID() % subband % output)),
  itsPortNumber(0),
  itsSocketName(getSocketName()),
  itsChildPid(0)
{
  LOG_DEBUG_STR(itsDescription << ": OutputThread::OutputThread()");

  // transpose the data holders: create queues streams for the output streams
  // itsPlans is the owner of the pointers to sample data structures
  for (unsigned i = 0; i < maxSendQueueSize; i ++) {
    StreamableData *clone = dataTemplate->clone();

    clone->allocate();
    itsFreeQueue.append(clone);
  }

  //thread = new Thread(this, &OutputThread::mainLoop, str(format("OutputThread (obs %d sb %d output %d)") % ps.observationID() % subband % output), 65536);
  forkSSH();
  itsThread = new InterruptibleThread(this, &OutputThread::mainLoop, 65536);
}


OutputThread::~OutputThread()
{
  // STOP our thread
  itsSendQueue.append(0); // 0 indicates that no more messages will be sent

#if 0
  if (itsConnecting)
    itsThread->abort();
#endif

  LOG_DEBUG_STR(itsDescription << "AAA 1");
  delete itsThread;
  LOG_DEBUG_STR(itsDescription << "AAA 2");

  //LOG_INFO_STR(itsDescription << ": waiting for Storage process to finish");
  joinSSH();
  LOG_DEBUG_STR(itsDescription << "AAA 3");

  while (!itsSendQueue.empty())
    delete itsSendQueue.remove();

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();

  if (itsPortNumber != 0) {
    ScopedLock scopedLock(theFreePortsMutex);
    theFreePorts.push(itsPortNumber);
  }
}


void OutputThread::execSSH(const char *sshKey, const char *userName, const char *hostName, const char *executable, const char *parset, const char *socketName, const char *subband, const char *output)
{
  // DO NOT DO ANY CALL THAT GRABS A LOCK, since the lock may be held by a thread
  // that is no longer part of our address space

  execl("/usr/bin/ssh",
    "ssh",
    "-i", sshKey,
    "-c", "blowfish",
    "-o", "StrictHostKeyChecking=no",
    "-o", "UserKnownHostsFile=/dev/null",
    "-q",
    "-l", userName,
    hostName,
    executable,
    parset,
    socketName,
    subband,
    output,
    static_cast<void *>(0)
  );

  write(2, "exec failed", 11); // Logger uses threads
  exit(1);
}


void OutputThread::forkSSH()
{
  std::string userName	 = itsParset.getString("OLAP.Storage.userName");
  std::string sshKey	 = std::string("/globalhome/") + userName + "/.ssh/id_rsa";
  std::string hostName	 = itsParset.storageHostName("OLAP.OLAP_Conn.IONProc_Storage_ServerHosts", itsSubband);
  std::string executable = itsParset.getString("OLAP.Storage.msWriter");
  std::string parset	 = itsParset.name();
  std::string subband	 = boost::lexical_cast<std::string>(itsSubband);
  std::string output	 = boost::lexical_cast<std::string>(itsOutput);

  const char *sshKeyPtr	    = sshKey.c_str();
  const char *userNamePtr   = userName.c_str();
  const char *hostNamePtr   = hostName.c_str();
  const char *executablePtr = executable.c_str();
  const char *parsetPtr	    = parset.c_str();
  const char *socketNamePtr = itsSocketName.c_str();
  const char *subbandPtr    = subband.c_str();
  const char *outputPtr     = output.c_str();

  LOG_INFO_STR(itsDescription << ": child will exec("
    "\"/usr/bin/ssh\", "
    "\"ssh\", "
    "\"-i\", \"" << sshKey << "\", "
    "\"-c\", \"blowfish\", "
    "\"-o\", \"StrictHostKeyChecking=no\", "
    "\"-o\", \"UserKnownHostsFile=/dev/null\", "
    "\"-q\", "
    "\"-l\", \"" << userName << "\", "
    "\"" << hostName << "\", "
    "\"" << executable << "\", "
    "\"" << parset << "\", "
    "\"" << itsSocketName << "\", "
    "\"" << itsSubband << "\", "
    "\"" << itsOutput << "\", "
    "0)"
  );

  switch (itsChildPid = fork()) {
    case -1 : throw SystemCallException("fork", errno, THROW_ARGS);

    case  0 : execSSH(sshKeyPtr, userNamePtr, hostNamePtr, executablePtr, parsetPtr, socketNamePtr, subbandPtr, outputPtr);
  }
}


void OutputThread::joinSSH()
{
  if (itsChildPid != 0) {
    int status;

    if (waitpid(itsChildPid, &status, 0) == -1)
      LOG_WARN_STR(itsDescription << ": waitpid failed"); // do not throw exception, may be in destructor
    else if (WIFSIGNALED(status) != 0)
      LOG_WARN_STR(itsDescription << ": storage writer was killed by signal " << WTERMSIG(status));
    else if (WEXITSTATUS(status) != 0)
      LOG_WARN_STR(itsDescription << ": storage writer exited with exit code " << WEXITSTATUS(status));
    else
      LOG_INFO_STR(itsDescription << ": storage writer terminated normally");
  }
}


void OutputThread::getPortNumber()
{
  ScopedLock scopedLock(theFreePortsMutex);

  static bool initialized;

  if (!initialized) {
    for (unsigned port = 6300; port < 6500; port ++)
      theFreePorts.push(port);

    initialized = true;
  }

  if (theFreePorts.empty())
    throw IONProcException("no free TCP ports to Storage left", THROW_ARGS);

  itsPortNumber = theFreePorts.top();
  theFreePorts.pop();
}


std::string OutputThread::getSocketName()
{
  getPortNumber();

  extern std::string myIPaddress;
  return std::string("tcp:") + myIPaddress + ':' + boost::lexical_cast<std::string>(itsPortNumber);
}


// set the maximum number of concurrent writers
static Semaphore semaphore(2);

void OutputThread::mainLoop()
{
  LOG_DEBUG_STR(itsDescription << ": OutputThread::mainLoop()");

#if defined HAVE_BGP_ION
  doNotRunOnCore0();
  nice(19);
#endif

  // connect to storage
  std::string prefix         = "OLAP.OLAP_Conn.IONProc_Storage";
  std::string connectionType = itsParset.getString(prefix + "_Transport");


#if 0
  std::auto_ptr<Stream> streamToStorage;

  if (connectionType == "NULL") {
    LOG_DEBUG_STR("subband " << itsSubband << " written to null:");
    streamToStorage.reset(new NullStream);
  } else if (connectionType == "TCP") {
    std::string    server = itsParset.storageHostName(prefix + "_ServerHosts", itsSubband);
    unsigned short port   = itsParset.getStoragePort(prefix, itsSubband, itsOutput);
  
    LOG_DEBUG_STR("subband " << itsSubband << " written to tcp:" << server << ':' << port << " connecting..");
    streamToStorage.reset(new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Client));
    LOG_DEBUG_STR("subband " << itsSubband << " written to tcp:" << server << ':' << port << " connect DONE");
  } else if (connectionType == "FILE") {
    std::string filename = itsParset.getString(prefix + "_BaseFileName") + '.' +
      boost::lexical_cast<std::string>(itsSubband);
    //boost::lexical_cast<std::string>(storagePortIndex);
  
    LOG_DEBUG_STR("subband " << itsSubband << " written to file:" << filename);
    streamToStorage.reset(new FileStream(filename.c_str(), 0666));
  } else {
    THROW(IONProcException, "unsupported ION->Storage stream type: " << connectionType);
  }
#endif

  LOG_INFO_STR(itsDescription << ": waiting for connection to " << itsSocketName);
  std::auto_ptr<Stream> streamToStorage(Parset::createStream(itsSocketName, true));
  itsConnecting = false;
  LOG_INFO_STR(itsDescription << ": connection to " << itsSocketName << " successful");

  // TODO: race condition on creation
  // TODO: if a storage node blocks, ionproc can't write anymore
  //       in any thread
  StreamableData *data;

  while ((data = itsSendQueue.remove()) != 0) {
    // prevent too many concurrent writers by locking this scope
    semaphore.down();
    try {
      // write data, including serial nr
      data->write(streamToStorage.get(), true);
    } catch (...) {
      semaphore.up();
      itsFreeQueue.append(data); // make sure data will be freed
      throw;
    }

    semaphore.up();

    // data can now be reused
    itsFreeQueue.append(data);
  }

  delete streamToStorage.release(); // close socket
}

} // namespace RTCP
} // namespace LOFAR
