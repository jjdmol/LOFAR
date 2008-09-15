// *******************************************
// Name : LofarSpeedTest
// *******************************************
// Description: Waits for a testname on dp:
//
//
//
// Returns:
//    None
//
// *******************************************

global bool isStation=false; 
global bool isRunning=false;
global time startTime;
global time endTime;
global int  nrReads=0;
global int  nrWrites=0;
global dyn_string strArray;
global string stationName="";

global bool isTestTriggered = false;
global bool isPingpongStationIntTrigger = false;
global bool isPingpongCentralIntTrigger = false;
global bool isPingpongStationStringTrigger = false;
global bool isPingpongCentralStringTrigger = false;
global bool isPingpongCentralStringTrigger = false;

#uses "LofarSpeedTest/lofarSpeedTestCommon.ctl"

//
 
void main()
{

  // determine if the script is running on a station or on the central server.
  
  DebugTN( "***************************" );
  stationName=getSystemName();  
  if (stationName == "MCU001:") {
    isStation=false;
    stationName="";
    DebugTN( "Test is starting on " + getSystemName()+" ==> Central" );
  } else {
    isStation=true;
    DebugTN( "Test is starting on " + getSystemName()+" ==> Station" );
  }
  DebugTN( "***************************" );
  
  // fill the strArray
  fillStringArray();
  
  // connect to the LofarSpeedTest point to get a trigger when a new speedtest is required.
  if (dpExists("lofarSpeedTest")) {
    if (! isTestTriggered) {
      dpConnect("testTriggered",false,"lofarSpeedTest.result.testName");
      isTestTriggered=true;
    }
  } else {
    DebugTN("lofarSpeedTest point not found, no testing possible for now");  
  }
}

// *******************************************
// Name : testTriggered
// *******************************************
// Description:
// Callback that a new test is required. this routine will start up the required test
// if not another test is running allready on this machine.
//
// Returns:
//    None
// *******************************************
void testTriggered(string dp1, string testName) {
  if (isRunning) {
    DebugTN("it seems another test is still running, can't start a new one yet.\n" +
           "Please try again later.");
    return;
  }
  
  if (testName == "One station to central(1000 int x 1)" ){
    oneStationToCentral1000IntTo1();
  } else if (testName == "One station to central(1000 string x 1)" ){
    oneStationToCentral1000StringTo1();
  } else if (testName == "One station to central(100 int x 10)" ){
    oneStationToCentral100IntTo10();
  } else if (testName == "One station to central(100 string x 10)" ){
    oneStationToCentral100StringTo10();
  } else if (testName == "One station to central(100 int x 10 x 1)" ){
    oneStationToCentral100IntTo10x1();
  } else if (testName == "One station to central(100 string x 10 x 1)" ){
    oneStationToCentral100StringTo10x1();
  }else if (testName == "Central from one station(1000 int x 1)" ){
    centralFromOneStation1000IntTo1();
  } else if (testName == "Central from one station(1000 string x 1)" ){
    centralFromOneStation1000StringTo1();
  } else if (testName == "Central from one station(100 int x 10)" ){
    centralFromOneStation100IntTo10();
  } else if (testName == "Central from one station(100 string x 10)" ){
    centralFromOneStation100StringTo10();
  } else if (testName == "Central from one station(100 int x 10 x 1)" ){
    centralFromOneStation100IntTo10x1();
  } else if (testName == "Central from one station(100 string x 10 x 1)" ){
    centralFromOneStation100StringTo10x1();
  } else if (testName == "Two stations to central(1000 int x 1)" ){
    twoStationsToCentral1000IntTo1();
  } else if (testName == "Two stations to central(1000 string x 1)" ){
    twoStationsToCentral1000StringTo1();
  } else if (testName == "Two stations to central(100 int x 10)" ){
    twoStationsToCentral100IntTo10();
  } else if (testName == "Two stations to central(100 string x 10)" ){
    twoStationsToCentral100StringTo10();
  } else if (testName == "Two stations to central(100 int x 10 x 1)" ){
    twoStationsToCentral100IntTo10x1();
  } else if (testName == "Two stations to central(100 string x 10 x 1)" ){
    twoStationsToCentral100StringTo10x1();
  } else if (testName == "Central from two stations(1000 int x 1)" ){
    centralFromTwoStations1000IntTo1();
  } else if (testName == "Central from two stations(1000 string x 1)" ){
    centralFromTwoStations1000StringTo1();
  } else if (testName == "Central from two stations(100 int x 10)" ){
    centralFromTwoStations100IntTo10();
  } else if (testName == "Central from two stations(100 string x 10)" ){
    centralFromTwoStations100StringTo10();
  } else if (testName == "Central from two stations(100 int x 10 x 1)" ){
    centralFromTwoStations100IntTo10x1();
  } else if (testName == "Central from two stations(100 string x 10 x 1)" ){
    centralFromTwoStations100StringTo10x1();
  } else if (testName == "Four stations to central(1000 int x 1)" ){
    fourStationsToCentral1000IntTo1();
  } else if (testName == "Four stations to central(1000 string x 1)" ){
    fourStationsToCentral1000StringTo1();
  } else if (testName == "Four stations to central(100 int x 10)" ){
    fourStationsToCentral100IntTo10();
  } else if (testName == "Four stations to central(100 string x 10)" ){
    fourStationsToCentral100StringTo10();
  } else if (testName == "Four stations to central(100 int x 10 x 1)" ){
    fourStationsToCentral100IntTo10x1();
  } else if (testName == "Four stations to central(100 string x 10 x 1)" ){
    fourStationsToCentral100StringTo10x1();
  } else if (testName == "Central from four stations(1000 int x 1)" ){
    centralFromFourStations1000IntTo1();
  } else if (testName == "Central from four stations(1000 string x 1)" ){
    centralFromFourStations1000StringTo1();
  } else if (testName == "Central from four stations(100 int x 10)" ){
    centralFromFourStations100IntTo10();
  } else if (testName == "Central from four stations(100 string x 10)" ){
    centralFromFourStations100StringTo10();    
  } else if (testName == "Central from four stations(100 int x 10 x 1)" ){
    centralFromFourStations100IntTo10x1();
  } else if (testName == "Central from four stations(100 string x 10 x 1)" ){
    centralFromFourStations100StringTo10x1();    
  } else if (testName == "Two stations from central(1000 int x 1)" ){
    twoStationsFromCentral1000IntTo1();
  } else if (testName == "Two stations from central(1000 string x 1)" ){
    twoStationsFromCentral1000StringTo1();
  } else if (testName == "Two stations from central(100 int x 10)" ){
    twoStationsFromCentral100IntTo10();
  } else if (testName == "Two stations from central(100 string x 10)" ){
    twoStationsFromCentral100StringTo10();
  } else if (testName == "Two stations from central(100 int x 10 x 1)" ){
    twoStationsFromCentral100IntTo10x1();
  } else if (testName == "Two stations from central(100 string x 10 x 1)" ){
    twoStationsFromCentral100StringTo10x1();
  }else if (testName == "Central to two stations(1000 int x 1)" ){
    centralToTwoStations1000IntTo1();
  } else if (testName == "Central to two stations(1000 string x 1)" ){
    centralToTwoStations1000StringTo1();
  } else if (testName == "Central to two stations(100 int x 10)" ){
    centralToTwoStations100IntTo10();
  } else if (testName == "Central to two stations(100 string x 10)" ){
    centralToTwoStations100StringTo10();
  } else if (testName == "Central to two stations(100 int x 10 x 1)" ){
    centralToTwoStations100IntTo10x1();
  } else if (testName == "Central to two stations(100 string x 10 x 1)" ){
    centralToTwoStations100StringTo10x1();
  } else if (testName == "Four stations from central(1000 int x 1)" ){
    fourStationsFromCentral1000IntTo1();
  } else if (testName == "Four stations from central(1000 string x 1)" ){
    fourStationsFromCentral1000StringTo1();
  } else if (testName == "Four stations from central(100 int x 10)" ){
    fourStationsFromCentral100IntTo10();
  } else if (testName == "Four stations from central(100 string x 10)" ){
    fourStationsFromCentral100StringTo10();
  } else if (testName == "Four stations from central(100 int x 10 x 1)" ){
    fourStationsFromCentral100IntTo10x1();
  } else if (testName == "Four stations from central(100 string x 10 x 1)" ){
    fourStationsFromCentral100StringTo10x1();
  } else if (testName == "Central to four stations(1000 int x 1)" ){
    centralToFourStations1000IntTo1();
  } else if (testName == "Central to four stations(1000 string x 1)" ){
    centralToFourStations1000StringTo1();
  } else if (testName == "Central to four stations(100 int x 10)" ){
    centralToFourStations100IntTo10();
  } else if (testName == "Central to four stations(100 string x 10)" ){
    centralToFourStations100StringTo10();    
  } else if (testName == "Central to four stations(100 int x 10 x 1)" ){
    centralToFourStations100IntTo10x1();
  } else if (testName == "Central to four stations(100 string x 10 x 1)" ){
    centralToFourStations100StringTo10x1();  
  } else if (testName == "Pingpong between central and one station (1000 int)") { 
    centralToOneStationPingPong1000Int();
  } else if (testName == "Pingpong between central and one station (1000 string)") { 
    centralToOneStationPingPong1000String();
  } else if (testName == "reset") {
    reset();  
  }else {
    DebugTN("Error, unsupported testname received: "+ testName );
    DebugTN("Supported testnames: One station to central(1000 int x 1)\n"+
            "                     One station to central(1000 string x 1)\n"+
            "                     One station to central(100 int x 10)\n"+
            "                     One station to central(100 string x 10)\n"+
            "                     One station to central(100 int x 10 x 1)\n"+
            "                     One station to central(100 string x 10 x 1)\n"+
            "                     Central from one station(1000 int x 1)\n"+
            "                     Central from one station(1000 string x 1)\n"+
            "                     Central from one station(100 int x 10)\n"+
            "                     Central from one station(100 string x 10)\n"+
            "                     Central from one station(100 int x 10 x 1)\n"+
            "                     Central from one station(100 string x 10 x 1)\n"+
            "                     Two stations to central(1000 int x 1)\n"+
            "                     Two stations to central(1000 string x 1)\n"+
            "                     Two stations to central(100 int x 10)\n"+
            "                     Two stations to central(100 string x 10)\n"+
            "                     Two stations to central(100 int x 10 x 1)\n"+
            "                     Two stations to central(100 string x 10 x 1)\n"+
            "                     Central from two stations(1000 int x 1)\n"+
            "                     Central from two stations(1000 string x 1)\n"+
            "                     Central from two stations(100 int x 10)\n"+
            "                     Central from two stations(100 string x 10)\n"+
            "                     Central from two stations(100 int x 10 x 1)\n"+
            "                     Central from two stations(100 string x 10 x 1)\n"+
            "                     Four stations to central(1000 int x 1)\n"+
            "                     Four stations to central(1000 string x 1)\n"+
            "                     Four stations to central(100 int x 10)\n"+
            "                     Four stations to central(100 string x 10)\n"+
            "                     Four stations to central(100 int x 10 x 1)\n"+
            "                     Four stations to central(100 string x 10 x 1)\n"+
            "                     Central from four stations(1000 int x 1)\n"+
            "                     Central from four stations(1000 string x 1)\n"+
            "                     Central from four stations(100 int x 10)\n"+
            "                     Central from four stations(100 string x 10)\n"+
            "                     Central from four stations(100 int x 10 x 1)\n"+
            "                     Central from four stations(100 string x 10 x 1)\n"+
            "                     Two stations from central(1000 int x 1)\n"+
            "                     Two stations from central(1000 string x 1)\n"+
            "                     Two stations from central(100 int x 10)\n"+
            "                     Two stations from central(100 string x 10)\n"+
            "                     Two stations from central(100 int x 10 x 1)\n"+
            "                     Two stations from central(100 string x 10 x 1)\n"+
            "                     Central to two stations(1000 int x 1)\n"+
            "                     Central to two stations(1000 string x 1)\n"+
            "                     Central to two stations(100 int x 10)\n"+
            "                     Central to two stations(100 string x 10)\n"+
            "                     Central to two stations(100 int x 10 x 1)\n"+
            "                     Central to two stations(100 string x 10 x 1)\n"+
            "                     Four stations from central(1000 int x 1)\n"+
            "                     Four stations from central(1000 string x 1)\n"+
            "                     Four stations from central(100 int x 10)\n"+
            "                     Four stations from central(100 string x 10)\n"+
            "                     Four stations from central(100 int x 10 x 1)\n"+
            "                     Four stations from central(100 string x 10 x 1)\n"+
            "                     Central to four stations(1000 int x 1)\n"+
            "                     Central to four stations(1000 string x 1)\n"+
            "                     Central to four stations(100 int x 10)\n"+
            "                     Central to four stations(100 string x 10)\n"+
            "                     Central to four stations(100 int x 10 x 1)\n"+
            "                     Central to four stations(100 string x 10 x 1)\n"+
            "                     Pingpong between central and one station (1000 int)\n"+
            "                     Pingpong between central and one station (1000 string)\n");


    return;
  }
}

