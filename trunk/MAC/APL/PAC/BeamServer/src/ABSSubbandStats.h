//#  ABSSubbandStats.h: interface of the SubbandStats class
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

#ifndef ABSSUBBANDSTATS_H_
#define ABSSUBBANDSTATS_H_

#include <GCF/GCF_RTAnswer.h>
#include <GCF/GCF_Fsm.h>
#include <GCF/GCF_Control.h>
#include <GCF/GCF_RTMyPropertySet.h>
#include <ABSAnswer.h>
#include <blitz/array.h>
#include <complex>

namespace ABS
{

  class SubbandStats : public GCFRTAnswer
      {
      public:
	  /**
	   * Constructor.
	   * @param n_subbands Number of subbands.
	   * @param n_accumulate Number of blocks to accumulate
	   * before updating the property.
	   */
	  SubbandStats(int n_subbands, int n_accumulate);

	  /**
	   * Destructor.
	   */
	  virtual ~SubbandStats();

	  /**
	   * Update statistics from a raw ethernet
	   * frame.
	   */
	  void updateRaw(void* rawdata, size_t count);

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
	  SubbandStats(); // no default constructor

      private:
	  /**
	   * Don't allow copying this object.
	   */
	  SubbandStats (const SubbandStats&); // not implemented
	  SubbandStats& operator= (const SubbandStats&); // not implemented

      private:
	  /**
	   * Maximum number of subbands to accumulate
	   * statistics for.
	   */
	  int                                   m_nsubbands;

	  /**
	   * Total number of block to accumulate before
	   * updating the property.
	   */
	  int                                   m_naccumulate;

	  /**
	   * Current accumulation count.
	   */
	  int                                   m_count;

	  /**
	   * Index of the subband for which statistics
	   * are updated.
	   */
	  int                                   m_selected_subband;

	  /**
	   * Array holding accumulated statistics.
	   */
	  blitz::Array<std::complex<double>, 1> m_subband_stats;

	  /**
	   * The property set.
	   */
	  GCFRTMyPropertySet                    m_pset;

      };

};
     
#endif /* ABSSUBBAND_H_ */
