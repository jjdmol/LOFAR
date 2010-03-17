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
#ifndef FOREACHMSACTION_H
#define FOREACHMSACTION_H

#include "actionblock.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
namespace rfiStrategy {

	class ForEachMSAction  : public ActionBlock {
		public:
			ForEachMSAction()
			{
			}
			~ForEachMSAction()
			{
			}
			virtual std::string Description()
			{
				return "For each measurement set";
			}
			virtual void Initialize()
			{
			}
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress);
			void AddDirectory(const std::string &name);

			std::vector<std::string> &Filenames() { return _filenames; }
			const std::vector<std::string> &Filenames() const { return _filenames; }
			virtual ActionType Type() const { return ForEachMSActionType; }
		private:
			std::vector<std::string> _filenames;
	};

}

#endif
