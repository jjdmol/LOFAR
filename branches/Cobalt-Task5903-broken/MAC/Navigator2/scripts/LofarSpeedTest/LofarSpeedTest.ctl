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
global int  totalWrites = 0;
global dyn_string strArray;
global string stationName="";

global bool isTestTriggered = false;

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
    dpConnect("testTriggered",false,"lofarSpeedTest.result.testName");
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
    oneStationToCentral1000To1("Int");
  } else if (testName == "One station to central(1000 string x 1)" ){
    oneStationToCentral1000To1("String");
  } else if (testName == "One station to central(100 int x 10)" ){
    oneStationToCentral100To10("Int");
  } else if (testName == "One station to central(100 string x 10)" ){
    oneStationToCentral100To10("String");
  } else if (testName == "One station to central(100 int x 10 x 1)" ){
    oneStationToCentral100To10x1("Int");
  } else if (testName == "One station to central(100 string x 10 x 1)" ){
    oneStationToCentral100StringTo10x1();
  }else if (testName == "Central from one station(1000 int x 1)" ){
    centralFromOneStation1000To1("Int");
  } else if (testName == "Central from one station(1000 string x 1)" ){
    centralFromOneStation1000StringTo1();
  } else if (testName == "Central from one station(100 int x 10)" ){
    centralFromOneStation100To10("Int");
  } else if (testName == "Central from one station(100 string x 10)" ){
    centralFromOneStation100StringTo10();
  } else if (testName == "Central from one station(100 int x 10 x 1)" ){
    centralFromOneStation100To10x1("Int");
  } else if (testName == "Central from one station(100 string x 10 x 1)" ){
    centralFromOneStation100StringTo10x1();
  } else if (testName == "Two stations to central(1000 int x 1)" ){
    twoStationsToCentral1000To1("Int");
  } else if (testName == "Two stations to central(1000 string x 1)" ){
    twoStationsToCentral1000To1("String");
  } else if (testName == "Two stations to central(100 int x 10)" ){
    twoStationsToCentral100To10("Int");
  } else if (testName == "Two stations to central(100 string x 10)" ){
    twoStationsToCentral100To10("String");
  } else if (testName == "Two stations to central(100 int x 10 x 1)" ){
    twoStationsToCentral100To10x1("Int");
  } else if (testName == "Two stations to central(100 string x 10 x 1)" ){
    twoStationsToCentral100To10x1("String");
  } else if (testName == "Central from two stations(1000 int x 1)" ){
    centralFromTwoStations1000To1("Int");
  } else if (testName == "Central from two stations(1000 string x 1)" ){
    centralFromTwoStations1000To1("String");
  } else if (testName == "Central from two stations(100 int x 10)" ){
    centralFromTwoStations100To10("Int");
  } else if (testName == "Central from two stations(100 string x 10)" ){
    centralFromTwoStations100To10("String");
  } else if (testName == "Central from two stations(100 int x 10 x 1)" ){
    centralFromTwoStations100To10x1("Int");
  } else if (testName == "Central from two stations(100 string x 10 x 1)" ){
    centralFromTwoStations100To10x1("String");
  } else if (testName == "Four stations to central(1000 int x 1)" ){
    fourStationsToCentral1000To1("Int");
  } else if (testName == "Four stations to central(1000 string x 1)" ){
    fourStationsToCentral1000To1("String");
  } else if (testName == "Four stations to central(100 int x 10)" ){
    fourStationsToCentral100To10("Int");
  } else if (testName == "Four stations to central(100 string x 10)" ){
    fourStationsToCentral100To10("String");
  } else if (testName == "Four stations to central(100 int x 10 x 1)" ){
    fourStationsToCentral100To10x1("Int");
  } else if (testName == "Four stations to central(100 string x 10 x 1)" ){
    fourStationsToCentral100To10x1("String");
  } else if (testName == "Central from four stations(1000 int x 1)" ){
    centralFromFourStations1000To1("Int");
  } else if (testName == "Central from four stations(1000 string x 1)" ){
    centralFromFourStations1000To1("String");
  } else if (testName == "Central from four stations(100 int x 10)" ){
    centralFromFourStations100To10("Int");
  } else if (testName == "Central from four stations(100 string x 10)" ){
    centralFromFourStations100To10("String");    
  } else if (testName == "Central from four stations(100 int x 10 x 1)" ){
    centralFromFourStations100To10x1("Int");
  } else if (testName == "Central from four stations(100 string x 10 x 1)" ){
    centralFromFourStations100To10x1("String");    
  } else if (testName == "Two stations from central(1000 int x 1)" ){
    twoStationsFromCentral1000To1("Int");
  } else if (testName == "Two stations from central(1000 string x 1)" ){
    twoStationsFromCentral1000To1("String");
  } else if (testName == "Two stations from central(100 int x 10)" ){
    twoStationsFromCentral100To10("Int");
  } else if (testName == "Two stations from central(100 string x 10)" ){
    twoStationsFromCentral100To10("String");
  } else if (testName == "Two stations from central(100 int x 10 x 1)" ){
    twoStationsFromCentral100To10x1("Int");
  } else if (testName == "Two stations from central(100 string x 10 x 1)" ){
    twoStationsFromCentral100To10x1("String");
  }else if (testName == "Central to two stations(1000 int x 1)" ){
    centralToTwoStations1000To1("Int");
  } else if (testName == "Central to two stations(1000 string x 1)" ){
    centralToTwoStations1000To1("String");
  } else if (testName == "Central to two stations(100 int x 10)" ){
    centralToTwoStations100To10("Int");
  } else if (testName == "Central to two stations(100 string x 10)" ){
    centralToTwoStations100To10("String");
  } else if (testName == "Central to two stations(100 int x 10 x 1)" ){
    centralToTwoStations100To10x1("Int");
  } else if (testName == "Central to two stations(100 string x 10 x 1)" ){
    centralToTwoStations100To10x1("String");
  } else if (testName == "Four stations from central(1000 int x 1)" ){
    fourStationsFromCentral1000To1("Int");
  } else if (testName == "Four stations from central(1000 string x 1)" ){
    fourStationsFromCentral1000To1("String");
  } else if (testName == "Four stations from central(100 int x 10)" ){
    fourStationsFromCentral100To10("Int");
  } else if (testName == "Four stations from central(100 string x 10)" ){
    fourStationsFromCentral100To10("String");
  } else if (testName == "Four stations from central(100 int x 10 x 1)" ){
    fourStationsFromCentral100To10x1("Int");
  } else if (testName == "Four stations from central(100 string x 10 x 1)" ){
    fourStationsFromCentral100To10x1("String");
  } else if (testName == "Central to four stations(1000 int x 1)" ){
    centralToFourStations1000To1("Int");
  } else if (testName == "Central to four stations(1000 string x 1)" ){
    centralToFourStations1000To1("String");
  } else if (testName == "Central to four stations(100 int x 10)" ){
    centralToFourStations100To10("Int");
  } else if (testName == "Central to four stations(100 string x 10)" ){
    centralToFourStations100To10("String");    
  } else if (testName == "Central to four stations(100 int x 10 x 1)" ){
    centralToFourStations100To10x1("Int");
  } else if (testName == "Central to four stations(100 string x 10 x 1)" ){
    centralToFourStations100To10x1("String");  
  } else if (testName == "Pingpong between central and one station (1000 int)") { 
    centralToOneStationPingPong1000("Int");
  } else if (testName == "Pingpong between central and one station (1000 string)") { 
    centralToOneStationPingPong1000("String");
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
// Name : oneStationToCentral1000To1
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
void oneStationToCentral1000To1(string choice) {
  DebugTN("oneStationToCentral1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 1000 single "+choice+" values to Central");
    writeSingle("MCU001:",choice);
  } else if (stationName == "") {
    DebugTN("Will start reading 1000 single "+choice+" values from Central");
    readSingle("MCU001:",choice);
  }
}

