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

#include <AOFlagger/rfi/strategy/spatialcompositionaction.h>

#include <AOFlagger/rfi/strategy/spatialmsimageset.h>

#include <AOFlagger/rfi/eigenvalue.h>

namespace rfiStrategy {
	void SpatialCompositionAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
	{
		size_t imageCount = artifacts.ContaminatedData().ImageCount();
		Image2DPtr images[imageCount];
		for(size_t p=0;p<imageCount;++p)
			images[p] = Image2D::CreateZeroImagePtr(artifacts.ContaminatedData().ImageWidth(), artifacts.ContaminatedData().ImageHeight());

		std::string filename = artifacts.ImageSet()->File();
		SpatialMSImageSet set(filename);
		ImageSetIndex *index = set.StartIndex();
		size_t progressStep = 0, totalProgress = artifacts.ContaminatedData().ImageWidth() * artifacts.ContaminatedData().ImageHeight()/256;
		while(index->IsValid())
		{
			TimeFrequencyData *data = set.LoadData(*index);
			SpatialMatrixMetaData metaData(set.SpatialMetaData(*index));
			for(size_t p=0;p!=imageCount;++p)
			{
				switch(_operation)
				{
					case SumCrossCorrelationsOperation:
						images[p]->SetValue(metaData.TimeIndex(), metaData.ChannelIndex(), sumCrossCorrelations(data->GetImage(p)));
						break;
					case EigenvalueDecompositionOperation:
						num_t value = eigenvalue(data->GetImage(p), data->GetImage(p+1));
						images[p]->SetValue(metaData.TimeIndex(), metaData.ChannelIndex(), value);
						images[p+1]->SetValue(metaData.TimeIndex(), metaData.ChannelIndex(), 0.0);
						++p;
						break;
				}
			}
			delete data;
			index->Next();
			++progressStep;
			progress.OnProgress(progressStep/256, totalProgress);
		}
		delete index;

		TimeFrequencyData newRevisedData = artifacts.RevisedData();
		for(size_t p=0;p<imageCount;++p)
			newRevisedData.SetImage(p, images[p]);
		
		newRevisedData.SetMaskFrom(artifacts.RevisedData());

		TimeFrequencyData *contaminatedData =
			TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), newRevisedData);
		contaminatedData->SetMaskFrom(artifacts.ContaminatedData());

		artifacts.SetRevisedData(newRevisedData);
		artifacts.SetContaminatedData(*contaminatedData);

		delete contaminatedData;

	}

	num_t SpatialCompositionAction::sumCrossCorrelations(Image2DCPtr image) const
	{
		num_t sum = 0;
		for(size_t y=0;y<image->Height();++y)
		{
			for(size_t x=0;x<y;++x)
				sum += image->Value(x, y);
		}
		return sum;
	}

	num_t SpatialCompositionAction::eigenvalue(Image2DCPtr real, Image2DCPtr imaginary) const
	{
		num_t ev = Eigenvalue::Compute(real, imaginary);
		return ev;
	}
}

