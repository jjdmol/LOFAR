//#  SSH.cc: setup an SSH connection using libssh2
//#
//#  Copyright (C) 2012
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
//#  $Id: SSH.cc 18226 2012-06-09 12:56:47Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <SSH.h>

#include <Common/Thread/Cancellation.h>
#include <Common/SystemCallException.h>
#include <Common/LofarLogger.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <string>
#include <sstream>

#ifdef HAVE_LIBSSH2
#include <Scheduling.h>
#include <sstream>
#include <sys/select.h>
#include <Common/lofar_string.h>
#include <Stream/SocketStream.h>
#endif

using namespace std;

namespace LOFAR {
namespace RTCP {

#ifdef HAVE_LIBSSH2
  
SSHconnection::SSHconnection(const string &logPrefix, const string &hostname, const string &commandline, const string &username, const string &sshkey)
:
  itsLogPrefix(logPrefix),
  itsHostName(hostname),
  itsCommandLine(commandline),
  itsUserName(username),
  itsSSHKey(sshkey)
{
}

void SSHconnection::start()
{
  itsThread = new Thread(this, &SSHconnection::commThread, itsLogPrefix + "[SSH Thread] ", 65536);
}

void SSHconnection::stop( const struct timespec &deadline )
{
  if (!itsThread->wait(deadline)) {
    itsThread->cancel();

    itsThread->wait();
  }
}

void SSHconnection::free_session( LIBSSH2_SESSION *session )
{
  if (!session)
    return;

  libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
  libssh2_session_free(session);
}

void SSHconnection::free_channel( LIBSSH2_CHANNEL *channel )
{
  if (!channel)
    return;

  libssh2_channel_free(channel);
}

bool SSHconnection::open_session( FileDescriptorBasedStream &sock )
{
  int rc;

  /* Create a session instance */
  session = libssh2_session_init();
  if (!session) {
    LOG_ERROR_STR( itsLogPrefix << "Cannot create SSH session object" );
    return false;
  }

  /* tell libssh2 we want it all done non-blocking */
  libssh2_session_set_blocking(session, 0);

  /* ... start it up. This will trade welcome banners, exchange keys,
   * and setup crypto, compression, and MAC layers
   */
  while ((rc = libssh2_session_handshake(session, sock.fd)) ==
         LIBSSH2_ERROR_EAGAIN) {
    waitsocket(sock);
  }

  if (rc) {
    LOG_ERROR_STR( itsLogPrefix << "Failure establishing SSH session: " << rc);
    return false;
  }

  /* Authenticate by public key */
  while ((rc = libssh2_userauth_publickey_fromfile(session,
                      itsUserName.c_str(), // remote username
                      NULL,                // public key filename
                      itsSSHKey.c_str(),   // private key filename
                      NULL                 // password
                      )) ==
         LIBSSH2_ERROR_EAGAIN) {
    waitsocket(sock);
  }

  if (rc) {
    LOG_ERROR_STR( itsLogPrefix << "Authentication by public key failed: " << rc);
    return false;
  }

  return true;
}

bool SSHconnection::open_channel( FileDescriptorBasedStream &sock )
{
  /* Exec non-blocking on the remote host */
  while( (channel = libssh2_channel_open_session(session)) == NULL &&
         libssh2_session_last_error(session,NULL,NULL,0) ==
         LIBSSH2_ERROR_EAGAIN )
  {
    waitsocket(sock);
  }

  if (!channel)
  {
    LOG_ERROR_STR( itsLogPrefix << "Could not set up SSH channel" );
    return false;
  }

  return true;
}

bool SSHconnection::close_channel( FileDescriptorBasedStream &sock )
{
  int rc;

  while( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN ) {
    waitsocket(sock);
  }

  return true;
}

bool SSHconnection::waitsocket( FileDescriptorBasedStream &sock )
{
  struct timeval timeout;
  int rc;
  fd_set fd;
  fd_set *writefd = NULL;
  fd_set *readfd = NULL;
  int dir;

  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  FD_ZERO(&fd);

  FD_SET(sock.fd, &fd);

  /* now make sure we wait in the correct direction */
  dir = libssh2_session_block_directions(session);

  if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
      readfd = &fd;

  if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
      writefd = &fd;

  {
    Cancellation::enable();

    // select() is a cancellation point
    rc = ::select(sock.fd + 1, readfd, writefd, NULL, &timeout);

    Cancellation::disable();
  }   

  return rc > 0;
}

void SSHconnection::commThread()
{
#if defined HAVE_BGP_ION
  //doNotRunOnCore0();
  runOnCore0();
  //nice(19);
#endif

  SocketStream sock( itsHostName, 22, SocketStream::TCP, SocketStream::Client );

  LOG_DEBUG_STR( itsLogPrefix << "Connected" );

  int rc;
  int exitcode;
  char *exitsignal=(char *)"none";

  /* Prevent cancellation from here on -- we manually insert cancellation points to avoid
     screwing up libssh2's internal administration. */
  Cancellation::disable();
  Cancellation::point();

  if (!open_session(sock))
    return;

  if (!open_channel(sock))
    return;

  while( (rc = libssh2_channel_exec(channel, itsCommandLine.c_str())) ==
         LIBSSH2_ERROR_EAGAIN )
  {
    waitsocket(sock);
  }

  if (rc)
  {
    LOG_ERROR_STR( itsLogPrefix << "Failure starting remote command: " << rc);
    return;
  }

  LOG_DEBUG_STR( itsLogPrefix << "Remote command started, waiting for output" );

  // raw input buffer
  char data[0x1000];

  // the current line (or line remnant)
  string line("");

  /* Session I/O */
  for( ;; )
  {
    /* loop until we block */
    do {
      rc = libssh2_channel_read( channel, data, sizeof data );
      if( rc > 0 )
      {
        // create a buffer for line + data
        stringstream buffer;

        buffer << line;
        buffer.write( data, rc );

        /* extract and log lines */
        for( ;; )
        {
          Cancellation::point();

          std::getline( buffer, line );

          if (!buffer.good()) {
            // 'line' now holds the remnant
            break;
          }

          // TODO: Use logger somehow (we'd duplicate the prefix if we just use LOG_* macros..)
          cout << line << endl;
        }
      } else {
        if( rc < 0 && rc != LIBSSH2_ERROR_EAGAIN ) {
          /* no need to output this for the EAGAIN case */
          LOG_ERROR_STR( itsLogPrefix << "libssh2_channel_read returned " << rc);
        }   
      }
    } while( rc > 0 );

    /* this is due to blocking that would occur otherwise so we loop on
       this condition */
    if( rc == LIBSSH2_ERROR_EAGAIN )
    {
      waitsocket(sock);
    } else {
      /* EOF */
      break;
    }    
  }

  LOG_DEBUG_STR( itsLogPrefix << "Disconnecting" );

  close_channel(sock);

  if (rc == 0)
  {
    exitcode = libssh2_channel_get_exit_status( channel );
    libssh2_channel_get_exit_signal(channel, &exitsignal,
                                    NULL, NULL, NULL, NULL, NULL);
  } else {
    exitcode = 127;
  }

  if (exitsignal) {
    LOG_ERROR_STR(itsLogPrefix << "SSH was killed by signal " << exitsignal);
  } else if(exitcode > 0) {
    LOG_ERROR_STR(itsLogPrefix << "Exited with exit code " << exitcode << " (" << explainExitStatus(exitcode) << ")" );
  } else {
    LOG_INFO_STR(itsLogPrefix << "Terminated normally");
  }
}

#endif

static void exitwitherror( const char *errorstr )
{
  // can't cast to (void) since gcc won't allow that as a method to drop the result
  int ignoreResult;

  ignoreResult = write(STDERR_FILENO, errorstr, strlen(errorstr)+1);

  // use _exit instead of exit to avoid calling atexit handlers in both
  // the master and the child process.
  _exit(1);
}

static void execSSH(const char * const sshParams[])
{
  // DO NOT DO ANY CALL THAT GRABS A LOCK, since the lock may be held by a
  // thread that is no longer part of our address space

  // use write() for output since the Logger uses a mutex, and printf also holds locks

  // Prevent cancellation due to race conditions. A cancellation can still be pending for this JobThread, in which case one of the system calls
  // below triggers it. If this thread/process can be cancelled, there will be multiple processes running, leading to all kinds of Bad Things.
  Cancellation::disable();

  // close all file descriptors other than stdin/out/err, which might have been openend by
  // other threads at the time of fork(). We brute force over all possible fds, most of which will be invalid.
  for (int f = sysconf(_SC_OPEN_MAX); f > 2; --f)
    (void)close(f);

  // create a valid stdin from which can be read (a blocking fd created by pipe() won't suffice anymore for since at least OpenSSH 5.8)
  // rationale: this forked process inherits stdin from the parent process, which is unusable because IONProc is started in the background
  // and routed through mpirun as well. Also, it is shared by all forked processes. Nevertheless, we want Storage to be able to determine
  // when to shut down based on whether stdin is open. So we create a new stdin.
  int devzero = open("/dev/zero", O_RDONLY);

  if (devzero < 0)
    exitwitherror("cannot open /dev/zero\n");

  if (close(0) < 0)
    exitwitherror("cannot close stdin\n");

  if (dup(devzero) < 0)
    exitwitherror("cannot dup /dev/zero into stdin\n");

  if (close(devzero) < 0)
    exitwitherror("cannot close /dev/zero\n");

  if (execv("/usr/bin/ssh", const_cast<char * const *>(sshParams)) < 0)
    exitwitherror("execv failed\n");

  exitwitherror("execv succeeded but did return\n");
}


pid_t forkSSH(const std::string &logPrefix, const char *hostName, const char * const extraParams[], const char *userName, const char *sshKey)
{
  pid_t pid;

  LOG_INFO_STR(logPrefix << "Starting");

  vector<const char*> sshParams;

  const char * const defaultParams[] = {
    "ssh",
    "-q",
    "-i", sshKey,
    "-c", "blowfish",
    "-o", "StrictHostKeyChecking=no",
    "-o", "UserKnownHostsFile=/dev/null",
    "-o", "ServerAliveInterval=30",
    "-l", userName,
    hostName,
    0 };

  for( const char * const *p = defaultParams; *p != 0; p++ )
    sshParams.push_back(*p);

  for( const char * const *p = extraParams; *p != 0; p++ )
    sshParams.push_back(*p);

  sshParams.push_back(0);

  switch (pid = fork()) {
    case -1 : throw SystemCallException("fork", errno, THROW_ARGS);

    case  0 : execSSH(&sshParams[0]);
  }

  return pid;
}


void joinSSH(const std::string &logPrefix, pid_t pid, unsigned &timeout)
{
  if (pid != 0) {
    int status;

    // always try at least one waitpid(). if child has not exited, optionally
    // sleep and try again.
    for (;;) {
      pid_t ret;

      if ((ret = waitpid(pid, &status, WNOHANG)) == (pid_t)-1) {
        int error = errno;

        if (error == EINTR) {
          LOG_DEBUG_STR(logPrefix << " waitpid() was interrupted -- retrying");
          continue;
        }

        // error
        LOG_WARN_STR(logPrefix << " waitpid() failed with errno " << error);
        return;
      } else if (ret == 0) {
        // child still running
        if (timeout == 0) {
          break;
        }

        timeout--;
        sleep(1);
      } else {
        // child exited
        if (WIFSIGNALED(status) != 0)
          LOG_WARN_STR(logPrefix << "SSH was killed by signal " << WTERMSIG(status));
        else if (WEXITSTATUS(status) != 0) {

          LOG_ERROR_STR(logPrefix << " exited with exit code " << WEXITSTATUS(status) << " (" << explainExitStatus(WEXITSTATUS(status)) << ")" );
        } else
          LOG_INFO_STR(logPrefix << " terminated normally");

        return;  
      }
    }

    // child did not exit within the given timeout

    LOG_WARN_STR(logPrefix << " sending SIGTERM");
    kill(pid, SIGTERM);

    if (waitpid(pid, &status, 0) == -1) {
      LOG_WARN_STR(logPrefix << " waitpid() failed");
    }

    LOG_WARN_STR(logPrefix << " terminated after sending SIGTERM");
  }
}


const char *explainExitStatus( int exitstatus )
{
  const char *explanation;

  switch (exitstatus) {
    default:
      explanation = "??";
      break;

    case 255:
      explanation = "Network or authentication error";
      break;
    case 127:
      explanation = "BASH: command/library not found";
      break;
    case 126:
      explanation = "BASH: command found but could not be executed (wrong architecture?)";
      break;

    case 128 + SIGHUP:
      explanation = "killed by SIGHUP";
      break;
    case 128 + SIGINT:
      explanation = "killed by SIGINT (Ctrl-C)";
      break;
    case 128 + SIGQUIT:
      explanation = "killed by SIGQUIT";
      break;
    case 128 + SIGILL:
      explanation = "illegal instruction";
      break;
    case 128 + SIGABRT:
      explanation = "killed by SIGABRT";
      break;
    case 128 + SIGKILL:
      explanation = "killed by SIGKILL";
      break;
    case 128 + SIGSEGV:
      explanation = "segmentation fault";
      break;
    case 128 + SIGPIPE:
      explanation = "broken pipe";
      break;
    case 128 + SIGALRM:
      explanation = "killed by SIGALRM";
      break;
    case 128 + SIGTERM:
      explanation = "killed by SIGTERM";
      break;
  }

  return explanation;
}

} // namespace RTCP
} // namespace LOFAR

