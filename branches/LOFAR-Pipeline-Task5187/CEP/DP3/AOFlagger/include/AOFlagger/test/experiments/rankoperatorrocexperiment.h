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
#ifndef AOFLAGGER_RANKOPERATORROCEXPERIMENT_H
#define AOFLAGGER_RANKOPERATORROCEXPERIMENT_H

#include <iostream>

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/strategy/algorithms/mitigationtester.h>
#include <AOFlagger/strategy/algorithms/scaleinvariantdilation.h>
#include <AOFlagger/strategy/algorithms/statisticalflagger.h>
#include <AOFlagger/strategy/algorithms/thresholdtools.h>

#include <AOFlagger/msio/pngfile.h>

#include <AOFlagger/util/rng.h>

#include <AOFlagger/strategy/actions/changeresolutionaction.h>
#include <AOFlagger/strategy/actions/foreachcomplexcomponentaction.h>
#include <AOFlagger/strategy/actions/iterationaction.h>
#include <AOFlagger/strategy/actions/setflaggingaction.h>
#include <AOFlagger/strategy/actions/setimageaction.h>
#include <AOFlagger/strategy/actions/strategyaction.h>
#include <AOFlagger/strategy/actions/sumthresholdaction.h>
#include <AOFlagger/strategy/actions/slidingwindowfitaction.h>

class RankOperatorROCExperiment : public UnitTest {
	public:
		RankOperatorROCExperiment() : UnitTest("Rank operator ROC experiments")
		{
			AddTest(RankOperatorROCGaussian(), "Constructing rank operator & dilation ROC curve for Gaussian broadband RFI");
			AddTest(RankOperatorROCSinusoidal(), "Constructing rank operator & dilation ROC curve for sinusoidal broadband RFI");
		}
		
	private:
		struct RankOperatorROCGaussian : public Asserter
		{
			void operator()();
		};
		struct RankOperatorROCSinusoidal : public Asserter
		{
			void operator()();
		};
		
		static const unsigned _repeatCount;
		
		enum TestType { GaussianBroadband, SinusoidalBroadband};
		
		static void TestNoisePerformance(size_t totalRFI, double totalRFISum, const std::string &testname);
		
		static rfiStrategy::Strategy *createThresholdStrategy();
		static void executeTest(enum TestType testType);
		
		static num_t getRatio(Image2DPtr groundTruth, Mask2DCPtr resultMask, bool inverseTruth, bool invertMask)
		{
			num_t totalTruth = groundTruth->Sum();
			if(inverseTruth)
			{
				totalTruth = groundTruth->Width() * groundTruth->Height() - totalTruth;
			}
			num_t sum = 0.0;
			for(size_t y=0;y<groundTruth->Height();++y)
			{
				for(size_t x=0;x<groundTruth->Width();++x)
				{
					num_t truth = inverseTruth ? (1.0 - groundTruth->Value(x, y)) : groundTruth->Value(x, y);
					if(resultMask->Value(x, y) != invertMask)
					{
						sum += truth;
					}
				}
			}
			return sum / totalTruth;
		}
};

const unsigned RankOperatorROCExperiment::_repeatCount = 2;

inline rfiStrategy::Strategy *RankOperatorROCExperiment::createThresholdStrategy()
{
	rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
	
	rfiStrategy::ActionBlock *current = strategy;

	current->Add(new rfiStrategy::SetFlaggingAction());

	rfiStrategy::ForEachComplexComponentAction *focAction = new rfiStrategy::ForEachComplexComponentAction();
	focAction->SetOnAmplitude(true);
	focAction->SetOnImaginary(false);
	focAction->SetOnReal(false);
	focAction->SetOnPhase(false);
	focAction->SetRestoreFromAmplitude(false);
	current->Add(focAction);
	current = focAction;

	rfiStrategy::IterationBlock *iteration = new rfiStrategy::IterationBlock();
	iteration->SetIterationCount(2);
	iteration->SetSensitivityStart(4.0);
	current->Add(iteration);
	current = iteration;
	
	rfiStrategy::SumThresholdAction *t2 = new rfiStrategy::SumThresholdAction();
	t2->SetBaseSensitivity(1.0);
	current->Add(t2);

	current->Add(new rfiStrategy::SetImageAction());
	rfiStrategy::ChangeResolutionAction *changeResAction2 = new rfiStrategy::ChangeResolutionAction();
	changeResAction2->SetTimeDecreaseFactor(3);
	changeResAction2->SetFrequencyDecreaseFactor(3);

	rfiStrategy::SlidingWindowFitAction *swfAction2 = new rfiStrategy::SlidingWindowFitAction();
	swfAction2->Parameters().timeDirectionKernelSize = 2.5;
	swfAction2->Parameters().timeDirectionWindowSize = 10;
	swfAction2->Parameters().frequencyDirectionKernelSize = 5.0;
	swfAction2->Parameters().frequencyDirectionWindowSize = 15;
	changeResAction2->Add(swfAction2);

	current->Add(changeResAction2);
	
	current = focAction;
	rfiStrategy::SumThresholdAction *t3 = new rfiStrategy::SumThresholdAction();
	current->Add(t3);
	
	return strategy;
}

