//////////////////////////////////////////////////////////////////////////////////
// FunctionName: jump2StationSubrack
//
// jumps/load the Station_Subrack.pnl
///////////////////////////////////////////////////////////////////////////////////
void jump2StationSubrack()
{
  if ("LOFAR Navigator" == myPanelName())
  {
    navConfigTriggerNavigatorRefreshWithDP($datapoint + "_Rack" + $RackNr + "_SubRack" + $SubrackNr);
  }
  else
  {  
    RootPanelOn("navigator/views/Station_Subrack.pnl",
                "Station - Subrack",
                makeDynString("$datapoint:" + $datapoint + "_Rack" + $RackNr + "_SubRack" + $SubrackNr));
//				"$RackNr:" + RackNr,
//				"$SubrackNr:" + SubrackNr));
  }
}


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: jump2StationSubrackRCU
//
// jumps/load the Station_Subrack.pnl
///////////////////////////////////////////////////////////////////////////////////
void jump2StationSubrackRCU()
{
  if ("LOFAR Navigator" == myPanelName())
  {
    navConfigTriggerNavigatorRefreshWithDP($datapoint + "_Board1_AP" + $APNr + "_RCU"+$RCUNr);
  }
  else
  {  
    RootPanelOn("navigator/views/Station_Subrack_RCU.pnl",
                "Station - Subrack - RCU",
                makeDynString("$datapoint:" + $datapoint + "_Board1_AP" + $APNr + "_RCU"+$RCUNr));
  }
}


//////////////////////////////////////////////////////////////////////////////////
//
// FunctionName: setColumnConfig
//
// Function    : Set the width of the configured amount of columns,
//               otherwise they will be set to invisible
///////////////////////////////////////////////////////////////////////////////////
void setColumnConfig(int TableNumber, int nrOfColumns, int MaximumColumns, string Addition)
{

  int k, table_x, table_y;

  getValue("Table"+TableNumber, "size", table_x, table_y);
  for (k=0; k<nrOfColumns ; k++)
  {
    dpConnect("Arrange" + Addition + "TableContent", $configDatapoint + "."+TableNumber+".Column" + k + "Titles:_online.._value",
                                     $configDatapoint + "."+TableNumber+".Column" + k + "dpNames:_online.._value");
    setValue("Table"+TableNumber, "columnVisibility", k, TRUE);
    setValue("Table"+TableNumber, "columnWidth", k, (table_x/nrOfColumns));
  }
  for (k=nrOfColumns ; k <= MaximumColumns ; k++)
  {
    setValue("Table"+TableNumber, "columnVisibility", k, FALSE);
  }
}


//////////////////////////////////////////////////////////////////////////////////
//
// FunctionName: ConvIndex
// Function    : Hoog de index op 1 op.
//////////////////////////////////////////////////////////////////////////////////
int ConvIndex(int i)
{
  return i+1;
}


//////////////////////////////////////////////////////////////////////////////////
//
// FunctionName: getTableNumber
// Function    : Retrieve the number of the current table
// Output: 
///////////////////////////////////////////////////////////////////////////////////
void getTableNumber(string dpName, int &TableNumber)
{
  dyn_string resultaat;
  resultaat = strsplit(dpName, ".");
  TableNumber = resultaat[2];
}


//////////////////////////////////////////////////////////////////////////////////
//
// FunctionName: getColumnTitle
// Function    : Retrieve the Name of the current column
// Output: 
///////////////////////////////////////////////////////////////////////////////////
void getColumnTitle(string dpName, string &ColumnTitle)
{
  dyn_string resultaat;
  string test = "System1:__nav_TLcuPicRack_config_Alert_Alert-1.1.Column1dpNames:_online.._value";
  resultaat = strsplit(dpName, ".");
  ColumnTitle = strrtrim(resultaat[3], "dpNames:_online");
}


/////////////////////////////////////////////////////////////////////
// Funtion: dpGetElementName for trend
//
// Returns the elementname of a Datapoint
/////////////////////////////////////////////////////////////////////
string dpGetElementName(string DPName)
{
  return strltrim(dpSubStr(DPName, DPSUB_DP_EL), dpSubStr(DPName, DPSUB_DP));
}

/////////////////////////////////////////////////////////////////////
// Funtion: dpGetElementValueInt
//
// Returns the value of an SubElement
/////////////////////////////////////////////////////////////////////
int dpGetElementValueInt(string DPName, string ViewType, string SubElement, int &Value)
{
return dpGet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}

/////////////////////////////////////////////////////////////////////
// Funtion: dpSetElementValueInt
//
// Sets the value of an SubElement
/////////////////////////////////////////////////////////////////////
int dpSetElementValueInt(string DPName, string ViewType, string SubElement, int Value)
{
return dpSet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}


/////////////////////////////////////////////////////////////////////
// Funtion: dpGetElementValueString
//
// Returns the value of an SubElement
/////////////////////////////////////////////////////////////////////
int dpGetElementValueString(string DPName, string ViewType, string SubElement, string &Value)
{
return dpGet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}

