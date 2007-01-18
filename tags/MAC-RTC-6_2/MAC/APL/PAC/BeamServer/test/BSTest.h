//#
//#  BSTest.h: Macros for using the testsuite more effectively.
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
//#

#ifndef BSTEST_H_
#define BSTEST_H_

#include <Suite/suite.h>

#define START_TEST(_test_, _descr_) \
  setCurSubTest(#_test_, _descr_)

#define STOP_TEST() \
  reportSubTest()

#define NEXT_TEST(_test_, _descr_) \
  { \
    setCurSubTest(#_test_, _descr_); \
    TRAN(_test_); \
  }

#define FINISH \
  { \
    reportSubTest(); \
  }

/* TRAN(Application::finished); */

#define ABORT_TESTS \
  { \
    FINISH; \
  }

#define FAIL_AND_ABORT(_txt_) \
  { \
    FAIL(_txt_);  \
    ABORT_TESTS; \
  }

#define TESTC_ABORT_ON_FAIL(cond) \
  if (!TESTC(cond)) \
  { \
    ABORT_TESTS; \
    break; \
  }

#define TESTC_DESCR_ABORT_ON_FAIL(cond, _descr_) \
  if (!TESTC_DESCR(cond, _descr_)) \
  { \
    ABORT_TESTS; \
    break; \
  }

#endif