//
//  ********** One Station To Central **********
//

// *******************************************
// Name : oneStationToCentral1000IntTo1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// station:        script will write 1000 times another value in 
//                 the central datapoint.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void oneStationToCentral1000IntTo1() {
  DebugTN("oneStationToCentral1000IntTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 1000 single Int values to Central");
    writeSingle("MCU001:","Int");
  } else if (stationName == "") {
    DebugTN("Will start reading 1000 single Int values from Central");
    readSingle("MCU001:","Int");
  }
}

// *******************************************
// Name : oneStationToCentral1000StringTo1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// station:        script will write 1000 times another value in 
//                 the central datapoint.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void oneStationToCentral1000StringTo1() {
  DebugTN("oneStationToCentral1000StringTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 1000 single String values to Central");
    writeSingle("MCU001:","String");
  } else if (stationName == "") {
    DebugTN("Will start reading 1000 single String values from Central");
    readSingle("MCU001:","String");
  }
}

// *******************************************
// Name : oneStationToCentral100IntTo10
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the central.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void oneStationToCentral100IntTo10() {
  DebugTN("oneStationToCentral100IntTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 Int values to Central");
    writeCollection("MCU001:","Int");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 Int values from Central");
    readCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : oneStationToCentral100StringTo10
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the central.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void oneStationToCentral100StringTo10() {
  DebugTN("oneStationToCentral100StringTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 String values to Central");
    writeCollection("MCU001:","String");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 String values from Central");
    readCollection("MCU001:","String");
  }
}

// *******************************************
// Name : oneStationToCentral100IntTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void oneStationToCentral100IntTo10x1() {
  DebugTN("oneStationToCentral100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 (single) Int  values to Central");
    writeSingleCollection("MCU001:","Int");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 (single) Int values from Central");
    readSingleCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : oneStationToCentral100StringTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void oneStationToCentral100StringTo10x1() {
  DebugTN("oneStationToCentral100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 (single) String values to Central");
    writeSingleCollection("MCU001:","String");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 (single) String values from Central");
    readSingleCollection("MCU001:","String");
  }
}

