//#  SpectralWindowConfig.h: interface of the SpectralWindow class
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

#include "SpectralWindowConfig.h"
#include "ABSSpectralWindow.h"
#include "PSAccess.h"

#include <sstream>
#include <math.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <blitz/array.h>

using namespace LOFAR;
using namespace blitz;
using namespace ABS;

#define N_SPW_PARAMETERS     6 /* number of parameters per spectral window */
#define N_SPW_PARAMETERS_DIM 2 /* # dimensions of the parameters array     */

SpectralWindowConfig* SpectralWindowConfig::m_config = 0;

SpectralWindowConfig::SpectralWindowConfig()
  : m_spws(0), m_n_spws(0), m_refcount(0), m_current(-1)
{
}

SpectralWindowConfig::~SpectralWindowConfig()
{
  if (m_spws)     delete [] m_spws;
  if (m_refcount) delete [] m_refcount;
  m_n_spws = 0;
}

SpectralWindowConfig& SpectralWindowConfig::getInstance()
{
  if (!m_config)
  {
    m_config = new SpectralWindowConfig();
  }
  
  return *m_config;
}

bool SpectralWindowConfig::load()
{
  // to reload, no spectral window must be in use
  if (m_refcount)
  {
    int sum = 0;
    for (int i; i < m_n_spws; i++) sum += m_refcount[i];
    
    if (sum > 0)
    {
      LOG_ERROR("Load failed, some spectral windows still in use.");
      return false;
    }
  }
  
  Array<double, N_SPW_PARAMETERS_DIM> params;
    
  istringstream config(GET_CONFIG_STRING("BeamServer.SpectralWindows"));
  config >> params;

  LOG_INFO_STR("BeamServer.SpectralWindows=" << params);

  m_n_spws = params.extent(firstDim);
  if ( (params.extent(secondDim) != N_SPW_PARAMETERS)
       || (m_n_spws < 0))
  {
    LOG_ERROR_STR("Invalid BeamServer.SpectralWindows array. Requires "
		  << N_SPW_PARAMETERS
		  << " parameters per spectral window.");
    return false;
  }

  if (m_spws) delete [] m_spws; // free previous
  m_spws = new SpectralWindow*[m_n_spws];

  for (int i = 0; i < m_n_spws; i++)
  {
    double sample_freq  = params(i,0);
    double nyquist_zone = params(i, 1);
    int    nsubbands    = int(floor(params(i, 2)));
    //double cutoff_low   = params(i, 3);
    //double cutoff_high  = params(i, 4);
    uint8  rcusettings  = uint8(params(i, 5));
    
    m_spws[i] = new SpectralWindow((sample_freq/2.0) * (nyquist_zone - 1),
				   (sample_freq/2.0) / nsubbands,
				   nsubbands, rcusettings);
  }

  if (m_refcount) delete [] m_refcount;
  m_refcount = new int[m_n_spws];
  for (int i = 0; i < m_n_spws; i++) m_refcount[i] = 0;

  return true;
}

const SpectralWindow* SpectralWindowConfig::get(int index) const
{
  if (index < 0 || index >= m_n_spws) return 0;
  return m_spws[index];
}

int SpectralWindowConfig::incRef(int index)
{
  if (!m_refcount
      || index < 0
      || index >= m_n_spws) return -1;

  if (0 == m_refcount[index])
  {
    int sum = 0;
    for (int i=0; i < m_n_spws; i++) sum += m_refcount[i];
    if (sum)
    {
      LOG_ERROR_STR("incRef failed for index=" << index << ", reason: other window still in use.");
      return -1;
    }

    m_current = index; // first increment, spw 'index' is now current
  }
      
  return ++m_refcount[index];
}

int SpectralWindowConfig::decRef()
{
  int index = getCurrent();
  if (!m_refcount 
      || index < 0
      || index >= m_n_spws
      || m_refcount[index] == 0) return -1;

  int result = --m_refcount[index];

  if (0 == result) m_current = -1; // last decrease.
  
  return result;
}


