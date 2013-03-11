#include <lofar_config.h>

#include <Storage/SSH.h>
#include <unistd.h>
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <Stream/SocketStream.h>
#include <Common/LofarLogger.h>

char pubkey[1024];
char privkey[1024];

using namespace LOFAR;
using namespace RTCP;


void test_SSHconnection( const char *cmd, bool capture ) {
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

int main() {
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
