//#  test.cpp
//#
//#  Source: C++ Users Journal - September 2000
//#          Chuck Allison - The Simplest Automated Unit Test Framework That Could Possibly Work
//#          Article: http://www.cuj.com/documents/s=8035/cuj0009allison1/
//#          code:    ftp://ftp.cuj.com/pub/2000/cujsep2000.zip
//# 
//#  Modifications for LOFAR:
//#  - removed TestSuiteError exception
//#  - removed runtime type information
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

#include "test.h"
#include <Common/lofar_iostream.h>

#ifdef _MSC_VER
//Allow return-less mains:
#pragma warning(disable: 4541)
#endif

using namespace std;

void Test::do_test(bool cond, const std::string& lbl,
                   const char* fname, long lineno)
{
    if (!cond)
        do_fail(lbl, fname, lineno);
    else
        _succeed();
}

void Test::do_fail(const std::string& lbl,
                   const char* fname, long lineno)
{
    ++m_nFail;
    if (m_osptr)
    {
        *m_osptr << m_name
                             << "failure: (" << lbl << ") , "
                                 << fname
                 << " (line " << lineno << ")\n";
    }
}

long Test::report() const
{
    if (m_osptr)
        {
            *m_osptr << "Test \"" 
                         << m_name << "\":\n"
                     << "\tPassed: " << m_nPass
                     << "\tFailed: " << m_nFail
                     << endl;
        }
    return m_nFail;
}

