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

#ifdef USE_GNUPLOT
#include <gnuplot_i.h>

static gnuplot_ctrl* gp_handle_x = 0;
static gnuplot_ctrl* gp_handle_y = 0;
static gnuplot_ctrl* gp_handle_t = 0;
static int snapshot_time = 0;

#define N_TIME 60
Array<double, 1> time_axis(N_TIME);
Array<double, 2> power_t;

#define PLOT_BIN_NR 10
#endif

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
    m_power_sum(n_beamlets / 2, N_POLARIZATIONS, 2),
    m_pset(BeamServerPSet, "BeamServer", this)
{
  m_pset.load();
  m_beamlet_power = 0.0;
  m_power_sum = 0;

  if (m_nbeamlets > N_BEAMLETS) m_nbeamlets = N_BEAMLETS;

#ifdef USE_GNUPLOT
  gp_handle_x = gnuplot_init();
  if (gp_handle_x)
  {
      gnuplot_setstyle(gp_handle_x, "impulses");
      gnuplot_set_xlabel(gp_handle_x, "beamlet");
      gnuplot_set_ylabel(gp_handle_x, "power_x");
  }
  gp_handle_y = gnuplot_init();
  if (gp_handle_y)
  {
      gnuplot_setstyle(gp_handle_y, "impulses");
      gnuplot_set_xlabel(gp_handle_y, "beamlet");
      gnuplot_set_ylabel(gp_handle_y, "power_y");
  }
  gp_handle_t = gnuplot_init();
  if (gp_handle_t)
  {
      //gnuplot_setstyle(gp_handle_t, "impulses");
      gnuplot_set_xlabel(gp_handle_t, "time");
      gnuplot_set_ylabel(gp_handle_t, "subband");
  }

  firstIndex i;
  time_axis = i;

  power_t.resize(n_beamlets, N_TIME);
  power_t = 0.0;
#endif
}

BeamletStats::~BeamletStats()
{
#ifdef USE_GNUPLOT
    if (gp_handle_x)
    {
	gnuplot_close(gp_handle_x);
	gp_handle_x = 0;
    }
    if (gp_handle_y)
    {
	gnuplot_close(gp_handle_y);
	gp_handle_y = 0;
    }
    if (gp_handle_t)
    {
	gnuplot_close(gp_handle_t);
	gp_handle_t = 0;
    }
#endif
}

