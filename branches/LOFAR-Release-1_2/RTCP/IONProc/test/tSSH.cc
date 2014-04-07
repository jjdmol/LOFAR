#include <lofar_config.h>

#include <SSH.h>
#include <unistd.h>
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <Stream/SocketStream.h>
#include <Common/LofarLogger.h>

// some useful environment variables
char *USER;
char *HOME;

// the existence of $HOME/.ssh/id_rsa is assumed,
// as well as the fact that it can be used to
// authenticate on localhost.
char privkey[1024];

using namespace LOFAR;
using namespace RTCP;


void test_SSHconnection() {
  SSHconnection ssh("", "localhost", "echo SSHconnection success", USER, privkey);

  ssh.start();

  struct timespec ts;
  ts.tv_sec = time(0) + 10;
  ts.tv_nsec = 0;

  ssh.stop(ts);

}


void test_forkExec() {
  pid_t pid;

  char * const params[] = {
    "echo",
    "forkExec success",
    0
  };
  
  pid = forkSSH("", "localhost", params, USER, privkey);

  unsigned timeout = 10;
  joinSSH("", pid, timeout);
}

int main() {
  INIT_LOGGER( "tSSH" );

  USER = getenv("USER");
  HOME = getenv("HOME");
  snprintf(privkey, sizeof privkey, "%s/.ssh/id_rsa", HOME);

  test_SSHconnection();
  test_forkExec();

  return 0;
}
