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

#ifndef GUI_IMAGESET_H
#define GUI_IMAGESET_H

#include <string>
#include <cstring>
#include <vector>

#include "../../msio/types.h"
#include "../../msio/timefrequencymetadata.h"

namespace rfiStrategy {

	class ImageSet;
	
	class ImageSetIndex {
		public:
			ImageSetIndex(ImageSet &set) : _set(&set) { }
			virtual ~ImageSetIndex() { }
			virtual void Previous() = 0;
			virtual void Next() = 0;
			virtual void LargeStepPrevious() = 0;
			virtual void LargeStepNext() = 0;
			virtual std::string Description() const = 0;
			virtual bool IsValid() const = 0;
			virtual ImageSetIndex *Copy() const = 0;
			void Reattach(ImageSet &imageSet) { _set = &imageSet; }
		protected:
			ImageSet &imageSet() const { return *_set; }
		private:
			class ImageSet *_set;
	};
	
	class ImageSet {
		public:
			virtual ~ImageSet() { };
			virtual ImageSet *Copy() = 0;

			virtual ImageSetIndex *StartIndex() = 0;
			
			/**
			 * Initialize is used to initialize the image set after it has been created and
			 * after all possible options have been set that might influence initialization
			 * (such as number of parts to read).
			 */
			virtual void Initialize() = 0;
			virtual std::string Name() = 0;
			virtual TimeFrequencyData *LoadData(ImageSetIndex &index) = 0;
			virtual void LoadFlags(ImageSetIndex &index, TimeFrequencyData &destination) = 0;
			virtual TimeFrequencyMetaDataCPtr LoadMetaData(ImageSetIndex &index) = 0;
			virtual void WriteFlags(ImageSetIndex &index, TimeFrequencyData &data) = 0;
			static class ImageSet *Create(const std::string &file);
			virtual size_t GetPart(ImageSetIndex &index) = 0;
			virtual size_t GetAntenna1(ImageSetIndex &index) = 0;
			virtual size_t GetAntenna2(ImageSetIndex &index) = 0;
	};

}

#endif
