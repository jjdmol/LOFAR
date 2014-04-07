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

#include <AOFlagger/strategy/actions/baselineselectionaction.h>

#include <iostream>

#include <AOFlagger/strategy/algorithms/thresholdtools.h>

#include <AOFlagger/util/plot.h>

#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/baselinereader.h>

#include <AOFlagger/strategy/actions/strategyaction.h>

#include <AOFlagger/strategy/control/artifactset.h>

#include <AOFlagger/strategy/imagesets/msimageset.h>

namespace rfiStrategy {
	
	void BaselineSelectionAction::prepare(class ArtifactSet &artifacts, class ProgressListener &)
	{
		if(artifacts.BaselineSelectionInfo() == 0)
			throw BadUsageException("ArtifactSet does not have baseline selection info");
		if(artifacts.MetaData() == 0)
		{
			AOLogger::Warn << "BaselineSelectionAction is used, but ArtifactSet does not have meta data\n";
			return;
		}
		if(!artifacts.MetaData()->HasBaseline())
		{
			AOLogger::Warn << "BaselineSelectionAction is used, but ArtifactSet does not have baseline meta data\n";
			return;
		}

		Mask2DCPtr mask = artifacts.ContaminatedData().GetSingleMask();

		BaselineSelectionInfo::SingleBaselineInfo baseline;

		baseline.length = artifacts.MetaData()->Baseline().Distance();
		if(baseline.length > 0)
		{
			baseline.antenna1 = artifacts.MetaData()->Antenna1().id;
			baseline.antenna2 = artifacts.MetaData()->Antenna2().id;
			baseline.antenna1Name = artifacts.MetaData()->Antenna1().name;
			baseline.antenna2Name = artifacts.MetaData()->Antenna2().name;
			baseline.band = artifacts.MetaData()->Band().windowIndex;
	
			baseline.rfiCount = mask->GetCount<true>();
			baseline.totalCount = mask->Width() * mask->Height();
	
			BaselineSelectionInfo &info = *artifacts.BaselineSelectionInfo();
	
			boost::mutex::scoped_lock lock(info.mutex);
	
			info.baselines.push_back(baseline);
		}
	}
	