//
//  ********** Central From One Station **********
//

// *******************************************
// Name : centralFromOneStation1000IntTo1
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// station:        script will write 1000 times another value in 
//                 the station datapoint.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromOneStation1000IntTo1() {
  DebugTN("centralFromOneStation1000IntTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 1000 single Int values to Station");
    writeSingle(STATION1,"Int");
  } else if (stationName == "") {
    DebugTN("Will start reading 1000 single Int values from Station");
    readSingle(STATION1,"Int");
  }
}

// *******************************************
// Name : centralFromOneStation1000StringTo1
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// station:        script will write 1000 times another value in 
//                 the station datapoint.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromOneStation1000StringTo1() {
  DebugTN("centralFromOneStation1000StringTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 1000 single String values to Station");
    writeSingle(STATION1,"String");
  } else if (stationName == "") {
    DebugTN("Will start reading 1000 single String values from Station");
    readSingle(STATION1,"String");
  }
}

// *******************************************
// Name : centralFromOneStation100IntTo10
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromOneStation100IntTo10() {
  DebugTN("centralFromOneStation100IntTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 Int values to Station");
    writeCollection(STATION1,"Int");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 Int values from Station");
    readCollection(STATION1,"Int");
  }
}

// *******************************************
// Name : centralFromOneStation100StringTo10
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromOneStation100StringTo10() {
  DebugTN("centralFromOneStation100StringTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 String values to Station");
    writeCollection(STATION1,"String");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 String values from Station");
    readCollection(STATION1,"String");
  }
}

// *******************************************
// Name : centralFromOneStation100IntTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromOneStation100IntTo10x1() {
  DebugTN("centralFromOneStation100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 (Single) Int values to Station");
    writeSingleCollection(STATION1,"Int");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Station");
    readSingleCollection(STATION1,"Int");
  }
}

// *******************************************
// Name : centralFromOneStation100StringTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromOneStation100StringTo10x1() {
  DebugTN("centralFromOneStation100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 (Single) String values to Station");
    writeSingleCollection(STATION1,"String");
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Station");
    readSingleCollection(STATION1,"String");
  }
}

//
//  ********** Two Stations To Central **********
//

// *******************************************
// Name : twoStationsToCentral1000IntTo1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 1000 times another value in 
//                 the central datapoint.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsToCentral1000IntTo1() {
  DebugTN("twoStationsToCentral1000IntTo1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Station  Will start writing 1000 single Int values to Central");
    writeSingle("MCU001:","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 2000 single Int values from Central");
    readSingle("MCU001:","Int");
  }
}

// *******************************************
// Name : twoStationsToCentral1000StringTo1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 1000 times another value in 
//                 the central datapoint.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsToCentral1000StringTo1() {
  DebugTN("twoStationsToCentral1000StringTo1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 1000 single String values to Central");
    writeSingle("MCU001:","String");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 2000 single String values from Central");
    readSingle("MCU001:","String");
  }
}

// *******************************************
// Name : twoStationsToCentral100IntTo10
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 100 times another value in 
//                 10 datapoints on the central.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsToCentral100IntTo10() {
  DebugTN("twoStationsToCentral100IntTo10 starting ");
  isRunning=true;
  nrWrites=200;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series if Int values to Central");
    writeCollection("MCU001:","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 Int values from Central");
    readCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : twoStationsToCentral100StringTo10
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 100 times another value in 
//                 10 datapoints on the central.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsToCentral100StringTo10() {
  DebugTN("twoStationsToCentral100StringTo10 starting ");
  isRunning=true;
  nrWrites=200;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 String values to Central");
    writeCollection("MCU001:","String");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 String values from Central");
    readCollection("MCU001:","String");
  }
}

// *******************************************
// Name : twoStationsToCentral100IntTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsToCentral100IntTo10x1() {
  DebugTN("twoStationsToCentral100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of (Single) Int values to Central");
    writeSingleCollection("MCU001:","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Central");
    readSingleCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : twoStationsToCentral100StringTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsToCentral100StringTo10x1() {
  DebugTN("twoStationsToCentral100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 (Single) String values to Central");
    writeSingleCollection("MCU001:","String");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Central");
    readSingleCollection("MCU001:","String");
  }
}

//
//  ********** Central From Two Stations **********
//

// *******************************************
// Name : centralFromTwoStations1000IntTo1
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// stations:       script will write 1000 times another value in 
//                 the station datapoint.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromTwoStations1000IntTo1() {
  DebugTN("centralFromTwoStations1000IntTo1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 1000 single Int values to Station");
    writeSingle("","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 2000 single Int values from Station");
    readSingleFromTwo(STATION1,STATION2,"Int");    
  }
}

// *******************************************
// Name : centralFromTwoStations1000StringTo1
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// stations:       script will write 2000 times another value in 
//                 the station datapoint.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromTwoStations1000StringTo1() {
  DebugTN("centralFromTwoStations1000StringTo1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 1000 single String values to Station");
    writeSingle("","String");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 2000 single  values from Station");
    readSingleFromTwo(STATION1,STATION2,"String");    
  }
}

// *******************************************
// Name : centralFromTwoStations100IntTo10
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromTwoStations100IntTo10() {
  DebugTN("centralFromTwoStations100IntTo10 starting ");
  isRunning=true;
  nrWrites=200;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 Int values to Station");
    writeCollection("","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 Int values from Stations");
    readCollection(STATION1,"Int");
    readCollection(STATION2,"Int");
    
  }
}

// *******************************************
// Name : centralFromTwoStations100StringTo10
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromTwoStations100StringTo10() {
  DebugTN("centralFromTwoStations100StringTo10 starting ");
  isRunning=true;
  nrWrites=200;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 String values to Station");
    writeCollection("","String");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 String values from Station");
    readCollectionFromTwo(STATION1,STATION2,"String");
  }
}

// *******************************************
// Name : centralFromTwoStations100IntTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromTwoStations100IntTo10x1() {
  DebugTN("centralFromTwoStations100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 (Single) Int values to Station");
    writeSingleCollection("","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Stations");
    readSingleCollection(STATION1,"Int");
    readSingleCollection(STATION2,"Int");
    
  }
}

// *******************************************
// Name : centralFromTwoStations100StringTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromTwoStations100StringTo10x1() {
  DebugTN("centralFromTwoStations100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 (Single) String values to Station");
    writeSingleCollection("","String");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Station");
    readSingleCollectionFromTwo(STATION1,STATION2,"String");
  }
}

//
//  ********** Four Stations To Central **********
//

// *******************************************
// Name : fourStationsToCentral1000IntTo1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 1000 times another value in 
//                 the central datapoint.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsToCentral1000IntTo1() {
  DebugTN("fourStationsToCentral1000IntTo1 starting ");
  isRunning=true;
  nrWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Station Will start writing 1000 single Int values to Central");
    writeSingle("MCU001:","Int");
  } else {
    DebugTN("Will start reading 4000 single Int values from Central");
    readSingle("MCU001:","Int");
  }
}

// *******************************************
// Name : fourStationsToCentral1000StringTo1
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 1000 times another value in 
//                 the central datapoint.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsToCentral1000StringTo1() {
  DebugTN("fourStationsToCentral1000StringTo1 starting ");
  isRunning=true;
  nrWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 1000 single String values to Central");
    writeSingle("MCU001:","String");
  } else {
    DebugTN("Will start reading 4000 single String values from Central");
    readSingle("MCU001:","String");
  }
}