// *******************************************
// Name : oneStationToCentral100To10
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
void oneStationToCentral100To10(string choice) { 
  DebugTN("oneStationToCentral100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=100;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 "+choice+" values to Central");
    writeCollection("MCU001:",choice);
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Central");
    readCollection("MCU001:",choice);
  }
}



// *******************************************
// Name : oneStationToCentral100To10x1
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
void oneStationToCentral100To10x1(string choice) {
  DebugTN("oneStationToCentral100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 (single) "+choice+"  values to Central");
    writeSingleCollection("MCU001:",choice);
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 (single) "+choice+" values from Central");
    readSingleCollection("MCU001:",choice);
  }
}


//
//  ********** Central From One Station **********
//

// *******************************************
// Name : centralFromOneStation1000To1
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
void centralFromOneStation1000To1(string choice) {
  DebugTN("centralFromOneStation1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 1000 single "+choice+" values to Station");
    writeSingle(STATION1,choice);
  } else if (stationName == "") {
    DebugTN("Will start reading 1000 single "+choice+" values from Station");
    readSingle(STATION1,choice);
  }
}


// *******************************************
// Name : centralFromOneStation100To10
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
void centralFromOneStation100To10(string choice) {
  DebugTN("centralFromOneStation100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=100;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 "+choice+" values to Station");
    writeCollection(STATION1,choice);
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Station");
    readCollection(STATION1,choice);
  }
}


