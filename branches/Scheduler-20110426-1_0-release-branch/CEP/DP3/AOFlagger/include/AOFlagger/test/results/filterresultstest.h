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
#ifndef AOFLAGGER_FILTERRESULTSTEST_H
#define AOFLAGGER_FILTERRESULTSTEST_H

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/msio/pngfile.h>

#include <AOFlagger/util/ffttools.h>

#include <AOFlagger/imaging/defaultmodels.h>

#include <AOFlagger/strategy/actions/foreachcomplexcomponentaction.h>
#include <AOFlagger/strategy/actions/frequencyconvolutionaction.h>
#include <AOFlagger/strategy/actions/fringestopaction.h>
#include <AOFlagger/strategy/actions/setimageaction.h>
#include <AOFlagger/strategy/actions/strategyaction.h>
#include <AOFlagger/strategy/actions/timeconvolutionaction.h>

class FilterResultsTest : public UnitTest {
	public:
		FilterResultsTest() : UnitTest("Filter results")
		{
			AddTest(TestConstantSource(), "Constant source");
			AddTest(TestVariableSource(), "Variable source");
			AddTest(TestFaintSource(), "Faint source");
			AddTest(TestMissedSource(), "Misslocated source");
		}
		
	private:
		struct TestConstantSource : public Asserter
		{
			void operator()();
		};
		struct TestVariableSource : public Asserter
		{
			void operator()();
		};
		struct TestFaintSource : public Asserter
		{
			void operator()();
		};
		struct TestMissedSource : public Asserter
		{
			void operator()();
		};
		
		static rfiStrategy::Strategy *createStrategy(bool withFringeFilter, bool withTimeConv, bool withFrequencyConv)
		{
			rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
			
			if(withFringeFilter)
			{
				rfiStrategy::FringeStopAction *fsAction = new rfiStrategy::FringeStopAction();
				fsAction->SetNewPhaseCentreRA(DefaultModels::DistortionRA());
				fsAction->SetNewPhaseCentreDec(DefaultModels::DistortionDec());
				fsAction->SetFringesToConsider(3.0);
				fsAction->SetMinWindowSize(500);
				fsAction->SetMaxWindowSize(500);
				strategy->Add(fsAction);

				rfiStrategy::SetImageAction *setAction = new rfiStrategy::SetImageAction();
				setAction->SetNewImage(rfiStrategy::SetImageAction::SwapRevisedAndContaminated);
				strategy->Add(setAction);
			}
			
			if(withFrequencyConv || withTimeConv)
			{
				rfiStrategy::ForEachComplexComponentAction *foccAction = new rfiStrategy::ForEachComplexComponentAction();
				strategy->Add(foccAction);

				if(withFrequencyConv)
				{
					rfiStrategy::FrequencyConvolutionAction *fcAction = new rfiStrategy::FrequencyConvolutionAction();
					fcAction->SetConvolutionSize(3.0);
					fcAction->SetKernelKind(rfiStrategy::FrequencyConvolutionAction::SincKernel);
					foccAction->Add(fcAction);
				}
				
				if(withTimeConv)
				{
					rfiStrategy::TimeConvolutionAction *tcAction = new rfiStrategy::TimeConvolutionAction();
					tcAction->SetSincScale(3.0);
					tcAction->SetIsSincScaleInSamples(false);
					tcAction->SetOperation(rfiStrategy::TimeConvolutionAction::SingleSincOperation);
					foccAction->Add(tcAction);
					
					rfiStrategy::SetImageAction *setAction = new rfiStrategy::SetImageAction();
					setAction->SetNewImage(rfiStrategy::SetImageAction::SwapRevisedAndContaminated);
					foccAction->Add(setAction);
				}
			}
			
			return strategy;
		}
		
		static void SaveImaged(rfiStrategy::ArtifactSet &artifacts, const std::string &filename, bool difference)
		{
			UVImager imager(1024*1.5, 1024*1.5);
			
			TimeFrequencyData *data;
			if(difference)
			{
				data =
					TimeFrequencyData::CreateTFDataFromDiff(artifacts.OriginalData(), artifacts.ContaminatedData());
			} else {
				data =
					new TimeFrequencyData(artifacts.ContaminatedData());
			}
			
			imager.SetUVScaling(0.0012);
			imager.Image(*data, artifacts.MetaData());

			RedWhiteBlueMap colorMap;
			//const Image2DPtr uvImage = Image2D::CreateCopyPtr(imager.RealUVImage());
			//PngFile::Save(*uvImage, std::string("UV-") + filename, colorMap);
			
			imager.PerformFFT();
			const Image2DPtr image = Image2D::CreateCopyPtr(imager.FTReal());
			
			image->SetTrim(400, 0, 1000, 1200);
			FFTTools::SignedSqrt(*image);
			
			PngFile::Save(*image, filename, colorMap, 0.0013);
			
			delete data;
		}
		
		static void Run(rfiStrategy::Strategy *strategy, std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data, const std::string &appliedName, const std::string &differenceName)
		{
			rfiStrategy::ArtifactSet artifacts(0);
			DummyProgressListener listener;
			
			artifacts.SetOriginalData(data.first);
			artifacts.SetContaminatedData(data.first);
			data.first.SetImagesToZero();
			artifacts.SetRevisedData(data.first);
			artifacts.SetMetaData(data.second);
			
			strategy->Perform(artifacts, listener);
			
			SaveImaged(artifacts, appliedName, false);
			SaveImaged(artifacts, differenceName, true);
		}
		
		static void RunAllMethods(std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data, const std::string &setPrefix, const std::string &setName)
		{
			rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
			Run(strategy, data, setPrefix + "0-" + setName + "-Original.png", "Empty.png");
			delete strategy;

			strategy = createStrategy(true, false, false);
			Run(strategy, data, setPrefix + "1-" + setName + "-FringeFilter-Applied.png", setPrefix + "1-" + setName + "-FringeFilter-Difference.png");
			delete strategy;
			
			strategy = createStrategy(false, true, false);
			Run(strategy, data, setPrefix + "2-" + setName + "-TimeFilter-Applied.png", setPrefix + "2-" + setName + "-TimeFilter-Difference.png");
			delete strategy;
			
			strategy = createStrategy(false, false, true);
			Run(strategy, data, setPrefix + "3-" + setName + "-FreqFilter-Applied.png", setPrefix + "3-" + setName + "-FreqFilter-Difference.png");
			delete strategy;
		}
};

inline void FilterResultsTest::TestConstantSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::ConstantDistortion, 1.0);
 	RunAllMethods(data, "A", "ConstantSource");
}

inline void FilterResultsTest::TestVariableSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::VariableDistortion, 1.0);
 	RunAllMethods(data, "B", "VariableSource");
}

inline void FilterResultsTest::TestFaintSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::FaintDistortion, 1.0);
 	RunAllMethods(data, "C", "FaintSource");
}

inline void FilterResultsTest::TestMissedSource::operator()()
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> data
		= DefaultModels::LoadSet(DefaultModels::B1834Set, DefaultModels::MisslocatedDistortion, 1.0);
 	RunAllMethods(data, "D", "MissedSource");
}

#endif
