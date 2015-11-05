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
#ifndef AOFLAGGER_SUMTHRESHOLDTEST_H
#define AOFLAGGER_SUMTHRESHOLDTEST_H

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/strategy/algorithms/mitigationtester.h>
#include <AOFlagger/strategy/algorithms/thresholdconfig.h>
#include <AOFlagger/strategy/algorithms/thresholdmitigater.h>

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

class SumThresholdTest : public UnitTest {
	public:
		SumThresholdTest() : UnitTest("Sumthreshold")
		{
			AddTest(SumThresholdSSE(), "SumThreshold optimized SSE version");
		}
		
	private:
		struct SumThresholdSSE : public Asserter
		{
			void operator()();
		};
};

void SumThresholdTest::SumThresholdSSE::operator()()
{
	const unsigned
		width = 2048,
		height = 256;
	Mask2DPtr
		mask1 = Mask2D::CreateUnsetMaskPtr(width, height),
		mask2 = Mask2D::CreateUnsetMaskPtr(width, height);
	Image2DPtr
		real = MitigationTester::CreateTestSet(26, mask1, width, height),
		imag = MitigationTester::CreateTestSet(26, mask2, width, height);
	TimeFrequencyData data(XXPolarisation, real, imag);
	Image2DCPtr image = data.GetSingleImage();
	
	ThresholdConfig config;
	config.InitializeLengthsDefault(9);
	num_t mode = image->GetMode();
	config.InitializeThresholdsFromFirstThreshold(6.0 * mode, ThresholdConfig::Rayleigh);
	for(unsigned i=0;i<9;++i)
	{
		mask1->SetAll<false>();
		mask2->SetAll<false>();
		
		const unsigned length = config.GetHorizontalLength(i);
		const double threshold = config.GetHorizontalThreshold(i);
		
		ThresholdMitigater::VerticalSumThresholdLargeReference(image, mask1, length, threshold);
		ThresholdMitigater::VerticalSumThresholdLargeSSE(image, mask2, length, threshold);
		
		std::stringstream s;
		s << "Equal SSE and reference masks produced by SumThreshold length " << length;
		AssertTrue(mask1->Equals(mask2), s.str());
	}
}

#endif
