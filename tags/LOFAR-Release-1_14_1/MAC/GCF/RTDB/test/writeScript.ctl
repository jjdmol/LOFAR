#uses "GCFCommon.ctl"

global time startTime;
global time stopTime;
global int nrWrites;

void main() {
  if (dpExists("Integer0000") == false) {
    DebugTN("DP 'Integer0000' does not exist");
    exit(1);
  }
  
  nrWrites = 0;
  DebugTN("Writing 1000 value changes to DP 'Integer0000'");
  startTime = getCurrentTime();
  while (nrWrites < 1000) {
    dpSet("Integer0000.", 6439-nrWrites);
    nrWrites++;
  }
  stopTime = getCurrentTime();
  time  delta = stopTime - startTime;
  string deltaStr = second(delta)+"."+milliSecond(delta);
  DebugTN(deltaStr);
  nrWrites = 0;
}
