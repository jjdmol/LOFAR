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
#include <AOFlagger/strategy/actions/timeselectionaction.h>

#include <map>

#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/samplerow.h>

#include <AOFlagger/strategy/algorithms/medianwindow.h>
#include <AOFlagger/strategy/algorithms/rfiplots.h>
#include <AOFlagger/strategy/algorithms/rfistatistics.h>

#include <AOFlagger/strategy/control/artifactset.h>

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

/**
 * Automatic selection selects all timesteps which RMS is higher than some value relative to the stddev of
 * all timesteps.
 */
void TimeSelectionAction::AutomaticSelection(ArtifactSet &artifacts)
{
	Image2DCPtr image = artifacts.ContaminatedData().GetSingleImage();
	SampleRowPtr timesteps = SampleRow::CreateEmpty(image->Width());
	Mask2DPtr mask = Mask2D::CreateCopy(artifacts.ContaminatedData().GetSingleMask());
	for(size_t x=0;x<image->Width();++x)
	{
		SampleRowPtr row = SampleRow::CreateFromColumnWithMissings(image, mask, x);
		timesteps->SetValue(x, row->RMSWithMissings());
	}
	bool change;
	MedianWindow<num_t>::SubtractMedian(timesteps, 512);
	do {
		num_t median = 0.0;
		num_t stddev = timesteps->StdDevWithMissings(0.0);
		change = false;
		for(size_t x=0;x<timesteps->Size();++x)
		{
			if(!timesteps->ValueIsMissing(x) && (timesteps->Value(x) - median > stddev * _threshold || median - timesteps->Value(x) > stddev * _threshold))
			{
				mask->SetAllVertically<true>(x);
				timesteps->SetValueMissing(x);
				change = true;
			}
		}
	} while(change);
	artifacts.ContaminatedData().SetGlobalMask(mask);
}

} // end of namespace
