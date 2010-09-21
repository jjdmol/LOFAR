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
#include <AOFlagger/rfi/strategy/baselineselectionaction.h>
#include <AOFlagger/rfi/strategy/changeresolutionaction.h>
#include <AOFlagger/rfi/strategy/combineflagresults.h>
#include <AOFlagger/rfi/strategy/foreachbaselineaction.h>
#include <AOFlagger/rfi/strategy/foreachpolarisationblock.h>
#include <AOFlagger/rfi/strategy/frequencyselectionaction.h>
#include <AOFlagger/rfi/strategy/iterationblock.h>
#include <AOFlagger/rfi/strategy/plotaction.h>
#include <AOFlagger/rfi/strategy/setflaggingaction.h>
#include <AOFlagger/rfi/strategy/setimageaction.h>
#include <AOFlagger/rfi/strategy/slidingwindowfitaction.h>
#include <AOFlagger/rfi/strategy/statisticalflagaction.h>
#include <AOFlagger/rfi/strategy/strategyiterator.h>
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
		ActionBlock *current;

		block.Add(new SetFlaggingAction());

		ForEachPolarisationBlock *fepBlock = new ForEachPolarisationBlock();
		block.Add(fepBlock);
		current = fepBlock;

		Adapter *adapter = new Adapter();
		current->Add(adapter);
		current = adapter;

		IterationBlock *iteration = new IterationBlock();
		iteration->SetIterationCount(2);
		iteration->SetSensitivityStart(4.0);
		current->Add(iteration);
		current = iteration;
		
		ThresholdAction *t2 = new ThresholdAction();
		t2->SetBaseSensitivity(1.0);
		if(pulsar)
			t2->SetFrequencyDirectionFlagging(false);
		current->Add(t2);

		CombineFlagResults *cfr2 = new CombineFlagResults();
		current->Add(cfr2);

		cfr2->Add(new FrequencySelectionAction());
		if(!pulsar)
			cfr2->Add(new TimeSelectionAction());
	
		current->Add(new SetImageAction());
		ChangeResolutionAction *changeResAction2 = new ChangeResolutionAction();
		if(pulsar)
			changeResAction2->SetTimeDecreaseFactor(1);
		else
			changeResAction2->SetTimeDecreaseFactor(3);

		SlidingWindowFitAction *swfAction2 = new SlidingWindowFitAction();
		if(pulsar)
		{
			swfAction2->Parameters().timeDirectionWindowSize = 1;
		} else {
			swfAction2->Parameters().timeDirectionKernelSize = 2.5;
			swfAction2->Parameters().timeDirectionWindowSize = 10;
		}
		changeResAction2->Add(swfAction2);

		current->Add(changeResAction2);
		current->Add(new SetFlaggingAction());

		current = adapter;
		ThresholdAction *t3 = new ThresholdAction();
		if(pulsar)
			t3->SetFrequencyDirectionFlagging(false);
		current->Add(t3);
		
		SetFlaggingAction *setFlagsInAllPolarizations = new SetFlaggingAction();
		setFlagsInAllPolarizations->SetNewFlagging(SetFlaggingAction::PolarisationsEqual);
		
		block.Add(setFlagsInAllPolarizations);
		block.Add(new StatisticalFlagAction());

		if(pedantic)
		{
			CombineFlagResults *cfr3 = new CombineFlagResults();
			block.Add(cfr3);
			cfr3->Add(new FrequencySelectionAction());
			if(!pulsar)
				cfr3->Add(new TimeSelectionAction());
		} else {
			if(!pulsar)
				block.Add(new TimeSelectionAction());
		}

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(true);
		block.Add(baselineSelection);

		SetFlaggingAction *orWithOriginals = new SetFlaggingAction();
		orWithOriginals->SetNewFlagging(SetFlaggingAction::OrOriginal);
		block.Add(orWithOriginals);
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

		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());

		PlotAction *plotAction = new PlotAction();
		plotAction->SetPlotKind(PlotAction::AntennaFlagCountPlot);
		feBaseBlock->Add(plotAction);

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(false);
		Add(baselineSelection);
	}

	void Strategy::LoadAverageStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);

		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());

		PlotAction *plotAction = new PlotAction();
		plotAction->SetPlotKind(PlotAction::AntennaFlagCountPlot);
		feBaseBlock->Add(plotAction);

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(false);
		Add(baselineSelection);
	}

	void Strategy::LoadBestStrategy(bool pedantic, bool pulsar)
	{
		ForEachBaselineAction *feBaseBlock = new ForEachBaselineAction();
		Add(feBaseBlock);
		
		LoadDefaultSingleStrategy(*feBaseBlock, pedantic, pulsar);

		feBaseBlock->Add(new WriteFlagsAction());

		PlotAction *plotAction = new PlotAction();
		plotAction->SetPlotKind(PlotAction::AntennaFlagCountPlot);
		feBaseBlock->Add(plotAction);

		BaselineSelectionAction *baselineSelection = new BaselineSelectionAction();
		baselineSelection->SetPreparationStep(false);
		Add(baselineSelection);
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

	void Strategy::SetThreadCount(Strategy &strategy, size_t threadCount)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachBaselineActionType)
			{
				ForEachBaselineAction &fobAction = static_cast<ForEachBaselineAction&>(*i);
				fobAction.SetThreadCount(threadCount);
			}
			if(i->Type() == WriteFlagsActionType)
			{
				WriteFlagsAction &writeAction = static_cast<WriteFlagsAction&>(*i);
				writeAction.SetMaxBufferItems(threadCount*5);
				writeAction.SetMinBufferItemsForWriting(threadCount*4);
			}
			++i;
		}
	}

	void Strategy::SetDataKind(Strategy &strategy, enum DataKind kind)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachBaselineActionType)
			{
				ForEachBaselineAction &action = static_cast<ForEachBaselineAction&>(*i);
				action.SetDataKind(kind);
			}
			++i;
		}
	}

	/** TODO : implement, also in ForEachPolarisation */
	void Strategy::SetPolarisations(Strategy &, enum PolarisationType)
	{
	}

	void Strategy::SetBaselines(Strategy &strategy, enum BaselineSelection baselineSelection)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachBaselineActionType)
			{
				ForEachBaselineAction &fobAction = static_cast<ForEachBaselineAction&>(*i);
				fobAction.SetSelection(baselineSelection);
			}
			++i;
		}
	}

	void Strategy::SetFlagStokes(Strategy &strategy, bool newValue)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		bool hasBeenAdapted = false;
		while(!i.PastEnd())
		{
			if(i->Type() == ForEachPolarisationBlockType)
			{
				if(hasBeenAdapted && newValue)
					throw std::runtime_error("Flagging on Stokes components was requested, but the separate real/imaginary values have already been converted to amplitude values before the polarization iteration.");

				ForEachPolarisationBlock &fopAction = static_cast<ForEachPolarisationBlock&>(*i);
				fopAction.SetIterateStokesValues(newValue);
			}
			else if(i->Type() == AdapterType)
			{
				hasBeenAdapted = true;
			}
			++i;
		}
	}

	void Strategy::SetTransientCompatibility(Strategy &strategy)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ThresholdActionType)
			{
				ThresholdAction &action = static_cast<ThresholdAction&>(*i);
				action.SetFrequencyDirectionFlagging(false);
			} else if(i->Type() == SlidingWindowFitActionType)
			{
				SlidingWindowFitAction &action = static_cast<SlidingWindowFitAction&>(*i);
				action.Parameters().timeDirectionWindowSize = 1;
			}
			++i;
		}
	}

	void Strategy::SetMultiplySensitivity(Strategy &strategy, num_t factor)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == ThresholdActionType)
			{
				ThresholdAction &action = static_cast<ThresholdAction&>(*i);
				action.SetBaseSensitivity(action.BaseSensitivity() * factor);
			}
			++i;
		}
	}

	void Strategy::SetFittingWindowSize(Strategy &strategy, size_t windowWidth, size_t windowHeight)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == SlidingWindowFitActionType)
			{
				SlidingWindowFitAction &action = static_cast<SlidingWindowFitAction&>(*i);
				action.Parameters().timeDirectionWindowSize = windowWidth;
				action.Parameters().frequencyDirectionWindowSize = windowHeight;
			}
			++i;
		}
	}

	void Strategy::SetFittingKernelSize(Strategy &strategy, num_t kernelWidth, num_t kernelHeight)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == SlidingWindowFitActionType)
			{
				SlidingWindowFitAction &action = static_cast<SlidingWindowFitAction&>(*i);
				action.Parameters().timeDirectionKernelSize = kernelWidth;
				action.Parameters().frequencyDirectionKernelSize = kernelHeight;
			}
			++i;
		}
	}

	void Strategy::DisableOptimizations(Strategy &strategy)
	{
		StrategyIterator i = StrategyIterator::NewStartIterator(strategy);
		while(!i.PastEnd())
		{
			if(i->Type() == AdapterType)
			{
				Adapter &adapter = static_cast<Adapter&>(*i);
				adapter.SetRestoreOriginals(true);
			}
			++i;
		}
	}
}
