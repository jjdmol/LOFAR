//#  ABSPointing.cc: implementation of the ABS::Pointing class
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

#include "Pointing.h"

using namespace LOFAR::BS;

Pointing::Pointing() : m_angle1(0.0), m_angle2(0.0), m_time(), m_type(J2000)
{
}

Pointing::Pointing(double angle1, double angle2, RTC::Timestamp time, Type type) :
  m_angle1(angle1), m_angle2(angle2), m_time(time), m_type(type)
{}

Pointing::~Pointing()
{}
