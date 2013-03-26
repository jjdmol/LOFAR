//# tSSH.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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
//# $Id: $

#include <lofar_config.h>

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <unistd.h>

#include <Common/LofarLogger.h>
#include <Stream/SocketStream.h>

#include <Storage/SSH.h>

char pubkey[1024];
char privkey[1024];

using namespace LOFAR;
using namespace Cobalt;


void test_SSHconnection( const char *cmd, bool capture )
{
  const char *USER = getenv("USER");
  SSHconnection ssh("", "localhost", cmd, USER, pubkey, privkey, capture);

  ssh.start();

  struct timespec ts;
  ts.tv_sec = time(0) + 10;
  ts.tv_nsec = 0;

  ssh.wait(ts);

  if (capture)
    cout << "Captured [" << ssh.stdoutBuffer() << "]" << endl;
}

int main()
{
  INIT_LOGGER( "tSSH" );

  // discover a working private key
  if (!discover_ssh_keys(pubkey, sizeof pubkey, privkey, sizeof privkey))
    return 3;

  SSH_Init();

  test_SSHconnection( "echo stdout read [stdout]", false );
  test_SSHconnection( "echo stderr read [stderr] 1>&2", false );

  test_SSHconnection( "echo capture stdout [stdout]", true );
  test_SSHconnection( "echo capture stdout [stdout]; echo but not capture stderr [stderr] 1>&2", true );

  test_SSHconnection( "echo stderr first [stderr] 1>&2; echo stdout second [stdout]", false );
  test_SSHconnection( "echo stdout first [stdout]; echo stderr second [stderr] 1>&2", false );

  SSH_Finalize();

  return 0;
}

