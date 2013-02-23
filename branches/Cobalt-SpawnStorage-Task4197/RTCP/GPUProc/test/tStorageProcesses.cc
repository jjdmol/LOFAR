#include <lofar_config.h>

#include <StorageProcesses.h>
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <Common/LofarLogger.h>
#include <string>

char privkey[1024];

using namespace LOFAR;
using namespace RTCP;
using namespace std;

void test_simple() {
  // Test whether executing an application works. The
  // communication protocol after startup is ignored.

  const char *USER = getenv("USER");
  Parset p;

  p.add("Observation.ObsID",            "12345");
  p.add("OLAP.Storage.hosts",           "[localhost]");
  p.add("OLAP.Storage.userName",        USER);
  p.add("OLAP.Storage.sshIdentityFile", privkey);
  p.add("OLAP.Storage.msWriter",        "/bin/echo");

  {
    StorageProcesses sp(p, "");

    // give 'ssh localhost' time to succeed
    sleep(2);
  }
}

void test_protocol() {
  // Test whether we follow the communication protocol
  // as expected by Storage_main.

  const char *USER = getenv("USER");
  Parset p;

  p.add("Observation.ObsID",            "12345");
  p.add("OLAP.Storage.hosts",           "[localhost]");
  p.add("OLAP.Storage.userName",        USER);
  p.add("OLAP.Storage.sshIdentityFile", privkey);
  p.add("OLAP.Storage.msWriter",        "./DummyStorage");
  p.add("OLAP.FinalMetaDataGatherer.host",            "localhost");
  p.add("OLAP.FinalMetaDataGatherer.userName",        USER);
  p.add("OLAP.FinalMetaDataGatherer.sshIdentityFile", privkey);

  // DummyStorage already emulates the FinalMetaDataGatherer, so use a dummy
  // instead.
  p.add("OLAP.FinalMetaDataGatherer.executable",      "/bin/echo");

  {
    StorageProcesses sp(p, "");

    // Give Storage time to log its parset
    sleep(2);

    // Give 10 seconds to exchange final meta data
    sp.forwardFinalMetaData(time(0) + 10);

    // Give 10 seconds to wrap up
    sp.stop(time(0) + 10);
  }
}

int main() {
  INIT_LOGGER( "tStorageProcesses" );

  // prevent stalls
  alarm(60);

  if (!discover_ssh_privkey(privkey, sizeof privkey))
    return 3;

  SSH_Init();

  test_simple();
  test_protocol();

  SSH_Finalize();

  return 0;
}
