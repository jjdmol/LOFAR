//# SSH.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#ifndef LOFAR_GPUPROC_SSH_H
#define LOFAR_GPUPROC_SSH_H

#include <ctime>
#include <string>
#include <sstream>
#include <iostream>
#include <libssh2.h>

#include <Common/Thread/Thread.h>
#include <Common/Exception.h>
#include <Stream/FileDescriptorBasedStream.h>
#include <CoInterface/SmartPtr.h>

namespace LOFAR
{
  namespace Cobalt
  {

    bool SSH_Init();
    void SSH_Finalize();

    // Sets up and maintains an SSH connection using LibSSH2.
    class SSHconnection
    {
    public:
      // The number of micro seconds to wait to retry if establishing a connection fails.
      static const unsigned RETRY_USECS = 10000000; // 10 secs

      EXCEPTION_CLASS(SSHException, LOFAR::Exception);

      /* Set up an SSH connection in the background.
       *
       * Emulates
       *   ssh username@hostname -p 22 -i pubkey -i privkey commandline 1>_cout 2>_cerr
       *
       * logPrefix:     prefix log lines with this string.
       * reportErrors:  errors are logged using LOG_ERROR (instead of LOG_DEBUG).
       * captureStdout: redirect stdout to a buffer, see stdoutBuffer().
       */
      SSHconnection(const std::string &logPrefix, const std::string &hostname,
                    const std::string &commandline, const std::string &username,
                    const std::string &pubkey, const std::string &privkey,
                    bool reportErrors = true, bool captureStdout = false, ostream &_cout = cout, ostream &_cerr = cerr);

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
      // remote SSH daemon (incl an opened session and an opened channel to send commands).
      bool connected() const;

      // If stdout is captured, return the captured output. Note that the
      // contents of the buffer is undefined if the connection is not finished.
      std::string stdoutBuffer() const;

    private:
      const std::string itsLogPrefix;
      const std::string itsHostName;
      const std::string itsCommandLine;
      const std::string itsUserName;
      const std::string itsPublicKey;
      const std::string itsPrivateKey;
      const bool itsReportErrors;
      const bool itsCaptureStdout;
      std::ostream &itsCout;
      std::ostream &itsCerr;

      bool itsConnected;
      std::stringstream itsStdoutBuffer;
      SmartPtr<Thread> itsThread;


      int waitsocket( LIBSSH2_SESSION *session, FileDescriptorBasedStream &sock );

      LIBSSH2_SESSION *open_session( FileDescriptorBasedStream &sock, bool& authError );
      void close_session( LIBSSH2_SESSION *session, FileDescriptorBasedStream &sock );
      LIBSSH2_CHANNEL *open_channel( LIBSSH2_SESSION *session, FileDescriptorBasedStream &sock );
      void close_channel( LIBSSH2_SESSION *session, LIBSSH2_CHANNEL *channel, FileDescriptorBasedStream &sock );

      void commThread();
    };

    // Discover the file name to the .ssh public/private key files,
    // and put them in pubkey and privkey. Returns true if the files
    // were found, and were usable for SSHconnection to localhost:22.
    bool discover_ssh_keys(char *pubkey, size_t pubkey_buflen, char *privkey, size_t privkey_buflen);

  } // namespace Cobalt
} // namespace LOFAR

#endif