/////////////////////////////////////////////////////////////////////
// Funtion: dpGetElementValueFloat
//
// Returns the value of an SubElement
/////////////////////////////////////////////////////////////////////
int dpGetElementValueFloat(string DPName, string ViewType, string SubElement, float &Value)
{
return dpGet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}

/////////////////////////////////////////////////////////////////////
//
// Function: Retrieve the trace number from the dpName
// FunctionName: getTraceNumber
//
/////////////////////////////////////////////////////////////////////
void getTraceNumber(string dpName, string &TraceNumber)
{
  string HulpString;
  dyn_string split;
  dyn_string HulpSplit;
  
  split       = strsplit(dpName, ".");
  HulpSplit   = strsplit(split[3], "race");
  TraceNumber = HulpSplit[dynlen(HulpSplit)];
}

///////////////////////////////////////////////////////////
//
// Function: Display for a certain time a text
//
///////////////////////////////////////////////////////////
void DisplayText(string ObjectName, string Text, string Value)
{
    setValue(ObjectName, "text", Text);
	  delay(0,600);
    setValue(ObjectName, "text", Value);
}

/////////////////////////////////////////////////////////////////////
//
// Function: Function te to retrieve the TrendNumber
// FunctionName: getTrendNumber
//
/////////////////////////////////////////////////////////////////////
void getTrendNumber(string dp1, int &TrendNumber)
{
  dyn_string split, split2, split3;
  dyn_string HulpSplit;
  
  split       = strsplit(dp1, ".");
//split2      = strsplit(split[1], "_");
//split3      = strsplit(split2[dynlen(split2)], "rend");
//TrendNumber = split3[dynlen(split3)];
	TrendNumber = split[2];
}

/////////////////////////////////////////////////////////////////////
//
// Function: connects an single element trace to trend
// FunctionName: ConnectTrace
//
/////////////////////////////////////////////////////////////////////
void ConnectTrace(int TraceNumber)
{
  dpConnect("TracePlot", "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".dpName",
  										   "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".AutoScale",
											   "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".ScaleMin",
											   "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".ScaleMax",
											   "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".LegendName");
}


/////////////////////////////////////////////////////////////////////
//
// Function: Creates and handles the RMB menu voor the RCU On the SubRack panel
// FunctionName: RCUContextMenu
//
/////////////////////////////////////////////////////////////////////
void RCUContextMenu()
{
  string txt_maintenance, txt_status;
  int Answer, status, maintenance;
  bool bOK;				//Variable with value FALSE

  dpGet($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +".status:_original.._value", status);
	dpGet($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_Maintenance.status:_original.._value", maintenance);

  BuildContextMenu(status, maintenance, Answer);

	//////////////////////////////////////////////////////////
	//
	//	Compute the chosen option
	//
	//////////////////////////////////////////////////////////
  	switch (Answer)
  	{
		case 2:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +".status:_original.._value", 0);
				break;
		case 3:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +".status:_original.._value", 1);
				break;
   	case 10:   	
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_Maintenance.status:_original.._value", 0);
      	break;
   	case 11:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_Maintenance.status:_original.._value", 1);
     	break;
    default:
    	break;
  	}       
}

/////////////////////////////////////////////////////////////////////
//
// Function: Builds the contextMenu for setting status and maintenance
// FunctionName: BuildContextMenu
//
/////////////////////////////////////////////////////////////////////
void BuildContextMenu(int status, int maintenance, int &Answer)
{
  string txt_status, txt_maintenance;
  if (status==1)
    txt_status = "PUSH_BUTTON, Set status to -> OK, 2, 1";
  else
    txt_status = "PUSH_BUTTON, Set status to -> Error, 3, 1";

 	if (maintenance==1)
    txt_maintenance = "PUSH_BUTTON, Turn off maintenance, 10, 1";
	else
	  txt_maintenance = "PUSH_BUTTON, Turn on maintenance, 11, 1";
	
	
  if  ((status==-1) && (maintenance!=-1))
    popupMenu(makeDynString(txt_maintenance), Answer);
  else if  ((status!=-1) && (maintenance==-1))
    popupMenu(makeDynString(txt_status), Answer);
  else // ((status!=-1) && (maintenance!=-1))
    popupMenu(makeDynString(txt_maintenance, "SEPARATOR", txt_status), Answer);
  
	
}

