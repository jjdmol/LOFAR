//#  ABSSubbandStats.h: implementation of the SubbandStats class
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

#include <ABSSubbandStats.h>
#include <ABSSubbandStatsPSet.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVDouble.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <iostream>

using namespace LOFAR;
using namespace ABS;
using namespace std;

//#define WAIT_FOR_INPUT

SubbandStats::SubbandStats(int n_subbands, int n_accumulate) :
    m_nsubbands(n_subbands),
    m_naccumulate(n_accumulate),
    m_count(0),
    m_selected_subband(0),
    m_subband_stats(n_subbands),
    m_pset(BeamServerPSet, "BeamServer", this)
{
  m_pset.load();
  m_subband_stats = 0.0;
}

SubbandStats::~SubbandStats()
{
}

void SubbandStats::updateRaw(void* rawdata, size_t count)
{
  //
  // accumulate statistics
  //
  //double* power = (double*)((char*)rawdata+6);
  for (int i = 0; i < m_nsubbands; i++)
  {
#if 0
      m_subband_stats(i) += *power++;
#else
      m_subband_stats(i) = count;
#endif
  }

  m_count++;
  if (m_count % m_naccumulate == 0)
  {
#if 0
      // update the seqnr property
      m_pset["seqnr"].setValue(GCFPVUnsigned(*(int*)((char*)rawdata+2)));
#else
      // update the seqnr property
      m_pset["seqnr"].setValue(GCFPVUnsigned(count));
#endif

      // update the power property
      m_pset["power_re"].setValue(GCFPVDouble(
		real(m_subband_stats(m_selected_subband))));
      m_pset["power_im"].setValue(GCFPVDouble(
                imag(m_subband_stats(m_selected_subband))));
  }
}

bool SubbandStats::isReady()
{
  return m_pset.isLoaded();
}

void SubbandStats::handleAnswer(GCFEvent& event)
{
  switch (event.signal)
  {
      case F_MYPLOADED_SIG:
      {
	  LOG_DEBUG("BeamServer PSet Loaded");
      }
      break;

      case F_VCHANGEMSG_SIG:
      {
	  GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&event);
	  if (strcmp(pResponse->pPropName, "BeamServer_subband") == 0)
	  {
	      const GCFPVUnsigned* subbandProp = static_cast<const GCFPVUnsigned*>(pResponse->pValue);
	      m_selected_subband = subbandProp->getValue();

	      if ((m_selected_subband <= 0)
		  || (m_selected_subband > m_nsubbands))
	      {
		  m_selected_subband = 0;
	      }
	  }
      }
      break;

      default:
	  LOG_ERROR("unhandled event");
	  break;
  }
}