// *******************************************
// Name : fourStationsToCentral100IntTo10
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 100 times another value in 
//                 10 datapoints on the central.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsToCentral100IntTo10() {
  DebugTN("fourStationsToCentral100IntTo10 starting ");
  isRunning=true;
  nrWrites=400;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series if Int values to Central");
    writeCollection("MCU001:","Int");
  } else {
    DebugTN("Will start reading 400 series of 10 Int values from Central");
    readCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : fourStationsToCentral100StringTo10
// *******************************************
// Description:
// central system: script connects to a DP in the central database
//                 and will count the updates.
// stations:       script will write 100 times another value in 
//                 10 datapoints on the central.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsToCentral100StringTo10() {
  DebugTN("fourStationsToCentral100StringTo10 starting ");
  isRunning=true;
  nrWrites=400;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of 10 String values to Central");
    writeCollection("MCU001:","String");
  } else {
    DebugTN("Will start reading 400 series of 10 String values from Central");
    readCollection("MCU001:","String");
  }
}


//
//  ********** Central From Four Stations **********
//

// *******************************************
// Name : centralFromFourStations1000IntTo1
// *******************************************
// Description:
// central system: script connects to a DP in the station database
//                 and will count the updates.
// stations:       script will write 1000 times another value in 
//                 the station datapoint.
//
// Results:        1 Time on the station to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromFourStations1000IntTo1() {
  DebugTN("centralFromFourStations1000IntTo1 starting ");
  isRunning=true;
  nrWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 1000 single Int values to Station");
    writeSingle("","Int");
  } else {
    DebugTN("Will start reading 4000 single Int values from Station");
    readSingleFromFour(STATION1,STATION2,STATION3,STATION4,"Int");    
  }
}

// *******************************************
// Name : centralFromFourStations1000StringTo1
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// stations:       script will write 2000 times another value in 
//                 the station datapoint.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromFourStations1000StringTo1() {
  DebugTN("centralFromFourStations1000StringTo1 starting ");
  isRunning=true;
  nrWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 1000 single String values to Station");
    writeSingle("","String");
  } else {
    DebugTN("Will start reading 4000 single  values from Station");
    readSingleFromFour(STATION1,STATION2,STATION3,STATION4,"String");    
  }
}

// *******************************************
// Name : centralFromFourStations100IntTo10
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromFourStations100IntTo10() {
  DebugTN("centralFromFourStation100IntTo10 starting ");
  isRunning=true;
  nrWrites=400;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of 10 Int values to Station");
    writeCollection("","Int");
  } else {
    DebugTN("Will start reading 100 series of 10 Int values from Stations");
    readCollection(STATION1,"Int");
    readCollection(STATION2,"Int");
    readCollection(STATION3,"Int");
    readCollection(STATION4,"Int");
    
  }
}

// *******************************************
// Name : centralFromFourStations100StringTo10
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromFourStations100StringTo10() {
  DebugTN("centralFromFourStations100StringTo10 starting ");
  isRunning=true;
  nrWrites=400;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of 10 String values to Station");
    writeCollection("","String");
  } else {
    DebugTN("Will start reading 100 series of 10 String values from Station");
    readCollectionFromFour(STATION1,STATION2,STATION3,STATION4,"String");
  }
}
    
// *******************************************
// Name : centralFromFourStations100IntTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromFourStations100IntTo10x1() {
  DebugTN("centralFromFourStation100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of 10 (Single) Int values to Station");
    writeSingleCollection("","Int");
  } else {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Stations");
    readSingleCollection(STATION1,"Int");
    readSingleCollection(STATION2,"Int");
    readSingleCollection(STATION3,"Int");
    readSingleCollection(STATION4,"Int");
    
  }
}

// *******************************************
// Name : centralFromFourStations100StringTo10x1
// *******************************************
// Description:
// central system: script connects to a DP in the stations database
//                 and will count the updates.
// station:        script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
//
// Results:        1 Time on the stations to write the values.
//                 2 Time on the central system to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralFromFourStations100StringTo10x1() {
  DebugTN("centralFromFourStations100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of 10 (Single) String values to Station");
    writeSingleCollection("","String");
  } else {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Station");
    readSingleCollection(STATION1,"String");
    readSingleCollection(STATION2,"String");
    readSingleCollection(STATION3,"String");
    readSingleCollection(STATION4,"String");
  }
}

//
//  ********** Two Stations From Central **********
//

// *******************************************
// Name : twoStationsFromCentral1000IntTo1
// *******************************************
// Description:
// central system: script will write 1000 times another value in 
//                 the central datapoint.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the central to write the values.
//                 2 Time on the stations to read the values.
//                 3 Did the station database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsFromCentral1000IntTo1() {
  DebugTN("twoStationsFromCentral1000IntTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Station 1 Will start reading 1000 single Int values from the Central");
    readSingle("MCU001:","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 1000 single Int values from Central");
    writeSingle("MCU001:","Int");
  }
}

// *******************************************
// Name : twoStationsFromCentral1000StringTo1
// *******************************************
// Description:
// central system: script will write 1000 times another value in 
//                 the central datapoint.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the stations to read the values.
//                 2 Time on the central system to write the values.
//                 3 Did the stations receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsFromCentral1000StringTo1() {
  DebugTN("twoStationsFromCentral1000StringTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 1000 single String values from Central");
    readSingle("MCU001:","String");
  } else if (stationName == "" ) {
    DebugTN("Will start reading 1000 single String values to Central");
    writeSingle("MCU001:","String");
  }
}

// *******************************************
// Name : twoStationsFromCentral100IntTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the stations to read the values.
//                 2 Time on the central system to write the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsFromCentral100IntTo10() {
  DebugTN("twoStationsFromCentral100IntTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 Int values from Central");
    readCollection("MCU001:","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series if Int values to Central");
    writeCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : twoStationsFromCentral100StringTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the stations to read the values.
//                 2 Time on the central system to write the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsFromCentral100StringTo10() {
  DebugTN("twoStationsFromCentral100StringTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 String values from Central");
    readCollection("MCU001:","String");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 String values to Central");
    writeCollection("MCU001:","String");
  }
}

// *******************************************
// Name : twoStationsFromCentral100IntTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the stations to read the values.
//                 2 Time on the central system to write the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsFromCentral100IntTo10x1() {
  DebugTN("twoStationsFromCentral100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Central");
    readSingleCollection("MCU001:","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 (Single) Int values to Central");
    writeSingleCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : twoStationsFromCentral100StringTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the stations to read the values.
//                 2 Time on the central system to write the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void twoStationsFromCentral100StringTo10x1() {
  DebugTN("twoStationsFromCentral100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Central");
    readSingleCollection("MCU001:","String");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 (Single) String values to Central");
    writeSingleCollection("MCU001:","String");
  }
}

//
//  ********** Central To Two Stations **********
//

// *******************************************
// Name : centralToTwoStations1000IntTo1
// *******************************************
// Description:
// central system: script will write 1000 times another value in 
//                 two stations datapoint.
// stations:       script connects to a DP in the station database
//                 and will count the updates.
//
// Results:        1 Time on the Central system to write the values.
//                 2 Time on the stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToTwoStations1000IntTo1() {
  DebugTN("centralToTwoStations1000IntTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 1000 single Int values from Station");
    readSingle("","Int");    
  } else if (stationName == "" ) {
    DebugTN("Will start writing 1000 single Int values to two Station");
    writeSingleToTwo(STATION1,STATION2,"Int");
  }
}

// *******************************************
// Name : centralToTwoStations1000StringTo1
// *******************************************
// Description:
// central system:script will write 1000 times another value in 
//                 two stations datapoint.
// stations:       script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the two stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToTwoStations1000StringTo1() {
  DebugTN("centralToTwoStations1000StringTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 2000 single  values from Station");
    readSingle("","String");    
  } else if (stationName == "" ) {
    DebugTN("Will start writing 1000 single String values to Station");
    writeSingleToTwo(STATION1,STATION2,"String");
  }
}

