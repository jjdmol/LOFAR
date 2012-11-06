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
#ifndef AOFLAGGER_ALGORITHMSTESTGROUP_H
#define AOFLAGGER_ALGORITHMSTESTGROUP_H

#include <AOFlagger/test/testingtools/testgroup.h>

#include <AOFlagger/test/strategy/algorithms/convolutionstest.h>
#include <AOFlagger/test/strategy/algorithms/dilationtest.h>
#include <AOFlagger/test/strategy/algorithms/eigenvaluetest.h>
#include <AOFlagger/test/strategy/algorithms/highpassfiltertest.h>
#include <AOFlagger/test/strategy/algorithms/noisestatisticstest.h>
#include <AOFlagger/test/strategy/algorithms/noisestatisticscollectortest.h>
#include <AOFlagger/test/strategy/algorithms/siroperatortest.h>
#include <AOFlagger/test/strategy/algorithms/statisticalflaggertest.h>
#include <AOFlagger/test/strategy/algorithms/sumthresholdtest.h>
#include <AOFlagger/test/strategy/algorithms/thresholdtoolstest.h>

class AlgorithmsTestGroup : public TestGroup {
	public:
		AlgorithmsTestGroup() : TestGroup("Algorithms") { }
		
		virtual void Initialize()
		{
			Add(new ConvolutionsTest());
			Add(new DilationTest());
			Add(new EigenvalueTest());
			Add(new HighPassFilterTest());
			Add(new NoiseStatisticsTest());
			Add(new NoiseStatisticsCollectorTest());
			Add(new SIROperatorTest());
			Add(new StatisticalFlaggerTest());
			Add(new SumThresholdTest());
			Add(new ThresholdToolsTest());
		}
};

#endif
