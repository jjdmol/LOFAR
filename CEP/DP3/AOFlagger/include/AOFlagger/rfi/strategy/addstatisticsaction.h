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

#ifndef RFIADDSTATISTICSACTION_H
#define RFIADDSTATISTICSACTION_H

#include "actioncontainer.h"
#include "artifactset.h"

#include <string>

#include <AOFlagger/util/progresslistener.h>

#include <AOFlagger/rfi/rfistatistics.h>

namespace rfiStrategy {

	class AddStatisticsAction : public Action
	{
		public:
			AddStatisticsAction() { }

			virtual std::string Description()
			{
				return "Add to statistics";
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &)
			{
				if(_comparison)
					statistics.Add(artifacts.ContaminatedData(), artifacts.MetaData());
				else
					statistics.Add(artifacts.ContaminatedData(), artifacts.MetaData(), artifacts.OriginalData().GetSingleMask());
			}
			virtual ActionType Type() const { return AddStatisticsActionType; }
			
			void SetFilePrefix(const std::string &filePrefix) { statistics.SetFilePrefix(filePrefix); }
			const std::string &FilePrefix() const { return statistics.FilePrefix(); }

			bool CompareOriginalAndAlternative() const { return _comparison; }
			void SetCompareOriginalAndAlternative(bool compare) { _comparison = compare; }
		private:
			RFIStatistics statistics;
			bool _comparison;
	};
}

#endif // RFIADDSTATISTICSACTION_H