// *******************************************
// Name : centralFromOneStation100To10x1
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
void centralFromOneStation100To10x1(string choice) {
  DebugTN("centralFromOneStation100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start writing 100 series of 10 (Single) "+choice+" values to Station");
    writeSingleCollection(STATION1,choice);
  } else if (stationName == "") {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Station");
    readSingleCollection(STATION1,choice);
  }
}



//
//  ********** Two Stations To Central **********
//

// *******************************************
// Name : twoStationsToCentral1000To1
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
void twoStationsToCentral1000To1(string choice) {
  DebugTN("twoStationsToCentral1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Station  Will start writing 1000 single "+choice+" values to Central");
    writeSingle("MCU001:",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start reading 2000 single "+choice+" values from Central");
    readSingle("MCU001:",choice);
  }
}


// *******************************************
// Name : twoStationsToCentral100To10
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
void twoStationsToCentral100To10(string choice) {
  DebugTN("twoStationsToCentral100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=200;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of "+choice+" values to Central");
    writeCollection("MCU001:",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Central");
    readCollection("MCU001:",choice);
  }
}

// *******************************************
// Name : twoStationsToCentral100To10x1
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
void twoStationsToCentral100To10x1(string choice) {
  DebugTN("twoStationsToCentral100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of (Single) "+choice+" values to Central");
    writeSingleCollection("MCU001:",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Central");
    readSingleCollection("MCU001:",choice);
  }
}

//
//  ********** Central From Two Stations **********
//

// *******************************************
// Name : centralFromTwoStations1000To1
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
void centralFromTwoStations1000To1(string choice) {
  DebugTN("centralFromTwoStations1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 1000 single "+choice+" values to Station");
    writeSingle("",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start reading 2000 single "+choice+" values from Station");
    readSingleFromTwo(STATION1,STATION2,choice);    
  }
}

// *******************************************
// Name : centralFromTwoStations100To10
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
void centralFromTwoStations100To10(string choice) {
  DebugTN("centralFromTwoStations100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=200;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 "+choice+" values to Station");
    writeCollection("",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Stations");
    readCollectionFromTwo(STATION1,STATION2,choice);
  }
}


// *******************************************
// Name : centralFromTwoStations100To10x1
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
void centralFromTwoStations100To10x1(string choice) {
  DebugTN("centralFromTwoStations100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=2000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start writing 100 series of 10 (Single) "+choice+" values to Station");
    writeSingleCollection("",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Stations");
    readSingleCollectionFromTwo(STATION1,STATION2,choice);

  }
}

//
//  ********** Four Stations To Central *********
//