inline void RankOperatorROCExperiment::RankOperatorROCGaussian::operator()()
{
	executeTest(GaussianBroadband);
}

inline void RankOperatorROCExperiment::RankOperatorROCSinusoidal::operator()()
{
	executeTest(SinusoidalBroadband);
}

void RankOperatorROCExperiment::executeTest(enum TestType testType)
{
	const size_t ETA_STEPS = 100, DIL_STEPS = 128;
	const size_t width = 1024, height = 1024;
	
	std::string testname;
	switch(testType)
	{
		case GaussianBroadband:
			testname = "gaussian";
			break;
		case SinusoidalBroadband:
			testname = "sinusoidal";
			break;
	}
	
	numl_t
		grRankTpRatio[ETA_STEPS+1], grRankFpRatio[ETA_STEPS+1],
		grRankTpSum[ETA_STEPS+1], grRankFpSum[ETA_STEPS+1],
		grDilTpRatio[DIL_STEPS+1], grDilFpRatio[DIL_STEPS+1],
		grDilTpSum[DIL_STEPS+1], grDilFpSum[DIL_STEPS+1];
	for(unsigned i=0;i<ETA_STEPS+1;++i)
	{
		grRankTpRatio[i] = 0.0;
		grRankFpRatio[i] = 0.0;
		grRankTpSum[i] = 0.0;
		grRankFpSum[i] = 0.0;
	}
	for(unsigned i=0;i<DIL_STEPS;++i)
	{
		grDilTpRatio[i] = 0.0;
		grDilFpRatio[i] = 0.0;
		grDilTpSum[i] = 0.0;
		grDilFpSum[i] = 0.0;
	}
	
	for(unsigned repeatIndex=0 ; repeatIndex<_repeatCount ; ++repeatIndex)
	{
		Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(width, height);
		Image2DPtr
			groundTruth = Image2D::CreateZeroImagePtr(width, height);
		Image2DPtr realImage, imagImage;
		Image2DCPtr rfiLessImage;
		TimeFrequencyData rfiLessData, data;
		switch(testType)
		{
			case GaussianBroadband:
			{
				Image2DPtr
					realTruth  = Image2D::CreateZeroImagePtr(width, height),
					imagTruth  = Image2D::CreateZeroImagePtr(width, height);
				realImage = MitigationTester::CreateTestSet(2, mask, width, height),
				imagImage = MitigationTester::CreateTestSet(2, mask, width, height);
				rfiLessData = TimeFrequencyData(SinglePolarisation, realImage, imagImage);
				rfiLessData.Trim(0, 0, 180, height);
				rfiLessImage = rfiLessData.GetSingleImage();
				MitigationTester::AddGaussianBroadbandToTestSet(realImage, mask);
				MitigationTester::AddGaussianBroadbandToTestSet(imagImage, mask);
				MitigationTester::AddGaussianBroadbandToTestSet(realTruth, mask);
				MitigationTester::AddGaussianBroadbandToTestSet(imagTruth, mask);
				groundTruth = Image2D::CreateCopy(TimeFrequencyData(SinglePolarisation, realTruth, imagTruth).GetSingleImage());
				data = TimeFrequencyData(SinglePolarisation, realImage, imagImage);
			} break;
			case SinusoidalBroadband:
			{
				Image2DPtr
					realTruth  = Image2D::CreateZeroImagePtr(width, height),
					imagTruth  = Image2D::CreateZeroImagePtr(width, height);
				realImage = MitigationTester::CreateTestSet(2, mask, width, height),
				imagImage = MitigationTester::CreateTestSet(2, mask, width, height);
				rfiLessData = TimeFrequencyData(SinglePolarisation, realImage, imagImage);
				rfiLessData.Trim(0, 0, 180, height);
				rfiLessImage = rfiLessData.GetSingleImage();
				MitigationTester::AddSinusoidalBroadbandToTestSet(realImage, mask);
				MitigationTester::AddSinusoidalBroadbandToTestSet(imagImage, mask);
				MitigationTester::AddSinusoidalBroadbandToTestSet(realTruth, mask);
				MitigationTester::AddSinusoidalBroadbandToTestSet(imagTruth, mask);
				groundTruth = Image2D::CreateCopy(TimeFrequencyData(SinglePolarisation, realTruth, imagTruth).GetSingleImage());
				data = TimeFrequencyData(SinglePolarisation, realImage, imagImage);
			} break;
		}
		
		data.Trim(0, 0, 180, height);
		groundTruth->SetTrim(0, 0, 180, height);
		groundTruth->MultiplyValues(1.0/groundTruth->GetMaximum());
		Image2DCPtr inputImage = data.GetSingleImage();
		
		rfiStrategy::ArtifactSet artifacts(0);
		
		artifacts.SetOriginalData(data);
		artifacts.SetContaminatedData(data);
		data.SetImagesToZero();
		artifacts.SetRevisedData(data);

		rfiStrategy::Strategy *strategy = createThresholdStrategy();
		DummyProgressListener listener;
		strategy->Perform(artifacts, listener);
		delete strategy;
		
		const size_t totalRFI = mask->GetCount<true>();
		
		Mask2DCPtr input = artifacts.ContaminatedData().GetSingleMask();
		for(unsigned i=0;i<ETA_STEPS+1;++i)
		{
			const num_t eta = (num_t) i / (num_t) ETA_STEPS;
			Mask2DPtr tempMask = Mask2D::CreateCopy(input);
			ScaleInvariantDilation::DilateVertically(tempMask, eta);
			
			size_t totalPositives = tempMask->GetCount<true>();
			double tpFuzzyRatio = getRatio(groundTruth, tempMask, false, false);
			double fpFuzzyRatio = getRatio(groundTruth, tempMask, true, false);
			
			tempMask->Intersect(mask);
			size_t truePositives = tempMask->GetCount<true>();
			size_t falsePositives = totalPositives - truePositives;
			
			double tpRatio = (double) truePositives / totalRFI;
			double fpRatio = (double) falsePositives / totalRFI;
			
			grRankTpRatio[i] += tpRatio;
			grRankFpRatio[i] += fpRatio;
			grRankTpSum[i] += tpFuzzyRatio;
			grRankFpSum[i] += fpFuzzyRatio;
		}
			
		for(size_t i=0;i<DIL_STEPS;++i)
		{
			const size_t dilSize = i * height / (4 * DIL_STEPS);
			
			Mask2DPtr tempMask = Mask2D::CreateCopy(input);
			StatisticalFlagger::EnlargeFlags(tempMask, 0, dilSize);
			
			size_t totalPositives = tempMask->GetCount<true>();
			double tpFuzzyRatio = getRatio(groundTruth, tempMask, false, false);
			double fpFuzzyRatio = getRatio(groundTruth, tempMask, true, false);
			
			tempMask->Intersect(mask);
			size_t truePositives = tempMask->GetCount<true>();
			size_t falsePositives = totalPositives - truePositives;
			
			double tpRatio = (double) truePositives / totalRFI;
			double fpRatio = (double) falsePositives / totalRFI;
			
			grDilTpRatio[i] += tpRatio;
			grDilFpRatio[i] += fpRatio;
			grDilTpSum[i] += tpFuzzyRatio;
			grDilFpSum[i] += fpFuzzyRatio;
		}
		
		//grTotalRFI += totalRFI;
		//grTotalRFISum += totalRFISum;
		
		std::cout << '.' << std::flush;
	}
	
	const std::string rankOperatorFilename(std::string("rank-operator-roc-") + testname + ".txt");
	std::ofstream rankOperatorFile(rankOperatorFilename.c_str());
	for(unsigned i=0;i<ETA_STEPS+1;++i)
	{
		const num_t eta = (num_t) i / (num_t) ETA_STEPS;
			rankOperatorFile
			<< "eta\t" << eta
			<< "\tTP\t" << grRankTpRatio[i] / _repeatCount
			<< "\tFP\t" << grRankFpRatio[i] / _repeatCount
			<< "\ttpSum\t" << grRankTpSum[i] / _repeatCount
			<< "\tfpSum\t" << grRankFpSum[i] / _repeatCount
			<< '\n';
	}
	const std::string dilationFilename(std::string("dilation-roc-") + testname + ".txt");
	std::ofstream dilationFile(dilationFilename.c_str());
	for(size_t i=0;i<DIL_STEPS;++i)
	{
		const size_t dilSize = i * height / (4 * DIL_STEPS);
		dilationFile
		<< "size\t" << dilSize
		<< "\tTP\t" << grDilTpRatio[i] / _repeatCount
		<< "\tFP\t" << grDilFpRatio[i] / _repeatCount
		<< "\ttpSum\t" << grDilTpSum[i] / _repeatCount
		<< "\tfpSum\t" << grDilFpSum[i] / _repeatCount
		<< '\n';
	}
	
	//TestNoisePerformance(grTotalRFI / _repeatCount, grTotalRFISum / _repeatCount, testname);
}

