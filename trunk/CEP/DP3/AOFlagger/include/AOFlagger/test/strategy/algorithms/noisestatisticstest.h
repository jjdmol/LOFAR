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
#ifndef AOFLAGGER_NOISESTATISTICSTEST_H
#define AOFLAGGER_NOISESTATISTICSTEST_H

#include <AOFlagger/test/testingtools/testfunctor.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/msio/mask2d.h>

#include <AOFlagger/strategy/algorithms/noisestatistics.h>

class NoiseStatisticsTest : public UnitTest {
	public:
		NoiseStatisticsTest() : UnitTest("Noise statistics")
		{
			AddTest(TestInitialization(), "Initialization");
			AddTest(TestCalculations(), "Calculations");
		}
		
	private:
		struct TestInitialization : public TestFunctor
		{
			void operator()();
		};
		
		struct TestCalculations : public TestFunctor
		{
			void operator()();
		};
		
		static void AssertValues(
			const NoiseStatistics &statistics,
			const TestFunctor *testFunctor,
			long unsigned count,
			NoiseStatistics::stat_t sum,
			NoiseStatistics::stat_t sum2,
			NoiseStatistics::stat_t sum3,
			NoiseStatistics::stat_t sum4)
		{
			testFunctor->AssertEquals(statistics.Count(), count, "Count()");
			testFunctor->AssertEquals(statistics.Sum(), sum, "Sum()");
			testFunctor->AssertEquals(statistics.Sum2(), sum2, "Sum2()");
			testFunctor->AssertEquals(statistics.Sum3(), sum3, "Sum3()");
			testFunctor->AssertEquals(statistics.Sum4(), sum4, "Sum4()");
		}

		static void AssertValues(
			const NoiseStatistics &statistics,
			const TestFunctor *testFunctor,
			long unsigned count,
			NoiseStatistics::stat_t sum,
			NoiseStatistics::stat_t sum2,
			NoiseStatistics::stat_t sum3,
			NoiseStatistics::stat_t sum4,
			NoiseStatistics::stat_t mean,
			NoiseStatistics::stat_t moment2,
			NoiseStatistics::stat_t moment4,
			NoiseStatistics::stat_t varianceEst,
			NoiseStatistics::stat_t varianceOfVarianceEst)
		{
			AssertValues(statistics, testFunctor, count, sum, sum2, sum3, sum4);
			testFunctor->AssertEquals(statistics.Mean(), mean, "Mean()");
			testFunctor->AssertEquals(statistics.SecondMoment(), moment2, "SecondMoment()");
			testFunctor->AssertEquals(statistics.FourthMoment(), moment4, "FourthMoment()");
			testFunctor->AssertEquals(statistics.VarianceEstimator(), varianceEst, "VarianceEstimator()");
			testFunctor->AssertEquals(statistics.VarianceOfVarianceEstimator(), varianceOfVarianceEst, "VarianceOfVarianceEstimator()");
		}
		
	static void AssertRunnable(const NoiseStatistics &statistics)
	{
		statistics.Count();
		statistics.Sum();
		statistics.Sum2();
		statistics.Sum3();
		statistics.Sum4();
		statistics.Mean();
		statistics.SecondMoment();
		statistics.FourthMoment();
		statistics.VarianceEstimator();
		statistics.VarianceOfVarianceEstimator();
	}
};

inline void NoiseStatisticsTest::TestInitialization::operator()()
{
	// Test without initialization
	NoiseStatistics statistics;
	AssertValues(statistics, this, 0, 0.0, 0.0, 0.0, 0.0);
	// Some values are undefined, but should not throw an exception:
	AssertRunnable(statistics);
	
	// Test assignment + initialization with an array
	NoiseStatistics::Array array;
	array.push_back(1.0);
	statistics = NoiseStatistics(array);
	AssertValues(statistics, this, 1, 1.0, 1.0, 1.0, 1.0);
	AssertRunnable(statistics);
	
	// Test copy constructor
	NoiseStatistics copy(statistics);
	AssertValues(statistics, this, 1, 1.0, 1.0, 1.0, 1.0);
	AssertRunnable(statistics);
}

inline void NoiseStatisticsTest::TestCalculations::operator()()
{
	NoiseStatistics::Array array;
	array.push_back(1.0);
	array.push_back(2.0);
	array.push_back(3.0);
	NoiseStatistics statistics(array);
	AssertValues(statistics, this, 3, 6.0, 14.0, 36.0, 98.0);
	AssertEquals(statistics.Mean(), 2.0, "Mean()");
	AssertEquals(statistics.SecondMoment(), 2.0/3.0, "SecondMoment()");
	AssertEquals(statistics.FourthMoment(), 2.0/3.0, "FourthMoment()");
}

#endif
