//  BBSTestLogger.h: Writes BBSTest loglines to the right file
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef LOFAR_BBS3_BBSTESTLOGGER_H
#define LOFAR_BBS3_BBSTESTLOGGER_H

// \file BBSTestLogger.h
// This is a class that writes BBSTest loglines to the correct file

#include <Common/lofar_string.h>
#include <fstream>
#include <MNS/MeqMatrix.h>
#include <Common/Timer.h>

using namespace std;

namespace LOFAR
{

// \addtogroup BBS3
// @{

class BBSTestLogger
{
public:
  BBSTestLogger();
  ~BBSTestLogger();

  static void log(const string& name, NSTimer& timer);
  static void log(const string& name, const MeqMatrix& mat);
  static void log(const string& text);

  static ofstream& getLogFile();

private:
  static ofstream* theirOutFile;
};

// @}

} // end namespace LOFAR

#endif