	void BaselineSelectionAction::mark(class ArtifactSet &artifacts, class ProgressListener &)
	{
		if(artifacts.BaselineSelectionInfo() == 0)
			throw BadUsageException("ArtifactSet does not have baseline selection info");
		BaselineSelectionInfo &info = *artifacts.BaselineSelectionInfo();
		if(info.baselines.size() == 0)
			throw BadUsageException("BaselineSelectionAction wrongly used: trying to mark baselines, but baselines have not been prepared previously (you need to add a BaselineSelectionAction within a for each baseline block, that calculates the statistics and prepares selection)");

		AOLogger::Debug << "Searching for bad baselines...\n";

		Strategy::SyncAll(*GetRoot());

		boost::mutex::scoped_lock lock(info.mutex);

		std::vector<BaselineSelectionInfo::SingleBaselineInfo> markedBaselines;

		// Perform a first quick threshold to remove baselines which deviate a lot (e.g. 100% flagged
		// baselines). Sometimes, there are a lot of them, causing instability if this would not be
		// done.
		for(int i=info.baselines.size()-1;i>=0;--i)
		{
			double currentValue = (double) info.baselines[i].rfiCount / (double) info.baselines[i].totalCount;
			if(currentValue>_absThreshold || (info.baselines[i].rfiCount==0 && info.baselines[i].totalCount>=2500))
			{
				AOLogger::Info << "Baseline " << info.baselines[i].antenna1Name << " x " << info.baselines[i].antenna2Name << " looks bad: "
				<< round(currentValue * 10000.0)/100.0 << "% rfi (zero or above " << (_absThreshold*100.0) << "% abs threshold)\n";
					
				info.baselines[i].marked = true;
				markedBaselines.push_back(info.baselines[i]);
				info.baselines.erase(info.baselines.begin()+i);
			}
		}

		bool foundMoreBaselines;
		do {
			std::sort(info.baselines.begin(), info.baselines.end());

			Plot *plot = 0;
			if(_makePlot)
			{
				plot = new Plot("baselineSelection.pdf");
				plot->SetXAxisText("Baseline length (meters)");
				plot->SetYAxisText("Percentage RFI");
			}

			size_t unmarkedBaselineCount = info.baselines.size();
			double *values = new double[unmarkedBaselineCount];
	
			// Calculate the smoothed values
			if(_makePlot)
				plot->StartLine("Smoothed values");

			size_t valueIndex = 0;
			for(BaselineSelectionInfo::BaselineVector::const_iterator i=info.baselines.begin();i!=info.baselines.end();++i)
			{
				double smoothedVal = smoothedValue(info, *i);
				if(_makePlot)
					plot->PushDataPoint(i->length, 100.0*smoothedVal);
				values[valueIndex] = smoothedVal - (double) i->rfiCount / (double) i->totalCount;
				++valueIndex;
			}
	
			// Calculate the std dev
			double mean, stddev;
			std::vector<double> valuesCopy;
			for(size_t i=0;i<unmarkedBaselineCount;++i)
				valuesCopy.push_back(values[i]);
			ThresholdTools::TrimmedMeanAndStdDev(valuesCopy, mean, stddev);

			if(_makePlot)
				AOLogger::Debug << "Estimated std dev for thresholding, in percentage of RFI: " << round(10000.0*stddev)/100.0 << "%\n";
	
			// unselect already marked baselines
			for(int i=markedBaselines.size()-1;i>=0;--i)
			{
				BaselineSelectionInfo::SingleBaselineInfo baseline =
					markedBaselines[i];
				double currentValue = (double) baseline.rfiCount / (double) baseline.totalCount;
				double baselineValue =
					smoothedValue(info, baseline.length) - currentValue;
				if(baselineValue >= mean - _threshold*stddev && baselineValue <= mean + _threshold*stddev && currentValue<_absThreshold && (baseline.rfiCount!=0 || baseline.totalCount<2500))
				{
					markedBaselines.erase(markedBaselines.begin()+i);
					info.baselines.push_back(baseline);
					AOLogger::Info << "Baseline " << baseline.antenna1Name << " x " << baseline.antenna2Name << " is now within baseline curve\n";
				}
			}
			
			// (re)select baselines to be thrown away
			foundMoreBaselines = false;
			if(_makePlot)
				plot->StartScatter("Threshold");
			double maxPlotY = 0.0;
			for(int i=unmarkedBaselineCount-1;i>=0;--i)
			{
				double currentValue = (double) info.baselines[i].rfiCount / (double) info.baselines[i].totalCount;
				if(_makePlot)
				{
					double plotY = 100.0*(values[i] + currentValue + mean + _threshold*stddev);
					plot->PushDataPoint(info.baselines[i].length, plotY);
					plot->PushDataPoint(info.baselines[i].length, 100.0*(values[i] + currentValue + mean - _threshold*stddev));
					if(plotY > maxPlotY) maxPlotY=plotY;
				}
				if(values[i] < mean - _threshold*stddev || values[i] > mean + _threshold*stddev || currentValue>_absThreshold || (info.baselines[i].rfiCount==0 && info.baselines[i].totalCount>=2500))
				{
					AOLogger::Info << "Baseline " << info.baselines[i].antenna1Name << " x " << info.baselines[i].antenna2Name << " looks bad: "
					<< round(currentValue * 10000.0)/100.0 << "% rfi, "
					<< round(10.0*fabs((values[i] - mean) / stddev))/10.0 << "*sigma away from est baseline curve\n";
						
					if(!info.baselines[i].marked)
					{
						foundMoreBaselines = true;
						info.baselines[i].marked = true;
					}
					markedBaselines.push_back(info.baselines[i]);
					info.baselines.erase(info.baselines.begin()+i);
				}
			}
			if(_makePlot)
			{
				plot->SetYRange(0.0, maxPlotY*1.5);
				plot->StartScatter("Accepted baselines");
				for(BaselineSelectionInfo::BaselineVector::const_iterator i=info.baselines.begin();i!=info.baselines.end();++i)
				{
					plot->PushDataPoint(i->length, 100.0 * (double) i->rfiCount / (double) i->totalCount);
				}
				plot->StartScatter("Rejected baselines");
				for(BaselineSelectionInfo::BaselineVector::const_iterator i=markedBaselines.begin();i!=markedBaselines.end();++i)
				{
					plot->PushDataPoint(i->length, 100.0 * (double) i->rfiCount / (double) i->totalCount);
				}
				plot->Close();
				delete plot;
			}

			delete[] values;
		} while(foundMoreBaselines);

		if(markedBaselines.size() > 0)
		{
			AOLogger::Info << "Found " << markedBaselines.size() << "/" << (markedBaselines.size()+info.baselines.size()) << " bad baselines: ";
			
			std::vector<BaselineSelectionInfo::SingleBaselineInfo>::const_iterator badBaselineIter = markedBaselines.begin();
			AOLogger::Info << badBaselineIter->antenna1Name << "x" << badBaselineIter->antenna2Name;
			++badBaselineIter;
			while(badBaselineIter!=markedBaselines.end())
			{
				AOLogger::Info << ", " << badBaselineIter->antenna1Name << "x" << badBaselineIter->antenna2Name;
				++badBaselineIter;
			}
			AOLogger::Info << '\n';
		} else {
			AOLogger::Info << "No bad baselines found.\n";
		}
		
		if(_flagBadBaselines)
		{
			flagBaselines(artifacts, markedBaselines);
		} else {
			if(markedBaselines.size() > 0)
				AOLogger::Info <<
					"Bad baseline finding is still experimental, please check the results.\n"
					"These baselines have therefore NOT been flagged yet. Writing flags to\n"
					"these baselines can be enabled by setting the flag-bad-baselines\n"
					"property of both BaselineSelectionAction's to '1' in your strategy\n"
					"file.\n";
		}
	}

