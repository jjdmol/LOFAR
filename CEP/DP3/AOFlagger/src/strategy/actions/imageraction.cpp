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
#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/strategy/actions/imageraction.h>

#include <boost/thread/mutex.hpp>

#include <AOFlagger/util/progresslistener.h>

namespace rfiStrategy {
	void ImagerAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
	{
		boost::mutex::scoped_lock lock(_imagerMutex);
		UVImager *imager = artifacts.Imager();
		if(imager == 0)
			throw BadUsageException("No imager available to create image.");
		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();
		if(data.PolarisationCount() > 1)
		{
			TimeFrequencyData *tmp = data.CreateTFData(StokesIPolarisation);
			data = *tmp;
			delete tmp;
		}

		progress.OnStartTask(*this, 0, 1, "Imaging baseline");
		for(size_t y=0;y<data.ImageHeight();++y)
		{
			imager->Image(data, metaData, y);
			progress.OnProgress(*this, y, data.ImageHeight());
		}
		progress.OnEndTask(*this);
	}
}
