//# gcfnav_view.ctl
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


global string CURRENT_DP_MESSAGE = "Current selection in tree";
global dyn_string g_ContectMenuDpAccessableErrorText = "PUSH_BUTTON, No actions possible, 2, 0";

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: trendApplySettings
//
//  
///////////////////////////////////////////////////////////////////////////////////
void TrendConfigApplyTraceSettings()
{
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

  if (ScaleMin<ScaleMax)
  {
    dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMax", ScaleMax);
	  dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMin", ScaleMin);
	}
	else
  {
    setValue("txt_scaleMax", "text", ScaleMax + 10);
    dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMax", ScaleMin+10);
	  dpSet($configDatapoint + "." + $TrendNr + ".Trace" + $TraceNr + ".ScaleMin", ScaleMin);
  }
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewGetDpFromDpSelectionList; retrieves the correct dpName
//                                            from the dpSelection
// Input:  1. list_system.selectedText
//         2. list_dp.selectedText
// Output: 1. datapoint name
///////////////////////////////////////////////////////////////////////////
string navViewGetDpFromDpSelectionList(string dollarDatapoint)
{
  string datapoint;
  if(list_system.selectedText==getSystemName())
  {
    if(CURRENT_DP_MESSAGE==list_dp.selectedText)
    {
      datapoint = dollarDatapoint;
    }
    else
    {
      datapoint = dollarDatapoint + "_" + list_dp.selectedText;
    }
  }
  else
  {
    datapoint = list_system.selectedText + list_dp.selectedText;
  }
  return datapoint;
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence3, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence3(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 3);
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotChangeState
//
// Input : 1. PLOT_STATE (global variable in panel) indication or the plot
//            is pauzed or running it's sequence.
// Output: 1. PLOT_STATE == "PLAY", icon_play visible
//         2. PLOT_STATE == "PAUSE", icon_play visible
///////////////////////////////////////////////////////////////////////////
void navViewPlotChangeState()
{
  if("PLAY"==PLOT_STATE || "STEP"==PLOT_STATE)
  {
    setValue("icon_play",   "visible", TRUE);
    setValue("icon_pause1", "visible", FALSE);
    setValue("icon_pause2", "visible", FALSE);
  }
  else if ("PAUSE"==PLOT_STATE)
  {
    setValue("icon_play",   "visible", FALSE);
    setValue("icon_pause1", "visible", TRUE);
    setValue("icon_pause2", "visible", TRUE);
  }
  else
  {
    setValue("icon_play",   "visible", FALSE);
    setValue("icon_pause1", "visible", FALSE);
    setValue("icon_pause2", "visible", FALSE);
  }
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotWhenActive
//
// Input : 1. state or the specific plot is active or not
// Output: if active==1; dpConnect datapoint.
//         if active!=1; dpDisconnect datapoint.
///////////////////////////////////////////////////////////////////////////
navViewPlotWhenActive(string dp1, int active,
                      string dp2, string dpName)
{
  dyn_string dp1Split = strsplit(dp1, ".");
 // string dpName;
//  dpGet($configDatapoint + "." + dp1Split[2] + ".dpName", dpName);
  if(1==active)
  {
    // if(PLOT_DPNAMES[dp1Split[2]]!=$datapoint+dpName)
    // dpDisconnect("navViewPlotMainPlotSequence" + dp1Split[2], PLOT_DPNAMES[dp1Split[2]]); 
    string dpToConnect = navViewRetrieveDpName($datapoint, dpName);
    navPMLloadPropertySet(dpToConnect);
    dpConnect("navViewPlotMainPlotSequence" + dp1Split[2], dpToConnect);
//    PLOT_DPNAMES[dp1Split[2]]=$datapoint+dpName;
  }
  else
  {
//    if(PLOT_DPNAMES[dp1Split[2]]==$datapoint+dpName)
      dpDisconnect("navViewPlotMainPlotSequence" + dp1Split[2], $datapoint+dpName);
  }
  
}

string navViewRetrieveDpName(string dollarDatapoint, string dpName)
{
  string systemName = dpSubStr(dpName, DPSUB_SYS);
  string dpToConnect;
  if(""!=systemName)
  {
    dpToConnect = dpName;
  }
  else
  {
    dpToConnect = $datapoint + dpName;
  }
  return dpToConnect;
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlot
//
// Input : variable off all the plots, whether they are active or not.
///////////////////////////////////////////////////////////////////////////
navViewPlotArrangePlots(string dp1, int dp1Active,
                        string dp2, int dp2Active,
                        string dp3, int dp3Active,
                        string dp4, int dp4Active,
                        string dp5, int dp5Active,
                        string dp6, int dp6Active,
                        string dp7, int dp7Active,
                        string dp8, int dp8Active)
{
  int panelSizeX=930;
  int panelSizeY=730;
  int totalActive = dp1Active + dp2Active + dp3Active + dp4Active;
  
  setValue("plot_1", "visible", dp1Active );
  setValue("plot_2", "visible", dp2Active );
  setValue("plot_3", "visible", dp3Active );
  setValue("plot_4", "visible", dp4Active );

  //Make a list of all the active plots
  dyn_string activePlots;
  if(dp1Active==1)
    activePlots[dynlen(activePlots)+1] = 1;
  if(dp2Active==1)
    activePlots[dynlen(activePlots)+1] = 2;
  if(dp3Active==1)
    activePlots[dynlen(activePlots)+1] = 3;
  if(dp4Active==1)
    activePlots[dynlen(activePlots)+1] = 4;

  float divider = dynlen(activePlots);
  if(totalActive>=1)
  {
    for(int i=1; i<=dynlen(activePlots); i++)
    {
      if(dynlen(activePlots)<5)
      {
        setValue("plot_"+activePlots[i], "scale",  1.0, 1.0/divider);
        setValue("plot_"+activePlots[i], "position", 10, (10 + (i-1)*(panelSizeY/divider)));
      }
      else
      {
        setValue("plot_"+activePlots[i], "scale",  0.5, 0.25);
      }
    }
    getValue("plot_"+activePlots[1], "size", local_plotSizeX, local_plotSizeY);
  }
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlot
//
// Input : 1. datapoint name to plot
// Output: starts a specific plot, given as argument to the general
//         function
///////////////////////////////////////////////////////////////////////////
//navViewPlot(string dp1, string dpName)
//{
//  dyn_string dp1Split = strsplit(dp1, ".");
//  dpConnect("navViewPlotMainPlotSequence" + dp1Split[2], $datapoint+dpName);
//}



///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotApplyChanges   
//           Returns the configured values to the program, where the changes
//           will be processed
//
// Input: 1. all settings
//
// Output: all settings
///////////////////////////////////////////////////////////////////////////
void navViewPlotApplyChanges()
{
  dyn_string returnValues;
  returnValues[1] = setting_xrangeMin.text;
  returnValues[2] = setting_xrangeMax.text;
  returnValues[3] = setting_yrangeMin.text;
  returnValues[4] = setting_yrangeMax.text;
  returnValues[5] = setting_updateRate.text;
  dpSet("_Ui_"+myManNum()+ ".ReturnValue.Text:_original.._value", returnValues);
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotConvertGnuplotOutput   
//           Converts the GNUPlot outputfile to a format useable by PVSS
//
// Input: 1. plotNumber: which plot is currently processed
//
// Output: file gnuplot<plotNumber>.bmp
///////////////////////////////////////////////////////////////////////////
int navViewPlotConvertGnuplotOutput(int plotNumber)
{
    string iview_dir = "c:\\etm\\GNUplot\\iview\\";

    if ((access(TEMP_PATH + "gnuplot" + plotNumber+ ".png", R_OK)==0) &&
        (access(iview_dir + "i_view32.exe", F_OK)==0))
    {
      system("start /b rm " + PICTURE_PATH + +"gnuplot"+plotNumber+ ".bmp");
      system(iview_dir + "i_view32.exe " + TEMP_PATH + "gnuplot" + plotNumber + ".png /convert=" + PICTURE_PATH + "gnuplot" + plotNumber+ ".bmp");
    }
    else
    {
      system("start /b copy " + PICTURE_PATH + "gnuplot_error.bmp " + PICTURE_PATH + "gnuplot" + plotNumber+ ".bmp");
    }

return 0;
}



///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotGenerateGnuPlotScriptFile   
//           Generates a script file, nessacery for useing wgnuplot
//
// Input: 1. plotNumber: which plot is currently processed
//        2. Title to be displayed in the plot 
//
// Output: file gnuplot<plotNumber>.dem, script for wgnuplot.exe
///////////////////////////////////////////////////////////////////////////
int navViewPlotGenerateGnuPlotScriptFile(int plotNumber, string plotTitle,
                                         int xRangeMin, int xRangeMax,
                                         int yRangeMin, int yRangeMax)
{
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
  if(xRangeMin!=0 && xRangeMax!=0)
  {
    fputs ("set xrange["+xRangeMin+":"+xRangeMax+"] \n", f );
  }
  if(yRangeMin!=0 && yRangeMax!=0)
  {
    fputs ("set yrange["+yRangeMin+":"+yRangeMax+"] \n", f );
  }
  fputs ("#[------------------ End of PVSS part ------------------]\n", f );
  fclose(f);

  
  dpGet($configDatapoint + ".scriptFilename", scriptFilename);
  scriptFilename = navConfigGetPathName(scriptFilename);
  
  //    output = pathName + "//";
  //DebugN("scriptFilename:"+scriptFilename);
  //DebugN("TEMP_PATH:"+TEMP_PATH);
  //DebugN("navConfigGetPathName:"+navConfigGetPathName(g_path_temp));

  //If the scriptFilename is configured and existing, combine both script files,
  // otherwise generated dumme
  if(scriptFilename!="" && access(scriptFilename, F_OK)==0)
  {
    system("start /b type " + scriptFilename + " >> " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot" + plotNumber+ ".dem");
  }
  else
  {

    f = fopen(navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot.dem", "w+");
    fputs ("#[---------------- Generated by PVSS II ----------------]\n", f );
    fputs ("#[ No scriptfile configured \n", f );
    fputs ("#[------------------------------------------------------]\n", f );
    fputs ("set yrange [-10:10]\n", f);
    fputs ("set xrange [0:10]\n", f);
    string messageText;
    if(scriptFilename=="")
    {
      messageText = "Script file is not configured";
    }
    else if(access(scriptFilename, F_OK)==-1)
    {
      messageText = "Configured script file can not be found";
    }
    else
    {
      messageText = "Unknown error with scriptfile";
    }
    fputs ("plot sin(1) title \""+messageText+"\" with lines\n" , f );
    fclose(f);
    system("start /b type "+navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot.dem >> " + navConfigGetPathName(g_path_temp)+navConfigGetSlashes() + "gnuplot" + plotNumber+ ".dem");
  }
  return 0;
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotGenerateGnuplotOutput   
//           Generates a wgnuplot output picture
//
// Input: 1. plotNumber: which plot is currently processed
//
// Output: file gnuplot<plotNumber>.png, gnuplot output picture
///////////////////////////////////////////////////////////////////////////
int navViewPlotGenerateGnuplotOutput(int plotNumber)
{
    string default_dir = "c:\\etm\\GNUplot\\gnuplot\\bin\\";

    if ((access(GNUPLOT_PATH + "wgnuplot.exe", F_OK)==0) && 
        (access(TEMP_PATH + "gnuplot" + plotNumber+ ".dem", F_OK)==0))
    {
      system("start /b rm " + TEMP_PATH + "gnuplot" + plotNumber+ ".png");
      system(GNUPLOT_PATH + "wgnuplot.exe " + TEMP_PATH + "gnuplot" + plotNumber+ ".dem");
      if (access(TEMP_PATH + "gnuplot" + plotNumber + ".png", F_OK)!=0)
        return -1;                
    }
    else
    {
      return -1;
    }

  return 0;
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence4, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence4(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 4);
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence5, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence5(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 5);
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence6, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence6(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 6);
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence7, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence7(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 7);
}


///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence8, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence8(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 8);
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence1, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence1(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 1);
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence2, used to start a specific gnuplot
//                                         sequence
// Input: 1. datapoint name  and spectrum data
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence2(string dp1, string spectrum_data)
{
  navViewPlotMainPlotSequence(spectrum_data, 2);
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewPlotMainPlotSequence
//
//
// Input: 1. datapoint name to plot
// Output: 
///////////////////////////////////////////////////////////////////////////
navViewPlotMainPlotSequence(string spectrum_data, int plotNumber)
{	

	file fSpectrum;
	int timeslot_size = 1;
  string plotTitle;
	//write content of string to spectrum[DP_number].dat
	fSpectrum = fopen(TEMP_PATH +"spectrum" + plotNumber+ ".tmp", "w+");
	fputs (spectrum_data, fSpectrum );
	fclose(fSpectrum);

	if (timeslot_size<=1)
	{
		system("rm " + TEMP_PATH + "output" + plotNumber+ ".dat");
		system("copy " + TEMP_PATH + "output" + plotNumber+ ".tmp " + TEMP_PATH + "output" + plotNumber+ ".dat");
	}
	else
	{
		system("type " + TEMP_PATH + "output" + plotNumber+ ".tmp >> " + TEMP_PATH + "output" + plotNumber+ ".dat");
	}
	
  dpGet($configDatapoint + "." + plotNumber + ".title", plotTitle);
  string timeString = getCurrentTime();
  plotTitle = plotTitle + " ["+timeString+"]";
  
  navViewPlotGenerateGnuPlotScriptFile(plotNumber, plotTitle);
  navViewPlotGenerateGnuplotOutput(plotNumber);
  navViewPlotConvertGnuplotOutput(plotNumber);
  delay(1,0);
  setValue("plot_" + plotNumber+ "", "fill","[pattern,[tile,bmp,gnuplot" + plotNumber+ ".bmp]]");
}


///////////////////////////////////////////////////////////////////////////
// Function navViewAlertGetDpFromColumn: Gets the selectedDP 
//                         
// Input: 1. Row
//        2. Column
//
// Output: stores the current configuration of the cell into the PVSS II
//         database
///////////////////////////////////////////////////////////////////////////

navViewAlertApplyCellSettings(string RowNumber, string Column)
{
	dyn_string ColumndpNames, ColumnTitles;
 
  dpGet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "dpNames", ColumndpNames); 
  dpGet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "Titles", ColumnTitles);

  ColumnTitles[ConvIndex($Row)] = txt_legend.text;
  ColumndpNames[ConvIndex($Row)] = list_dp.selectedText + "." + list_dpe.selectedText;

  dpSet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "dpNames", ColumndpNames); 
  dpSet($configDatapoint + "." + $AreaNr + ".Column" + $Column + "Titles", ColumnTitles);
}


///////////////////////////////////////////////////////////////////////////
// Function navViewControlDisplayText: Displays the text: textToDisplay
//                                     
// Input: text to display
// Output: output is written to the controllerLog
///////////////////////////////////////////////////////////////////////////
navViewControlDisplayText(string dp1, string textToDisplay)
{
  string timeToDisplay = getCurrentTime();
  string elementName = dpGetElementName(dp1);
  strreplace(elementName, ".", "");
  setValue("txt_" + elementName, "text", textToDisplay);
  controllerLog.appendItem = timeToDisplay + " | " + elementName + ": " + textToDisplay;
}
///////////////////////////////////////////////////////////////////////////
// Function navViewControlDisplayStatus: Displays the text: textToDisplay
//                                     
// Input: text to display
// Output: output is written to the controllerLog
///////////////////////////////////////////////////////////////////////////
navViewControlDisplayStatus(string dp1, int statusNumber)
{
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
// Function navViewControlDisplayTime: converts the serialtime to the a  
//                                     normal, readable time notation
//
// Input: serialTime
// Output: output is written to the controllerLog
///////////////////////////////////////////////////////////////////////////
navViewControlDisplayTime(string dp1, int serialTime)
{
  dyn_string dpPlit = strsplit(dp1, ".");
  string timeToDisplay = getCurrentTime();
  string elementName = dpGetElementName(dp1);
  strreplace(elementName, ".", "");
  time t;
  setPeriod(t, serialTime); 
  //formatTime("%H:%M:%S %d-%m-%Y", t)
  controllerLog.appendItem = timeToDisplay + " | " + elementName + ": " + formatTime("%H:%M:%S %d-%m-%Y", t);
  setValue("txt_"+elementName, "text", formatTime("%H:%M:%S %d-%m-%Y", t));

}




///////////////////////////////////////////////////////////////////////////
// Function navViewAlertGetDpFromColumn: Gets the selectedDP 
//                         
// Input: 1. Row
//        2. Column
//
// Output: string with the current selectedDP for the current cell,
//         can also be empty ("")
///////////////////////////////////////////////////////////////////////////
string navViewAlertGetDpFromColumn(int Row,int Column)
{

  dyn_string ColumndpNames;
  string connectDatapoint;

  getValue("", "currentCell", Row, Column);
  dpGet($configDatapoint + "." + $AreaNr + ".Column" + Column + "dpNames", ColumndpNames); 
  if (dynlen(ColumndpNames)>0)
  {
    if ((0 != ColumndpNames[ConvIndex(Row)]) && ("" != ColumndpNames[ConvIndex(Row)]))
    {
    
/*      if (1==dynlen(strsplit(ColumndpNames[ConvIndex(Row)], ".")))
      {
        connectDatapoint = $datapoint + "." + ColumndpNames[ConvIndex(Row)];
      }
      else
      {
        connectDatapoint = $datapoint + "_" + ColumndpNames[ConvIndex(Row)];
      }*/
      connectDatapoint = ColumndpNames[ConvIndex(Row)];
      return connectDatapoint;
    }
    else
    {
      return "";
    }
  }
  return "";
}



//////////////////////////////////////////////////////////////////////////////////
// FunctionName: getPropertyNames
//
//  
///////////////////////////////////////////////////////////////////////////////////
getPropertyNames(string newSelectionQuery, int newSelectionAmount, dyn_string &propNames)
{
  dyn_dyn_anytype tab;

  dpQuery("SELECT '_online.._value' FROM '" + newSelectionQuery + "' FIRST " + newSelectionAmount, tab);
  for(int i=2 ; i<=dynlen(tab) ; i++)
  {
    string temp =tab[i][1];
    temp = substr(temp, (strlen($datapoint)+1), strlen(temp));
    TableProperties.appendLine("propertyName", temp ,"value", "");
    if(((dynlen(strsplit(temp, "_")))==1) && ((dynlen(strsplit(temp, ".")))==1))
    {
      propNames[i-1] = $datapoint + "." + temp + ":_online.._value";
    }
    else
    {
      propNames[i-1] = $datapoint + "_" + temp + ":_online.._value";
    }
  }
}



//////////////////////////////////////////////////////////////////////////////////
// FunctionName: clearListTable
//
//  Disconnects the List table and clears the content
///////////////////////////////////////////////////////////////////////////////////
void clearListTable(dyn_string &propNames)
{
  dpGet($configDatapoint + ".queryResult", propNames);
  for(int i=1 ; i<=dynlen(propNames); i++)
  {
   navPMLunloadPropertySet(dpSubStr(propNames[i], DPSUB_SYS_DP));
   dpDisconnect("connectActualValue", propNames[i]);
  }
  TableProperties.deleteAllLines;
  propNames="";
}


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: connectTable
//
//  
///////////////////////////////////////////////////////////////////////////////////
connectTable(dyn_string propNames)
{
  dpSet($configDatapoint + ".queryResult", propNames);
  for(int i=1; i<=dynlen(propNames); i++)
  {
    navPMLloadPropertySet(dpSubStr(propNames[i], DPSUB_SYS_DP));
    dpConnect("connectActualValue", propNames[i]);
  }
}


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: connectActualValue
//
//  
///////////////////////////////////////////////////////////////////////////////////
connectActualValue(string dp1, anytype actualValue)
{
  dyn_string queryResult; 
  anytype valueString;
  dpGet($configDatapoint + ".queryResult", queryResult);
  if(elementTypeValid(dp1))
  { 
    if(dpElementType(dp1)==22)
    {
      int floatDecimals;
      dpGet($configDatapoint + ".floatDecimals", floatDecimals);
      if(floatDecimals<=0)
        floatDecimals=1;
      string newActualValue;
      sprintf(newActualValue, "%."+floatDecimals+"f", actualValue);
      valueString = newActualValue;
    }
    else
    {
      valueString = actualValue;
    }
    TableProperties.cellValueRC((dynContains(queryResult,(dp1))-1), "value", valueString);
  }
  else
  {
    TableProperties.cellValueRC((dynContains(queryResult,(dp1))-1), "value", "Cannot display complex type");
    TableProperties.cellBackColRC((dynContains(queryResult,(dp1))-1), "value", "_3DFace");
  }
  TableProperties.cellValueRC((dynContains(queryResult,(dp1))-1), "unit", dpGetUnit(dp1));
}



//////////////////////////////////////////////////////////////////////////////////
// FunctionName: elementTypeValid
//
// returns TRUE is element is one of these types 
///////////////////////////////////////////////////////////////////////////////////
bool elementTypeValid(string element)
{

  if ((19==dpElementType(element)) || (20==dpElementType(element)) ||
      (21==dpElementType(element)) || (22==dpElementType(element)) ||
      (23==dpElementType(element)) || (24==dpElementType(element)) ||
      (25==dpElementType(element)) || (26==dpElementType(element)) ||
      (27==dpElementType(element)) || (41==dpElementType(element)) || (42==dpElementType(element)))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }

}


//////////////////////////////////////////////////////////////////////////////////
//
// FunctionName: setColumnConfig
//
// Function    : Set the width of the configured amount of columns,
//               otherwise they will be set to invisible
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
///////////////////////////////////////////////////////////////////////////////////
void setColumnConfig(int TableNumber, int TableNumberToDisplay, int nrOfColumns, int MaximumColumns, string Addition)
{

  int k, table_x, table_y;

  getValue("Table"+TableNumberToDisplay, "size", table_x, table_y);
  for (k=0; k<nrOfColumns ; k++)
  {
    dpConnect("Arrange" + Addition + "TableContent", $configDatapoint + "."+TableNumber+".Column" + k + "Titles:_online.._value",
                                     $configDatapoint + "."+TableNumber+".Column" + k + "dpNames:_online.._value");
    setValue("Table"+TableNumberToDisplay, "columnVisibility", k, TRUE);
    setValue("Table"+TableNumberToDisplay, "columnWidth", k, (table_x/nrOfColumns));
  }
  for (k=nrOfColumns ; k <= MaximumColumns ; k++)
  {
    setValue("Table"+TableNumberToDisplay, "columnVisibility", k, FALSE);
  }
}


//////////////////////////////////////////////////////////////////////////////////
// Function: ConvIndex
// Input: i [integer]
// Ouput: input i increased with 1.
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
  subViewName = text;
}


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: NavConfigTrendFillDpeSelection
//
// Fills the dpe selectionlist for a datapoint selection
///////////////////////////////////////////////////////////////////////////////////
void NavConfigTrendFillDpeSelection(string datapoint)
{
  string selectedDP;
  dyn_dyn_string elementNames;
  dyn_dyn_int elementTypes;
  getValue("list_dp", "selectedText", selectedDP);
  dyn_string output;
  int elementIndex;
  list_dpe.deleteAllItems; 
  if(selectedDP!="")
    selectedDP = "_" + selectedDP;
  
  dpTypeGet(getDpTypeFromEnabled(datapoint + "__enabled."),elementNames,elementTypes);
  for(elementIndex=2;elementIndex<=dynlen(elementNames);elementIndex++) 
  {
    int elementLevel = dynlen(elementNames[elementIndex])-1; // how deep is the element?
    string elementName = elementNames[elementIndex][elementLevel+1];
    if(!("__"==substr(elementName, 0,2))) //references, dpe starting with __ are not allowed in the list
    {
      output[dynlen(output)+1] = elementName;
    }
  }
  dynSortAsc(output);
  list_dpe.items =output;
  list_dpe.selectedPos=0;
}


//////////////////////////////////////////////////////////////////////////////////
// Function: ProgressBar
//           With this function a progress bar can be made. The progress is shown
//           horizontal and is going from the left to the right.
//           [0                                  100%]
// 
//Input: 1. maximum value (range)== 100%
//       2. current value to show in the progress bar.
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
  
  if (value>=Maximum)
  {
    delay(0,200);
    setValue("bar", "visible", FALSE);
    setValue("bar_border", "visible", FALSE);
  }
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//             GARBAGE  GARBAGE  GARBAGE  GARBAGE  GARBAGE  GARBAGE
/*
//////////////////////////////////////////////////////////////////////////////////
// FunctionName: jump2StationSubrackRCU
//
// jumps/load the Station_Subrack.pnl
///////////////////////////////////////////////////////////////////////////////////
void jump2StationSubrackRCU()
{

  if(dpAccessable($datapoint + "_Board0_AP" + $APNr + "_RCU"+$RCUNr + "__enabled"))
  {
    if ("LOFAR Navigator" == myPanelName())
    { 
      //string datapointPath;
      //convertOriginal2ReferenceDP($datapoint + "_Board0_AP" + $APNr + "_RCU"+$RCUNr, datapointPath);
      if($referenceDatapoint=="")
      {
        navConfigTriggerNavigatorRefreshWithDP($datapoint + "_Board0_AP" + $APNr + "_RCU"+$RCUNr);
      }
      else
      {
        navConfigTriggerNavigatorRefreshWithDP($referenceDatapoint + "_Board0_AP" + $APNr + "_RCU"+$RCUNr);
      }
    }
    else
    {  
      RootPanelOn("navigator/views/Station_Subrack_RCU.pnl",
                  "Station - Subrack - RCU",
                  makeDynString("$datapoint:" + $datapoint + "_Board0_AP" + $APNr + "_RCU"+$RCUNr));
    }
  }
}
*/

/*
//////////////////////////////////////////////////////////////////////////////////
// FunctionName: jump2Station
//
// jumps/load the Station.pnl
///////////////////////////////////////////////////////////////////////////////////
void jump2Station()
{
  if(dpAccessable($datapoint + "_Station01"))
  {
    if ("LOFAR Navigator" == myPanelName())
    {
      if($referenceDatapoint=="")
      {
        navConfigTriggerNavigatorRefreshWithDP($datapoint + "_Station01");
      }
      else
      {
        navConfigTriggerNavigatorRefreshWithDP($referenceDatapoint + "_Station01");
      }
    }
    else
    {  
      RootPanelOn("navigator/views/Station.pnl",
                  "Station ",
                  makeDynString("$datapoint:" + $datapoint + "_Station01"));
    }
  }
}


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: jump2StationSubrack
//
// jumps/load the Station_Subrack.pnl
///////////////////////////////////////////////////////////////////////////////////
void jump2StationSubrack()
{
  if(dpAccessable($datapoint + "_Rack" + $RackNr + "_SubRack" + $SubrackNr))
  {
    if ("LOFAR Navigator" == myPanelName())
    {
      DebugN("$referenceDatapoint:"+$referenceDatapoint);
      DebugN("$datapoint         :"+$datapoint);
      DebugN("$configDatapoint   :"+$configDatapoint);
      if($referenceDatapoint=="") //If the datapoint is a reference, use the reference to navigate to!!
      {
        navConfigTriggerNavigatorRefreshWithDP($datapoint + "_Rack" + $RackNr + "_SubRack" + $SubrackNr);        
      }
      else
      {
        navConfigTriggerNavigatorRefreshWithDP($referenceDatapoint + "_Rack" + $RackNr + "_SubRack" + $SubrackNr);
      }
    }
    else // Panel is undocked.
    {  
      RootPanelOn("navigator/views/Station_Subrack.pnl",
                  "Station - Subrack",
                  makeDynString("$datapoint:" + $datapoint + "_Rack" + $RackNr + "_SubRack" + $SubrackNr));
    }
  }
}

*/

