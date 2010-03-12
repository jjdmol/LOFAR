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
#include <AOFlagger/rfi/strategy/foreachmsaction.h>

#include <boost/filesystem.hpp>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/imageset.h>

namespace rfiStrategy {

void ForEachMSAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
{
	unsigned taskIndex = 0;
	
	for(std::vector<std::string>::const_iterator i=_filenames.begin();i!=_filenames.end();++i)
	{
		std::string filename = *i;
		
		progress.OnStartTask(taskIndex, _filenames.size(), std::string("Processing measurement set ") + filename);
		
		ImageSet *imageSet = ImageSet::Create(filename);
		imageSet->Initialize();
		ImageSetIndex *index = imageSet->StartIndex();
		artifacts.SetImageSet(imageSet);
		artifacts.SetImageSetIndex(index);
		
		ActionBlock::Perform(artifacts, progress);
		
		artifacts.SetNoImageSet();
		delete index;
		delete imageSet;
	
		progress.OnEndTask();
		
		++taskIndex;
	}
}

void ForEachMSAction::AddDirectory(const std::string &name)
{
  // get all files ending in .MS
  boost::filesystem::path dir_path(name);
  boost::filesystem::directory_iterator end_it;

  for(boost::filesystem::directory_iterator it(dir_path); it != end_it; ++it) {
    if( is_directory(it->status()) && extension(it->path()) == ".MS" ) {
      _filenames.push_back( it->path().string() );
    }
  }
}

}
