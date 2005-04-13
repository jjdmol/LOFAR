//#  test.h
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

#ifndef TEST_H
#define TEST_H

#include <Common/lofar_string.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

using std::string;
using std::ostream;
using std::map;
using std::list;
// The following have underscores because they are macros
// (and it's impolite to usurp other users' functions!).
// For consistency, _succeed() also has an underscore.
#define TESTC(cond) do_test(cond, #cond, 0, __FILE__, __LINE__)
#define TESTC_DESCR(cond, _descr_) do_test(cond, #cond, _descr_, __FILE__, __LINE__)
#define FAIL(str) do_fail("", str, __FILE__, __LINE__)

class Test
{
  public:
    Test(const string& name, ostream* osptr = 0);
    virtual ~Test(){}
    virtual void run() = 0;

    long getNumPassed() const;
    long getNumFailed() const;
    const ostream* getStream() const;
    void setStream(ostream* osptr);
    
    long report() const;
    virtual void reset();

  protected:
    bool do_test(bool cond, const string& lbl, const char* descr, 
                 const char* fname, long lineno);
    void do_fail(const string& lbl,
                 const char* descr,
                 const char* fname, long lineno);
    
    void setCurSubTest(const char* testname, const char* description);
    void reportSubTest();      

  private:

    ostream* m_osptr;
    long m_nPass;
    long m_nFail;
    string m_name;
    
    typedef struct
    {
      unsigned int passed;
      unsigned int failed;
    } TSubTest;
    
    typedef map<string /*testname*/, TSubTest> TSubTests;
    TSubTests m_subTests;
    typedef list<string /*failurestring*/> TFailures;
    TFailures m_failures;
    string m_curSubTest;

    // Disallowed:
    Test(const Test&);
    Test& operator=(const Test&);
    void succeed(const string& lbl,
                 const char* descr,
                 const char* fname, long lineno);
};

inline
Test::Test(const string& name,ostream* osptr)
{
    m_osptr = osptr;
    m_nPass = m_nFail = 0;
    m_name = name;
}

inline
long Test::getNumPassed() const
{
    return m_nPass;
}

inline
long Test::getNumFailed() const
{
    return m_nFail;
}

inline
const ostream* Test::getStream() const
{
    return m_osptr;
}

inline
void Test::setStream(ostream* osptr)
{
    m_osptr = osptr;
}


#endif

