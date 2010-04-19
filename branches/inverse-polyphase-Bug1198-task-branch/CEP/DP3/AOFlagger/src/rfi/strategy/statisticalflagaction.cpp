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

#include <AOFlagger/util/progresslistener.h>

#include <AOFlagger/rfi/statisticalflagger.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/statisticalflagaction.h>

namespace rfiStrategy {

	void StatisticalFlagAction::Perform(ArtifactSet &artifacts, class ProgressListener &)
	{
		TimeFrequencyData &data = artifacts.ContaminatedData();
		Mask2DPtr mask = Mask2D::CreateCopy(data.GetSingleMask());
		
		StatisticalFlagger::EnlargeFlags(mask, _enlargeTimeSize, _enlargeFrequencySize);
		//StatisticalFlagger::LineRemover(mask, (size_t) (_maxContaminatedTimesRatio * (double) mask->Width()), (size_t) (_maxContaminatedFrequenciesRatio * (double) mask->Height()));
		StatisticalFlagger::DensityTimeFlagger(mask, _minimumGoodTimeRatio);
		StatisticalFlagger::DensityFrequencyFlagger(mask, _minimumGoodFrequencyRatio);
		data.SetGlobalMask(mask);
		//artifacts.SetRevisedData(data);
	}

} // namespace rfiStrategy