// *******************************************
// Name : centralToTwoStations100IntTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on two stations.
// station:        script connects to a DP in two stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on two stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToTwoStations100IntTo10() {
  DebugTN("centralToTwoStations100IntTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 Int values from Station");
    readCollection("","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 Int values to Station");
    writeCollectionToTwo(STATION1,STATION2,"Int");    
  }
}

// *******************************************
// Name : centralToTwoStations100StringTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on two stations.
// station:        script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the two stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToTwoStations100StringTo10() {
  DebugTN("centralToTwoStations100StringTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 String values from Station");
    readCollection("","String");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 String values to two Station");
    writeCollectionToTwo(STATION1,STATION2,"String");
  }
}

// *******************************************
// Name : centralToTwoStations100IntTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on two stations in 10 different writes.
// station:        script connects to a DP in two stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on two stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToTwoStations100IntTo10x1() {
  DebugTN("centralToTwoStations100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Station");
    readSingleCollection("","Int");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 (Single) Int values to Station");
    writeSingleCollectionToTwo(STATION1,STATION2,"Int");    
  }
}

// *******************************************
// Name : centralToTwoStations100StringTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on two stations in 10 different writes.
// station:        script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the two stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToTwoStations100StringTo10x1() {
  DebugTN("centralToTwoStations100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Station");
    readSingleCollection("","String");
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 (Single) String values to two Station");
    writeSingleCollectionToTwo(STATION1,STATION2,"String");
  }
}
//
//  ********** Four Stations From Central **********
//

// *******************************************
// Name : fourStationsFromCentral1000IntTo1
// *******************************************
// Description:
// central system: script will write 1000 times another value in 
//                 the central datapoint.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsFromCentral1000IntTo1() {
  DebugTN("fourStationsFromCentral1000IntTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 1000 single Int values from Central");
    readSingle("MCU001:","Int");
  } else {
    DebugTN("Script Will start writing 1000 single Int values to Central");
    writeSingle("MCU001:","Int");
  }
}

// *******************************************
// Name : fourStationsFromCentral1000StringTo1
// *******************************************
// Description:
// central system: script will write 1000 times another value in 
//                 the central datapoint.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsFromCentral1000StringTo1() {
  DebugTN("fourStationsFromCentral1000StringTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 1000 single String values from Central");
    readSingle("MCU001:","String");
  } else {
    DebugTN("Will start writing 1000 single String values to Central");
    writeSingle("MCU001:","String");
  }
}

// *******************************************
// Name : fourStationsFromCentral100IntTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsFromCentral100IntTo10() {
  DebugTN("fourStationsFromCentral100IntTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 Int values from Central");
    readCollection("MCU001:","Int");
  } else {
    DebugTN("Will start writing 100 series if Int values to Central");
    writeCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : fourStationsFromCentral100StringTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the central script to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsFromCentral100StringTo10() {
  DebugTN("fourStationsFromCentral100StringTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 String values from Central");
    readCollection("MCU001:","String");
  } else {
    DebugTN("Will start writing 100 series of 10 String values to Central");
    writeCollection("MCU001:","String");
  }
}

// *******************************************
// Name : fourStationsFromCentral100IntTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsFromCentral100IntTo10x1() {
  DebugTN("fourStationsFromCentral100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Central");
    readSingleCollection("MCU001:","Int");
  } else {
    DebugTN("Will start writing 100 series of 10 (Single) Int values to Central");
    writeSingleCollection("MCU001:","Int");
  }
}

// *******************************************
// Name : fourStationsFromCentral100StringTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the central in 10 different writes.
// stations:       script connects to a DP in the central database
//                 and will count the updates.
//
// Results:        1 Time on the central script to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void fourStationsFromCentral100StringTo10x1() {
  DebugTN("fourStationsFromCentral100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Central");
    readSingleCollection("MCU001:","String");
  } else {
    DebugTN("Will start writing 100 series of 10 (Single) String values to Central");
    writeSingleCollection("MCU001:","String");
  }
}


//
//  ********** Central To Four Stations **********
//

// *******************************************
// Name : centralToFourStations1000IntTo1
// *******************************************
// Description:
// central system: script will write 1000 times another value in 
//                 four stationa datapoint.
// stations:       script connects to a DP in the station database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToFourStations1000IntTo1() {
  DebugTN("centralToFourStations1000IntTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 1000 single Int values from Station");
    readSingle("","Int");    
  } else {
    DebugTN("Will start writing 1000 single Int values to four Stations");
    writeSingleToFour(STATION1,STATION2,STATION3,STATION4,"Int");
  }
}

// *******************************************
// Name : centralToFourStations1000StringTo1
// *******************************************
// Description:
// central system: script will write 1000 times another value in 
//                 the station datapoint.
// stations:       script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central sysyem to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToFourStations1000StringTo1() {
  DebugTN("centralToFourStations1000StringTo1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 1000 single  values from Station");
    readSingle("","String");    
  } else {
    DebugTN("Will start writing 1000 single String values to Station");
    writeSingleToFour(STATION1,STATION2,STATION3,STATION4,"String");
  }
}

// *******************************************
// Name : centralToFourStations100IntTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the station.
// station:        script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToFourStations100IntTo10() {
  DebugTN("centralToFourStation100IntTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 Int values from Stations");
    readCollection("","Int");
  } else {
    DebugTN("Will start writing 100 series of 10 Int values to Station");
    writeCollectionToFour(STATION1,STATION2,STATION3,STATION4,"Int");
    
  }
}

// *******************************************
// Name : centralToFourStations100StringTo10
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the station.
// station:        script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToStations100StringTo10() {
  DebugTN("centralFromFourStations100StringTo10 starting ");
  isRunning=true;
  nrWrites=100;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 String values from Station");
    readCollection("","String");
  } else {
    DebugTN("Will start writing 100 series of 10 String values to Station");
    writeCollectionToFour(STATION1,STATION2,STATION3,STATION4,"String");
  }
}

// *******************************************
// Name : centralToFourStations100IntTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
// station:        script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on the four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToFourStations100IntTo10x1() {
  DebugTN("centralToFourStation100IntTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 (Single) Int values from Stations");
    readSingleCollection("","Int");
  } else {
    DebugTN("Will start writing 100 series of 10 (Single) Int values to Station");
    writeSingleCollectionToFour(STATION1,STATION2,STATION3,STATION4,"Int");
    
  }
}

// *******************************************
// Name : centralToFourStations100StringTo10x1
// *******************************************
// Description:
// central system: script will write 100 times another value in 
//                 10 datapoints on the station in 10 different writes.
// station:        script connects to a DP in the stations database
//                 and will count the updates.
//
// Results:        1 Time on the central system to write the values.
//                 2 Time on four stations to read the values.
//                 3 Did the central database receive all changes.
//
// Returns:
//    None
// *******************************************
void centralToStations100StringTo10x1() {
  DebugTN("centralFromFourStations100StringTo10x1 starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 (Single) String values from Station");
    readSingleCollection("","String");
  } else {
    DebugTN("Will start writing 100 series of 10 (Single) String values to Station");
    writeSingleCollectionToFour(STATION1,STATION2,STATION3,STATION4,"String");
  }
}


