//# gcfnav-view.ctl
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//#
//# global functions for the views of the Navigator.
//#

#uses "nav_fw/gcf-logging.ctl"

global string CURRENT_DP_MESSAGE = "Current selection in tree";
global dyn_string g_ContectMenuDpAccessableErrorText = "PUSH_BUTTON, No actions possible, 2, 0";


///////////////////////////////////////////////////////////////////////////
//
// Function navViewPlotGetRecordStatus(configDP, DP) : statusStr
//
// This functions checks for the current selected view, the status of the 
// recording..
//           options: 1. "recording"; for these dp's a recording is running
//                    2. "idle"; for these dp's NO recording is running
//                    3. "unknown"; the status is unknown or the record
//                       control manager can no be reached.
//                                            
// Input:  1. $configDatapoint
//         2. $datapoint
// Output: 1. record status of the specific $configDatapoint and $datapoint
//
///////////////////////////////////////////////////////////////////////////
string navViewPlotGetRecordStatus(string configDatapoint, string datapoint) {
	LOG_DEBUG("navViewPlotGetRecordStatus: ",configDatapoint,datapoint);

	string 		recordStatus, messageRCV;
	dyn_string	messageRCVsplit;
	bool 		messageReceived = FALSE;
	int 		retryCounter = 0;

	dpSet("__navigator.recordRCV", "");
	dpSet("__navigator.recordCMD", configDatapoint + "|" + datapoint + "|status");
	while(!messageReceived && (retryCounter<=80)) {
		dpGet("__navigator.recordRCV", messageRCV);
		if (patternMatch(configDatapoint + "|" + datapoint + "|*", messageRCV)) {
			messageReceived = TRUE;
			messageRCVsplit = strsplit(messageRCV, "|");
		}
		delay(0,0.050);
		retryCounter++;
	}
	if (dynlen(messageRCVsplit)>=3) {
		return messageRCVsplit[3];
	}
	else {
		return "unknown";
	}
}