	void BaselineSelectionAction::flagBaselines(ArtifactSet &artifacts, std::vector<BaselineSelectionInfo::SingleBaselineInfo> baselines)
	{
		boost::mutex::scoped_lock lock(artifacts.IOMutex());

		ImageSet *imageSet = artifacts.ImageSet();
		BaselineReaderPtr reader = dynamic_cast<MSImageSet&>(*imageSet).Reader();

		size_t scans = reader->Set().GetObservationTimesSet().size();
		size_t frequencyCount = reader->Set().FrequencyCount();
		Mask2DPtr flaggedMask = Mask2D::CreateSetMaskPtr<true>(scans, frequencyCount);
		std::vector<Mask2DCPtr> masks;
		for(size_t i=0;i<reader->PolarizationCount();++i)
			masks.push_back(flaggedMask);

		for(std::vector<BaselineSelectionInfo::SingleBaselineInfo>::const_iterator i=baselines.begin();
			i!=baselines.end();++i)
		{
			reader->AddWriteTask(masks, i->antenna1, i->antenna2, i->band);
		}
		reader->PerformFlagWriteRequests();
	}

	double BaselineSelectionAction::smoothedValue(const BaselineSelectionInfo &info, double length)
	{
		double logLength = log(length);

		double sum = 0.0;
		double weight = 0.0;

		for(BaselineSelectionInfo::BaselineVector::const_iterator i=info.baselines.begin();i!=info.baselines.end();++i)
		{
			double otherLogLength = log(i->length);
			double otherValue = (double) i->rfiCount / (double) i->totalCount;
			double x = otherLogLength-logLength;
			double curWeight = exp(-x*x/(2.0*_smoothingSigma*_smoothingSigma));
			sum += curWeight * otherValue;
			weight += curWeight;
		}

		return sum / weight;
	}
}