// *******************************************
// Name : centralToOneStationPingPong1000Int
// *******************************************
// Description:
// central system: will write an int into a point at the stations database.
//                 and will have a dpconnect in his own db on the same point.
//                 as soon as the dpconnect is triggered , he will read the 
//                 value and send it back to the station again, after 1000 bounces
//                 the test will stop.
// station:        will have a dpconnect on a point in his own db. As soon as
//                 he is triggered, he will send the received value back to
//                 the central system
//
// Results:        1 Time between first and last trigger
//
// Returns:
//    None
// *******************************************
void centralToOneStationPingPong1000Int() {
  DebugTN("centralToOneStationPingPong1000Int starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start station pingpong test with 1000 bounces and an int");
    emptyPoints();    
    if (dpExists(STATION1+"lofarSpeedTest.singleInt") ){
      if (!isPingpongStationIntTrigger) {
        dpConnect("pingpongStationIntTrigger",false,STATION1+"lofarSpeedTest.singleInt");
        isPingpongStationIntTrigger=true;
      }
    } else {
      DebugTN("Error: couldn't connect to "+STATION1+"lofarSpeedTest.singleInt. Test halted");
    }
  } else {
    DebugTN("Will start maincu pingpong test with 1000 bounces and an int");
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);

    // 1st connect to own point
    if (dpExists("MCU001:lofarSpeedTest.singleInt") ){
      if (!isPingpongCentralIntTrigger) {
        dpConnect("pingpongCentralIntTrigger",false,"MCU001:lofarSpeedTest.singleInt");
        isPingpongCentralIntTrigger = true;
      }
      // now check stations point and write first value to start the game
      if (dpExists(STATION1+"lofarSpeedTest.singleInt") ){
        dpSet(STATION1+"lofarSpeedTest.singleInt",10);
      } else {
        DebugTN("Error: couldn't connect to "+STATION1+"lofarSpeedTest.singleInt");          
      }
    } else {
      DebugTN("Error: couldn't connect to MCU001:lofarSpeedTest.singleInt. Test halted");
    }
  }
}

// *******************************************
// Name : centralToOneStationPingPong1000String
// *******************************************
// Description:
// central system: will write an int into a point at the staations database.
//                 and will have a dpconnect in his own db on the same point.
//                 as soon as the dpconnect is triggered , he will read the 
//                 value and send it back to the station again, after 1000 bounces
//                 the test will stop.
// station:        will have a dpconnect on a point in his own db. As soon as
//                 he is triggered, he will send the received value back to
//                 the central system
//
// Results:        1 Time between first and last trigger
//
// Returns:
//    None
// *******************************************
void centralToOneStationPingPong1000String() {
  DebugTN("centralToOneStationPingPong1000String starting ");
  isRunning=true;
  nrWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start station pingpong test with 1000 bounces and a string");
    emptyPoints();
    if (dpExists(STATION1+"lofarSpeedTest.singleString") ){
      if (!isPingpongStationStringTrigger) {
        dpConnect("pingpongStationStringTrigger",false,STATION1+"lofarSpeedTest.singleString");
        isPingpongStationStringTrigger= true;
      }
    } else {
      DebugTN("Error: couldn't connect to "+STATION1+"lofarSpeedTest.singleString. Test halted");
    }
  } else {
    DebugTN("Will start maincu pingpong test with 1000 bounces and an int");
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);

    // 1st connect to own point
    if (dpExists("MCU001:lofarSpeedTest.singleString") ){
      if (isPingpongCentralStringTrigger) {
        dpConnect("pingpongCentralStringTrigger",false,"MCU001:lofarSpeedTest.singleString");
        isPingpongCentralStringTrigger=true;
      }
      // now check stations point and write first value to start the game
      if (dpExists(STATION1+"lofarSpeedTest.singleString") ){
        dpSet(STATION1+"lofarSpeedTest.singleString","testing");
      } else {
        DebugTN("Error: couldn't connect to "+STATION1+"lofarSpeedTest.singleString");          
      }
    } else {
      DebugTN("Error: couldn't connect to MCU001:lofarSpeedTest.singleString. Test halted");
    }
  }
}
//
// *********** Callbacks **********
//

//
//  ********** One Station To Central **********
//

// *******************************************
// Name : centralSingleTrigger
// *******************************************
// Description:
//    Callback on MainCU single point
//    Counts the nr of ints read and compares with nr send
//    if equal then test is finished
//    we need a mechanism to determine if the test failed. Best
//    would be to check if time is way out for the expected total time
//
// Returns:
//    None
// *******************************************
void centralSingleTrigger(string dp1, anytype aVal) {
  // Start count when 1st callback received, to avoid setup/startup times
  if (nrReads == 0) {
    startTime = getCurrentTime();
  }
  nrReads++;
  if (nrReads == nrWrites) {
    endTime = getCurrentTime();
//    DebugTN("starttijd: " + startTime);
//    DebugTN("eindtijd: " + endTime);
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrReads",nrReads,
          "lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    dpDisconnect("centralSingleTrigger",dp1);
    DebugTN("*****End read cycle reached*****");
  }
}

// *******************************************
// Name : centralCollectionTrigger
// *******************************************
// Description:
//    Callback on MainCU  intCollection of points
//    Counts the nr of ints read and compares with nr send
//    if equal then test is finished
//    we need a mechanism to determine if the test failed. Best
//    would be to check if time is way out for the expected total time
//
// Returns:
//    None
// *******************************************
void centralCollectionTrigger(string dp1, anytype val1,
                                 string dp2, anytype val2,
                                 string dp3, anytype val3,
                                 string dp4, anytype val4,
                                 string dp5, anytype val5,
                                 string dp6, anytype val6,
                                 string dp7, anytype val7,
                                 string dp8, anytype val8,
                                 string dp9, anytype val9,
                                 string dp10, anytype val10) {
  // Start count when 1st callback received, to avoid setup/startup times
  if (nrReads == 0) {
    startTime = getCurrentTime();
  }
  nrReads++;
  if (nrReads == nrWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrReads",nrReads,
          "lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    dpDisconnect("centralCollectionTrigger",dp1);
    dpDisconnect("centralCollectionTrigger",dp2);
    dpDisconnect("centralCollectionTrigger",dp3);
    dpDisconnect("centralCollectionTrigger",dp4);
    dpDisconnect("centralCollectionTrigger",dp5);
    dpDisconnect("centralCollectionTrigger",dp6);
    dpDisconnect("centralCollectionTrigger",dp7);
    dpDisconnect("centralCollectionTrigger",dp8);
    dpDisconnect("centralCollectionTrigger",dp9);
    dpDisconnect("centralCollectionTrigger",dp10);
    DebugTN("*****End read cycle reached*****");
  }
}



//
//  ********** Central From One Station **********
//

// *******************************************
// Name : stationSingleIntTrigger
// *******************************************
// Description:
//    Callback on Station single point
//    Counts the nr of ints read and compares with nr send
//    if equal then test is finished
//    we need a mechanism to determine if the test failed. Best
//    would be to check if time is way out for the expected total time
//
// Returns:
//    None
// *******************************************
void stationSingleTrigger(string dp1, anytype aVal) {
  // Start count when 1st callback received, to avoid setup/startup times
  if (nrReads == 0) {
    startTime = getCurrentTime();
  }
  nrReads++;
  if (nrReads == nrWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    dpDisconnect("stationSingleTrigger",dp1);
    DebugTN("*****End read cycle reached*****");
  }
}


