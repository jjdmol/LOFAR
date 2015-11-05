//#  GCF_PropertySetAnswer.cc: Implementation of the PropertySet Answer
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
#include <Common/LofarLogger.h>

#include "GCF/PAL/GCF_PropertySetAnswer.h"
#include "GCF/PAL/GCF_PropertySetAnswerHandlerInterface.h"

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

GCFPropertySetAnswer::GCFPropertySetAnswer(GCFPropertySetAnswerHandlerInterface& handler) :
  GCFAnswer(),
  m_handler(handler)
{
  LOG_DEBUG("GCFPropertySetAnswer::GCFPropertySetAnswer");
}

GCFPropertySetAnswer::~GCFPropertySetAnswer()
{
  LOG_DEBUG("GCFPropertySetAnswer::~GCFPropertySetAnswer");
}

void GCFPropertySetAnswer::handleAnswer(GCFEvent& answer)
{
  m_handler.handlePropertySetAnswer(answer);
}
