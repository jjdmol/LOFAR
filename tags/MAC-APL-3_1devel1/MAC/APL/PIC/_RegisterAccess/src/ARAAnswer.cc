//#  ARAAnswer.cc: Implementation of the ARAAnswer object
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

#include "ARAAnswer.h"
#include <GCF/TM/GCF_Task.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  
namespace ARA
{

ARAAnswer::ARAAnswer() : 
  m_dummyPort(),
  m_task(0)
{
}

ARAAnswer::~ARAAnswer()
{
}

void ARAAnswer::setTask(GCFTask* pTask)
{
  m_task=pTask;
}

void ARAAnswer::handleAnswer(GCFEvent& answer)
{
  if(m_task!=0)
    m_task->dispatch(answer,m_dummyPort);
}


} // namespace ARA


} // namespace LOFAR

