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

#include <AOFlagger/remote/clusteredobservation.h>

#include <stdexcept>

#include <LMWCommon/VdsDesc.h>
#include <auto_ptr.h>

namespace aoRemote
{

ClusteredObservation::ClusteredObservation()
{
}

ClusteredObservation *ClusteredObservation::LoadFromVds(const std::string &vdsFilename)
{
	LOFAR::CEP::VdsDesc vdsDesc(vdsFilename);
	const std::vector<LOFAR::CEP::VdsPartDesc> &parts = vdsDesc.getParts();
	
	std::auto_ptr<ClusteredObservation> cObs(new ClusteredObservation());
	
	for(std::vector<LOFAR::CEP::VdsPartDesc>::const_iterator i=parts.begin();i!=parts.end();++i)
	{
		const std::string &filename = i->getFileName();
		const std::string &filesystem = i->getFileSys();
		
		size_t separatorPos = filesystem.find(':');
		if(separatorPos == std::string::npos || separatorPos == 0)
			throw std::runtime_error("One of the file system descriptors in the VDS file has an unexpected format");
		const std::string hostname = filesystem.substr(0, separatorPos);
		
		ClusteredObservationItem newItem(filename, hostname);
		cObs->AddItem(newItem);
	}
	
	return cObs.release();
}

}
