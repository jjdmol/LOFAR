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
#ifndef AOFLAGGER_HIGHPASSFILTERTEST_H
#define AOFLAGGER_HIGHPASSFILTERTEST_H

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>
#include <AOFlagger/test/testingtools/imageasserter.h>

#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/image2d.h>

#include <AOFlagger/strategy/algorithms/localfitmethod.h>
#include <AOFlagger/strategy/algorithms/highpassfilter.h>

class HighPassFilterTest : public UnitTest {
	public:
		HighPassFilterTest() : UnitTest("High-pass filter algorithm")
		{
			AddTest(TestFilter(), "High-pass filter algorithm");
		}
		
	private:
		struct TestFilter : public Asserter
		{
			void operator()();
		};
};

inline void HighPassFilterTest::TestFilter::operator()()
{
	const size_t width = 100, height = 100;
	Image2DPtr testImage = Image2D::CreateZeroImagePtr(width, height);
	testImage->SetValue(10,10,1.0);
	testImage->SetValue(15,15,2.0);
	testImage->SetValue(20,20,0.5);
	
	// Fitting
	LocalFitMethod fitMethod;
	TimeFrequencyData data(TimeFrequencyData::AmplitudePart, StokesIPolarisation, Image2D::CreateCopy(testImage));
	fitMethod.SetToWeightedAverage(10, 20, 2.5, 5.0);
	fitMethod.Initialize(data);
	for(size_t i=0;i<fitMethod.TaskCount();++i)
		fitMethod.PerformFit(i);
	Image2DCPtr fitResult = Image2D::CreateFromDiff(testImage, fitMethod.Background().GetSingleImage());
	
	// High-pass filter
	HighPassFilter filter;
	Image2DPtr filterResult = Image2D::CreateCopy(testImage);
	filter.SetHWindowSize(21);
	filter.SetVWindowSize(41);
	filter.SetHKernelSigma(2.5);
	filter.SetVKernelSigma(5.0);
	filterResult = filter.Apply(filterResult, Mask2D::CreateSetMaskPtr<false>(width, height));
	
	ImageAsserter::AssertEqual(filterResult, fitResult, "Simple convolution with three high values");
}

#endif
