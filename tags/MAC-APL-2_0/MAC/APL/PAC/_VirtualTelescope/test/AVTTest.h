//#  AVTTest.h: Automatic test of the Virtual Telescope logical device
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

#ifndef AVTTest_H
#define AVTTest_H

//# Includes
//# Common Includes
#include "../../../APLCommon/src/test.h"

//# GCF Includes

// forward declaration


// redefine the _test and _fail macros to get the correct file and linenumbers
// in the output.
#define _avttest(cond) avt_do_test(cond, #cond, __FILE__, __LINE__)
#define _avtfail(str)  avt_do_fail(str, __FILE__, __LINE__)

template<class T>
class AVTTest : public Test
{
  public:
    AVTTest(const string& name);
    virtual ~AVTTest();
    
    virtual void run();
    
    void avt_do_test(bool cond, const string& lbl,
                     const char* fname, long lineno);
    void avt_do_fail(const string& lbl,
                     const char* fname, long lineno);
        
  protected:
    // protected copy constructor
    AVTTest(const AVTTest&);
    // protected assignment operator
    AVTTest& operator=(const AVTTest&);
    
  private: 
    
    T m_testTask;
};

#include "AVTTest.cc" // include template class implementation
#endif
