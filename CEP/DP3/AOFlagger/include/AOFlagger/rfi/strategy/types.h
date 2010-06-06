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

#ifndef RFI_STRATEGY_TYPES
#define RFI_STRATEGY_TYPES

namespace rfiStrategy
{
	class Action;
	class ActionBlock;
	class ActionContainer;
	class ActionFactory;
	class ArtifactSet;
	class CombineFlagResults;
	class ForEachPolarisationBlock;
	class ImageSet;
	class ImageSetIndex;
	class IterationBlock;
	class MSImageSet;
	class Strategy;

	enum BaselineSelection
	{
		All, CrossCorrelations, AutoCorrelations, EqualToCurrent, AutoCorrelationsOfCurrentAntennae, Current
	};
}

#define STRATEGY_FILE_FORMAT_VERSION 1.0
#define STRATEGY_FILE_FORMAT_VERSION_REQUIRED 1.0
#define STRATEGY_FILE_READER_VERSION_REQUIRED 1.0

#endif // RFI_STRATEGY_TYPES
