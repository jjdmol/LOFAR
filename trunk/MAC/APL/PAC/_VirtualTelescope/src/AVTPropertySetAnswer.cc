//#  AVTPropertySetAnswer.cc: Implementation of the PropertySet Answer
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

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTPropertySetAnswer.h"
#include "AVTPropertySetAnswerHandlerInterface.h"

AVTPropertySetAnswer::AVTPropertySetAnswer(AVTPropertySetAnswerHandlerInterface& handler) :
  m_handler(handler)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTPropertySetAnswer::AVTPropertySetAnswer"));
}

AVTPropertySetAnswer::~AVTPropertySetAnswer()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTPropertySetAnswer::~AVTPropertySetAnswer"));
}

void AVTPropertySetAnswer::handleAnswer(GCFEvent& answer)
{
  m_handler.handlePropertySetAnswer(answer);
}