/////////////////////////////////////////////////////////////////////
//
// Function: Creates and handles the RMB menu for the antennas
// FunctionName: AntennaContextMenu
//
/////////////////////////////////////////////////////////////////////
void AntennaContextMenu(string antenna)
{
	string txt_maintenance;
  int Answer, status, maintenance;
  bool bOK;				//Variable with value FALSE

  dpGet($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", status);
	dpGet($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", maintenance);

  BuildContextMenu(status, maintenance, Answer);
  
	//########################################################
	//
	//	Compute the chosen option
	//
	//########################################################
  	switch (Answer)
  	{
		case 2:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", 0);
				break;
		case 3:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", 1);
				break;
   	case 10:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", 0);
      	break;
   	case 11:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", 1);
     	break;
    default:
    	break;
  	}       
}


/////////////////////////////////////////////////////////////////////
//
// Function: Creates and handles the RMB menu for the BP's
// FunctionName: BPContextMenu
//
/////////////////////////////////////////////////////////////////////
void BPContextMenu()
{
  string txt_maintenance, txt_status;
  int Answer, maintenance, status;
  bool bOK;				//Variable with value FALSE
  	
  dpGet($datapoint + "_Board1_BP.status:_original.._value", status);
	dpGet($datapoint + "_Board1_Maintenance.status:_original.._value", maintenance);
  BuildContextMenu(status, maintenance, Answer);


  //////////////////////////////////////////////////////////
  //
  //	Compute the chosen option
  //
  //////////////////////////////////////////////////////////
  	switch (Answer)
  	{
  	case 2:
			  dpSetWait($datapoint + "_Board1_BP.status:_original.._value", 0);
  			break;
   	case 3:
			  dpSetWait($datapoint + "_Board1_BP.status:_original.._value", 1);
  			break;
   	case 10:   	
			  dpSetWait($datapoint + "_Board1_Maintenance.status:_original.._value", 0);
/* 		  dpActivateAlert($datapoint + "_Board1_BP.status", bOK);
 		  	dpActivateAlert($datapoint + "_Board1_AP1.status", bOK);
 		  	dpActivateAlert($datapoint + "_Board1_AP2.status", bOK);
 		  	dpActivateAlert($datapoint + "_Board1_AP3.status", bOK);
 		  	dpActivateAlert($datapoint + "_Board1_AP4.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP1_RCU1.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP1_RCU2.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP2_RCU1.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP2_RCU2.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP3_RCU1.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP3_RCU2.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP4_RCU1.status", bOK);
		  	dpActivateAlert($datapoint + "_Board1_AP4_RCU2.status", bOK); */
      	break;
   	case 11:
			  dpSetWait($datapoint + "_Board1_Maintenance.status:_original.._value", 1);
/* 		  dpDeactivateAlert($datapoint + "_Board1_BP.status", bOK);
 		  	dpDeactivateAlert($datapoint + "_Board1_AP1.status", bOK);
 		  	dpDeactivateAlert($datapoint + "_Board1_AP2.status", bOK);
 		  	dpDeactivateAlert($datapoint + "_Board1_AP3.status", bOK);
 		  	dpDeactivateAlert($datapoint + "_Board1_AP4.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP1_RCU1.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP1_RCU2.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP2_RCU1.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP2_RCU2.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP3_RCU1.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP3_RCU2.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP4_RCU1.status", bOK);
		  	dpDeactivateAlert($datapoint + "_Board1_AP4_RCU2.status", bOK); */
     	break;
    default:
    	break;
  	}       
}


/////////////////////////////////////////////////////////////////////
//
// Function: Creates and handles the RMB menu for the AP's
// FunctionName: APContextMenu
//
/////////////////////////////////////////////////////////////////////
void APContextMenu()
{
  // Local data
  dyn_string txt;
  int Answer;
  int status;
  bool bOK;				//Variable with value FALSE
  	
  dpGet($datapoint + "_Board1_AP"+ $APNr + ".status:_original.._value", status);
  BuildContextMenu(status, -1, Answer);

  ///////////////////////////////////////////////////////////
  //
  //	Compute the chosen option
  //
  ///////////////////////////////////////////////////////////
  	switch (Answer)
  	{
  	case 2:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +".status:_original.._value", 0);
  			break;
   	case 3:
			  dpSetWait($datapoint + "_Board1_AP"+ $APNr +".status:_original.._value", 1);
  			break;
    default:
    	break;
  	}       
}


/////////////////////////////////////////////////////////////////////
//
// Function: Retrieves the Name of the subview config in the RTDB
// FunctionName: getSubviewName
//
/////////////////////////////////////////////////////////////////////
void getSubviewName(string &subViewName)
{
  string text ="";
  string bestandsnaam;
  dyn_string split;
  
  int i;  
  split=strsplit($configDatapoint, "_");
  for (i=7; i<=dynlen(split) ; i++)
  {
    if (i>7)
      text = text + "_" + split[i];
    else
      text = text + split[i];
  }
//	subViewName = "System1:__nav_subview_Trend_" + text;
    subViewName = text;
//  	DebugN("subViewName inside: "+ subViewName);
//	DebugN("s$configDatapoint: "+ $configDatapoint);
}


//////////////////////////////////////////////////////////////////////////////////
//
// FunctionName: ProgressBar
// Function    : display
// 
///////////////////////////////////////////////////////////////////////////////////
void ProgressBar(float Maximum, float value)
{
  float Minimum = 0;
  int waarde;
  float positie;
	if (value>Minimum)
	{
    setValue("bar", "visible", TRUE);
    setValue("bar_border", "visible", TRUE);
  }
	
  setValue("bar", "scale", value/Maximum, 1.0);
  
  if (Maximum==value)
  {
    delay(0,200);
    setValue("bar", "visible", FALSE);
    setValue("bar_border", "visible", FALSE);
  }
}
