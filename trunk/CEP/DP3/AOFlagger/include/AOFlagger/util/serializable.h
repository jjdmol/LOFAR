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
#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include <iostream>

class Serializable
{
	public:
		virtual void Serialize(std::ostream &stream) const = 0;
		
	protected:
		template<typename T>
		void serializeToInt64(std::ostream &stream, T value) const
		{
			uint64_t val64t = value;
			stream.write(reinterpret_cast<char *>(&val64t), sizeof(val64t));
		}
		
		template<typename T>
		void serializeToInt32(std::ostream &stream, T value) const
		{
			uint32_t val32t = value;
			stream.write(reinterpret_cast<char *>(&val32t), sizeof(val32t));
		}
		
		template<typename T>
		void serializeToDouble(std::ostream &stream, T value) const
		{
			double valDouble = value;
			stream.write(reinterpret_cast<char *>(&valDouble), sizeof(valDouble));
		}
};

#endif
