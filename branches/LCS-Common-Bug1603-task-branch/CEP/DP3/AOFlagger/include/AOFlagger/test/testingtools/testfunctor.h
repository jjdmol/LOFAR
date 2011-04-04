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
#ifndef AOFLAGGER_TESTFUNCTOR_H
#define AOFLAGGER_TESTFUNCTOR_H

#include <string>
#include <stdexcept>
#include <sstream>
#include <typeinfo>

class TestFunctor {
	public:
		TestFunctor()
		{
		}
		
		~TestFunctor()
		{
		}
		
		void AssertEquals(bool first, bool second) const
		{
			if(!(first == second))
			{
				throw std::runtime_error(
					std::string("AssertEquals failed: ") + boolToString(first) + " == " + boolToString(second) + " was false, type = " + typeid(bool).name()
				);
			}
		}
		
		void AssertEquals(bool first, bool second, const std::string &description) const
		{
			if(!(first == second))
			{
				throw std::runtime_error(
					std::string("AssertEquals failed on test '") + description + "': "  + boolToString(first) + " == " + boolToString(second) + ", type = " + typeid(bool).name()
				);
			}
		}
		
		void AssertEquals(const std::string &first, const std::string &second) const
		{
			if(!(first == second))
			{
				throw std::runtime_error(
					std::string("AssertEquals failed: '") + first + "' == '" + second + "' was false, type = " + typeid(const std::string &).name()
				);
			}
		}
		
		void AssertEquals(const std::string &first, const std::string &second, const std::string &description) const
		{
			if(!(first == second))
			{
				throw std::runtime_error(
					std::string("AssertEquals failed on test '") + description + "': '" + first + "' == '" + second + "' was false, type = " + typeid(bool).name()
				);
			}
		}
		
		template<typename T>
		void AssertEquals(T first, T second) const
		{
			if(!(first == second))
			{
				std::stringstream s;
				s << "AssertEquals failed: " << first << " == " << second << ", type = "
				<< typeid(T).name();
				throw std::runtime_error(s.str());
			}
		}
		
		template<typename T>
		void AssertEquals(T first, T second, const std::string &description) const
		{
			if(!(first == second))
			{
				std::stringstream s;
				s << "AssertEquals failed on test '" << description << "': " << first << " == " << second << " was false, type = "
				<< typeid(T).name();
				throw std::runtime_error(s.str());
			}
		}
	private:
		const char *boolToString(bool value) const
		{
			if(value)
				return "true";
			else
				return "false";
		}
};

#endif
