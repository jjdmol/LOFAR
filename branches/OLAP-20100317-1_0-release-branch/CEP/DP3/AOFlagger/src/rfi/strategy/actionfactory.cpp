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
#include <AOFlagger/rfi/strategy/actionfactory.h>

#include <AOFlagger/rfi/strategy/action.h>
#include <AOFlagger/rfi/strategy/adapter.h>
#include <AOFlagger/rfi/strategy/changeresolutionaction.h>
#include <AOFlagger/rfi/strategy/combineflagresults.h>
#include <AOFlagger/rfi/strategy/frequencyselectionaction.h>
#include <AOFlagger/rfi/strategy/foreachbaselineaction.h>
#include <AOFlagger/rfi/strategy/foreachpolarisationblock.h>
#include <AOFlagger/rfi/strategy/foreachmsaction.h>
#include <AOFlagger/rfi/strategy/fringestopaction.h>
#include <AOFlagger/rfi/strategy/imageraction.h>
#include <AOFlagger/rfi/strategy/iterationblock.h>
#include <AOFlagger/rfi/strategy/loadflagsaction.h>
#include <AOFlagger/rfi/strategy/loadimageaction.h>
#include <AOFlagger/rfi/strategy/plotaction.h>
#include <AOFlagger/rfi/strategy/setflaggingaction.h>
#include <AOFlagger/rfi/strategy/setimageaction.h>
#include <AOFlagger/rfi/strategy/slidingwindowfitaction.h>
#include <AOFlagger/rfi/strategy/statisticalflagaction.h>
#include <AOFlagger/rfi/strategy/svdaction.h>
#include <AOFlagger/rfi/strategy/thresholdaction.h>
#include <AOFlagger/rfi/strategy/timeselectionaction.h>
#include <AOFlagger/rfi/strategy/writeflagsaction.h>

namespace rfiStrategy {

const std::vector<std::string> ActionFactory::GetActionList()
{
	std::vector<std::string> list;
	list.push_back("Change resolution");
	list.push_back("Combine flag results");
	list.push_back("For each baseline");
	list.push_back("For each polarisation");
	list.push_back("For each measurement set");
	list.push_back("Frequency selection");
	list.push_back("Fringe stopping recovery");
	list.push_back("Image");
	list.push_back("Iteration");
	list.push_back("Load flags");
	list.push_back("Load image");
	list.push_back("Phase adapter");
	list.push_back("Plot");
	list.push_back("Set flagging");
	list.push_back("Set image");
	list.push_back("Singular value decomposition");
	list.push_back("Sliding window fit");
	list.push_back("Statistical flagging");
	list.push_back("Threshold");
	list.push_back("Time selection");
	list.push_back("Write flags");
	return list;
}

Action *ActionFactory::CreateAction(const std::string &action)
{
	if(action == "Change resolution")
		return new ChangeResolutionAction();
	else if(action == "Combine flag results")
		return new CombineFlagResults();
	else if(action == "For each baseline")
		return new ForEachBaselineAction();
	else if(action == "For each measurement set")
		return new ForEachMSAction();
	else if(action == "For each polarisation")
		return new ForEachPolarisationBlock();
	else if(action == "Frequency selection")
		return new FrequencySelectionAction();
	else if(action == "Fringe stopping recovery")
		return new FringeStopAction();
	else if(action == "Image")
		return new ImagerAction();
	else if(action == "Iteration")
		return new IterationBlock();
	else if(action == "Load flags")
		return new LoadFlagsAction();
	else if(action == "Load image")
		return new LoadImageAction();
	else if(action == "Phase adapter")
		return new Adapter();
	else if(action == "Plot")
		return new PlotAction();
	else if(action == "Set flagging")
		return new SetFlaggingAction();
	else if(action == "Set image")
		return new SetImageAction();
	else if(action == "Singular value decomposition")
		return new SVDAction();
	else if(action == "Sliding window fit")
		return new SlidingWindowFitAction();
	else if(action == "Statistical flagging")
		return new StatisticalFlagAction();
	else if(action == "Threshold")
		return new ThresholdAction();
	else if(action == "Time selection")
		return new TimeSelectionAction();
	else if(action == "Write flags")
		return new WriteFlagsAction();
	else
		throw BadUsageException(std::string("Trying to create unknown action \"") + action + "\"");
}

} // namespace
