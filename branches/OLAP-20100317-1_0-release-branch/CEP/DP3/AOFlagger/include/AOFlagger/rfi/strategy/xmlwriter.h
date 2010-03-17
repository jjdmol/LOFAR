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
#ifndef RFISTRATEGYXMLWRITER_H
#define RFISTRATEGYXMLWRITER_H

#include <sstream>
#include <string>
#include <stdexcept>

#include <libxml/xmlwriter.h>

namespace rfiStrategy {

	class XmlWriteError : public std::runtime_error
	{
		public:
			XmlWriteError(const std::string &arg) : std::runtime_error(arg) { }
	};

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class XmlWriter{
		public:
			XmlWriter();
			~XmlWriter();

			void WriteStrategy(const class Strategy &strategy, const std::string &filename);
		private:
			xmlTextWriterPtr _writer;

			void writeAction(const class Action &action);
			void writeContainerItems(const class ActionContainer &actionContainer);

			void writeAdapter(const class Adapter &action);
			void writeChangeResolutionAction(const class ChangeResolutionAction &action);
			void writeCombineFlagResults(const class CombineFlagResults &action);
			void writeForEachBaselineAction(const class ForEachBaselineAction &action);
			void writeForEachMSAction(const class ForEachMSAction &action);
			void writeForEachPolarisationBlock(const class ForEachPolarisationBlock &action);
			void writeFrequencySelectionAction(const class FrequencySelectionAction &action);
			void writeFringeStopAction(const class FringeStopAction &action);
			void writeImagerAction(const class ImagerAction &action);
			void writeIterationBlock(const class IterationBlock &action);
			void writeLoadFlagsAction(const class LoadFlagsAction &action);
			void writeLoadImageAction(const class LoadImageAction &action);
			void writeSetFlaggingAction(const class SetFlaggingAction &action);
			void writeSetImageAction(const class SetImageAction &action);
			void writeSlidingWindowFitAction(const class SlidingWindowFitAction &action);
			void writeStatisticalFlagAction(const class StatisticalFlagAction &action);
			void writeStrategy(const class Strategy &action);
			void writeSVDAction(const class SVDAction &action);
			void writeThresholdAction(const class ThresholdAction &action);
			void writeTimeSelectionAction(const class TimeSelectionAction &action);
			void writeWriteFlagsAction(const class WriteFlagsAction &action);

			inline void Start(const char *element) const
			{
				if(xmlTextWriterStartElement(_writer, BAD_CAST element) < 0)
					throw XmlWriteError("Start(element): Error at xmlTextWriterStartElement");
			}
			inline void End() const
			{
				if (xmlTextWriterEndElement(_writer) < 0)
					throw XmlWriteError("End(): Error at xmlTextWriterEndElement");
			}
			inline void Attribute(const char *attribute, const char *value) const
			{ 
				if(xmlTextWriterWriteAttribute(_writer, BAD_CAST attribute, BAD_CAST value) < 0)
					throw XmlWriteError("Attribute(..): Error at xmlTextWriterWriteAttribute");
			}
			inline void Write(const char *element, const char *value) const
			{
				if(xmlTextWriterWriteElement(_writer, BAD_CAST element, BAD_CAST value) < 0)
					throw XmlWriteError("Write(..): Error at xmlTextWriterWriteElement");
			}
			template<typename ValueType>
			inline void Write(const char *element, ValueType value) const
			{
				std::stringstream s;
				s << value;
				Write(element, s.str().c_str());
			}
	};

}

#endif