// *******************************************
// Name : fourStationsToCentral1000To1
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
void fourStationsToCentral1000To1(string choice) {
  DebugTN("fourStationsToCentral1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Station Will start writing 1000 single "+choice+" values to Central");
    writeSingle("MCU001:",choice);
  } else {
    DebugTN("Will start reading 4000 single "+choice+" values from Central");
    readSingle("MCU001:",choice);
  }
}

// *******************************************
// Name : fourStationsToCentral100To10
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
void fourStationsToCentral100To10(string choice) {
  DebugTN("fourStationsToCentral100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=400;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of "+choice+" values to Central");
    writeCollection("MCU001:",choice);
  } else {
    DebugTN("Will start reading 400 series of 10 "+choice+" values from Central");
    readCollection("MCU001:",choice);
  }
}

//
//  ********** Central From Four Stations **********
//

// *******************************************
// Name : centralFromFourStations1000To1
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
void centralFromFourStations1000To1(string choice) {
  DebugTN("centralFromFourStations1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 1000 single "+choice+" values to Station");
    writeSingle("",choice);
  } else {
    DebugTN("Will start reading 4000 single "+choice+" values from Station");
    readSingleFromFour(STATION1,STATION2,STATION3,STATION4,choice);    
  }
}


// *******************************************
// Name : centralFromFourStations100To10
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
void centralFromFourStations100To10(string choice) {
  DebugTN("centralFromFourStation100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=400;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of 10 "+choice+" values to Station");
    writeCollection("",choice);
  } else {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Stations");
    readCollectionFromFour(STATION1,STATION2,STATION3,STATION4,choice);
   }
}

    
// *******************************************
// Name : centralFromFourStations100To10x1
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
void centralFromFourStations100To10x1(string choice) {
  DebugTN("centralFromFourStation100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=4000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start writing 100 series of 10 (Single) "+choice+" values to Station");
    writeSingleCollection("",choice);
  } else {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Stations");
    readSingleCollectionFromFour(STATION1,STATION2,STATION3,STATION4,choice);
  }
}


//
//  ********** Two Stations From Central **********
//

// *******************************************
// Name : twoStationsFromCentral1000To1
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
void twoStationsFromCentral1000To1(string choice) {
  DebugTN("twoStationsFromCentral1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Station Will start reading 1000 single "+choice+" values from the Central");
    readSingle("MCU001:",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start writing 1000 single "+choice+" values to Central");
    writeSingle("MCU001:",choice);
  }
}

// *******************************************
// Name : twoStationsFromCentral100To10
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
void twoStationsFromCentral100To10(string choice) {
  DebugTN("twoStationsFromCentral100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=100;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Central");
    readCollection("MCU001:",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series if "+choice+" values to Central");
    writeCollection("MCU001:",choice);
  }
}

// *******************************************
// Name : twoStationsFromCentral100To10x1
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
void twoStationsFromCentral100To10x1(string choice) {
  DebugTN("twoStationsFromCentral100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Central");
    readSingleCollection("MCU001:",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 (Single) "+choice+" values to Central");
    writeSingleCollection("MCU001:",choice);
  }
}

//
//  ********** Central To Two Stations **********
//

// *******************************************
// Name : centralToTwoStations1000To1
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
void centralToTwoStations1000To1(string choice) {
  DebugTN("centralToTwoStations1000To1 starting ");
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 1000 single "+choice+" values from Station");
    readSingle("",choice);    
  } else if (stationName == "" ) {
    DebugTN("Will start writing 1000 single "+choice+" values to two Station");
    writeSingleToTwo(STATION1,STATION2,choice);
  }
}

// *******************************************
// Name : centralToTwoStations100To10
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
void centralToTwoStations100To10(string choice) {
  DebugTN("centralToTwoStations100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=100;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Station");
    readCollection("",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 "+choice+" values to Station");
    writeCollectionToTwo(STATION1,STATION2,choice);    
  }
}


