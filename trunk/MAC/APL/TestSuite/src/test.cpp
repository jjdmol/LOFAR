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

#include <lofar_config.h>
#include <TestSuite/test.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

using std::endl;

#ifdef _MSC_VER
//Allow return-less mains:
#pragma warning(disable: 4541)
#endif

bool Test::do_test(bool cond, const std::string& lbl,
                   const char* descr, 
                   const char* fname, long lineno)
{  
  if (!cond)
    do_fail(lbl, descr, fname, lineno);
  else
    succeed(lbl, descr, fname, lineno);

  return cond;
}

void Test::do_fail(const std::string& lbl,
                   const char* descr,
                   const char* fname, long lineno)
{
  ++m_nFail;
  m_subTests[m_curSubTest].failed++;
  string failure;
  failure += "Test: (" + m_curSubTest +") ";
  if (descr) failure += descr;    
  if (lbl.length() > 0) failure += " (" + lbl + ")";
  failure += LOFAR::formatString(" => FAILED!!! [%s:%d]", (strrchr(fname, '/') + 1), lineno);
  if (m_osptr)
  {
    *m_osptr << failure << endl;
  }
  m_failures.push_back(failure);
}

void Test::succeed(const std::string& lbl,
                   const char* descr,
                   const char* fname, long lineno)
{
  m_subTests[m_curSubTest].passed++;
  ++m_nPass;
  if (m_osptr)
  {
    *m_osptr << "Test: ";
    if (descr) *m_osptr << descr;    
    *m_osptr << "(" << lbl << ") => succeed [" 
             << (strrchr(fname, '/') + 1)
             << ":" << lineno << "]\n";
  }
}

long Test::report() const
{
  if (m_osptr)
  {
    *m_osptr << "Test summary \"" 
             << m_name << "\":\n"
             << "\tPassed: " << m_nPass
             << "\tFailed: " << m_nFail
             << endl << "Failures:" << endl;
    
    for (TFailures::const_iterator iter = m_failures.begin();
         iter != m_failures.end(); ++iter)
    {
      *m_osptr << "\t" << *iter << endl;
    }
  }
  return m_nFail;
}

void Test::reportSubTest()
{
  if (m_osptr && m_curSubTest != "")
  {
    *m_osptr << "Finish (sub)test " 
               << m_name << "." << m_curSubTest
             << "\tPassed: " << m_subTests[m_curSubTest].passed
             << "\tFailed: " << m_subTests[m_curSubTest].failed
             << endl;
  }
}

void Test::setCurSubTest(const char* testname, const char* description)
{
  if (m_curSubTest != testname && m_osptr)
  {
    if (m_curSubTest.length() > 0)
    {
      *m_osptr << "Finish (sub)test " 
               << m_name << "." << m_curSubTest
               << "\tPassed: " << m_subTests[m_curSubTest].passed
               << "\tFailed: " << m_subTests[m_curSubTest].failed
               << endl;
    }
    *m_osptr << "Start (sub)test " 
               << m_name << "." << testname;
    if (description) *m_osptr << ": " << description;
    *m_osptr << endl;
  }
  m_curSubTest = testname;
  TSubTests::iterator iter = m_subTests.find(m_curSubTest);
  if (iter != m_subTests.end())
  {
    TSubTest subTest;    
    subTest.passed = 0;
    subTest.failed = 0;
    m_subTests[m_curSubTest] = subTest;
  }
}


void Test::reset()
{
    m_nPass = m_nFail = 0;
}
