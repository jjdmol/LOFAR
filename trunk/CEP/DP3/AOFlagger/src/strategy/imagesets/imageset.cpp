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

#include <AOFlagger/strategy/imagesets/imageset.h>

#include <AOFlagger/strategy/imagesets/fitsimageset.h>
#include <AOFlagger/strategy/imagesets/msimageset.h>
#include <AOFlagger/strategy/imagesets/parmimageset.h>
#include <AOFlagger/strategy/imagesets/rspimageset.h>
#include <AOFlagger/strategy/imagesets/timefrequencystatimageset.h>

namespace rfiStrategy {
	ImageSet *ImageSet::Create(const std::string &file, bool indirectReader, bool readUVW)
	{
		if(IsFitsFile(file))
			return new FitsImageSet(file);
		else if(IsRawFile(file))
			return new RSPImageSet(file);
		else if(IsParmFile(file))
			return new ParmImageSet(file);
		else if(IsTimeFrequencyStatFile(file))
			return new TimeFrequencyStatImageSet(file);
		else {
			MSImageSet *set = new MSImageSet(file, indirectReader);
			set->SetReadUVW(readUVW);
			return set;
		}
	}
	
	bool ImageSet::IsFitsFile(const std::string &file)
	{
		return
		(file.size() > 4 && file.substr(file.size()- 4) == ".UVF")
		||
		(file.size() > 5 && file.substr(file.size() - 5) == ".fits" );
	}
	
	bool ImageSet::IsRawFile(const std::string &file)
	{
		return file.size() > 4 && file.substr(file.size()-4) == ".raw";
	}
	
	bool ImageSet::IsParmFile(const std::string &file)
	{
		return file.size() >= 10 && file.substr(file.size()-10) == "instrument";
	}
	
	bool ImageSet::IsTimeFrequencyStatFile(const std::string &file)
	{
		return
		(file.size()>=24 && file.substr(file.size()-24) == "counts-timefreq-auto.txt")
		||
		(file.size()>=25 && file.substr(file.size()-25) == "counts-timefreq-cross.txt");
	}
	
	bool ImageSet::IsMSFile(const std::string &file)
	{
		return (!IsFitsFile(file)) && (!IsRawFile(file)) && (!IsParmFile(file)) && (!IsTimeFrequencyStatFile(file));
	}
}
