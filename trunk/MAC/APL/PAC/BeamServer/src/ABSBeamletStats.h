//#  ABSBeamletStats.h: interface of the BeamletStats class
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

#ifndef ABSBEAMLETSTATS_H_
#define ABSBEAMLETSTATS_H_

#include <GCF/GCF_RTAnswer.h>
#include <GCF/GCF_Fsm.h>
#include <GCF/GCF_Control.h>
#include <GCF/GCF_RTMyPropertySet.h>
#include <blitz/array.h>
#include <complex>

namespace ABS
{

  class BeamletStats : public GCFRTAnswer
      {
      public:
	  /**
	   * Constructor.
	   * @param n_beamlets Number of beamlets.
	   * @param n_integrations Number of blocks to integrate.
	   * before updating the property.
	   */
	  BeamletStats(int n_beamlets, int n_integrations);

	  /**
	   * Destructor.
	   */
	  virtual ~BeamletStats();

	  /**
	   * Update statistics from a raw ethernet frame.
	   * power_sum has dimensions: N_BEAMLETS / 2, N_POL, 2
	   */
	  void update(blitz::Array<unsigned int, 3>& power_sum, unsigned int seqnr);

	  /**
	   * @return true if the property set is loaded.
	   */
	  bool isReady();

	  /**
	   * Handle the asynchronous answer from the
	   * property agent.
	   */
	  void handleAnswer(GCFEvent& event);

      protected:
	  BeamletStats(); // no default constructor

      private:
	  /**
	   * Don't allow copying this object.
	   */
	  BeamletStats (const BeamletStats&); // not implemented
	  BeamletStats& operator= (const BeamletStats&); // not implemented

      private:
	  /**
	   * Maximum number of beamlets to integrate
	   * statistics for.
	   */
	  int                                   m_nbeamlets;

	  /**
	   * Total number of block to integrate before
	   * updating the property.
	   */
	  int                                   m_nintegrations;

	  /**
	   * Current integration count.
	   */
	  int                                   m_count;

	  /**
	   * Current sequence number.
	   */
	  unsigned int                          m_seqnr;

	  /**
	   * Index of the beamlet for which statistics
	   * are updated.
	   */
	  int                                   m_selected_beamlet;

	  /**
	   * Array holding integrated power statistic.
	   * Dimensions m_beamlet_power(first, second)
	   * first  = n_subbands
	   * second = 2 -- x and y polarization.
	   */
	  blitz::Array<double, 2>               m_beamlet_power;

	  /**
	   * Array holding power_sum data from last packet
	   * received.
	   */
	  blitz::Array<unsigned int, 3>         m_power_sum;

	  /**
	   * The property set.
	   */
	  GCFRTMyPropertySet                    m_pset;
      };

};
     
#endif /* ABSBEAMLET_H_ */
