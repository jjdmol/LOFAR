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
#include <AOFlagger/rfi/strategy/strategy.h>

#include <AOFlagger/rfi/strategy/adapter.h>
#include <AOFlagger/rfi/strategy/combineflagresults.h>
#include <AOFlagger/rfi/strategy/foreachbaselineaction.h>
#include <AOFlagger/rfi/strategy/foreachpolarisationblock.h>
#include <AOFlagger/rfi/strategy/frequencyselectionaction.h>
#include <AOFlagger/rfi/strategy/iterationblock.h>
#include <AOFlagger/rfi/strategy/loadimageaction.h>
#include <AOFlagger/rfi/strategy/setflaggingaction.h>
#include <AOFlagger/rfi/strategy/setimageaction.h>
#include <AOFlagger/rfi/strategy/slidingwindowfitaction.h>
#include <AOFlagger/rfi/strategy/statisticalflagaction.h>
#include <AOFlagger/rfi/strategy/thresholdaction.h>
#include <AOFlagger/rfi/strategy/timeselectionaction.h>
#include <AOFlagger/rfi/strategy/writeflagsaction.h>

namespace rfiStrategy {

	Strategy *Strategy::CreateDefaultSingleStrategy()
	{
		Strategy *strategy = new Strategy();
		strategy->LoadDefaultSingleStrategy();
		return strategy;
	}

	void Strategy::LoadDefaultSingleStrategy()
	{
		Add(new SetFlaggingAction());
		ForEachPolarisationBlock *fepBlock = new ForEachPolarisationBlock();
		Add(fepBlock);

		Adapter *adapter = new Adapter();
		fepBlock->Add(adapter);
	
		ThresholdAction *t1 = new ThresholdAction();
		t1->SetBaseSensitivity(4.0);
		adapter->Add(t1);

		adapter->Add(new StatisticalFlagAction());

		CombineFlagResults *cfr1 = new CombineFlagResults();
		adapter->Add(cfr1);

		cfr1->Add(new FrequencySelectionAction());
		cfr1->Add(new TimeSelectionAction());
	
		adapter->Add(new SetImageAction());
		adapter->Add(new SlidingWindowFitAction());
		adapter->Add(new SetFlaggingAction());

		ThresholdAction *t2 = new ThresholdAction();
		t2->SetBaseSensitivity(2.0);
		adapter->Add(t2);

		adapter->Add(new StatisticalFlagAction());

		CombineFlagResults *cfr2 = new CombineFlagResults();
		adapter->Add(cfr2);

		cfr2->Add(new FrequencySelectionAction());
		cfr2->Add(new TimeSelectionAction());
	
		adapter->Add(new SetImageAction());
		adapter->Add(new SlidingWindowFitAction());
		adapter->Add(new SetFlaggingAction());

		adapter->Add(new ThresholdAction());
		adapter->Add(new StatisticalFlagAction());

		CombineFlagResults *cfr3 = new CombineFlagResults();
		adapter->Add(cfr3);
		cfr3->Add(new FrequencySelectionAction());
		cfr3->Add(new TimeSelectionAction());
	}

	void Strategy::LoadOldDefaultSingleStrategy()
	{
		Add(new SetFlaggingAction());
		ForEachPolarisationBlock *fepBlock = new ForEachPolarisationBlock();
		Add(fepBlock);

		CombineFlagResults *cfr = new CombineFlagResults();
		fepBlock->Add(cfr);
	
		Adapter *adapter = new Adapter();
		cfr->Add(adapter);
	
		IterationBlock *iteration = new IterationBlock();
		adapter->Add(iteration);
	
		iteration->Add(new ThresholdAction());
		//iteration->Add(new StatisticalFlagAction());
		iteration->Add(new SetImageAction());
		iteration->Add(new SlidingWindowFitAction());

		adapter->Add(new ThresholdAction());
		//adapter->Add(new StatisticalFlagAction());
	}

	void Strategy::LoadDefaultStrategy(size_t threadCount)
	{
		LoadAverageStrategy();
	}

	void Strategy::LoadFastStrategy()
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadImageAction *loadImageAction = new LoadImageAction();
		loadImageAction->SetReadStokesI();
		feBaseBlock->Add(loadImageAction);
		feBaseBlock->Add(new SetFlaggingAction());
		ForEachPolarisationBlock *fePolBlock = new ForEachPolarisationBlock();
		feBaseBlock->Add(fePolBlock);
	
		Adapter *adapter = new Adapter();
		fePolBlock->Add(adapter);
	
		IterationBlock *iteration = new IterationBlock();
		iteration->SetIterationCount(3);
		adapter->Add(iteration);
	
		iteration->Add(new ThresholdAction());
		iteration->Add(new SetImageAction());

		SlidingWindowFitAction *fitAction = new SlidingWindowFitAction();
		fitAction->Parameters().timeDirectionWindowSize = 20;
		fitAction->Parameters().timeDirectionKernelSize = 7.5;
		iteration->Add(fitAction);