///////////////////////////////////////////////////////////////////////////
//
// Function trendApplySettings()
//
// This functions checks for the
//
// Input : All elements of the config screen
// Output: Only the changed settings are stored into the PVSS database
//
///////////////////////////////////////////////////////////////////////////
void TrendConfigApplyTraceSettings() {
  	LOG_DEBUG("TrendConfigApplyTraceSettings");
  	int ScaleMin, ScaleMax;
  	bool AutoScale;
  	string dpName, dpeName, legendName;

  	//Set the values in the graphical objects
  	getValue("txt_legendName", "text", legendName);
  	getValue("txt_scaleMax", "text", ScaleMax);
  	getValue("txt_scaleMin", "text", ScaleMin);
  	getValue("btn_AutoScale", "state", 0, AutoScale);
  	getValue("list_dp",  "selectedText", dpName);
  	getValue("list_dpe", "selectedText", dpeName);

  	//Get the values from the PVSS database
  	dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".AutoScale", AutoScale);
  	dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".dpName", dpName+"."+dpeName);
  	dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".LegendName", legendName);

  	if (ScaleMin<ScaleMax) {
    	dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMax", ScaleMax);
    	dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMin", ScaleMin);
  	}
  	else {
    	setValue("txt_scaleMax", "text", ScaleMax + 10);
    	dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMax", ScaleMin+10);
    	dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMin", ScaleMin);
  	}
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewGetDpFromDpSelectionList($DP) : DPname
//
// Retrieves the correct dpName from the dpSelection
//
// Input:  1. list_system.selectedText
//         2. list_dp.selectedText
// Output: 1. datapoint name
//
///////////////////////////////////////////////////////////////////////////
string navViewGetDpFromDpSelectionList(string dollarDatapoint) {
  	LOG_DEBUG("navViewGetDpFromDpSelectionList: ",dollarDatapoint);
	
  	string datapoint;
  	if(list_system.selectedText==getSystemName()) {
  	  	if(CURRENT_DP_MESSAGE==list_dp.selectedText) {
      		datapoint = dollarDatapoint;
    	}
   	 	else {
      		datapoint = dollarDatapoint + "_" + list_dp.selectedText;
    	}
  	}
  	else {
    	datapoint = list_system.selectedText + list_dp.selectedText;
  	}
  	return datapoint;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewPlotChangeState()
//
// Input : 1. PLOT_STATE (global variable in panel) indication or the plot
//            is pauzed or running it's sequence.
// Output: 1. PLOT_STATE == "PLAY", icon_play visible
//         2. PLOT_STATE == "PAUSE", icon_play visible
//         3. PLOT_STATE with record is handled seperatly
///////////////////////////////////////////////////////////////////////////
void navViewPlotChangeState() {
  	LOG_DEBUG("navViewPlotChangeState");
  	if("PLAY"==PLOT_STATE || "STEP"==PLOT_STATE) {
    	setValue("icon_play",   "visible", TRUE);
    	setValue("icon_pause1", "visible", FALSE);
    	setValue("icon_pause2", "visible", FALSE);
  	}
  	else if ("PAUSE"==PLOT_STATE) {
    	setValue("icon_play",   "visible", FALSE);
    	setValue("icon_pause1", "visible", TRUE);
    	setValue("icon_pause2", "visible", TRUE);
  	}
  	else {
    	setValue("icon_play",   "visible", FALSE);
    	setValue("icon_pause1", "visible", FALSE);
    	setValue("icon_pause2", "visible", FALSE);
  	}
  
  	if("RECORDSTART"==RECORD_STATE) {
    	setValue("icon_record",   "visible", TRUE);
  	}
  	else if ("RECORDSTOP"==RECORD_STATE) {
    	setValue("icon_record",   "visible", FALSE);
  	}
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewPlotWhenActive(DP1, active, DP2, dpname)
//
// Input : 1. state or the specific plot is active or not
// Output: if active==1; dpConnect datapoint.
//         if active!=1; dpDisconnect datapoint.
//
///////////////////////////////////////////////////////////////////////////
void navViewPlotWhenActive(string dp1, int active,
                           string dp2, string dpName) {
  	LOG_DEBUG("navViewPlotWhenActive: ",dp1, active, dp2, dpName);
  	dyn_string dp1Split = strsplit(dp1, ".");
  	if (active==1) {
    	string dpToConnect = navViewRetrieveDpName($datapoint, dpName);
    	dpConnect("navViewPlotMainPlotSequence" + dp1Split[2], dpToConnect);
  	}
  	else {
      	dpDisconnect("navViewPlotMainPlotSequence" + dp1Split[2], $datapoint+dpName);
  	}
  
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewRetrieveDpName ($DP, DPname) : DPconnectName
//
// The configured datapoint can be relative or can be absolute. This function 
// returns a absolute path, so it can be used for further processing in the 
// panels .
//
// Input : 1. configured datapoint dpName
//         2. $datapoint
// Output: if active==1; dpConnect datapoint.
//         if active!=1; dpDisconnect datapoint.
//
///////////////////////////////////////////////////////////////////////////
string navViewRetrieveDpName(string dollarDatapoint, string dpName) {
  	LOG_DEBUG("navViewRetrieveDpName: ",dollarDatapoint, dpName);
  	string systemName = dpSubStr(dpName, DPSUB_SYS);
  	string dpToConnect;
  	if(""!=systemName) {
    	dpToConnect = dpName;
  	}
  	else {
    	dpToConnect = dollarDatapoint + dpName;
  	}
  	return dpToConnect;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewPlotApplyChanges()
//
// Returns the configured values to the program, where the changes will be 
// processed
//
// Input: 1. all settings
//
// Output: all settings
//
///////////////////////////////////////////////////////////////////////////
void navViewPlotApplyChanges() {
  	LOG_DEBUG("navViewPlotApplyChanges");
  	dyn_string returnValues;
  	returnValues[1] = setting_xrangeMin.text;
  	returnValues[2] = setting_xrangeMax.text;
  	returnValues[3] = setting_yrangeMin.text;
  	returnValues[4] = setting_yrangeMax.text;
  	returnValues[5] = setting_updateRate.text;
  	dpSet("_Ui_"+myManNum()+ ".ReturnValue.Text:_original.._value", returnValues);
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewPlotConvertGnuplotOutput(plotNr) : gnuplot-filenr
//
// Converts the GNUPlot outputfile to a format useable by PVSS.
//
// Input: 1. plotNumber: which plot is currently processed
//
// Output: file gnuplot<plotNumber>.bmp
//
///////////////////////////////////////////////////////////////////////////
int navViewPlotConvertGnuplotOutput(int plotNumber) {
  	LOG_DEBUG("navViewPlotConvertGnuplotOutput: ",plotNumber);

  	if ((access(navConfigGetPathName(g_path_temp) + navConfigGetPathName("/") + "gnuplot" + plotNumber+ ".png", R_OK)==0) &&
        (access(navConfigGetPathName(g_path_pictureconverter) + navConfigGetPathName("/") + "i_view32.exe", F_OK)==0)) {
    	//system("start /b rm " + navConfigGetPathName(g_path_temp) + navConfigGetPathName("/") + +"gnuplot"+plotNumber+ ".png");
    	system(navConfigGetPathName(g_path_pictureconverter) + navConfigGetPathName("/") + "i_view32.exe " + navConfigGetPathName(g_path_temp) + navConfigGetPathName("/") + "gnuplot" + plotNumber + ".png /convert=" + navConfigGetPathName(PROJ_PATH) + "pictures" + navConfigGetPathName("/") + "gnuplot" + plotNumber+ ".bmp");
  	}
  	else {
    	system("start /b copy " + navConfigGetPathName(PROJ_PATH) + "pictures" +navConfigGetPathName("/") + "gnuplot_error.bmp " + navConfigGetPathName(PROJ_PATH) + "pictures" + navConfigGetPathName("/") + "gnuplot" + plotNumber+ ".bmp");
  	}
  	return 0;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewPlotGenerateGnuPlotScriptFile(...) gnuplot-filenumber
//
// Generates a script file, nessacery for useing wgnuplot
//
// Input: 1. plotNumber: which plot is currently processed
//        2. Title to be displayed in the plot 
//
// Output: file gnuplot<plotNumber>.dem, script for wgnuplot.exe
//
///////////////////////////////////////////////////////////////////////////
int navViewPlotGenerateGnuPlotScriptFile(int plotNumber, string plotTitle,
                                         int xRangeMin, int xRangeMax,
                                         int yRangeMin, int yRangeMax) {
  	LOG_DEBUG("navViewPlotGenerateGnuPlotScriptFile: ",plotNumber, plotTitle, xRangeMin, xRangeMax, yRangeMin, yRangeMax);
	
  	int blue=2, green=4, yellow=6, red=8, timeslot_size=1;
  	time t=getCurrentTime();
  	int subbandLength = 511; //starting from zero (0-511)
  	string current_time;
  	string scriptFilename;
  	string script_content, spectrum_data, test, font_size=1;
  	file f, fSpectrum ;
  	string data_file = "output" + plotNumber + ".dat";
  	current_time = t;

  	// Create and fill script file
  	f = fopen(navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot" + plotNumber+ ".dem", "w+");
  	fputs ("#[---------------- Generated by PVSS II ----------------]\n", f );
  	fputs ("#[ Date and time: " + current_time + "  \n", f );
  	fputs ("#[------------------------------------------------------]\n", f );
  	fputs ("set terminal png "+ font_size +" size 900,700\n" , f );
  	fputs ("set output \"C:\\\\temp\\\\gnuplot" + plotNumber+ ".png\"\n" , f );
  	fputs ("set title \"Plot time: "+current_time+"\" \n", f );
  	fputs ("show title\n", f );
  	//if(xRangeMin!=0 && xRangeMax!=0)
  	if(!(0==xRangeMin && 0==xRangeMax)) {
    	fputs ("set xrange["+xRangeMin+":"+xRangeMax+"] \n", f );
  	}
  	//if(yRangeMin!=0 && yRangeMax!=0)
  	if(!(0==yRangeMin && 0==yRangeMax)) {
    	fputs ("set yrange["+yRangeMin+":"+yRangeMax+"] \n", f );
  	}
  	fputs ("#[------------------ End of PVSS part ------------------]\n", f );
  	fclose(f);

  
  	dpGet($configDatapoint + ".scriptFilename", scriptFilename);
  	scriptFilename = navConfigGetPathName(scriptFilename);

  	//If the scriptFilename is configured and existing, combine both script files,
  	// otherwise generated dummy

  	if(scriptFilename!="" && access(scriptFilename, F_OK)==0) {
    	system("type " + scriptFilename + " >> " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot" + plotNumber+ ".dem");
  	}
  	else {
    	f = fopen(navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot.dem", "w+");
    	fputs ("#[---------------- Generated by PVSS II ----------------]\n", f );
    	fputs ("#[ No scriptfile configured \n", f );
    	fputs ("#[------------------------------------------------------]\n", f );
    	fputs ("set yrange [-10:10]\n", f);
    	fputs ("set xrange [0:10]\n", f);
    	string messageText;
    	if(scriptFilename=="") {
      		messageText = "Script file is not configured";
    	}
    	else if(access(scriptFilename, F_OK)==-1) {
      		messageText = "Configured script file can not be found";
    	}
    	else {
      		messageText = "Unknown error with scriptfile";
    	}
    	fputs ("plot sin(1) title \""+messageText+"\" with lines\n" , f );
    	fclose(f);
    	system("type "+navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot.dem >> " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot" + plotNumber+ ".dem");
  	}
  	return 0;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewPlotGenerateGnuplotOutput (plotNr) : gnuplot filenumber
//
// Generates a wgnuplot output picture
//
// Input: 1. plotNumber: which plot is currently processed
//
// Output: file gnuplot<plotNumber>.png, gnuplot output picture
//
///////////////////////////////////////////////////////////////////////////
int navViewPlotGenerateGnuplotOutput(int plotNumber) {
    LOG_DEBUG("navViewPlotGenerateGnuplotOutput: ",plotNumber);
    string wgnuplotPath   = navConfigGetPathName(g_path_gnuplot) + navConfigGetPathName("/") + "wgnuplot.exe";
    string plotConfigPath = navConfigGetPathName(g_path_temp) + navConfigGetPathName("/") + "gnuplot" + plotNumber+ ".dem";
    string plotPath       = navConfigGetPathName(g_path_temp) + navConfigGetPathName("/") + "gnuplot" + plotNumber+ ".png";
    if ((access(wgnuplotPath, F_OK)==0) && (access(plotConfigPath, F_OK)==0)) {
      system("start /b rm " + plotPath);
      system(wgnuplotPath + " " + plotConfigPath);
      if (access(plotPath, F_OK)!=0) {
        LOG_ERROR("Error accessing plot file",plotPath);
        return -1;                
      }
    }
    else {
      LOG_ERROR("Error accessing wgnuplot and plot configuration",wgnuplotPath,plotConfigPath);
      return -1;
    }

  return 0;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewAlertGetDpFromColumn(row,column)
//
// Gets the selectedDP .
//                         
// Input: 1. Row
//        2. Column
//
// Output: stores the current configuration of the cell into the PVSS II
//         database
//
///////////////////////////////////////////////////////////////////////////
void navViewAlertApplyCellSettings(string RowNumber, string Column) {
  	LOG_DEBUG("navViewAlertApplyCellSettings: ",RowNumber, Column);
  	dyn_string ColumndpNames, ColumnTitles;
 	
  	dpGet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "dpNames", ColumndpNames); 
  	dpGet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "Titles", ColumnTitles);
	
  	ColumnTitles[ConvIndex($Row)] = txt_legend.text;
  	ColumndpNames[ConvIndex($Row)] = list_dp.selectedText + "." + list_dpe.selectedText;
	
  	dpSet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "dpNames", ColumndpNames); 
  	dpSet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "Titles", ColumnTitles);
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewControlDisplayText(DP, text2display)
//
// Displays the text: textToDisplay
//                                     
// Input: text to display
// Output: output is written to the controllerLog
//
///////////////////////////////////////////////////////////////////////////
void navViewControlDisplayText(string dp1, string textToDisplay) {
  	LOG_DEBUG("navViewControlDisplayText: ", dp1, textToDisplay);
  	string timeToDisplay = getCurrentTime();
  	string elementName = dpGetElementName(dp1);
  	strreplace(elementName, ".", "");
  	setValue("txt_" + elementName, "text", textToDisplay);
  	controllerLog.appendItem = timeToDisplay + " | " + elementName + ": " + textToDisplay;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewControlDisplayStatus(DP, statusNr)
//
// Writes the statusname of the given statusnr to the controllerlog.
//                                     
// Input: text to display
// Output: output is written to the controllerLog
//
///////////////////////////////////////////////////////////////////////////
navViewControlDisplayStatus(string dp1, int statusNumber) {
  	LOG_DEBUG("navViewControlDisplayStatus: ", dp1, statusNumber);
  	string timeToDisplay = getCurrentTime();
  	string elementName = dpGetElementName(dp1);
  	string textToDisplay;
  	strreplace(elementName, ".", "");
  	if(statusNumber==0)
    	textToDisplay = "Ok";
  	else if(statusNumber==1)
    	textToDisplay = "Unspecified";
  	else if(statusNumber==2)
    	textToDisplay = "VI not found";
  	else if(statusNumber==3)
    	textToDisplay = "Parameters not found";
  	else if(statusNumber==4)
    	textToDisplay = "Incorrect number of parameters";
  	else if(statusNumber==5)
    	textToDisplay = "Unknown command";
  	else if(statusNumber==6)
    	textToDisplay = "Disabled";
  	else if(statusNumber==7)
  	  	textToDisplay = "Low quality";
  	else if(statusNumber==8)
    	textToDisplay = "Timing failure";
  	else if(statusNumber==9)
    	textToDisplay = "Wrong version";
  	else if(statusNumber==10)
    	textToDisplay = "Unsupported Logical Device";
  	else if(statusNumber==11)
    	textToDisplay = "Logical Device not found";
  	else if(statusNumber==12)
    	textToDisplay = "Wrong state";
  	else if(statusNumber==13)
    	textToDisplay = "Shutdown ";
  	else if(statusNumber==14)
    	textToDisplay = "Already exists";
  	else
    	textToDisplay = "Unknown";

  	setValue("txt_status", "text", textToDisplay);
  	controllerLog.appendItem = timeToDisplay + " | " + elementName + ": " + textToDisplay;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewControlDisplayTime(DP, serialTime)
//
// Converts the serialtime to the a  normal, readable time notation
//
// Input: serialTime
// Output: output is written to the controllerLog
//
///////////////////////////////////////////////////////////////////////////
void navViewControlDisplayTime(string dp1, int serialTime) {
  	LOG_DEBUG("navViewControlDisplayTime: ", dp1, serialTime);
  	dyn_string dpPlit = strsplit(dp1, ".");
  	string timeToDisplay = getCurrentTime();
  	string elementName = dpGetElementName(dp1);
  	strreplace(elementName, ".", "");
  	time t;
  	setPeriod(t, serialTime); 
  	controllerLog.appendItem = timeToDisplay + " | " + elementName + ": " + formatTime("%H:%M:%S %d-%m-%Y", t);
  	setValue("txt_"+elementName, "text", formatTime("%H:%M:%S %d-%m-%Y", t));

}


///////////////////////////////////////////////////////////////////////////
//
// Function navViewAlertGetDpFromColumn(row, column) : celContents
//
// Gets the selectedDP 
//                         
// Input: 1. Row
//        2. Column
//
// Output: string with the current selectedDP for the current cell,
//         can also be empty ("")
//
///////////////////////////////////////////////////////////////////////////
string navViewAlertGetDpFromColumn(int Row,int Column) {
  	LOG_DEBUG("navViewAlertGetDpFromColumn: ", Row, Column);
  	dyn_string ColumndpNames;
  	string connectDatapoint;

  	getValue("", "currentCell", Row, Column);
  	dpGet($configDatapoint + "." + $AreaNr + ".Column" + Column + "dpNames", ColumndpNames); 
  	if (dynlen(ColumndpNames)>0) {
    	if ((0 != ColumndpNames[ConvIndex(Row)]) && ("" != ColumndpNames[ConvIndex(Row)])) {
      		connectDatapoint = ColumndpNames[ConvIndex(Row)];
      		return connectDatapoint;
    	}
    	else {
      		return "";
    	}
  	}
  	return "";
}

//////////////////////////////////////////////////////////////////////////////////
//
// Function getTablePropertyNames(query, amount, dyn &propnames)
//
//  
///////////////////////////////////////////////////////////////////////////////////
void getTablePropertyNames(string newSelectionQuery, int newSelectionAmount, 
					  dyn_string &propNames) {
  	LOG_DEBUG("getTablePropertyNames: ", newSelectionQuery, newSelectionAmount, propNames);
  	dyn_dyn_anytype tab;

  	dpQuery("SELECT '_online.._value' FROM '" + newSelectionQuery + "' FIRST " + newSelectionAmount, tab);
  	for(int i=2 ; i<=dynlen(tab) ; i++) {
    	string temp =tab[i][1];
    	temp = substr(temp, (strlen($datapoint)+1), strlen(temp));
    	TableProperties.appendLine("propertyName", temp ,"value", "");
    	if(((dynlen(strsplit(temp, "_")))==1) && ((dynlen(strsplit(temp, ".")))==1)) {
      		propNames[i-1] = $datapoint + "." + temp + ":_online.._value";
    	}
    	else {
      		propNames[i-1] = $datapoint + "_" + temp + ":_online.._value";
    	}
  	}
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function clearListTable()
//
//  Disconnects the List table and clears the content
//
///////////////////////////////////////////////////////////////////////////////////
void clearListTable() {
  	LOG_DEBUG("clearListTable");
  	dyn_string propNames;
  	dpGet($configDatapoint + ".queryResult", propNames);
  	for (int i = 1; i <= dynlen(propNames); i++) {
    	dpDisconnect("connectActualValue", propNames[i]);
  	}
  	TableProperties.deleteAllLines;
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function connectTable(propNames)
//
// Do a dpConnect on all given DPs and tell the PA to load them.
//
///////////////////////////////////////////////////////////////////////////////////
connectTable(dyn_string propNames) {
  	LOG_DEBUG("connectTable: ", propNames);
  	dpSet($configDatapoint + ".queryResult", propNames);
  	for(int i = 1; i <= dynlen(propNames); i++) {
    	dpConnect("connectActualValue", propNames[i]);
  	}
}


//////////////////////////////////////////////////////////////////////////////////
//
// EventHandler connectActualValue(DP, value)
//
//  
///////////////////////////////////////////////////////////////////////////////////
connectActualValue(string dp1, anytype actualValue) {
  	LOG_DEBUG("connectActualValue: ", dp1, actualValue);
  	dyn_string queryResult; 
  	anytype valueString;
  	dpGet($configDatapoint + ".queryResult", queryResult);
  	int rowNr = dynContains(queryResult, dpSubStr(dp1, DPSUB_SYS_DP_EL)) - 1;
  	if (elementTypeValid(dp1)) { 
    	if (dpElementType(dp1) == DPEL_FLOAT) {
      		int floatDecimals;
      		dpGet($configDatapoint + ".floatDecimals", floatDecimals);
      		if (floatDecimals <= 0)
        		floatDecimals = 1;
      		string newActualValue;
      		sprintf(newActualValue, "%." + floatDecimals + "f", actualValue);
      		valueString = newActualValue;
    	}
    	else {
      		valueString = actualValue;
    	}
    	TableProperties.cellValueRC(rowNr, "value", valueString);
  	}
  	else {
    	TableProperties.cellValueRC(rowNr, "value", "Cannot display complex type");
    	TableProperties.cellBackColRC(rowNr, "value", "_3DFace");
  	}
  	TableProperties.cellValueRC(rowNr, "unit", dpGetUnit(dp1));
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function elementTypeValid(element) : bool
//
// returns TRUE is element is one of these types 
//
///////////////////////////////////////////////////////////////////////////////////
bool elementTypeValid(string element) {
  	LOG_DEBUG("elementTypeValid: ", element);
  	if ((DPEL_CHAR == dpElementType(element)) || (DPEL_UINT   == dpElementType(element)) ||
        (DPEL_INT  == dpElementType(element)) || (DPEL_FLOAT  == dpElementType(element)) ||
        (DPEL_BOOL == dpElementType(element)) || (DPEL_STRING == dpElementType(element))) {
    	return TRUE;
  	}

  	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function setTableColumnConfig(TblNr, TblNr2Displ, #cols, MaxCols, option)
//
// Set the width of the configured amount of columns, otherwise they will be set 
// to invisible.
//
// Input: 1. Table/Area number for which this function is executed
//        2. TableNumber in which the grahpical updates must take place.
//           The view has three different tables, the config one, but content is
//           always for another $AreaNr
//        3. Number of columns
//        4. Maximum number of columns hard configured in the table object
//        5. Which function to connect dp's must be used.
//           option 1. ""       => use function ArrangeTableContent
//           option 2. "Config" => use function ArrangeConfigTableContent
// Output: 1. Table
//
///////////////////////////////////////////////////////////////////////////////////
void setTableColumnConfig(int TableNumber, int TableNumberToDisplay, int nrOfColumns, int MaximumColumns, string Addition) {
  	LOG_DEBUG("etColumnConfig: ", TableNumber, TableNumberToDisplay, nrOfColumns, MaximumColumns, Addition);

  	int k, table_x, table_y;
	
  	getValue("Table"+TableNumberToDisplay, "size", table_x, table_y);
  	for (k=0; k<nrOfColumns ; k++) {
    	dpConnect("Arrange" + Addition + "TableContent", $configDatapoint + "."+TableNumber+".Column" + k + "Titles:_online.._value",
                   $configDatapoint + "."+TableNumber+".Column" + k + "dpNames:_online.._value");
    	setValue("Table"+TableNumberToDisplay, "columnVisibility", k, TRUE);
    	setValue("Table"+TableNumberToDisplay, "columnWidth", k, (table_x/nrOfColumns));
  	}
  	for (k=nrOfColumns ; k <= MaximumColumns ; k++) {
    	setValue("Table"+TableNumberToDisplay, "columnVisibility", k, FALSE);
  	}
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function ConvIndex(int) : int+1
//
// Input: i [integer]
// Ouput: input i increased with 1.
//
//////////////////////////////////////////////////////////////////////////////////
int ConvIndex(int i) {
  	return i+1;
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function getTableNumber(DP, &TblNr)
//
// Retrieve the number of the current table
//
///////////////////////////////////////////////////////////////////////////////////
void getTableNumber(string dpName, int &TableNumber) {
  	LOG_DEBUG("getTableNumber: ", dpName, TableNumber);
  	dyn_string resultaat;
  	resultaat = strsplit(dpName, ".");
  	TableNumber = resultaat[2];
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function getColumnTitle(DP, &columnNr)
//
// Retrieve the Name of the current column
//
///////////////////////////////////////////////////////////////////////////////////
void getColumnTitle(string dpName, string &ColumnTitle) {
  	LOG_DEBUG("getColumnTitle: ", dpName, ColumnTitle);
  	dyn_string resultaat;
  	resultaat = strsplit(dpName, ".");
  	ColumnTitle = strrtrim(resultaat[3], "dpNames:_online");
}




/////////////////////////////////////////////////////////////////////
//
// Function getTraceNumber(DP, &traceNr)
//
// Retrieve the trace number from the dpName
//
/////////////////////////////////////////////////////////////////////
void getTraceNumber(string dpName, string &TraceNumber) {
  	LOG_DEBUG("getTraceNumber: ", dpName, TraceNumber);
  	string HulpString;
  	dyn_string split;
  	dyn_string HulpSplit;
  
  	split       = strsplit(dpName, ".");
  	HulpSplit   = strsplit(split[3], "race");
  	TraceNumber = HulpSplit[dynlen(HulpSplit)];
}

/////////////////////////////////////////////////////////////////////
//
// Function getTrendNumber(DP, &trendNr)
//
// Retrieves the TrendNumber
//
/////////////////////////////////////////////////////////////////////
void getTrendNumber(string dp1, int &TrendNumber) {
  	LOG_DEBUG("getTrendNumber: ", dp1, TrendNumber);
  	dyn_string split, split2, split3;
  	dyn_string HulpSplit;
  
  	split       = strsplit(dp1, ".");
  	TrendNumber = split[2];
}

/////////////////////////////////////////////////////////////////////
//
// Function ConnectTrace(traceNr)
//
// Connects an single element trace to trend
//
/////////////////////////////////////////////////////////////////////
void ConnectTrace(int TraceNumber) {
  	LOG_DEBUG("ConnectTrace: ", TraceNumber);
  	dpConnect("TracePlot", "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".dpName",
                           "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".AutoScale",
                           "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".ScaleMin",
                           "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".ScaleMax",
                           "__nav_SingleElement_Trend.1.Trace" + TraceNumber + ".LegendName");
}


/////////////////////////////////////////////////////////////////////
//
// Function getSubviewName(&subviewName)
//
// Retrieves the Name of the subview config in the RTDB
//
/////////////////////////////////////////////////////////////////////
void getSubviewName(string &subViewName) {
  	LOG_DEBUG("getSubviewName: ", subViewName);
  	string text ="";
  	string bestandsnaam;
  	dyn_string split;
  
  	int i;  
  	split=strsplit($configDatapoint, "_");
  	for (i=7; i<=dynlen(split) ; i++) { // REO why 7 ???
    	if (i>7) {
      		text = text + "_" + split[i];
    	} 
    	else {
      		text = text + split[i];
      	}
  	}
  	subViewName = text;
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function NavConfigTrendFillDpeSelection(DP)
//
// Fills the dpe selectionlist for a datapoint selection
//
///////////////////////////////////////////////////////////////////////////////////
void NavConfigTrendFillDpeSelection(string datapoint) {
  	LOG_DEBUG("NavConfigTrendFillDpeSelection: ",datapoint);
  	string selectedDP = list_dp.selectedText;
  	list_dpe.deleteAllItems; 
  	if (selectedDP != "")
    	selectedDP = "_" + selectedDP;
  
  	list_dpe.items = getElementsFromDp(datapoint, TRUE); // TRUE == without references, refs are not allowed in the list
  	list_dpe.selectedPos = 1;
}
