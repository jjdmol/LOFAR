//#
//#  BeamServerTest.cc: class definition for the Beam Server task.
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

#include "ABSSpectralWindow.h"
#include "ABSBeam.h"

#include "test.h"
#include "suite.h"

#include <iostream>

#define N_BEAMS              (256)
#define N_BEAMLETS           (256)
#define N_SUBBANDS_PER_BEAM  (N_BEAMLETS/N_BEAMS)

using namespace ABS;
using namespace std;

class BeamServerTest : public Test
{
private:
    Beam*          m_beam[N_BEAMS];
    SpectralWindow m_spw;

public:
    void setUp()
	{
	  //cerr << "s";
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      m_beam[i] = Beam::getInstance(i);
	      _test(m_beam[i] != 0);
	  }
	}

    void tearDown()
	{
	  //cerr << "t";
	  // nothing needed
	}

    void run()
	{
	  setUp();
	  
	  allocate();
	  deallocate();
	  subbandSelection();
	  oneTooManyBeam();
	  oneTooManyBeamlet();
	  emptyBeam();
	  doubleAllocate();
	  pointing();

	  tearDown();
	}

    BeamServerTest() :
	Test("BeamServerTest"),
	m_spw(10e6, 256*1e3, 80*(1000/256))
	{
	  //cerr << "c";
	  Beam::setNInstances(N_BEAMS);
	  Beamlet::setNInstances(N_BEAMLETS);
	}

    void allocate()
	{
	  // create a spectral window from 10MHz to 90Mhz
	  // steps of 256kHz
	  //SpectralWindow spw(10e6, 256*1e3, 80*(1000/256));

	  set<int> subbands;

	  subbands.clear();
	  for (int i = 0; i < N_SUBBANDS_PER_BEAM; i++)
	  {
	      subbands.insert(i);
	  }
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      _test(m_beam[i]->allocate(m_spw, subbands) == 0);
	  }
	}

    void deallocate()
	{
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      _test(m_beam[i]->deallocate() == 0);
	  }
	}

    void subbandSelection()
	{
	  allocate();

	  map<int,int>   selection;
	  selection.clear();
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      m_beam[i]->getSubbandSelection(selection);
	  }

	  _test(selection.size() == N_BEAMS*N_SUBBANDS_PER_BEAM);

	  deallocate();
	}

    void oneTooManyBeam()
	{
	  // try to get N_BEAMS instance, should fail
	  _test(Beam::getInstance(N_BEAMS) == 0);
	 
	  // try to get -1 instance, should fail
	  _test(Beam::getInstance(-1) == 0);
	}

    void oneTooManyBeamlet()
	{
	  // insert one too many subbands
	  set<int> subbands;
	  subbands.clear();
	  for (int i = 0; i < N_BEAMLETS + 1; i++)
	  {
	      subbands.insert(i);
	  }
	  _test(m_beam[0]->allocate(m_spw, subbands) < 0);
	}

    void emptyBeam()
	{
	  set<int> subbands;
	  subbands.clear();
	  _test(m_beam[0]->allocate(m_spw, subbands) == 0);
	  _test(m_beam[0]->deallocate() == 0);
	}

    void doubleAllocate()
	{
	  set<int> subbands;
	  subbands.clear();
	  _test(m_beam[0]->allocate(m_spw, subbands) == 0);
	  _test(m_beam[0]->allocate(m_spw, subbands) < 0);
	  _test(m_beam[0]->deallocate() == 0);
	}

    void pointing()
	{
	  set<int> subbands;
	  subbands.clear();
	  struct timeval t = {0,0};

	  // allocate beam, addPointing, should succeed
	  _test(m_beam[0]->allocate(m_spw, subbands) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0, Direction::J2000), t)) == 0);

	  // deallocate beam, addPointing, should fail
	  _test(m_beam[0]->deallocate() == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0, Direction::J2000), t)) < 0);
	}
};

int main(int /*argc*/, char** /*argv*/)
{
  Suite s("Beam Server Test Suite", &cout);

  s.addTest(new BeamServerTest);
  s.run();
  long nFail = s.report();
  s.free();
  return nFail;
}
