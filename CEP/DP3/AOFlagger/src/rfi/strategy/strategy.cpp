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
#include <AOFlagger/rfi/strategy/changeresolutionaction.h>
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

	void Strategy::LoadDefaultSingleStrategy(bool pedantic, bool pulsar)
	{
		LoadDefaultSingleStrategy(*this, pedantic, pulsar);
	}

	void Strategy::LoadDefaultSingleStrategy(ActionBlock &block, bool pedantic, bool pulsar)
	{
		block.Add(new SetFlaggingAction());
		ForEachPolarisationBlock *fepBlock = new ForEachPolarisationBlock();
		block.Add(fepBlock);

		Adapter *adapter = new Adapter();
		fepBlock->Add(adapter);
	
		ThresholdAction *t1 = new ThresholdAction();
		t1->SetBaseSensitivity(4.0);
		if(pulsar)
			t1->SetFrequencyDirectionFlagging(false);
		adapter->Add(t1);

		CombineFlagResults *cfr1 = new CombineFlagResults();
		adapter->Add(cfr1);

		cfr1->Add(new FrequencySelectionAction());
		if(!pulsar)
			cfr1->Add(new TimeSelectionAction());
	
		adapter->Add(new SetImageAction());

		ChangeResolutionAction *changeResAction1 = new ChangeResolutionAction();
		if(pulsar)
			changeResAction1->SetDecreaseFactor(1);
		else
			changeResAction1->SetDecreaseFactor(3);

		SlidingWindowFitAction *swfAction1 = new SlidingWindowFitAction();
		if(pulsar)
		{
			swfAction1->Parameters().timeDirectionWindowSize = 1;
		} else {
			swfAction1->Parameters().timeDirectionKernelSize = 2.5;
			swfAction1->Parameters().timeDirectionWindowSize = 10;
		}
		changeResAction1->Add(swfAction1);

		adapter->Add(changeResAction1);
		adapter->Add(new SetFlaggingAction());

		ThresholdAction *t2 = new ThresholdAction();
		t2->SetBaseSensitivity(2.0);
		if(pulsar)
			t2->SetFrequencyDirectionFlagging(false);
		adapter->Add(t2);

		CombineFlagResults *cfr2 = new CombineFlagResults();
		adapter->Add(cfr2);

		cfr2->Add(new FrequencySelectionAction());
		if(!pulsar)
			cfr2->Add(new TimeSelectionAction());
	
		adapter->Add(new SetImageAction());
		ChangeResolutionAction *changeResAction2 = new ChangeResolutionAction();
		if(pulsar)
			changeResAction2->SetDecreaseFactor(1);
		else
			changeResAction2->SetDecreaseFactor(3);

		SlidingWindowFitAction *swfAction2 = new SlidingWindowFitAction();
		if(pulsar)
		{
			swfAction2->Parameters().timeDirectionWindowSize = 1;
		} else {
			swfAction2->Parameters().timeDirectionKernelSize = 2.5;
			swfAction2->Parameters().timeDirectionWindowSize = 10;
		}
		changeResAction2->Add(swfAction2);

		adapter->Add(changeResAction2);
		adapter->Add(new SetFlaggingAction());

		ThresholdAction *t3 = new ThresholdAction();
		if(pulsar)
			t3->SetFrequencyDirectionFlagging(false);
		adapter->Add(t3);
		adapter->Add(new StatisticalFlagAction());

		if(pedantic)
		{
			CombineFlagResults *cfr3 = new CombineFlagResults();
			adapter->Add(cfr3);
			cfr3->Add(new FrequencySelectionAction());
			if(!pulsar)
				cfr3->Add(new TimeSelectionAction());
		} else {
			if(!pulsar)
				adapter->Add(new TimeSelectionAction());
		}
		
		SetFlaggingAction *setFlagsInAllPolarizations = new SetFlaggingAction();
		setFlagsInAllPolarizations->SetNewFlagging(SetFlaggingAction::PolarisationsEqual);
		block.Add(setFlagsInAllPolarizations);
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

	void Strategy::LoadDefaultStrategy()
	{
		LoadAverageStrategy();
	}

	void Strategy::LoadFastStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadImageAction *loadImageAction = new LoadImageAction();
		loadImageAction->SetReadStokesI();

		feBaseBlock->Add(loadImageAction);
		
		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());
	}

	void Strategy::LoadAverageStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadImageAction *loadImageAction = new LoadImageAction();
		loadImageAction->SetReadDipoleAutoPolarisations();

		feBaseBlock->Add(loadImageAction);
		
		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());
	}

	void Strategy::LoadBestStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadImageAction *loadImageAction = new LoadImageAction();
		loadImageAction->SetReadAllPolarisations();

		feBaseBlock->Add(loadImageAction);
		
		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

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
