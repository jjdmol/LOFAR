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
#include <AOFlagger/rfi/strategy/loadflagsaction.h>

#include <boost/thread.hpp>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/msimageset.h>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/util/stopwatch.h>

namespace rfiStrategy {
	void LoadFlagsAction::Perform(class ArtifactSet &artifacts, class ProgressListener &)
	{
		Stopwatch watch(true);

		ImageSet *imageSet = artifacts.ImageSet();
		ImageSetIndex *index = artifacts.ImageSetIndex();

		MSImageSet *msImageSet = dynamic_cast<MSImageSet*>(imageSet);

		boost::mutex::scoped_lock lock(artifacts.IOMutex());
		if(msImageSet != 0)
		{
			msImageSet->SetReadFlags(true);
			switch(artifacts.RevisedData().Polarisation())
			{
				case DipolePolarisation:
					msImageSet->SetReadAllPolarisations();
					break;
				case StokesIPolarisation:
					msImageSet->SetReadStokesI();
					break;
				case AutoDipolePolarisation:
					msImageSet->SetReadDipoleAutoPolarisations();
					break;
				case CrossDipolePolarisation:
				default:
					throw BadUsageException("Unimplemented polarisation type for reading flags");
					break;
			}
		}

		if(_joinFlags)
		{
			TimeFrequencyData copy(artifacts.OriginalData());
			
			imageSet->LoadFlags(*index, copy);
			lock.unlock();

			artifacts.OriginalData().JoinMask(copy);
			artifacts.ContaminatedData().JoinMask(copy);
		} else {
			TimeFrequencyData &data = artifacts.OriginalData();
	
			imageSet->LoadFlags(*index, data);
			lock.unlock();
	
			artifacts.ContaminatedData().SetMaskFrom(data);
		}

		std::cout << "Flags load time: " << watch.ToString() << std::endl;
	}
} // end of namespace
