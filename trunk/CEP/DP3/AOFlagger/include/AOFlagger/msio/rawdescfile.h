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
#ifndef RAWDESCFILE_H
#define RAWDESCFILE_H

#include <fstream>
#include <string>
#include <vector>

#include <AOFlagger/util/aologger.h>

class RawDescFile
{
	public:
		explicit RawDescFile(const std::string &filename) :
		_filename(filename)
		{
			readFile();
		}
		
		size_t GetCount() const { return _sets.size(); }
		
		std::string GetSet(size_t index) const { return _sets[index]; }
		
		const std::string &Filename() const { return _filename; }
		
		const double TimeResolution() const { return _timeRes; }
	private:
		const std::string _filename;
		std::vector<std::string> _sets;
		double _timeRes;
		
		void readFile()
		{
			std::ifstream file(_filename.c_str());
			std::string l;
			std::getline(file, l);
			_timeRes = atof(l.c_str());
			while(file.good())
			{
				std::getline(file, l);
				if(l != "")
					_sets.push_back(l);
			}
		}
};

#endif
