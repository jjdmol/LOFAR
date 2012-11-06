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
#include <AOFlagger/strategy/actions/foreachmsaction.h>

#include <boost/filesystem.hpp>

#include <AOFlagger/msio/measurementset.h>

#include <AOFlagger/strategy/actions/strategyaction.h>

#include <AOFlagger/strategy/control/artifactset.h>

#include <AOFlagger/strategy/imagesets/imageset.h>
#include <AOFlagger/strategy/imagesets/msimageset.h>

#include <AOFlagger/util/aologger.h>
#include <AOFlagger/util/progresslistener.h>

namespace rfiStrategy {

void ForEachMSAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
{
	unsigned taskIndex = 0;
	
	FinishAll();

	for(std::vector<std::string>::const_iterator i=_filenames.begin();i!=_filenames.end();++i)
	{
		std::string filename = *i;
		
		progress.OnStartTask(*this, taskIndex, _filenames.size(), std::string("Processing measurement set ") + filename);
		
		bool skip = false;
		if(_skipIfAlreadyProcessed)
		{
			MeasurementSet set(filename);
			if(set.HasRFIConsoleHistory())
			{
				skip = true;
				AOLogger::Info << "Skipping " << filename << ",\n"
					"because the set contains AOFlagger history and -skip-flagged was given.\n";
			}
		}
		
		if(!skip)
		{
			ImageSet *imageSet = ImageSet::Create(filename, _baselineIOMode, _readUVW);
			if(dynamic_cast<MSImageSet*>(imageSet))
			{ 
				MSImageSet *msImageSet = static_cast<MSImageSet*>(imageSet);
				msImageSet->SetDataColumnName(_dataColumnName);
				msImageSet->SetSubtractModel(_subtractModel);
			}
			imageSet->Initialize();
			ImageSetIndex *index = imageSet->StartIndex();
			artifacts.SetImageSet(imageSet);
			artifacts.SetImageSetIndex(index);

			InitializeAll();
			
			ActionBlock::Perform(artifacts, progress);
			
			FinishAll();

			artifacts.SetNoImageSet();
			delete index;
			delete imageSet;

			writeHistory(*i);
		}
	
		progress.OnEndTask(*this);

		
		++taskIndex;
	}

	InitializeAll();
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

void ForEachMSAction::writeHistory(const std::string &filename)
{
	if(GetChildCount() != 0)
	{
		MeasurementSet ms(filename);
		const Strategy *strategy = 0;
		if(GetChildCount() == 1 && dynamic_cast<const Strategy*>(&GetChild(0)) != 0)
		{
			strategy = static_cast<const Strategy*>(&GetChild(0));
		} else {
			const ActionContainer *root = GetRoot();
			if(dynamic_cast<const Strategy*>(root) != 0)
				strategy = static_cast<const Strategy*>(root);
		}
		AOLogger::Debug << "Adding strategy to history table of MS...\n";
		if(strategy != 0) {
			try {
				ms.AddAOFlaggerHistory(*strategy, _commandLineForHistory);
			} catch(std::exception &e)
			{
				AOLogger::Warn << "Failed to write history to MS: " << e.what() << '\n';
			}
		}
		else
			AOLogger::Error << "Could not find root strategy to write to Measurement Set history table!\n";
	}
}

}