void BeamletStats::update(Array<unsigned int,3>& power_sum, unsigned int seqnr)
{
  //
  // Even packets contain the power sums for the first
  // half of the beamlets and odd packets for the second half.
  //
  if (seqnr % 2) // odd packet
  {
      if (seqnr != m_seqnr + 1)
      {
	  LOG_ERROR(formatString("\n\n**** MISSED %d PACKETS! ****", seqnr - m_seqnr - 1));
	  m_seqnr = seqnr;
	  return; // skip loose packet
      }

      /**
       * Got a complete result, add it to the total power
       */

      //
      // only count the first of m_nintegrations updates
      //
      if (m_count + 1 == m_nintegrations)
      {
	// first m_nbeamlets/2 beamlets
	for (int i = 0; i < m_nbeamlets / 2; i++)
	{
	  // x-polarization
	  m_beamlet_power(i, 0) += m_power_sum(i, 0, 0) + m_power_sum(i, 0, 1);
	  
	  // y-polarization
	  m_beamlet_power(i, 1) += m_power_sum(i, 1, 0) + m_power_sum(i, 1, 1);
	}
	
	// next m_nbeamlets/2 beamlets
	for (int i = 0; i < m_nbeamlets / 2; i++)
	{
	  // x-polarization
	  m_beamlet_power(i + m_nbeamlets / 2, 0) += power_sum(i, 0, 0) + power_sum(i, 0, 1);
	  
	  // y-polarization
	  m_beamlet_power(i + m_nbeamlets / 2, 1) += power_sum(i, 1, 0) + power_sum(i, 1, 1);
	}
      }

      m_count++;
  }
  else
  {
    // even packet, first packet of statistics update, save it in m_power_sum
    m_power_sum = power_sum;
  }

  m_seqnr = seqnr;

  if ( m_count && ((m_count % m_nintegrations) == 0) )
  {
      // divide by number of samples = m_count * EPA_SCALING_FACTOR

      //      m_beamlet_power /= m_nintegrations * EPA_SCALING_FACTOR;
      // only count first of m_nintegrations
      m_beamlet_power /= EPA_SCALING_FACTOR;

      LOG_DEBUG(formatString("Updating statistics properties: totalpower = %f",
			     sum(m_beamlet_power)));

      cout << "m_beamlet_power(x) = ";
      for (int i = 0; i < m_nbeamlets; i++)
      {
	  cout << m_beamlet_power(i, 0) << ";";
      }
      cout << endl;
      cout << "m_beamlet_power(y) = ";
      for (int i = 0; i < m_nbeamlets; i++)
      {
	  cout << m_beamlet_power(i, 1) << ";";
      }
      cout << endl;

#ifdef USE_GNUPLOT
      Array<double, 1> freq(m_nbeamlets);
      firstIndex i;
      freq = i;
      Array<double, 1> power(m_nbeamlets);
      power = m_beamlet_power(Range::all(), 0);
      if (gp_handle_x)
      {
	  gnuplot_resetplot(gp_handle_x);
	  gnuplot_plot_xy(gp_handle_x, 
			  freq.data(), 
			  power.data(), 
			  m_nbeamlets, 
			  "Power per beamlet x-polarization");
      }
      power = m_beamlet_power(Range::all(), 1);
      if (gp_handle_y)
      {
	  gnuplot_resetplot(gp_handle_y);
	  gnuplot_plot_xy(gp_handle_y, 
			  freq.data(), 
			  power.data(), 

			  m_nbeamlets, 
			  "Power per beamlet y-polarization");
      }

#if 1

      // shift the array
      for (int t = N_TIME - 2; t >= 0 ; t--)
      {
	  power_t(Range::all(), t + 1) = power_t(Range::all(), t);
      }

      power_t(Range::all(), 0) = m_beamlet_power(Range::all(), 0);

      snapshot_time++;
      snapshot_time %= N_TIME;

      if (gp_handle_t)
      {
	  gnuplot_resetplot(gp_handle_t);
	  gnuplot_splot(gp_handle_t, 
			power_t,
			"Time-Frequency plane");
      }
#endif

#else
      char propnamex[64];
      char propnamey[64];
      for (int i = 0; i < m_nbeamlets / 8; i++)
      {
	  snprintf(propnamex, 64, "power%03d_x", i);
	  snprintf(propnamey, 64, "power%03d_y", i);

	  //m_pset[i*2  ].setValue(GCFPVDouble(m_beamlet_power(i, 0)));
	  //m_pset[i*2+1].setValue(GCFPVDouble(m_beamlet_power(i, 1)));
	  m_pset[propnamex].setValue(GCFPVDouble(m_beamlet_power(i, 0)));
	  m_pset[propnamey].setValue(GCFPVDouble(m_beamlet_power(i, 1)));
//	  LOG_DEBUG(formatString("Updating statistics properties: %s=%.3f,%s=%.3f",propnamex,m_beamlet_power(i,0),propnamey,m_beamlet_power(i,1)));
      }

      // update the seqnr property
      m_pset["seqnr"].setValue(GCFPVUnsigned(m_seqnr));
#endif

      // reset m_beamlet_power array
      m_beamlet_power = 0.0;
      m_count = 0;
  }
}

bool BeamletStats::isReady()
{
#ifdef USE_GNUPLOT
  return true;
#else
  return m_pset.isLoaded();
#endif
}

void BeamletStats::handleAnswer(GCFEvent& event)
{
  switch (event.signal)
  {
      case F_MYPLOADED:
      {
	  LOG_DEBUG("BeamServer PSet Loaded");
      }
      break;

      case F_VCHANGEMSG:
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
