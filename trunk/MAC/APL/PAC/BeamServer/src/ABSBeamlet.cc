//#  ABSBeamlet.h: implementation of the Beamlet class
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

#include <ABSBeamlet.h>
#include <ABSBeam.h>

#include <iostream>
#include <queue>

#include <blitz/array.h>
using namespace blitz;

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::gregorian;
using namespace boost::posix_time;

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace ABS;
using namespace std;

int      Beamlet::m_ninstances = 0;
Beamlet* Beamlet::m_beamlets   = 0;

const W_TYPE SPEED_OF_LIGHT_MS = 299792458.0; // speed of light in meters/sec
const complex<W_TYPE> I_COMPLEX = complex<W_TYPE>(0.0,1.0);

Beamlet::Beamlet() :
    m_spw(0), m_subband(0), m_index(-1), m_beam(0)
{}

Beamlet::~Beamlet()
{
  if (m_beamlets) delete [] m_beamlets;
  m_ninstances = 0;
}

Beamlet* Beamlet::getInstance()
{
  // if not yet initialised, just return 0
  if (!m_beamlets) return 0;

  for (int i = 0; i < m_ninstances; i++)
  {
      if (!m_beamlets[i].allocated())
	  return &m_beamlets[i];
  }

  // otherwise return 0
  return 0;
}

int Beamlet::init(int ninstances)
{
  // if already initialised just return
  // only one initialisation is allowed
  if (m_beamlets) return -1;

  m_beamlets = new Beamlet[ninstances];
  if (!m_beamlets) return -1;
  m_ninstances = ninstances;

  for (int i = 0; i < m_ninstances; i++)
  {
      m_beamlets[i].m_index = i;
  }

  return (m_beamlets ? 0 : -1);
}

int Beamlet::allocate(const Beam& beam,
		      SpectralWindow const& spw, int subband)
{
  // don't allow second allocation
  if (m_beam) return -1;

  // check that the subband is within the spectral window
  if ((subband >= spw.nsubbands()) 
      || (subband < 0) ) return -1;

  m_spw     = &spw;
  m_subband = subband;
  m_beam = &beam;

  return 0;
}

int Beamlet::deallocate()
{
  if (!m_beam) return -1;

  m_spw     = 0;
  m_subband = -1;
  m_beam    = 0;

  return 0;
}

const Beam* Beamlet::getBeam() const
{
  return m_beam;
}

void Beamlet::calculate_weights(const Array<W_TYPE, 3>&          pos,
				      Array<complex<W_TYPE>, 4>& weights)
{
  const Array<W_TYPE,2>* lmn = 0;
  int compute_interval = weights.extent(firstDim);
  int nelements        = weights.extent(secondDim);
  int npolarizations   = weights.extent(fourthDim);
  Range all = Range::all();

  if ((weights.extent(thirdDim) != m_ninstances)
      || (pos.extent(firstDim) != nelements)
      || (pos.extent(secondDim) != npolarizations))
  {
      LOG_ERROR("mismatching pos and weight array shapes");
      return;
  }

  for (int bi = 0; bi < m_ninstances; bi++)
  {
      Beamlet* beamlet = &m_beamlets[bi];
      if (beamlet && beamlet->allocated())
      {
	  const Beam* beam = beamlet->getBeam();

	  // get coordinates from beam
	  if (!beam)
	  {
	      LOG_ERROR(formatString("no beam for beamlet %d?", beamlet->m_index));
	      continue;
	  }
	  lmn = &beam->getLMNCoordinates();

	  if (!lmn)
	  {
	      LOG_ERROR(formatString("invalid (l,m,n) vector for beamlet %d", beamlet->m_index));
	      continue;
	  }

	  LOG_DEBUG_STR("lmn(t=0)=" << (*lmn)(0,all));

	  if (compute_interval != lmn->extent(firstDim))
	  {
	      LOG_ERROR(formatString("lmn vector length (%d) != compute_interval (%d)",
				     lmn->extent(firstDim), compute_interval));
	      continue;
	  }

	  W_TYPE freq = 0.0;
	  if (beamlet->spw())
	  {
	      freq = beamlet->spw()->getFrequency(beamlet->subband());
	  }

  	  for (int si = 0; si < nelements; si++)
	  {
	    //
	    // calculate (xm - yl + zn) for both polarizations
	    // of all elements
	    //
	    
	    for (int pol = 0; pol < npolarizations; pol++)
	    {
	      weights(all, si, bi, pol) =
		(pos(si, pol, 0) * (*lmn)(all, 1))
		- (pos(si, pol, 1) * (*lmn)(all, 0))
		+ (pos(si, pol, 2) * (*lmn)(all, 2));
	    }
	  }

#if 1
	  weights(all, all, bi, all) =
	      exp((I_COMPLEX * ((W_TYPE)2.0) * ((W_TYPE)M_PI) * freq) * weights(all, all, bi, all) / SPEED_OF_LIGHT_MS);
#endif
      }
      else
      {
	  weights(all, all, bi, all) = complex<W_TYPE>(0.0, 0.0);
      }
  }
  
  //cout << "M0(t=0) = " << weights(0, all, 0, 0) << endl;

#if 0
  for (int bi = 0; bi < m_ninstances; bi++)
  {
    Beamlet* beamlet = &m_beamlets[bi];
    if (beamlet && beamlet->allocated())
    {
      W_TYPE freq = 0.0;
      if (beamlet->spw())
	{
	  freq = beamlet->spw()->getFrequency(beamlet->subband());
	}
      weights(all, all, bi, all) =
	exp((I_COMPLEX * ((W_TYPE)2.0) * ((W_TYPE)M_PI) * freq) * weights(all, all, bi, all) / SPEED_OF_LIGHT_MS);
    }
  }
#endif

  //cout << "weights(t=0) = " << weights(0,all,0,0) << endl;

  LOG_DEBUG(formatString("sizeof weights() = %d bytes", weights.size()*sizeof(complex<W_TYPE>)));
  LOG_DEBUG(formatString("contiguous storage? %s", (weights.isStorageContiguous()?"yes":"no")));
}

