//#  ARATest.cc: Implementation of the RegisterAccess test
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

#include "ARATest.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace ARA;
using namespace std;

ARATest::ARATest() :
  Test(string("ARATest")),
  m_testTask(*this)
{
}

ARATest::~ARATest()
{
}

void ARATest::run()
{  
  m_testTask.start();
  
  GCFTask::run();
}

void ARATest::avt_do_test(bool cond, const string& lbl,
                          const char* fname, long lineno)
{
  do_test(cond,lbl,fname,lineno);
}

void ARATest::avt_do_fail(const string& lbl,
                          const char* fname, long lineno)
{
  do_fail(lbl,fname,lineno);
}