// *******************************************
// Name : centralToTwoStations100To10x1
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
void centralToTwoStations100To10x1(string choice) {
  DebugTN("centralToTwoStations100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=100;
  nrReads=0;
  
  if (isStation && (stationName==STATION1 || stationName==STATION2)) {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Station");
    readSingleCollection("",choice);
  } else if (stationName == "" ) {
    DebugTN("Will start writing 100 series of 10 (Single) "+choice+" values to Station");
    writeSingleCollectionToTwo(STATION1,STATION2,choice);    
  }
}

//
//  ********** Four Stations From Central **********
//

// *******************************************
// Name : fourStationsFromCentral1000To1
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
void fourStationsFromCentral1000To1(string choice) {
  DebugTN("fourStationsFromCentral1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 1000 single "+choice+" values from Central");
    readSingle("MCU001:",choice);
  } else {
    DebugTN("Script Will start writing 1000 single "+choice+" values to Central");
    writeSingle("MCU001:",choice);
  }
}

// *******************************************
// Name : fourStationsFromCentral100To10
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
void fourStationsFromCentral100To10(string choice) {
  DebugTN("fourStationsFromCentral100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=100;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Central");
    readCollection("MCU001:",choice);
  } else {
    DebugTN("Will start writing 100 series if "+choice+" values to Central");
    writeCollection("MCU001:",choice);
  }
}

// *******************************************
// Name : fourStationsFromCentral100To10x1
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
void fourStationsFromCentral100To10x1(string choice) {
  DebugTN("fourStationsFromCentral100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Central");
    readSingleCollection("MCU001:",choice);
  } else {
    DebugTN("Will start writing 100 series of 10 (Single) "+choice+" values to Central");
    writeSingleCollection("MCU001:",choice);
  }
}


//
//  ********** Central To Four Stations **********
//

// *******************************************
// Name : centralToFourStations1000To1
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
void centralToFourStations1000To1(string choice) {
  DebugTN("centralToFourStations1000To1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 1000 single "+choice+" values from Station");
    readSingle("",choice);    
  } else {
    DebugTN("Will start writing 1000 single "+choice+" values to four Stations");
    writeSingleToFour(STATION1,STATION2,STATION3,STATION4,choice);
  }
}


// *******************************************
// Name : centralToFourStations100To10
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
void centralToFourStations100To10(string choice) {
  DebugTN("centralToFourStation100To10 starting for "+choice);
  isRunning=true;
  nrWrites=100;
  totalWrites=100;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 "+choice+" values from Stations");
    readCollection("",choice);
  } else {
    DebugTN("Will start writing 100 series of 10 "+choice+" values to Station");
    writeCollectionToFour(STATION1,STATION2,STATION3,STATION4,choice);
    
  }
}

// *******************************************
// Name : centralToFourStations100To10x1
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
void centralToFourStations100To10x1(string choice) {
  DebugTN("centralToFourStation100To10x1 starting for "+choice);
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation) {
    DebugTN("Will start reading 100 series of 10 (Single) "+choice+" values from Stations");
    readSingleCollection("",choice);
  } else {
    DebugTN("Will start writing 100 series of 10 (Single) "+choice+" values to Station");
    writeSingleCollectionToFour(STATION1,STATION2,STATION3,STATION4,choice);
    
  }
}