inline void RankOperatorROCExperiment::TestNoisePerformance(size_t totalRFI, double totalRFISum, const std::string &testname)
{
	const size_t ETA_STEPS = 100, DIL_STEPS = 128;
	const size_t width = 1024, height = 1024;
	numl_t
		grRankFpRatio[ETA_STEPS+1], grRankFpSum[ETA_STEPS+1],
		grDilFpRatio[DIL_STEPS+1], grDilFpSum[DIL_STEPS+1];
		
	for(unsigned i=0;i<ETA_STEPS+1;++i)
	{
		grRankFpRatio[i] = 0.0;
		grRankFpSum[i] = 0.0;
	}
	for(unsigned i=0;i<DIL_STEPS+1;++i)
	{
		grDilFpRatio[i] = 0.0;
		grDilFpSum[i] = 0.0;
	}
	
	for(unsigned repeatIndex=0; repeatIndex<_repeatCount; ++repeatIndex)
	{
		Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(width, height);
		Image2DPtr
			realImage = MitigationTester::CreateTestSet(2, mask, width, height),
			imagImage = MitigationTester::CreateTestSet(2, mask, width, height);
			
		TimeFrequencyData data(SinglePolarisation, realImage, imagImage);
		data.Trim(0, 0, 180, height);
		Image2DCPtr inputImage = data.GetSingleImage();
		
		rfiStrategy::ArtifactSet artifacts(0);
		
		artifacts.SetOriginalData(data);
		artifacts.SetContaminatedData(data);
		data.SetImagesToZero();
		artifacts.SetRevisedData(data);

		rfiStrategy::Strategy *strategy = createThresholdStrategy();
		DummyProgressListener listener;
		strategy->Perform(artifacts, listener);
		delete strategy;
		
		Mask2DCPtr input = artifacts.ContaminatedData().GetSingleMask();
		for(unsigned i=0;i<101;++i)
		{
			Mask2DPtr tempMask = Mask2D::CreateCopy(input);
			const num_t eta = i/100.0;
			ScaleInvariantDilation::DilateVertically(tempMask, eta);
			size_t falsePositives = tempMask->GetCount<true>();
			tempMask->Invert();
			num_t fpSum = ThresholdTools::Sum(inputImage, tempMask);
			double fpRatio = (double) falsePositives / totalRFI;
			
			grRankFpRatio[i] += fpRatio;
			grRankFpSum[i] += fpSum/totalRFISum;
		}
		
		for(size_t i=0;i<DIL_STEPS;++i)
		{
			const size_t dilSize = i * height / (4 * DIL_STEPS);
			Mask2DPtr tempMask = Mask2D::CreateCopy(input);
			StatisticalFlagger::EnlargeFlags(tempMask, 0, dilSize);
			size_t falsePositives = tempMask->GetCount<true>();
			tempMask->Invert();
			num_t fpSum = ThresholdTools::Sum(inputImage, tempMask);
			double fpRatio = (double) falsePositives / totalRFI;
			
			grDilFpRatio[i] += fpRatio;
			grDilFpSum[i] += fpSum/totalRFISum;
		}
		
		std::cout << '.' << std::flush;
	}
	
	const std::string rankOperatorFilename(std::string("rank-operator-noise-") + testname + ".txt");
	std::ofstream rankOperatorFile(rankOperatorFilename.c_str());
	for(unsigned i=0;i<ETA_STEPS+1;++i)
	{
		const num_t eta = (num_t) i / (num_t) ETA_STEPS;
			rankOperatorFile
			<< "eta\t" << eta
			<< "\tTP\t" << 0
			<< "\tFP\t" << grRankFpRatio[i] / _repeatCount
			<< "\ttpSum\t" << 0.0
			<< "\tfpSum\t" << grRankFpSum[i] / _repeatCount
			<< '\n';
	}
	const std::string dilationFilename(std::string("dilation-noise-") + testname + ".txt");
	std::ofstream dilationFile(dilationFilename.c_str());
	for(size_t i=0;i<DIL_STEPS;++i)
	{
		const size_t dilSize = i * height / (4 * DIL_STEPS);
		dilationFile
		<< "size\t" << dilSize
		<< "\tTP\t" << 0
		<< "\tFP\t" << grDilFpRatio[i] / _repeatCount
		<< "\ttpSum\t" << 0.0
		<< "\tfpSum\t" << grDilFpSum[i] / _repeatCount
		<< '\n';
	}
}

#endif
