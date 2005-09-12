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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <Beamlet.h>
#include <Beam.h>

#include <iostream>
#include <queue>

#include <blitz/array.h>

using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace std;

const double SPEED_OF_LIGHT_MS = 299792458.0; // speed of light in meters/sec
const complex<double> I_COMPLEX = complex<double>(0.0,1.0);

Beamlet::Beamlet() : m_subband(0), m_beam(0)
{}

Beamlet::~Beamlet()
{
}

int Beamlet::allocate(const Beam& beam, int subband, int nsubbands)
{
  // don't allow second allocation
  if (m_beam) return -1;

  // check that the subband is within the spectral window
  if ((subband >= nsubbands) || (subband < 0) ) return -1;

  m_subband = subband;
  m_beam    = &beam;

  return 0;
}

int Beamlet::deallocate()
{
  if (!m_beam) return -1;

  m_subband = -1;
  m_beam    = 0;

  return 0;
}

const Beam* Beamlet::getBeam() const
{
  return m_beam;
}

const CAL::SpectralWindow& Beamlet::getSPW() const
{
  ASSERT(m_beam);
  return m_beam->getSPW();
}

Beamlets::Beamlets(int nbeamlets) : m_nbeamlets(nbeamlets)
{
  m_beamlets = new Beamlet[m_nbeamlets];
}

Beamlets::~Beamlets()
{
  delete [] m_beamlets;
}

Beamlet* Beamlets::get(int index) const
{
  if (index < 0 || index >= m_nbeamlets) return 0;

  return m_beamlets + index;
}

void Beamlets::calculate_weights(const Array<double, 3>&         pos,
				      Array<complex<double>, 3>& weights)
{
  const Array<double,2>* lmn = 0;
  int compute_interval = weights.extent(firstDim);
  int nrcus            = weights.extent(secondDim);
  Range all = Range::all();

  if ((weights.extent(thirdDim) != m_nbeamlets)
      || (pos.extent(firstDim) * pos.extent(secondDim) < nrcus)
      || (nrcus % pos.extent(secondDim) != 0))
    {
      LOG_ERROR("\nmismatching pos and weight array shapes\n");
      return;
    }

  for (int bi = 0; bi < m_nbeamlets; bi++)
    {
      Beamlet* beamlet = m_beamlets + bi;
      if (beamlet && beamlet->allocated())
	{
	  const Beam* beam = beamlet->getBeam();
	  const CAL::AntennaGains& gains = beam->getCalibration();

	  LOG_DEBUG_STR("gains[" << (gains.isDone()?"valid":"invalid") << "]=" << gains.getGains().shape());

	  // get coordinates from beam
	  if (!beam)
	    {
	      LOG_ERROR(formatString("\nno beam for beamlet %d?\n", bi));
	      continue;
	    }
	  lmn = &beam->getLMNCoordinates();

	  if (!lmn)
	    {
	      LOG_ERROR(formatString("\ninvalid (l,m,n) vector for beamlet %d\n", bi)); 
	      continue;
	    }

	  if (0 == bi) LOG_DEBUG_STR("lmn[" << bi << "=" << (*lmn));

	  if (compute_interval != lmn->extent(firstDim))
	    {
	      LOG_ERROR(formatString("\nlmn vector length (%d) != compute_interval (%d)\n",
				     lmn->extent(firstDim), compute_interval));
	      continue;
	    }

	  double freq = 0.0;
	  freq = beamlet->getSPW().getSubbandFreq(beamlet->subband());
	  if (0 == bi) LOG_DEBUG_STR("freq = " << freq);

	  //
	  // calculate (xm - yl + zn) for both polarizations
	  // of all elements
	  //
	  for (int rcu = 0; rcu < nrcus; rcu++)
	    {
	      weights(all, rcu, bi) =
	        (pos(rcu / pos.extent(secondDim), rcu % pos.extent(secondDim), 0) * (*lmn)(all, 0))
		+ (pos(rcu / pos.extent(secondDim), rcu % pos.extent(secondDim), 1) * (*lmn)(all, 1))
		+ (pos(rcu / pos.extent(secondDim), rcu % pos.extent(secondDim), 2) * (*lmn)(all, 2));
	    }
	  
	  weights(all, all, bi) =
	    (exp(2.0 * M_PI * freq * complex<double>(0.0,1.0))/SPEED_OF_LIGHT_MS) * weights(all, all, bi);
	}
      else
	{
	  weights(all, all, bi) = complex<double>(0.0, 0.0);
	}
    }

  //cout << "weights(t=0) = " << weights(0,all,0) << endl;

  LOG_DEBUG(formatString("sizeof weights() = %d bytes", weights.size()*sizeof(complex<double>)));
  LOG_DEBUG(formatString("contiguous storage? %s", (weights.isStorageContiguous()?"yes":"no")));
}

