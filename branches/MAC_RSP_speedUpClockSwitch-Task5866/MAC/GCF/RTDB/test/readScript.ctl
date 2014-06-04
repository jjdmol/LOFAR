#uses "GCFCommon.ctl"

global time startTime;
global time stopTime;
global int nrReads;

void main() {
  if (dpExists("Integer0000") == false) {
    DebugTN("DP 'Integer0000' does not exist");
    exit(1);
  }
  
  nrReads = 0;
  dpConnect("readTrigger",false,"Integer0000.");
  DebugTN("Waiting for 1000 value changes on DP 'Integer0000'");
}

void readTrigger(string dp1, dyn_int systemList) {
  if (nrReads == 0) {
    DebugTN("Received first trigger");
    startTime = getCurrentTime();
  }
  nrReads++;
  if (nrReads >= 1000) {
    stopTime = getCurrentTime();
    time  delta = stopTime - startTime;
    string deltaStr = second(delta)+"."+milliSecond(delta);
    DebugTN(deltaStr);
    nrReads = 0;
  }
}

