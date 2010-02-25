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

#include <AOFlagger/rfi/fringestoppingfitter.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/fringestopaction.h>

namespace rfiStrategy {

	void FringeStopAction::Perform(ArtifactSet &artifacts, class ProgressListener &listener)
	{
		if(!artifacts.HasMetaData())
			throw BadUsageException("No meta data available for fringe stopping");
		if(!artifacts.MetaData()->HasField())
			throw BadUsageException("Baseline field data not set");
		if(!artifacts.MetaData()->HasBand())
			throw BadUsageException("Baseline band data not set");
		if(!artifacts.MetaData()->HasAntenna1() || !artifacts.MetaData()->HasAntenna2())
			throw BadUsageException("Baseline antenna info not set");
		if(!artifacts.MetaData()->HasObservationTimes())
			throw BadUsageException("Baseline observation times not set");
 
		FringeStoppingFitter fitter;
		fitter.SetFringesToConsider(_fringesToConsider);
		fitter.SetMaxWindowSize(_maxWindowSize);
		fitter.SetFitChannelsIndividually(_fitChannelsIndividually);
		fitter.SetMetaData(artifacts.MetaData());
		fitter.Initialize(artifacts.ContaminatedData());
		if(_onlyFringeStop)
			fitter.PerformFringeStop();
		else {
			for(size_t i=0;i<fitter.TaskCount();++i)
			{
				fitter.PerformFit(i);
				listener.OnProgress(i+1, fitter.TaskCount());
			}
		}

		TimeFrequencyData newContaminatedData = fitter.Background();
		newContaminatedData.SetMaskFrom(artifacts.ContaminatedData());

		TimeFrequencyData *newRevisedData =
			TimeFrequencyData::CreateTFDataFromDiff(artifacts.OriginalData(), newContaminatedData);
		newRevisedData->SetMaskFrom(artifacts.RevisedData());

		artifacts.SetRevisedData(*newRevisedData);
		artifacts.SetContaminatedData(newContaminatedData);

		delete newRevisedData;
	}

} // namespace rfiStrategy
