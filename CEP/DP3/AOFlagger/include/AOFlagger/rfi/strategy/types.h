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

// The current file format version
// 1.0 : start
// 1.1 : add AddStatisticsAction
// 1.2 : add restore-originals property to Adapter
#define STRATEGY_FILE_FORMAT_VERSION 1.2

// The earliest format version which can be read by this version of the software
#define STRATEGY_FILE_FORMAT_VERSION_REQUIRED 1.2

// The earliest software version which is required to read the written files
#define STRATEGY_FILE_READER_VERSION_REQUIRED 1.2

#endif // RFI_STRATEGY_TYPES
