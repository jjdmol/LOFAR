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

#include <AOFlagger/rfi/strategy/baselineselectionaction.h>

#include <iostream>

#include <AOFlagger/rfi/thresholdtools.h>

#include <AOFlagger/util/plot.h>

#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/baselinereader.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/msimageset.h>

namespace rfiStrategy {
	
	void BaselineSelectionAction::prepare(class ArtifactSet &artifacts, class ProgressListener &)
	{
		if(artifacts.BaselineSelectionInfo() == 0)
			throw BadUsageException("ArtifactSet does not have baseline selection info");

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

		std::cout << "Searching for bad baselines..." << std::endl;

		const double threshold = 5.0;

		boost::mutex::scoped_lock lock(info.mutex);

		bool foundMoreBaselines;
		std::vector<BaselineSelectionInfo::SingleBaselineInfo> markedBaselines;
		do {
			Plot *plot = 0;
			if(_makePlot)
			{
				plot = new Plot("baselineSelection.pdf");
				plot->SetXAxisText("Baseline length (meters)");
				plot->SetYAxisText("Percentage RFI");
				plot->StartScatter("Baselines RFI ratio");
				for(BaselineSelectionInfo::BaselineVector::const_iterator i=info.baselines.begin();i!=info.baselines.end();++i)
				{
					plot->PushDataPoint(i->length, 100.0 * (double) i->rfiCount / (double) i->totalCount);
				}
				for(BaselineSelectionInfo::BaselineVector::const_iterator i=markedBaselines.begin();i!=markedBaselines.end();++i)
				{
					plot->PushDataPoint(i->length, 100.0 * (double) i->rfiCount / (double) i->totalCount);
				}
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
				std::cout << "Estimated std dev for thresholding, in percentage of RFI: " << round(10000.0*stddev)/100.0 << "%" << std::endl;
	
			// Select baselines to be thrown away
			foundMoreBaselines = false;
			if(_makePlot)
				plot->StartLine("Threshold");
			double maxPlotY = 0.0;
			for(int i=info.baselines.size()-1;i>=0;--i)
			{
				double currentValue = (double) info.baselines[i].rfiCount / (double) info.baselines[i].totalCount;
				if(_makePlot)
				{
					double plotY = 100.0*(values[i] + currentValue + mean + threshold*stddev);
					plot->PushDataPoint(info.baselines[i].length, plotY);
					plot->PushDataPoint(info.baselines[i].length, 100.0*(values[i] + currentValue + mean - threshold*stddev));
					if(plotY > maxPlotY) maxPlotY=plotY;
				}
				if(values[i] < mean - threshold*stddev || values[i] > mean + threshold*stddev)
				{
					std::cout << "Baseline " << info.baselines[i].antenna1Name << " x " << info.baselines[i].antenna2Name << " looks bad: "
					<< round(currentValue * 10000.0)/100.0 << "% rfi, "
					<< round(10.0*fabs((values[i] - mean) / stddev))/10.0 << "*sigma away from est baseline curve)"
					<< std::endl;
					
					markedBaselines.push_back(info.baselines[i]);
					info.baselines.erase(info.baselines.begin()+i);
					foundMoreBaselines = true;
				}
			}
			if(_makePlot)
				plot->SetYRange(0.0, maxPlotY*1.5);

			if(_makePlot)
			{
				plot->Close();
				delete plot;
			}

			delete[] values;
		} while(foundMoreBaselines);

		std::cout << "Found " << markedBaselines.size() << "/" << (markedBaselines.size()+info.baselines.size()) << " bad baselines" << std::endl;
		if(_flagBadBaselines)
		{
			flagBaselines(artifacts, markedBaselines);
		} else {
			if(markedBaselines.size() > 0)
				std::cout <<
					"Bad baseline finding is still experimental, please check the results.\n"
					"These baselines have therefore NOT been flagged yet. Writing flags to\n"
					"these baselines can be enabled by setting the flag-bad-baselines\n"
					"property of both BaselineSelectionAction's to '1' in your strategy\n"
					"file." << std::endl;
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
		reader->PerformWriteRequests();
	}

	double BaselineSelectionAction::smoothedValue(const BaselineSelectionInfo &info, const BaselineSelectionInfo::SingleBaselineInfo &baseline)
	{
		const double sigma = 0.3;

		double logLength = log(baseline.length);

		double sum = 0.0;
		double weight = 0.0;

		for(BaselineSelectionInfo::BaselineVector::const_iterator i=info.baselines.begin();i!=info.baselines.end();++i)
		{
			double otherLogLength = log(i->length);
			double otherValue = (double) i->rfiCount / (double) i->totalCount;
			double x = otherLogLength-logLength;
			double curWeight = exp(-x*x/(2.0*sigma*sigma));
			sum += curWeight * otherValue;
			weight += curWeight;
		}

		return sum / weight;
	}
}
