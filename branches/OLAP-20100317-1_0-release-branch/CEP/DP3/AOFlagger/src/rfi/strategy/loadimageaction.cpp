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

#include <boost/thread.hpp>

#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/msio/antennainfo.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/imageset.h>
#include <AOFlagger/rfi/strategy/loadimageaction.h>
#include <AOFlagger/rfi/strategy/msimageset.h>

namespace rfiStrategy {

	void LoadImageAction::Perform(class ArtifactSet &artifacts, class ProgressListener &listener)
	{
		ImageSet *imageSet = artifacts.ImageSet();
		ImageSetIndex *index = artifacts.ImageSetIndex();

		boost::mutex::scoped_lock lock(artifacts.IOMutex());

		// Note that setting the image set needs to be done ATOMICLY with reading the data, as another
		// thread might change the settings otherwise...
		MSImageSet *msImageSet = dynamic_cast<MSImageSet*>(imageSet);
		if(msImageSet != 0)
		{
			switch(_polarisations)
			{
				case ReadAllPol:
					msImageSet->SetReadAllPolarisations();
					break;
				case ReadAutoPol:
					msImageSet->SetReadDipoleAutoPolarisations();
					break;
				case ReadSI:
					msImageSet->SetReadStokesI();
					break;
			}
			msImageSet->SetImageKind(_imageKind);
			msImageSet->SetReadFlags(false);
		}

		TimeFrequencyData *newData = imageSet->LoadData(*index);
		TimeFrequencyMetaDataCPtr newMetaData = imageSet->LoadMetaData(*index);

		lock.unlock();
		
		artifacts.SetOriginalData(*newData);
		artifacts.SetContaminatedData(*newData);
		TimeFrequencyData *zero = new TimeFrequencyData(*newData);
		zero->SetImagesToZero();
		artifacts.SetRevisedData(*zero);
		delete zero;
		artifacts.SetMetaData(newMetaData);
		delete newData;
	}
} // end of namespace
