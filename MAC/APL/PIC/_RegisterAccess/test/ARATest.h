//#  ARATest.h: Automatic test of the RegisterAccess application
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

#ifndef ARATest_H
#define ARATest_H

//# Includes
//# Common Includes
#include "../../../APLCommon/src/test.h"

//# GCF Includes

//# RegisterAccess Includes
#include "ARATestTask.h"

// forward declaration


// redefine the _test and _fail macros to get the correct file and linenumbers
// in the output.
#define _avttest(cond) avt_do_test(cond, #cond, __FILE__, __LINE__)
#define _avtfail(str)  avt_do_fail(str, __FILE__, __LINE__)

class ARATest : public Test
{
  public:
    ARATest();
    virtual ~ARATest();

    virtual void run();
    
    void avt_do_test(bool cond, const string& lbl,
                     const char* fname, long lineno);
    void avt_do_fail(const string& lbl,
                     const char* fname, long lineno);
        
  protected:
    // protected copy constructor
    ARATest(const ARATest&);
    // protected assignment operator
    ARATest& operator=(const ARATest&);
    
  private: 
    
    ARATestTask m_testTask;
};
#endif