// *******************************************
// Name : stationIntCollectionTrigger
// *******************************************
// Description:
//    Callback on Station  Collection of points
//    Counts the nr of ints read and compares with nr send
//    if equal then test is finished
//    we need a mechanism to determine if the test failed. Best
//    would be to check if time is way out for the expected total time
//
// Returns:
//    None
// *******************************************
void stationCollectionTrigger(string dp1, anytype val1,
                                 string dp2, anytype val2,
                                 string dp3, anytype val3,
                                 string dp4, anytype val4,
                                 string dp5, anytype val5,
                                 string dp6, anytype val6,
                                 string dp7, anytype val7,
                                 string dp8, anytype val8,
                                 string dp9, anytype val9,
                                 string dp10, anytype val10) {
  // Start count when 1st callback received, to avoid setup/startup times
  if (nrReads == 0) {
    startTime = getCurrentTime();
  }
  nrReads++;
  if (nrReads == nrWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    dpDisconnect("stationCollectionTrigger",dp1);
    dpDisconnect("stationCollectionTrigger",dp2);
    dpDisconnect("stationCollectionTrigger",dp3);
    dpDisconnect("stationCollectionTrigger",dp4);
    dpDisconnect("stationCollectionTrigger",dp5);
    dpDisconnect("stationCollectionTrigger",dp6);
    dpDisconnect("stationCollectionTrigger",dp7);
    dpDisconnect("stationCollectionTrigger",dp8);
    dpDisconnect("stationCollectionTrigger",dp9);
    dpDisconnect("stationCollectionTrigger",dp10);
    
    DebugTN("*****End read cycle reached*****");
  }
}

//
//  ********** Pingpong between central and one station**********
//

// *******************************************
// Name : pingpongStationIntTrigger
// *******************************************
// Description:
//    Callback on Station single point
//    echos the received point back to the central
//
// Returns:
//    None
// *******************************************
void pingpongStationIntTrigger(string dp1, int aVal) {
  nrReads++;
  if (nrReads >= nrWrites) {
    isRunning=false;
    DebugTN("*****End pingpong cycle reached*****");
  }
  dpSet("MCU001:lofarSpeedTest.singleInt",aVal);
}

// *******************************************
// Name : pingpongCentralIntTrigger
// *******************************************
// Description:
//    Callback on Central single point
//    echos the received point back to the Station
//    will Start the timer on first bounce and ends 
//    after 1000 have been reached.
//
// Returns:
//    None
// *******************************************
void pingpongCentralIntTrigger(string dp1, int aVal) {
  // Start count when 1st callback received, to avoid setup/startup times
  if (nrReads == 0) {
    startTime = getCurrentTime();
  }
  nrReads++;
  if (nrReads >= nrWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    DebugTN("*****End pingpong cycle reached*****");
  } else {
    dpSet(STATION1+"lofarSpeedTest.singleInt",aVal);
  }
}

// *******************************************
// Name : pingpongStationStringTrigger
// *******************************************
// Description:
//    Callback on Station single point
//    echos the received point back to the central
//
// Returns:
//    None
// *******************************************
void pingpongStationStringTrigger(string dp1, string aVal) {
  nrReads++;
  if (nrReads == nrWrites) {
    isRunning=false;
    DebugTN("*****End pingpong cycle reached*****");
  }
  dpSet("MCU001:lofarSpeedTest.singleString",aVal);
}

// *******************************************
// Name : pingpongCentralStringTrigger
// *******************************************
// Description:
//    Callback on Central single point
//    echos the received point back to the Station
//    will Start the timer on first bounce and ends 
//    after 1000 have been reached.
//
// Returns:
//    None
// *******************************************
void pingpongCentralStringTrigger(string dp1, string aVal) {
  // Start count when 1st callback received, to avoid setup/startup times
  if (nrReads == 0) {
    startTime = getCurrentTime();
  }
  nrReads++;
  if (nrReads == nrWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    DebugTN("*****End pingpong cycle reached*****");
  } else {
    dpSet(STATION1+"lofarSpeedTest.singleString",aVal);
  }
}

//
// ******* Helper functions *******
//


void writeSingle(string machineName,string val) {
  if (dpExists(machineName+"lofarSpeedTest.single"+val) ){
    emptyPoints();
    delay(0,500);
    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName+"lofarSpeedTest.single"+val,i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName+"lofarSpeedTest.single"+val,strArray[i]);
      }
    }
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",1000,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName+"lofarSpeedTest.single"+val+" does not exist, no write possible");
    isRunning=false;
    return;
  }
}

void writeSingleToTwo(string machineName1,string machineName2,string val) {
  if (dpExists(machineName1+"lofarSpeedTest.single"+val) && dpExists(machineName2+"lofarSpeedTest.single"+val) ){
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);
    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName1+"lofarSpeedTest.single"+val,i);
        dpSet(machineName2+"lofarSpeedTest.single"+val,i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName1+"lofarSpeedTest.single"+val,strArray[i]);
        dpSet(machineName2+"lofarSpeedTest.single"+val,strArray[i]);
      }
    }
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",2000,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest.single"+val+" does not exist or "+
            "dp "+machineName2+"lofarSpeedTest.single"+val+" does not exist, no write possible");
    isRunning=false;
    return;
  }
}

void readSingle(string machineName,string val) {
  if (dpExists(machineName+"lofarSpeedTest.single"+val) ){
    emptyPoints();
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest.single"+val);
  } else {
    DebugTN("Error: dp "+machineName+"lofarSpeedTest.single"+val+" does not exist, no connect possible"); 
    isRunning = false;
    return;
  }
}

void readSingleFromFour(string machineName1,string machineName2,string machineName3,string machineName4,string val) {
  if (dpExists(machineName1+"lofarSpeedTest.single"+val) && 
      dpExists(machineName2+"lofarSpeedTest.single"+val) &&
      dpExists(machineName1+"lofarSpeedTest.single"+val) &&
      dpExists(machineName1+"lofarSpeedTest.single"+val) ){
    emptyPoints();
    dpConnect("stationSingleIntTrigger",false,machineName1+"lofarSpeedTest.single"+val,
                                              machineName2+"lofarSpeedTest.single"+val,
                                              machineName3+"lofarSpeedTest.single"+val,
                                              machineName4+"lofarSpeedTest.single"+val);
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest.single"+val+" does not exist or,"+
                  " dp "+machineName2+"lofarSpeedTest.single"+val+" does not exist or,"+
                  " dp "+machineName3+"lofarSpeedTest.single"+val+" does not exist or,"+
                  " dp "+machineName4+"lofarSpeedTest.single"+val+" does not exist no connect possible"); 
    isRunning = false;
    return;
  }
}

void readSingleFromTwo(string machineName1,string machineName2,string val) {
  if (dpExists(machineName1+"lofarSpeedTest.single"+val) && dpExists(machineName2+"lofarSpeedTest.single"+val) ){
    emptyPoints();
    dpConnect("stationSingleIntTrigger",false,machineName1+"lofarSpeedTest.single"+val,machineName2+"lofarSpeedTest.single"+val);
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest.single"+val+" does not exist or,"+
                  " dp "+machineName2+"lofarSpeedTest.single"+val+" does not exist no connect possible"); 
    isRunning = false;
    return;
  }
}

void writeCollection(string machineName,string val) {
  if (dpExists(machineName+"lofarSpeedTest."+val+"Collection.val1") ){
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);
    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val1",i,
              machineName+"lofarSpeedTest."+val+"Collection.val2",i,
              machineName+"lofarSpeedTest."+val+"Collection.val3",i,
              machineName+"lofarSpeedTest."+val+"Collection.val4",i,
              machineName+"lofarSpeedTest."+val+"Collection.val5",i,
              machineName+"lofarSpeedTest."+val+"Collection.val6",i,
              machineName+"lofarSpeedTest."+val+"Collection.val7",i,
              machineName+"lofarSpeedTest."+val+"Collection.val8",i,
              machineName+"lofarSpeedTest."+val+"Collection.val9",i,
              machineName+"lofarSpeedTest."+val+"Collection.val10",i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val1",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val2",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val3",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val4",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val5",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val6",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val7",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val8",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val9",strArray[i],
              machineName+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
      }
    }      
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",100,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName+"lofarSpeedTest."+val+"Collection.val1 does not exist, no write possible");
    isRunning=false;
    return;
  }
}

