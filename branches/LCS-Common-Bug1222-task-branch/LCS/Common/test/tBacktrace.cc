//#  tBacktrace.cc: one line description
//#
//#  Copyright (C) 2002-2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Backtrace.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

Backtrace gs;

void g()
{
  LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  // Create a large object on the stack, pushing the stack boundaries.
  char c[1000000];
  Backtrace s;
  std::cout << s;
}

void f()
{
  LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  g();
}

void e()
{
  LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  f();
}

void d()
{
  LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  e();
}

void c()
{
  LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  d();
}

void b()
{
  LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  c();
}

void a()
{
  LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
  b();
}


int main()
{
  INIT_LOGGER("tBacktrace");
  std::cout << gs;
  std::cout << "************" << std::endl;
  a();
  std::cout << "************" << std::endl;
  return 0;
}
