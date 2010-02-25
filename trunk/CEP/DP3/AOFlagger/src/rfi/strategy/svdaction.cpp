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

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/svdaction.h>

#include <AOFlagger/rfi/svdmitigater.h>

namespace rfiStrategy {

	void SVDAction::Perform(ArtifactSet &artifacts, class ProgressListener &listener)
	{
		SVDMitigater mitigater;
		mitigater.Initialize(artifacts.ContaminatedData());
		mitigater.SetRemoveCount(_singularValueCount);
		for(size_t i=0;i<mitigater.TaskCount();++i)
		{
			mitigater.PerformFit(i);
			listener.OnProgress(i+1, mitigater.TaskCount());
		}

		TimeFrequencyData newRevisedData = mitigater.Background();
		newRevisedData.SetMaskFrom(artifacts.RevisedData());

		TimeFrequencyData *contaminatedData =
			TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), newRevisedData);
		contaminatedData->SetMaskFrom(artifacts.ContaminatedData());

		artifacts.SetRevisedData(newRevisedData);
		artifacts.SetContaminatedData(*contaminatedData);

		delete contaminatedData;
	}

} // namespace rfiStrategy
