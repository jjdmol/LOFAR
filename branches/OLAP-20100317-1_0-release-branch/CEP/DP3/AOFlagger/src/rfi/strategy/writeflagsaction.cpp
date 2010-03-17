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
#include <AOFlagger/rfi/strategy/writeflagsaction.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/imageset.h>

#include <boost/thread.hpp>

namespace rfiStrategy {

	WriteFlagsAction::WriteFlagsAction()
	{
	}
	
	
	WriteFlagsAction::~WriteFlagsAction()
	{
	}
	
	void WriteFlagsAction::Perform(class ArtifactSet &artifacts, ProgressListener &progress)
	{
		if(!artifacts.HasImageSet())
			throw BadUsageException("No image set active: can not write flags");
		ImageSet *imageSet = artifacts.ImageSet();
		boost::mutex::scoped_lock lock(artifacts.IOMutex());
		imageSet->WriteFlags(*artifacts.ImageSetIndex(), artifacts.ContaminatedData());
		lock.unlock();
	}
}
