//#  ABSBeamletStats.h: implementation of the BeamletStats class
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

#include <ABSConstants.h>
#include <ABSBeamletStats.h>
#include <ABSBeamletStatsPSet.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVDouble.h>

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <iostream>

using namespace LOFAR;
using namespace ABS;
using namespace std;
using namespace blitz;

#define N_PHASE 2 // complex number has two components (re, im)
#define EPA_SCALING_FACTOR 512 // number of samples in power_sum

BeamletStats::BeamletStats(int n_beamlets, int n_integrations) :
    m_nbeamlets(n_beamlets),
    m_nintegrations(n_integrations),
    m_count(0),
    m_seqnr(0),
    m_selected_beamlet(0),
    m_beamlet_power(n_beamlets, N_POLARIZATIONS),
    m_pset(BeamServerPSet, "BeamServer", this)
{
  m_pset.load();
  m_beamlet_power = 0.0;

  if (m_nbeamlets > N_BEAMLETS) m_nbeamlets = N_BEAMLETS;
}

BeamletStats::~BeamletStats()
{
}

void BeamletStats::update(Array<unsigned int,3>& power_sum, unsigned int seqnr)
{
  int beamlet_offset = 0;

  //
  // Even packets contain the power sums for the first
  // half of the beamlets and odd packets for the second half.
  //
  if (seqnr % 2) // odd packet
  {
      if (seqnr != m_seqnr + 1)
      {
	  m_seqnr = seqnr;
	  return; // skip loose packet
      }
      beamlet_offset = m_nbeamlets / 2;
  }
  else // even packet
  {
      m_count++;
  }

  for (int i = 0; i < m_nbeamlets / 2; i++)
  {
      // x-polarization
      m_beamlet_power(i + beamlet_offset, 0) += power_sum(i, 0, 0) + power_sum(i, 0, 1);
      
      // y-polarization
      m_beamlet_power(i + beamlet_offset, 1) += power_sum(i, 1, 0) + power_sum(i, 1, 1);
  }

  m_seqnr = seqnr;

  if ( (m_count % m_nintegrations) == 0)
  {
      // divide by number of samples = m_count * EPA_SCALING_FACTOR
      m_beamlet_power /= m_nintegrations * EPA_SCALING_FACTOR;

      LOG_DEBUG(formatString("Updating statistics properties: totalpower = %f",
			     sum(m_beamlet_power)));

      cout << "m_beamlet_power = ";
      for (int i = 0; i < m_nbeamlets; i++)
      {
	  cout << m_beamlet_power(i, 0) << ";";
      }
      cout << endl;

      char propnamex[64];
      char propnamey[64];
      for (int i = 0; i < m_nbeamlets; i++)
      {
	  snprintf(propnamex, 64, "power%03d_x", i);
	  snprintf(propnamey, 64, "power%03d_y", i);

	  //m_pset[i*2  ].setValue(GCFPVDouble(m_beamlet_power(i, 0)));
	  //m_pset[i*2+1].setValue(GCFPVDouble(m_beamlet_power(i, 1)));
	  m_pset[propnamex].setValue(GCFPVDouble(m_beamlet_power(i, 0)));
	  m_pset[propnamey].setValue(GCFPVDouble(m_beamlet_power(i, 1)));
      }

      // update the seqnr property
      m_pset["seqnr"].setValue(GCFPVUnsigned(m_seqnr));

      // reset m_beamlet_power array
      m_beamlet_power = 0.0;
      m_count = 0;
  }
}

bool BeamletStats::isReady()
{
  return m_pset.isLoaded();
}

void BeamletStats::handleAnswer(GCFEvent& event)
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
	  if (strcmp(pResponse->pPropName, "BeamServer_beamlet") == 0)
	  {
	      const GCFPVUnsigned* beamletProp = static_cast<const GCFPVUnsigned*>(pResponse->pValue);
	      m_selected_beamlet = beamletProp->getValue();

	      if ((m_selected_beamlet <= 0)
		  || (m_selected_beamlet > m_nbeamlets))
	      {
		  m_selected_beamlet = 0;
	      }
	  }
      }
      break;

      default:
	  LOG_ERROR("unhandled event");
	  break;
  }
}
