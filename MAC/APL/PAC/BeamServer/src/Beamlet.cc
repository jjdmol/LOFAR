//#  Beamlet.h: implementation of the Beamlet class
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

#include <iostream>
#include <queue>
#include <blitz/array.h>

#include "BeamServerConstants.h"
#include "Beamlet.h"
#include "Beam.h"

#undef NDIM
#define NDIM 3

using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace std;

//
// Beamlet()
//
Beamlet::Beamlet() : m_subband(0), m_beam(0)
{}

//
// Beamlet~()
//
Beamlet::~Beamlet()
{
}

//
// allocate(beam, subband, nrSubbands)
//
int Beamlet::allocate(const Beam& beam, int subband, int nsubbands)
{
	// don't allow second allocation
	if (m_beam) {
		return (-1);
	}

	// check that the subband is within the spectral window
	if ((subband >= nsubbands) || (subband < 0)) {
		return (-1);
	}

	m_subband = subband;
	m_beam    = &beam;

	return (0);
}

//
// dellocate()
//
int Beamlet::deallocate()
{
	if (!m_beam) {
		return (-1);
	}

	m_subband = -1;
	m_beam    = 0;

	return (0);
}

//
// getBeam()
//
const Beam* Beamlet::getBeam() const
{
	return (m_beam);
}

//
// getSPW()
//
const CAL::SpectralWindow& Beamlet::getSPW() const
{
	ASSERTSTR(m_beam, "Beamlet not attached to a beam");

	return (m_beam->getSPW());
}
