/***************************************************************************
 *   Copyright (C) 2006 by ASTRON, Adriaan Renting                         *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <casa/Exceptions.h>
#include <iostream>
#include <casa/Inputs/Input.h>
#include <ms/MeasurementSets.h>
#include "MS_File.h"
#include "ComplexMedianFlagger.h"

#define VERSION "0.60"

using namespace casa;

int main(int argc, char *argv[])
try
{
  using std::cout;
  using std::cerr;
  using std::cin;
  using std::endl;
  Input inputs(1);
  inputs.version(string(VERSION) + string(" autoflagging by Adriaan Renting and Michiel Brentjens for WSRT data\n") +
                 string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
                 string("Documentation can be found at: www.astron.nl/~renting\n"));
  inputs.create("ms",        "",      "Measurementset",             "String");
  inputs.create("window",    "13",    "Median window size",         "Int");
  inputs.create("crosspol",  "false", "Use only crosscorrelations", "Bool");
  inputs.create("min",       "5",     "Minimal flag threshold",     "Double");
  inputs.create("max",       "5",     "Maximal flag threshold",     "Double");
  inputs.create("flagrms",   "true",  "Flag baselines on RMS",      "Bool");
  inputs.create("flagdata",  "true",  "Flag datapoints",            "Bool");
  inputs.create("existing",  "true",  "Respect existing flags",     "Bool");
  inputs.readArguments(argc, argv);
  
  String ms = inputs.getString("ms");
  if (ms == "")
  {
    cerr  << "Usage: WSRT_flagger ms=test.MS crosspol=false window=13 min=5 max=5.5 flagrms=true flagdata=true existing=true" << endl
        << "Where ms       [no default]    is the Measurementset" << endl
        << "      window   [default 13]    is the window size for the used median" << endl
        << "      crosspol [default false] will only use the crosspolarizations to determine flag" << endl
        << "      min      [default 5]     min*sigma is the flag threshold at baselinelength 0" << endl
        << "      max      [default 5]     max*sigma is the flag threshold at maximum baselinelength" << endl
        << "      flagrms  [default true]  determines if baselines are flagged on RMS" << endl
        << "      flagdata [default true]  determines if datapoints are flagged on complex size" << endl
        << "      existing [default true]  determines if existing flags are kept (no new flag cat. is created)" << endl << endl
        << "Data at point window/2 will be flagged on:" << endl
        << "      vis[window/2] - (median(Re(vis[])) + median(Im(vis[]))) " << endl
        << "    > (min + (max-min)*baselinelength/maxbaselinelength)*sigma" << endl;
    return -1;
  }
  cout << "Runnning flagger please wait..." << endl;
  WSRT::MS_File Test(ms);
  WSRT::ComplexMedianFlagger Flagger(&Test, 
                                     inputs.getInt("window"), 
                                     inputs.getBool("crosspol"), 
                                     inputs.getDouble("min"),
                                     inputs.getDouble("max"));
  Flagger.FlagDataOrBaselines(inputs.getBool("flagdata"), 
                              inputs.getBool("flagrms"),
                              inputs.getBool("existing"));
  
  return 0;
} //try
catch(casa::AipsError& err)
{
  std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
  return -2;
}
catch(...)
{
  std::cerr << "** PROBLEM **: Unhandled exception caught." << std::endl;
  return -3;
}