void writeSingleCollection(string machineName,string val) {
  if (dpExists(machineName+"lofarSpeedTest."+val+"Collection") ){
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);
    int loop=nrWrites/10;
    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= loop;i++) {
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val1",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val2",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val3",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val4",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val5",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val6",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val7",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val8",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val9",i);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val10",i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= loop;i++) {
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val1",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val2",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val3",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val4",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val5",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val6",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val7",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val8",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val9",strArray[i]);
        dpSet(machineName+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
      }
    }      
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",100,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName+"lofarSpeedTest."+val+"Collection.val1 does not exist, no write possible");
    isRunning=false;
    return;
  }
}

void writeCollectionToTwo(string machineName1,string machineName2,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection.val1") ){
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);
    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val2",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val3",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val4",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val5",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val6",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val7",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val8",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val9",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val10",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val2",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val3",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val4",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val5",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val6",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val7",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val8",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val9",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val10",i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val2",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val3",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val4",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val5",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val6",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val7",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val8",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val9",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val2",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val3",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val4",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val5",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val6",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val7",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val8",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val9",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
      }
    }      
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",100,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 does not exist, no write possible");
    isRunning=false;
    return;
  }
}

void writeCollectionToFour(string machineName1,string machineName2,string machineName3,string machineName4,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName3+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName4+"lofarSpeedTest."+val+"Collection.val1")){
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);
    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val2",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val3",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val4",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val5",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val6",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val7",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val8",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val9",i,
              machineName1+"lofarSpeedTest."+val+"Collection.val10",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val2",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val3",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val4",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val5",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val6",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val7",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val8",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val9",i,
              machineName2+"lofarSpeedTest."+val+"Collection.val10",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val1",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val2",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val3",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val4",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val5",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val6",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val7",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val8",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val9",i,
              machineName3+"lofarSpeedTest."+val+"Collection.val10",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val1",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val2",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val3",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val4",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val5",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val6",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val7",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val8",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val9",i,
              machineName4+"lofarSpeedTest."+val+"Collection.val10",i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= nrWrites;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val2",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val3",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val4",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val5",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val6",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val7",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val8",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val9",strArray[i],
              machineName1+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val2",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val3",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val4",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val5",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val6",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val7",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val8",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val9",strArray[i],
              machineName2+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val1",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val2",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val3",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val4",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val5",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val6",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val7",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val8",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val9",strArray[i],
              machineName3+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val1",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val2",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val3",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val4",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val5",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val6",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val7",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val8",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val9",strArray[i],
              machineName4+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
      }
    }      
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",100,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName3+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName4+"lofarSpeedTest."+val+"Collection.val1 does not exist, no write possible");
    isRunning=false;
    return;
  }
}
    
void writeSingleCollectionToTwo(string machineName1,string machineName2,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection.val1") ){
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);
    int loop=nrWrites/10;
    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= loop;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val2",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val3",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val4",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val5",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val6",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val7",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val8",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val9",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val10",i);
        
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val2",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val3",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val4",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val5",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val6",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val7",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val8",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val9",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val10",i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= loop;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val2",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val3",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val4",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val5",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val6",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val7",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val8",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val9",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);

        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val2",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val3",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val4",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val5",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val6",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val7",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val8",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val9",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);

      }
    }      
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",100,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 does not exist, no write possible");
    isRunning=false;
    return;
  }
}

void writeSingleCollectionToFour(string machineName1,string machineName2,string machineName3,string machineName4,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName3+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName4+"lofarSpeedTest."+val+"Collection.val1")){
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);
    int loop=nrWrites/10;

    if (val == "Int") {
      startTime = getCurrentTime();
      for (int i=1;i <= loop;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val2",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val3",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val4",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val5",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val6",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val7",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val8",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val9",i);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val10",i);
        
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val2",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val3",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val4",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val5",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val6",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val7",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val8",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val9",i);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val10",i);

        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val1",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val2",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val3",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val4",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val5",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val6",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val7",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val8",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val9",i);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val10",i);
        
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val1",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val2",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val3",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val4",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val5",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val6",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val7",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val8",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val9",i);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val10",i);
      }
    } else if (val == "String") {
      startTime = getCurrentTime();
      for (int i=1;i <= loop;i++) {
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val1",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val2",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val3",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val4",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val5",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val6",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val7",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val8",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val9",strArray[i]);
        dpSet(machineName1+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);

        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val1",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val2",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val3",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val4",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val5",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val6",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val7",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val8",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val9",strArray[i]);
        dpSet(machineName2+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);
        
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val1",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val2",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val3",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val4",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val5",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val6",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val7",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val8",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val9",strArray[i]);
        dpSet(machineName3+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);

        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val1",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val2",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val3",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val4",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val5",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val6",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val7",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val8",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val9",strArray[i]);
        dpSet(machineName4+"lofarSpeedTest."+val+"Collection.val10",strArray[i]);

      }
    }      
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+"h "+minute(timed)+"m "+second(timed)+"s "+ milliSecond(timed)+"ms";
    dpSet("lofarSpeedTest.result.nrWrites",100,
          "lofarSpeedTest.result.writeTime",tStr);
    isRunning=false;
    DebugTN("*****End write cycle reached*****");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName3+"lofarSpeedTest."+val+"Collection.val1 does not exist or "+
            "dp "+machineName4+"lofarSpeedTest."+val+"Collection.val1 does not exist, no write possible");
    isRunning=false;
    return;
  }
}
    
void readCollection(string machineName,string val) {
  if (dpExists(machineName+"lofarSpeedTest."+val+"Collection.val1") ){
    emptyPoints();
    dpConnect("centralCollectionTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val1",
                                               machineName+"lofarSpeedTest."+val+"Collection.val2",
                                               machineName+"lofarSpeedTest."+val+"Collection.val3",
                                               machineName+"lofarSpeedTest."+val+"Collection.val4",
                                               machineName+"lofarSpeedTest."+val+"Collection.val5",
                                               machineName+"lofarSpeedTest."+val+"Collection.val6",
                                               machineName+"lofarSpeedTest."+val+"Collection.val7",
                                               machineName+"lofarSpeedTest."+val+"Collection.val8",
                                               machineName+"lofarSpeedTest."+val+"Collection.val9",
                                               machineName+"lofarSpeedTest."+val+"Collection.val10");
  } else {
    DebugTN("Error: dp "+machineName+"lofarSpeedTest."+val+"Collection.val1 does not exist, no connect possible"); 
    isRunning = false;
    return;
  }
}     

void readSingleCollection(string machineName,string val) {
  if (dpExists(machineName+"lofarSpeedTest."+val+"Collection") ){
    emptyPoints();
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val1");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val2");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val3");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val4");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val5");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val6");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val7");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val8");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val9");
    dpConnect("centralSingleTrigger",false,machineName+"lofarSpeedTest."+val+"Collection.val10");
  } else {
    DebugTN("Error: dp "+machineName+"lofarSpeedTest."+val+"Collection.val1 does not exist, no connect possible"); 
    isRunning = false;
    return;
  }
}

void emptyPoints() {
  dpSet("lofarSpeedTest.result.nrWrites",0,
        "lofarSpeedTest.result.nrReads",0,
        "lofarSpeedTest.result.writeTime","",
        "lofarSpeedTest.result.readTime","");
}

void fillStringArray() {
  for (int i=1;i<=1000;i++) {
    strArray[i] = "testing step"+i;
  }
}

void reset() {
  emptyPoints();
  isRunning=false;
}
