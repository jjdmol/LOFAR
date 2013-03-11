//#  SSH.h
//#
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
//#  $Id: SSH.h 15296 2010-03-24 10:19:41Z mol $


//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#ifndef LOFAR_RTCP_SSH_H
#define LOFAR_RTCP_SSH_H

#include <string>

#include <Common/Thread/Thread.h>
#include <Common/Exception.h>
#include <Stream/FileDescriptorBasedStream.h>
#include <libssh2.h>
#include <CoInterface/SmartPtr.h>
#include <time.h>
#include <string>
#include <sstream>

namespace LOFAR {
namespace RTCP {

bool SSH_Init();
void SSH_Finalize();

// Sets up and maintains an SSH connection using LibSSH2.
class SSHconnection {
public:
  // The number of seconds to wait to retry if establishing a connection fails.
  static const unsigned RETRY_DELAY = 60;

  EXCEPTION_CLASS(SSHException, LOFAR::Exception);

  SSHconnection(const std::string &logPrefix, const std::string &hostname, const std::string &commandline, const std::string &username, const std::string &pubkey, const std::string &privkey, bool captureStdout = false);

  ~SSHconnection();

  // Start connecting
  void start();

  // Abort the connection
  void cancel();

  // Wait for the connection to finish
  void wait();
  void wait( const struct timespec &deadline );

  // Returns whether the connection is finished
  bool isDone();

  // Returns whether the SSH session is (or was) connected succesfully to the
  // remote SSH daemon.
  bool connected() const;

  // If stdout is captured, return the captured output
  std::string stdoutBuffer() const;

private:
  const string itsLogPrefix;
  const string itsHostName;
  const string itsCommandLine;
  const string itsUserName;
  const string itsPublicKey;
  const string itsPrivateKey;

  bool itsConnected;

  SmartPtr<Thread> itsThread;
  const bool itsCaptureStdout;
  std::stringstream itsStdoutBuffer;

  bool waitsocket( LIBSSH2_SESSION *session, FileDescriptorBasedStream &sock );

  LIBSSH2_SESSION *open_session( FileDescriptorBasedStream &sock );
  void close_session( LIBSSH2_SESSION *session, FileDescriptorBasedStream &sock );
  LIBSSH2_CHANNEL *open_channel( LIBSSH2_SESSION *session, FileDescriptorBasedStream &sock );
  void close_channel( LIBSSH2_SESSION *session, LIBSSH2_CHANNEL *channel, FileDescriptorBasedStream &sock );

  void commThread();
};

// Discover the file name to the .ssh public/private key files,
// and put them in pubkey and privkey. Returns true if the files
// were found, and were usable for SSHconnection to localhost:22.
bool discover_ssh_keys(char *pubkey, size_t pubkey_buflen, char *privkey, size_t privkey_buflen);

} // namespace RTCP
} // namespace LOFAR


#endif
