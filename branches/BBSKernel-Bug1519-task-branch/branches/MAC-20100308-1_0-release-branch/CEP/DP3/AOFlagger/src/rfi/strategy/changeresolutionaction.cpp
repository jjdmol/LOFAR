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
#include <AOFlagger/rfi/strategy/changeresolutionaction.h>

#include <AOFlagger/rfi/strategy/artifactset.h>

namespace rfiStrategy {
	
	void ChangeResolutionAction::Perform(class ArtifactSet &artifacts, class ProgressListener &listener)
	{
		if(_decreaseFactor != 1)
		{
			ArtifactSet artifactsCopy(artifacts);
			artifactsCopy.SetNoImageSet();
	
			DecreaseSize(artifactsCopy.OriginalData());
			DecreaseSize(artifactsCopy.ContaminatedData());
			DecreaseSize(artifactsCopy.RevisedData());
	
			ActionBlock::Perform(artifactsCopy, listener);
	
			IncreaseSize(artifacts.OriginalData(), artifactsCopy.OriginalData());
			IncreaseSize(artifacts.ContaminatedData(), artifactsCopy.ContaminatedData());
			IncreaseSize(artifacts.RevisedData(), artifactsCopy.RevisedData());
		} else {
			ActionBlock::Perform(artifacts, listener);
		}
	}

	void ChangeResolutionAction::DecreaseSize(TimeFrequencyData &timeFrequencyData)
	{
		size_t imageCount = timeFrequencyData.ImageCount();
		for(size_t i=0;i<imageCount;++i)
		{
			Image2DCPtr image = timeFrequencyData.GetImage(i);
			Image2DPtr newImage = image->ShrinkHorizontally(_decreaseFactor);
			timeFrequencyData.SetImage(i, newImage);
		}
		size_t maskCount = timeFrequencyData.MaskCount();
		for(size_t i=0;i<maskCount;++i)
		{
			Mask2DCPtr mask = timeFrequencyData.GetMask(i);
			Mask2DPtr newMask = mask->ShrinkHorizontally(_decreaseFactor);
			timeFrequencyData.SetMask(i, newMask);
		}
	}

	void ChangeResolutionAction::IncreaseSize(TimeFrequencyData &originalData, TimeFrequencyData &changedData)
	{
		size_t maskCount = changedData.MaskCount();
		for(size_t i=0;i<maskCount;++i)
		{
			Mask2DCPtr mask = changedData.GetMask(i);
			Mask2DPtr newMask = Mask2D::CreateUnsetMaskPtr(originalData.ImageWidth(), originalData.ImageHeight());
			newMask->EnlargeHorizontallyAndSet(mask, _decreaseFactor);
			originalData.SetMask(i, newMask);
		}
	}
}