// Name : centralToOneStationPingPong1000
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
void centralToOneStationPingPong1000(string choice) {
  DebugTN("centralToOneStationPingPong1000 starting ");
  isRunning=true;
  nrWrites=1000;
  totalWrites=1000;
  nrReads=0;
  
  if (isStation && stationName==STATION1) {
    DebugTN("Will start station pingpong test with 1000 bounces with "+choice);
    emptyPoints();    
    if (dpExists(STATION1+"lofarSpeedTest.single"+choice) ){
      dpConnect("pingpongStation"+choice+"Trigger",false,STATION1+"lofarSpeedTest.single"+choice);
    } else {
      DebugTN("Error: couldn't connect to "+STATION1+"lofarSpeedTest.single"+choice+". Test halted");
    }
  } else {
    DebugTN("Will start maincu pingpong test with 1000 bounces with "+choice);
    emptyPoints();
    // Give readcycle a chance to start up
    delay(0,500);

    // 1st connect to own point
    if (dpExists("MCU001:lofarSpeedTest.single"+choice) ){
      dpConnect("pingpongCentral"+choice+"Trigger",false,"MCU001:lofarSpeedTest.single"+choice);
      // now check stations point and write first value to start the game
      if (dpExists(STATION1+"lofarSpeedTest.single"+choice) ){
        dpSet(STATION1+"lofarSpeedTest.single"+choice,10);
      } else {
        DebugTN("Error: couldn't connect to "+STATION1+"lofarSpeedTest.single"+choice);          
      }
    } else {
      DebugTN("Error: couldn't connect to MCU001:lofarSpeedTest.single"+choice+". Test halted");
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
  if (nrReads == totalWrites) {
    endTime = getCurrentTime();
//    DebugTN("starttijd: " + startTime);
//    DebugTN("eindtijd: " + endTime);
    time timed=endTime-startTime;
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrReads",nrReads,
          "lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    if (dpDisconnect("centralSingleTrigger",dp1) != 0) {
      DebugN("Error disconnecting centralSingleTrigger, dp: " ,dp1, " Error: ",getLastError());
    }
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
  if (nrReads == totalWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrReads",nrReads,
          "lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    if (dpDisconnect("centralCollectionTrigger",dp1,dp2,dp3,dp4,dp5,dp6,dp7,dp8,dp9,dp10)!= 0) {
      DebugN("Error disconnecting centralCollectionTrigger, dp1: " ,dp1, " Error: ",getLastError());
    }
    DebugTN("*****End read cycle reached*****");
  }
}



//
//  ********** Central From One Station **********
//

// *******************************************
// Name : stationSingleTrigger
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
  if (nrReads == totalWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    if (dpDisconnect("stationSingleTrigger",dp1) != 0) {
      DebugN("Error disconnecting stationSingleTrigger, dp: " ,dp1, " Error: ",getLastError());
    }

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
  if (nrReads == totalWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;

    if (dpDisconnect("stationCollectionTrigger",dp1,dp2,dp3,dp4,dp5,dp6,dp7,dp8,dp9,dp10) != 0) {
      DebugN("Error disconnecting stationCollectionTrigger, dp: " ,dp1, " Error: ",getLastError());
    }
    
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
  if (nrReads >= totalWrites) {
    isRunning=false;
    if (dpDisconnect("pingpongStationIntTrigger",dp1) != 0) {
      DebugN("Error disconnecting pingpongStationIntTrigger, dp: " ,dp1, " Error: ",getLastError());
    }
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
  if (nrReads >= totalWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    if (dpDisconnect("pingpongCentralIntTrigger",dp1) != 0) {
      DebugN("Error disconnecting pingpongCentralIntTrigger, dp: " ,dp1, " Error: ",getLastError());
    }
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
  if (nrReads == totalWrites) {
    isRunning=false;
    if (dpDisconnect("pingpongStationStringTrigger",dp1) != 0) {
      DebugN("Error disconnecting pingpongStationStringTrigger, dp: " ,dp1, " Error: ",getLastError());
    }
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
  if (nrReads == totalWrites) {
    endTime = getCurrentTime();
    time timed=endTime-startTime;
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("MCU001:lofarSpeedTest.result.nrReads",nrReads,
          "MCU001:lofarSpeedTest.result.readTime",tStr);
    isRunning=false;
    if (dpDisconnect("pingpongCentralStringTrigger",dp1) != 0) {
      DebugN("Error disconnecting pingpongCentralStringTrigger, dp: " ,dp1, " Error: ",getLastError());
    }
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",totalWrites,
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",nrWrites,
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
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest.single"+val);
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest.single"+val);
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest.single"+val);
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest.single"+val);
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
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest.single");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest.single");
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",totalWrites,
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",nrWrites,
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",totalWrites,
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",totalWrites,
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",totalWrites,
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
    string tStr=hour(timed)+","+minute(timed)+","+second(timed)+","+ milliSecond(timed);
    dpSet("lofarSpeedTest.result.nrWrites",totalWrites,
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

void readCollectionFromTwo(string machineName1,string machineName2,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection.val1") && 
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection.val1")  ){
    emptyPoints();
    dpConnect("centralCollectionTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val1",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val2",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val3",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val4",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val5",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val6",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val7",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val8",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val9",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val10");
    dpConnect("centralCollectionTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val1",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val2",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val3",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val4",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val5",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val6",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val7",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val8",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val9",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val10");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 or "+
                   "dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 does not exist no connect possible"); 
    isRunning = false;
    return;
  }
}     

void readCollectionFromFour(string machineName1,string machineName2,string machineName3, string machineName4,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection.val1") && 
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName3+"lofarSpeedTest."+val+"Collection.val1") &&
      dpExists(machineName4+"lofarSpeedTest."+val+"Collection.val1")){
    emptyPoints();
    dpConnect("centralCollectionTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val1",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val2",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val3",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val4",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val5",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val6",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val7",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val8",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val9",
                                               machineName1+"lofarSpeedTest."+val+"Collection.val10");
    dpConnect("centralCollectionTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val1",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val2",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val3",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val4",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val5",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val6",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val7",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val8",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val9",
                                               machineName2+"lofarSpeedTest."+val+"Collection.val10");
    dpConnect("centralCollectionTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val1",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val2",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val3",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val4",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val5",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val6",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val7",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val8",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val9",
                                               machineName3+"lofarSpeedTest."+val+"Collection.val10");
    dpConnect("centralCollectionTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val1",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val2",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val3",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val4",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val5",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val6",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val7",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val8",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val9",
                                               machineName4+"lofarSpeedTest."+val+"Collection.val10");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 or "+
                   "dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 or "+
                   "dp "+machineName3+"lofarSpeedTest."+val+"Collection.val1 or "+
                   "dp "+machineName4+"lofarSpeedTest."+val+"Collection.val1 does not exist no connect possible"); 
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

void readSingleCollectionFromTwo(string machineName1,string machineName2,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection") &&
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection") ){
    emptyPoints();
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val1");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val2");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val3");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val4");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val5");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val6");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val7");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val8");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val9");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val10");
    
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val1");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val2");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val3");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val4");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val5");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val6");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val7");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val8");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val9");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val10");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 or "+
                  " dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 does not exist, no connect possible"); 
    isRunning = false;
    return;
  }
}

