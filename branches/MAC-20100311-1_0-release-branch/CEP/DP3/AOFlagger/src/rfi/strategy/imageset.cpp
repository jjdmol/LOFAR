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

#include <AOFlagger/rfi/strategy/imageset.h>

#include <AOFlagger/rfi/strategy/fitsimageset.h>
#include <AOFlagger/rfi/strategy/msimageset.h>

namespace rfiStrategy {
	ImageSet *ImageSet::Create(const std::string &file)
	{
		size_t l = file.size();
		if((l > 4 && file.substr(file.length()-4) == ".UVF") || (l > 5 && file.substr(file.length() -5) == ".fits" ) )
			return new FitsImageSet(file);
		else
			return new MSImageSet(file);
	}
}
