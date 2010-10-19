//#  suite.cpp
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

#include <Suite/suite.h>
#include <Common/lofar_iostream.h>
#include <assert.h>

void Suite::addTest(Test* t)
{
    // Make sure test has a stream:
    if (t != 0)
    {
      if (m_osptr != 0 && t->getStream() == 0)
      {
        t->setStream(m_osptr);
      }
      m_tests.push_back(t);
      t->reset();
    }
}

void Suite::addSuite(const Suite& s)
{
    for (size_t i = 0; i < s.m_tests.size(); ++i)
        addTest(s.m_tests[i]);
}

void Suite::free()
{
    // This is not a destructor because tests
    // don't have to be on the heap.
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        delete m_tests[i];
        m_tests[i] = 0;
    }
}

void Suite::run()
{
    reset();
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        m_tests[i]->run();
    }
}


long Suite::report() const
{
    if (m_osptr)
    {
        long totFail = 0;
        *m_osptr << "Test suite \"" << m_name << "\"\n=======";
        size_t i;
        for (i = 0; i < m_name.size(); ++i)
            *m_osptr << '=';
        *m_osptr << "=\n";

        for (i = 0; i < m_tests.size(); ++i)
        {
            assert(m_tests[i]);
            totFail += m_tests[i]->report();
        }

        *m_osptr << "=======";
        for (i = 0; i < m_name.size(); ++i)
            *m_osptr << '=';
        *m_osptr << "=\n";
        return totFail;
    }
    else
        return getNumFailed();
}

long Suite::getNumPassed() const
{
    long totPass = 0;
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        totPass += m_tests[i]->getNumPassed();
    }
    return totPass;
}

long Suite::getNumFailed() const
{
    long totFail = 0;
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        totFail += m_tests[i]->getNumFailed();
    }
    return totFail;
}

void Suite::reset()
{
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        m_tests[i]->reset();
    }
}