void readSingleCollectionFromFour(string machineName1,string machineName2,string machineName3,string machineName4 ,string val) {
  if (dpExists(machineName1+"lofarSpeedTest."+val+"Collection") &&
      dpExists(machineName2+"lofarSpeedTest."+val+"Collection") &&
      dpExists(machineName3+"lofarSpeedTest."+val+"Collection") &&
      dpExists(machineName4+"lofarSpeedTest."+val+"Collection") ){
    emptyPoints();
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val1");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val2");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val3");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val4");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val5");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val6");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val7");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val8");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val9");
    dpConnect("centralSingleTrigger",false,machineName1+"lofarSpeedTest."+val+"Collection.val10");
    
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val1");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val2");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val3");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val4");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val5");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val6");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val7");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val8");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val9");
    dpConnect("centralSingleTrigger",false,machineName2+"lofarSpeedTest."+val+"Collection.val10");

    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val1");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val2");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val3");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val4");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val5");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val6");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val7");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val8");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val9");
    dpConnect("centralSingleTrigger",false,machineName3+"lofarSpeedTest."+val+"Collection.val10");
    
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val1");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val2");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val3");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val4");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val5");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val6");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val7");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val8");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val9");
    dpConnect("centralSingleTrigger",false,machineName4+"lofarSpeedTest."+val+"Collection.val10");
  } else {
    DebugTN("Error: dp "+machineName1+"lofarSpeedTest."+val+"Collection.val1 or "+
                  " dp "+machineName2+"lofarSpeedTest."+val+"Collection.val1 or "+
                  " dp "+machineName3+"lofarSpeedTest."+val+"Collection.val1 or "+
                  " dp "+machineName4+"lofarSpeedTest."+val+"Collection.val1 does not exist, no connect possible"); 
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
