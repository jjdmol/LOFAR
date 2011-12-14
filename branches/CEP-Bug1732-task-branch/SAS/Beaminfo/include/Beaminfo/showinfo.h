//# showinfo.h: show contents of vectors and maps 
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
//# $Id: addbeaminfo.cc 18832 2011-11-09 17:22:32Z duscha $
//#
//# @author Sven Duscha

#include <vector>
#include <map>

// SAS
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/Converter.h>
#include <OTDB/TreeTypeConv.h>


// TODO: use template definitions

// DEBUG SAS output functions
void showTreeList(const std::vector<LOFAR::OTDB::OTDBtree>&	trees);
void showNodeList(const std::vector<LOFAR::OTDB::OTDBnode>&	nodes);
void showValueList(const std::vector<LOFAR::OTDB::OTDBvalue>&	items);

void showVector(const std::vector<std::string> &v, const std::string &key="");
void showVector(const std::vector<int> &v);
void showVector(const std::vector<unsigned int> &v);
void showVector(const std::vector<ptime> &v);

void showMap(const std::map<std::string, std::string> &m, const std::string &key="");
void showMap(const std::map<std::string, std::vector<std::string> > &m, const std::string &key="");
void showMap(const std::map<std::string, int> &m, const std::string &key="");
void showMap(const std::map<std::string, std::vector<unsigned int> > &m, const std::string &key="");
void showMap(const std::map<std::string, std::vector<int> > &m, const std::string &key="");
void showMap(const std::map<std::string, ptime> &m, const std::string &key="");
void showMap(const std::map<std::vector<unsigned int>, std::vector<unsigned int> > &m);
void showMap(const std::map<unsigned int, std::vector<unsigned int> > &m);

// Taken from addbeaminfo.h
#ifndef __FAILED_TILE__
#define __FAILED_TILE__
typedef struct
{
  unsigned int antennaId;
  std::vector<unsigned int> rcus;
  std::vector<ptime> timeStamps;
} failedTile;
#endif

void showFailedTiles(const std::vector<failedTile> &failedTiles);

void padTo(std::string &str, const size_t num, const char paddingChar);
