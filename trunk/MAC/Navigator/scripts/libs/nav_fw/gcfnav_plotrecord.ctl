#uses "nav_fw/gcf-common.ctl"
#uses "nav_fw/gcfnav-functions.ctl"
#uses "nav_fw/gcfnav_view.ctl"
global int g_counter=0;
global string g_path_temp = "";
global dyn_string recordList;
global string g_path_gnuplot;
global string g_path_pictureconverter;

main()
{
  dpSet("__navigator.recordRCV", "");    //Set the output to ""
  //Connects to the datapoint .CMD, this is the commandInput the this
  //function.
  navConfigInitPathNames();
  dpConnect("navViewPlotRecordMain", FALSE, "__navigator.recordCMD");
}




///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotRecordMain; handles the commands from the plot-view
//                                            
// Input:  1. commandInput
//            content: <$configDatapoint>|<$datapocurrentDatapointint>|<command>
//            possible commands: start, stop, status
// Output: 1. 
///////////////////////////////////////////////////////////////////////////
navViewPlotRecordMain(string dp1, string commandInput)
{
  dyn_string commandInputSplit;
  string cmd_command = "";
  string cmd_configDatapoint = "";
  string cmd_datapoint = "";
  
  //////////////////////////////////////////////////////////////
  // Split the incomming CMD in a config datapoint and a command
  //////////////////////////////////////////////////////////////
  commandInputSplit = strsplit(commandInput, "|");
  if(dynlen(commandInputSplit)>=1)
  {
    cmd_configDatapoint = commandInputSplit[1];
    if(dynlen(commandInputSplit)>=2)
    {
      cmd_datapoint = commandInputSplit[2];
      if(dynlen(commandInputSplit)>=3)
      {
        cmd_command = strtolower(commandInputSplit[3]);
      }
    }
  }
  
  ////////////////////////////
  // Compute the chosen option
  ////////////////////////////
  switch (cmd_command)
  {
  case "status":
    // There is asked for the status of this cmd_configDatapoint and cmd_datapoint,
    // the status is returned to "__navigator.recordRCV" so the requester can handle this
    // response for further computation.
    LOG_TRACE("record received cmd: status");
    string match = cmd_configDatapoint+"|"+cmd_datapoint+"*";
    dyn_string searchResult = dynPatternMatch(match, recordList);
    dyn_string dpToDisconnect;
    if(dynlen(searchResult)<=0)
    {
      dpSet("__navigator.recordRCV", cmd_configDatapoint+"|"+cmd_datapoint+"|idle");
    }
    else
    {
      dpSet("__navigator.recordRCV", cmd_configDatapoint+"|"+cmd_datapoint+"|recording");
    }
    
    break;
  case "start":
    // Place all as active configured DP's in a dyn_string, add only the not already
    // recorded datapoints. Then start the record function.
    LOG_TRACE("record received cmd: start");
    dyn_string dpNamesToRecord = navViewPlotRecordGetDpNamesToRecord(cmd_configDatapoint);
    dpNamesToRecord = navViewPlotRecordListAddItems(dpNamesToRecord);
    if(dynlen(dpNamesToRecord)>0)
    {
      navViewPlotRecordControl(TRUE, dpNamesToRecord);
    }
    break;
  case "show all":
    LOG_TRACE("record received cmd: show all");
    dyn_string showAllDPs;
    for(int i=1; i<=dynlen(recordList); i++)
    {
      showAllDPs[dynlen(showAllDPs)+1] = navViewPlotGetSplitPart(recordList[i], 2);
    }
    dynSortAsc(showAllDPs);
    dpSet("__navigator.recordRCV", showAllDPs);
    break;
  case "stop":
    LOG_TRACE("record received cmd: stop");    
    dyn_string searchResult = dynPatternMatch(cmd_configDatapoint+"|"+cmd_datapoint+"*", recordList);
    dyn_string dpToDisconnect;
    if(dynlen(searchResult)>0)
    {
      for(int i=1; i<=dynlen(searchResult); i++)
      {
        int position = dynContains(recordList, searchResult[i]);
        dpToDisconnect[dynlen(dpToDisconnect)+1]= navViewPlotGetSplitPart(recordList[position], 2);
        //dynRemove(recordList, position); replaced to function navViewPlotWriteRecordDesciptionFile
      }
      navViewPlotRecordControl(FALSE, dpToDisconnect);
    }
    break;
  case "stop all":
    LOG_TRACE("record received cmd: stop all");    
    dyn_string stopAllDPs;
    for(int i=1; i<=dynlen(recordList); i++)
    {
      stopAllDPs[dynlen(stopAllDPs)+1] = navViewPlotGetSplitPart(recordList[i], 2);
    }
    navViewPlotRecordControl(FALSE, stopAllDPs);
    //dynClear(recordList); replaced to function navViewPlotWriteRecordDesciptionFile
    break;
  default:
    LOG_TRACE("record received cmd: unknown (default)");      
    dpSet("__navigator.recordRCV", "unknown command");
    break;
  }
  //DebugN("###  recordList  ###");
  //DebugN(recordList);
}
 
