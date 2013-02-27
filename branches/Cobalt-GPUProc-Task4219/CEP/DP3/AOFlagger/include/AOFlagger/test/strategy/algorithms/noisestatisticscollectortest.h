/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef AOFLAGGER_NOISESTATISTICSCOLLECTORTEST_H
#define AOFLAGGER_NOISESTATISTICSCOLLECTORTEST_H

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/msio/mask2d.h>

#include <AOFlagger/strategy/algorithms/noisestatisticscollector.h>

class NoiseStatisticsCollectorTest : public UnitTest {
	public:
		NoiseStatisticsCollectorTest() : UnitTest("Noise statistics collector")
		{
			AddTest(InitializationTest(), "Initialization");
		}
		
	private:
		struct InitializationTest : public Asserter
		{
			void operator()();
		};
};


void NoiseStatisticsCollectorTest::InitializationTest::operator()()
{
	NoiseStatisticsCollector collector;
	AssertTrue(collector.Empty(), "Empty()");

	AssertEquals(collector.TBMap().size(), (size_t) 0, "TBMap().size()");
}

#endif