		adapter->Add(new ThresholdAction());

		feBaseBlock->Add(new WriteFlagsAction());
	}

	void Strategy::LoadAverageStrategy()
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadImageAction *loadImageAction = new LoadImageAction();
		loadImageAction->SetReadStokesI();
		feBaseBlock->Add(loadImageAction);
		feBaseBlock->Add(new SetFlaggingAction());
		ForEachPolarisationBlock *fePolBlock = new ForEachPolarisationBlock();
		feBaseBlock->Add(fePolBlock);
	
		Adapter *adapter = new Adapter();
		fePolBlock->Add(adapter);
	
		IterationBlock *iteration = new IterationBlock();
		iteration->SetIterationCount(4);
		adapter->Add(iteration);
	
		iteration->Add(new ThresholdAction());
		iteration->Add(new SetImageAction());

		SlidingWindowFitAction *fitAction = new SlidingWindowFitAction();
		fitAction->Parameters().timeDirectionWindowSize = 40;
		fitAction->Parameters().timeDirectionKernelSize = 15.0;
		iteration->Add(fitAction);

		adapter->Add(new ThresholdAction());

		feBaseBlock->Add(new WriteFlagsAction());
	}

	void Strategy::LoadBestStrategy()
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadImageAction *loadImageAction = new LoadImageAction();
		loadImageAction->SetReadAllPolarisations();
		feBaseBlock->Add(loadImageAction);
		feBaseBlock->Add(new SetFlaggingAction());
		ForEachPolarisationBlock *fePolBlock1 = new ForEachPolarisationBlock();
		feBaseBlock->Add(fePolBlock1);
	
		Adapter *adapter1 = new Adapter();
		fePolBlock1->Add(adapter1);
	
		IterationBlock *iteration1 = new IterationBlock();
		adapter1->Add(iteration1);
		iteration1->SetIterationCount(5);
	
		iteration1->Add(new ThresholdAction());
		iteration1->Add(new SetImageAction());

		SlidingWindowFitAction *fitAction1 = new SlidingWindowFitAction();
		fitAction1->Parameters().timeDirectionWindowSize = 50;
		fitAction1->Parameters().timeDirectionKernelSize = 18.0;
		iteration1->Add(fitAction1);

		adapter1->Add(new ThresholdAction());

		SetFlaggingAction *setFlaggingAction = new SetFlaggingAction();
		setFlaggingAction->SetNewFlagging(SetFlaggingAction::PolarisationsEqual);
		feBaseBlock->Add(setFlaggingAction);

		/*
		// One last iteration
		ForEachPolarisationBlock *fePolBlock2 = new ForEachPolarisationBlock();
		feBaseBlock->Add(fePolBlock2);
	
		Adapter *adapter2 = new Adapter();
		fePolBlock2->Add(adapter2);
	
		IterationBlock *iteration2 = new IterationBlock();
		adapter2->Add(iteration2);
		iteration2->SetIterationCount(1);
	
		iteration2->Add(new SetImageAction());

		SlidingWindowFitAction *fitAction2 = new SlidingWindowFitAction();
		fitAction2->Parameters().timeDirectionWindowSize = 50;
		fitAction2->Parameters().timeDirectionKernelSize = 18.0;
		iteration2->Add(fitAction2);

		iteration2->Add(new ThresholdAction());*/

		feBaseBlock->Add(new StatisticalFlagAction());

		feBaseBlock->Add(new WriteFlagsAction());
	}

	
	ArtifactSet *Strategy::JoinThread()
	{
		ArtifactSet *artifact = 0;
		if(_thread != 0)
		{
			_thread->join();
			delete _thread;
			artifact = new ArtifactSet(*_threadFunc->_artifacts);
			delete _threadFunc->_artifacts;
			delete _threadFunc;
		}
		_thread = 0;
		return artifact;
	}

	void Strategy::StartPerformThread(const ArtifactSet &artifacts, ProgressListener &progress)
	{
		JoinThread();
		_threadFunc = new PerformFunc(this, new ArtifactSet(artifacts), &progress);
		_thread = new boost::thread(*_threadFunc);
	}

	void Strategy::PerformFunc::operator()()
	{
		_strategy->Perform(*_artifacts, *_progress);
	}

	void Strategy::setThreadCount(ActionBlock &actionBlock, size_t threadCount)
	{
		for(size_t i=0;i<actionBlock.GetChildCount();++i)
		{
			Action &action = actionBlock.GetChild(i);
			ActionBlock *innerActionBlock = dynamic_cast<ActionBlock*>(&action);
			if(innerActionBlock != 0)
			{
				setThreadCount(*innerActionBlock, threadCount);
			}
			ForEachBaselineAction *fobAction = dynamic_cast<ForEachBaselineAction*>(&action);
			if(fobAction != 0)
			{
				fobAction->SetThreadCount(threadCount);
			}
		}
	}

}
