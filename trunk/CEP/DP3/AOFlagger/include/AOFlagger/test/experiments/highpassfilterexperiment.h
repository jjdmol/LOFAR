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
#ifndef AOFLAGGER_HIGHPASSFILTEREXPERIMENT_H
#define AOFLAGGER_HIGHPASSFILTEREXPERIMENT_H

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/strategy/algorithms/highpassfilter.h>
#include <AOFlagger/strategy/algorithms/localfitmethod.h>

#include <AOFlagger/util/rng.h>

class HighPassFilterExperiment : public UnitTest {
	public:
		HighPassFilterExperiment() : UnitTest("Hgh-pass filter experiments")
		{
			AddTest(TimeFitting(), "Timing 'Fitting' algorithm");
			AddTest(TimeHighPassFilter(), "Timing 'high-pass filter' algorithm");
		}
		
	private:
		static void Initialize(Image2DPtr &image, Mask2DPtr &mask)
		{
			const size_t width = 10000, height=256;
			image = Image2D::CreateUnsetImagePtr(width, height);
			mask = Mask2D::CreateUnsetMaskPtr(width, height);
			size_t i=0;
			for(size_t y=0;y<height;++y)
			{
				for(size_t x=0;x<width;++x)
				{
					image->SetValue(x, y, i);
					mask->SetValue(x, y, (i%25==0) || (y==5));
					++i;
				}
			}
		}
		
		struct TimeFitting : public Asserter
		{
			void operator()();
		};
		struct TimeHighPassFilter : public Asserter
		{
			void operator()();
		};
};

inline void HighPassFilterExperiment::TimeFitting::operator()()
{
	Image2DPtr image;
	Mask2DPtr mask;
	Initialize(image, mask);
	
	HighPassFilter filter;
	filter.SetHWindowSize(21);
	filter.SetVWindowSize(41);
	filter.SetHKernelSigma(2.5);
	filter.SetVKernelSigma(5.0);
	Stopwatch watch(true);
	filter.Apply(image, mask);
	std::cout << " time token: " << watch.ToString() << ' ';
}

inline void HighPassFilterExperiment::TimeHighPassFilter::operator()()
{
	Image2DPtr image;
	Mask2DPtr mask;
	Initialize(image, mask);
	
	LocalFitMethod fitMethod;
	TimeFrequencyData data(TimeFrequencyData::AmplitudePart, StokesIPolarisation, image);
	fitMethod.SetToWeightedAverage(10, 20, 2.5, 5.0);
	fitMethod.Initialize(data);
	Stopwatch watch(true);
	for(size_t i=0;i<fitMethod.TaskCount();++i)
		fitMethod.PerformFit(i);
	std::cout << " time token: " << watch.ToString() << ' ';
}

#endif
