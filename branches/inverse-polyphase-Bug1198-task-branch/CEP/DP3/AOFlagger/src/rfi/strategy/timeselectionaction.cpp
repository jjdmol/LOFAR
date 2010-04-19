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
#include <map>

#include <AOFlagger/rfi/strategy/timeselectionaction.h>

#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/samplerow.h>

#include <AOFlagger/rfi/medianwindow.h>
#include <AOFlagger/rfi/rfistatistics.h>

#include <AOFlagger/rfi/strategy/artifactset.h>

#include <AOFlagger/rfi/rfiplots.h>
#include <AOFlagger/util/multiplot.h>

namespace rfiStrategy {

void TimeSelectionAction::ManualSelection(ArtifactSet &artifacts)
{
	TimeFrequencyData &model = artifacts.RevisedData();
	TimeFrequencyData &original = artifacts.OriginalData();
	TimeFrequencyData &contaminated = artifacts.ContaminatedData();
	size_t timeSteps = model.ImageWidth();
	Mask2DPtr mask = Mask2D::CreateCopy(contaminated.GetSingleMask());
	std::multimap<double, size_t> orderedQualities;
	size_t
		partCount = _partCount,
		selectCount = _selectionCount;
	if(partCount > original.ImageWidth())
		partCount = original.ImageWidth();
	if(selectCount > partCount)
		selectCount = partCount;

	Image2DCPtr
		originalImg = original.GetSingleImage(),
		modelImg = model.GetSingleImage();

	for(size_t p = 0; p < partCount; ++p)
	{
		size_t
			startX = p * timeSteps / partCount,
			endX = (p+1) * timeSteps / partCount;
		double quality = RFIStatistics::DataQuality(originalImg, modelImg, mask, startX, endX);
		orderedQualities.insert(std::pair<double, size_t>(quality, p));
	}
	for(size_t i=0;i<partCount - selectCount; ++i)
	{
		std::map<double, size_t>::iterator mi = orderedQualities.begin();
		size_t part = mi->second;
		orderedQualities.erase(mi);
		size_t
			startX = part * timeSteps / partCount,
			endX = (part+1) * timeSteps / partCount;
		mask->SetAllVertically<true>(startX, endX);
	}
	contaminated.SetGlobalMask(mask);
}

void TimeSelectionAction::AutomaticSelection(ArtifactSet &artifacts)
{
	Image2DCPtr image = artifacts.ContaminatedData().GetSingleImage();
	SampleRowPtr channels = SampleRow::CreateEmpty(image->Width());
	Mask2DPtr mask = Mask2D::CreateCopy(artifacts.ContaminatedData().GetSingleMask());
	for(size_t x=0;x<image->Width();++x)
	{
		SampleRowPtr row = SampleRow::CreateFromColumnWithMissings(image, mask, x);
		channels->SetValue(x, row->RMSWithMissings());
	}
	bool change;
	MedianWindow<num_t>::SubtractMedian(channels, 512);
	do {
		num_t median = 0.0;
		num_t stddev = channels->StdDevWithMissings(0.0);
		change = false;
		for(size_t x=0;x<channels->Size();++x)
		{
			if(!channels->ValueIsMissing(x) && (channels->Value(x) - median > stddev * _threshold || median - channels->Value(x) > stddev * _threshold))
			{
				mask->SetAllVertically<true>(x);
				channels->SetValueMissing(x);
				change = true;
			}
		}
	} while(change);
	artifacts.ContaminatedData().SetGlobalMask(mask);
}

} // end of namespace
