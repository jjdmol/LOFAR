//# addUV2MS.h: header file of addUV2MS tool
//# adds uv data from a (casa) MS file to a named column of another (LOFAR) MS
//#
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: addUV2MS.h xxxx 2011-10-21 14:16:37Z duscha $


#ifndef LOFAR_BBSTOOLS_ADDUV2MS_H
#define LOFAR_BBSTOOLS_ADDUV2MS_H

//#include <vector>
#include <casa/Arrays/Vector.h>
#include <synthesis/MeasurementEquations/Imager.h>          // casarest ft()

void parseOptions(const vector<string> &arguments, 
                  string &msName, 
                  casa::Vector<casa::String> &patchNames, 
                  unsigned int &nwplanes);
casa::MDirection getPatchDirection(const string &patchName);
void addDirectionKeyword( casa::Table LofarMS, 
                          const std::string &patchName);
void addChannelSelectionKeyword(casa::Table &LofarTable, 
                                const std::string &columnName);
string createColumnName(const casa::String &);
void removeExistingColumns( const std::string &MSfilename, 
                            const casa::Vector<casa::String> &patchNames);
void addImagerColumns (casa::MeasurementSet& ms);
void addModelColumn ( casa::MeasurementSet &ms, 
                      const casa::String &dataManName);
casa::Double getMSReffreq(const casa::MeasurementSet &ms);
casa::Double getMSChanwidth(const casa::MeasurementSet &ms);
map<string, double>  patchFrequency(casa::MeasurementSet &ms, 
                                    const casa::Vector<casa::String> &patchNames);
bool validModelImage(const casa::String &imageName, string &error);                                    
unsigned int makeTempImages(const casa::Vector<casa::String> &patchNames, 
                            const std::string &prefix="tmp");
unsigned int removeTempImages(const casa::Vector<casa::String> &patchNames, 
                              const std::string &prefix="tmp");
double updateFrequency(const std::string &imageName, 
                      double reffreq);
void restoreFrequency(const std::map<std::string, 
                      double> &refFrequencies);
void getImageOptions( const string &patchName, 
                      unsigned int &imSizeX, unsigned int &imSizeY, 
                      casa::Quantity &cellSizeX, casa::Quantity &cellSizeY, 
                      unsigned int &nchan, unsigned int &npol, string &stokes);

//--------------------------------------------------------------
// Function declarations (debug functions)
//
casa::Vector<casa::String> getColumnNames(const casa::Table &table);
void showColumnNames(casa::Table &table);
void usage(const string &);

void showVector(const vector<string> &v, const string &key="");

#endif