///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotRecordListAddItems; add's new datapoint with 
//           addional information to the global recordList. But only if
//           they are unique!! Otherwise the are ignored.
//                                            
// Input:  1. datapoint names with config to add to recordList
// Output: 1. dyn_string with unique datapoint to add
///////////////////////////////////////////////////////////////////////////
dyn_string navViewPlotRecordListAddItems(dyn_string dpNamesToAdd)
{
  dyn_string matchedDPs;
  for(int i=1; i<=dynlen(dpNamesToAdd); i++)
  {
    dyn_string match = dynPatternMatch("*"+navViewPlotGetSplitPart(dpNamesToAdd[i], 2)+"*", recordList); 
    if(dynlen(match)==0)
    {
      dynAppend(recordList, dpNamesToAdd[i]);
      matchedDPs[dynlen(matchedDPs)+1] = dpNamesToAdd[i];
    }
  }
  return matchedDPs;
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotRecordGetDpNamesToRecord; gathers all the active
//           configured datapoints in the $configDatapoint
//                                            
// Input:  1. configDatapoint
// Output: 1. all active and existing configured datcurrentDatapointapoints.
///////////////////////////////////////////////////////////////////////////
dyn_string navViewPlotRecordGetDpNamesToRecord(string configDatapoint)
{
  bool plotActive;
  string plotDpName;
  string currentDatapoint;
  dyn_string newDatapoints;
  string fileNamePrefix;

  if(dpAccessable(configDatapoint))
  {
    dpGet(configDatapoint + ".currentDatapoint", currentDatapoint);
    dpGet(configDatapoint + ".fileNamePrefix",  fileNamePrefix);
    if(fileNamePrefix=="")
    {
      fileNamePrefix = "NoPrefixDefined";   
    }
    fileNamePrefix = fileNamePrefix + navViewGetTimeString();
    for(int i=1;i<=8;i++)
    {
      plotActive = FALSE;
      dpGet(configDatapoint + "." + i + ".active", plotActive);
      if(plotActive)
      {
        plotDpName = "";
        dpGet(configDatapoint + "." + i + ".dpName", plotDpName);
        string dpToConnect = navViewRetrieveDpName(currentDatapoint, plotDpName);
        if(plotDpName!="" && dpAccessable(dpToConnect))
        {
          newDatapoints[dynlen(newDatapoints)+1] = configDatapoint + "|"+ currentDatapoint + plotDpName + "|"+ i+"|"  + fileNamePrefix + "|0";
        }
      }
    }
  }
  return newDatapoints;
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotRecordControl;
//                                            
// Input:  1. start=TRUE  => do a dpConnect    on all datapoint
//            start=FALSE => do a dpDisconnect on all datapoint
// Output: 1. None
///////////////////////////////////////////////////////////////////////////
navViewPlotRecordControl(bool start, dyn_string newDatapoints)
{
  if(start)
  {
    for(int i=1; i<=dynlen(newDatapoints); i++)
    {
      string dpToConnect = navViewPlotGetSplitPart(newDatapoints[i],2 );
      navPMLloadPropertySet(dpToConnect);
      dpConnect("RecordSpectrum", FALSE, dpToConnect);
      //Write file record info to disc
      navViewPlotWriteRecordDesciptionFile(newDatapoints[i], TRUE);
    }
  }
  else
  {
    for(int i=1; i<=dynlen(newDatapoints); i++)
    {
      navPMLunloadPropertySet(newDatapoints[i]);
      navViewPlotWriteRecordDesciptionFile(newDatapoints[i], FALSE);
      dpDisconnect("RecordSpectrum", newDatapoints[i]);
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotWriteRecordDesciptionFile;
//                                            
// Input:  1. Record configuration data
// Output: 1. Data is stored in the ".dat" file as header
///////////////////////////////////////////////////////////////////////////
navViewPlotWriteRecordDesciptionFile(string datapoint, bool startRecord)
{
  dyn_string searchMatch = dynPatternMatch("*" + datapoint + "*", recordList);
  int position = dynContains(recordList, searchMatch[1]);
  string configDpName = navViewPlotGetSplitPart(searchMatch[1],1);
  string dpName = navViewPlotGetSplitPart(searchMatch[1],2);
  string plotNumber = navViewPlotGetSplitPart(searchMatch[1],3);
  string prefixFileName = navViewPlotGetSplitPart(searchMatch[1],4);
  
  // If no prefix is givin, display this in filename
  if(""==prefixFileName)
  {
    prefixFileName = "NoPrefix";
  }
  
  // Next if statement set the environment for the header(startRecord=TRUE)
  // or footer (startRecord=FALSE) in the dat file
  string fileExtention;
  if(startRecord)
  {
    fileExtention = ".dat";
  }
  else
  {
    fileExtention = ".tmp";
  }
  
  string timeString = getCurrentTime();
  string userName = getUserName(getUserId());
  file f = fopen(navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + prefixFileName + "_record_"+plotNumber + fileExtention, "w+");
    fputs ("#----------------------------------------------------------------------#\n", f );
    fputs ("# Automatic generated file                                             #\n", f );
    fputs ("# By Navigator recordFunction                                          #\n", f );
    fputs ("#----------------------------------------------------------------------#\n", f );
    if(startRecord)
    {
      fputs ("# Record started on: " + timeString +"\n", f );
      fputs ("# By user          : " + userName+"\n", f );
      fputs ("# For datapoint    : " + dpName+"\n", f );
      fputs ("# Config datapoint : " + configDpName+"\n", f );
      fputs ("# Plot number      : " + plotNumber+"\n", f );
      fputs ("# Data fileName    : " + prefixFileName + "_record_"+plotNumber+".dat"+"\n", f );
      fputs ("#----------------------------------------------------------------------#\n", f );
    }
    else
    {
      fputs ("# Record stopped on: " + timeString +"\n", f );
      fputs ("# By user          : " + userName+"\n", f );
      fputs ("#------------------------------- EOF ----------------------------------#\n", f );
    }
  fclose(f);
  
  // if footer, then perform atlast a type <tmp-file> >> <dat-file>
  if(!startRecord)
  {
    dynRemove(recordList, position);
    system("type " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + prefixFileName + "_record_"+plotNumber+".tmp >> " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + prefixFileName + "_record_"+plotNumber+".dat");
  }
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotGetSplitPart;
//                                            
// Input:  1. string with <content>|<content>|<content>
//         2. with split part you want te be returned
// Output: 1. split part of type string
///////////////////////////////////////////////////////////////////////////
string navViewPlotGetSplitPart(string datapoint, int splitNr)
{
  dyn_string dpSplit = strsplit(datapoint, "|");
  return dpSplit[splitNr];
}


///////////////////////////////////////////////////////////////////////////
// Function: RecordSpectrum;
//                                            
// Input:  1. datapoint name te record
// Output: 1. spectrum data file on disc
///////////////////////////////////////////////////////////////////////////
void RecordSpectrum(string dp1, string spectrum)
{
  ///////////////////////////////////////////////////////////////////////////////
  //Get all information from the recordList for this specific datapoint to record
  ////////////////////////////////////////////////////////////////////////////////
  string datapoint = dpSubStr(dp1, DPSUB_SYS_DP_EL);
  dyn_string searchMatch = dynPatternMatch("*" + datapoint + "*", recordList);
  int position = dynContains(recordList, searchMatch[1]);
  string configDpName = navViewPlotGetSplitPart(searchMatch[1],1);
  string dpName = navViewPlotGetSplitPart(searchMatch[1],2);
  string plotNumber = navViewPlotGetSplitPart(searchMatch[1],3);
  string prefixFileName = navViewPlotGetSplitPart(searchMatch[1],4);
  int counter = navViewPlotGetSplitPart(searchMatch[1],5);


  file f;
  f = fopen(navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + prefixFileName + "_record_"+plotNumber+".tmp", "w+");
    dyn_string out = strsplit(spectrum, "\n");
    int spectrumLength = dynlen(out);
    for(int i=0; i<=spectrumLength; i++)
    {
      strreplace(out[i+1], i + " ", counter + " " + i + " ");
      fputs (out[i+1] + "\n", f );    
    }
    fputs ("\n", f );
  fclose(f);
  system("type " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + prefixFileName + "_record_"+plotNumber+".tmp >> " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + prefixFileName + "_record_"+plotNumber+".dat");

  //Write the complete config back to recordList with the changed counter info
  counter++;
  recordList[position]= configDpName +"|"+ dpName +"|"+ plotNumber +"|"+ prefixFileName +"|"+ counter;
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewGetTimeString              ;
//                                            
// Input:  1. 
// Output: 1. 
///////////////////////////////////////////////////////////////////////////
string navViewGetTimeString()
{
  time t = getCurrentTime();
  string txtYear   = year(t);
  string txtMonth  = month(t);
  string txtDay    = day(t);
  string txtHour   = hour(t);
  string txtMinute = minute(t);
  string txtSecond = second(t);
  
  return "_"+txtYear + txtMonth + txtDay + "_"+txtHour + txtMinute + txtSecond;
}


///////////////////////////////////////////////////////////////////////////
// Function: RecordFunction;
//                                            
// Input:  1. 
// Output: 1. spectrum
///////////////////////////////////////////////////////////////////////////
RecordFunction(string dp1, bool triggerRecord)
{
  string dpName;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + ".dpRecord", dpName);
  if(triggerRecord)
  { 
    dpConnect("RecordSpectrum", dpName);
    file f;
    f = fopen(navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "record_1.dat", "w+");
    fclose(f);
  }
  else
  {
    dpDisconnect("RecordSpectrum", dpName);
    g_counter = 0;
  }
}
