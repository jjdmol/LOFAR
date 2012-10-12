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


void test_SSHconnection( const char *cmd ) {
#ifdef HAVE_LIBSSH2
  SSHconnection ssh("", "localhost", cmd, USER, privkey);

  ssh.start();

  struct timespec ts;
  ts.tv_sec = time(0) + 10;
  ts.tv_nsec = 0;

  ssh.stop(ts);
#endif
}


void test_forkExec() {
  pid_t pid;

  const char * const params[] = {
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

  // can we even ssh to localhost?
  char sshcmd[1024];
  snprintf(sshcmd, sizeof sshcmd, "ssh %s@localhost -o PasswordAuthentication=no -o KbdInteractiveAuthentication=no -o NoHostAuthenticationForLocalhost=yes -i %s echo system success", USER, privkey);
  int ret = system(sshcmd);
  if (ret < 0 || WEXITSTATUS(ret) != 0) {
    // no -- mark this test as unrunnable and don't attempt to try with libssh then
    return 3;
  }  

  SSH_Init();

  test_SSHconnection( "echo SSHconnection success [stdout]" );
  test_SSHconnection( "echo SSHconnection success [stderr] 1>&2" );
  test_SSHconnection( "echo SSHconnection success [stderr] 1>&2; echo SSHconnection success [stdout]" );
  test_SSHconnection( "echo SSHconnection success [stdout]; echo SSHconnection success [stderr] 1>&2" );
  test_forkExec();

  SSH_Finalize();

  return 0;
}
