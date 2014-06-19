//#  suite.h
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


#ifndef SUITE_H
#define SUITE_H

#include <TestSuite/test.h>   // includes <string>, <iosfwd>
#include <Common/lofar_vector.h>

using std::vector;

class Suite
{
public:
    Suite(const string& name, ostream* osptr = 0);
    
    string getName() const;
    long getNumPassed() const;
    long getNumFailed() const;
    const ostream* getStream() const;
    void setStream(ostream* osptr);
    
    void addTest(Test* t);
    void addSuite(const Suite&);
    void run();     // Calls Test::run() repeatedly
    long report() const;
    void free();    // deletes tests

private:
    string m_name;
    ostream* m_osptr;
    vector<Test*> m_tests;
    void reset();

    // Disallowed ops:
    Suite(const Suite&);
    Suite& operator=(const Suite&);
};

inline
Suite::Suite(const string& name, ostream* osptr)
     : m_name(name)
{
    m_osptr = osptr;
}

inline
string Suite::getName() const
{
    return m_name;
}

inline
const ostream* Suite::getStream() const
{
    return m_osptr;
}

inline
void Suite::setStream(ostream* osptr)
{
    m_osptr = osptr;
}

#endif

