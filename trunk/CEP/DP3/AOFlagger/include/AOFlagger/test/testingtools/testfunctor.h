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
		
		template <typename T>
		void throwComparisonError(T actual, T expected, const std::type_info &type) const
		{
			std::stringstream s("AssertEquals failed: ");
			s << actual << " == " << expected << " was false, type = " << type.name() << "\n ("
			<< expected << " was expected, " << actual << " was the actual value)";
			throw std::runtime_error(s.str());
		}
		
		template <typename T>
		void throwComparisonError(T actual, T expected, const std::type_info &type, const std::string &description) const
		{
			std::stringstream s("AssertEquals failed on test '");
			s << description << "': " << actual << " == " << expected << " was false, type = " << type.name()
			<< "\n ("
			<< expected << " was expected, " << actual << " was the actual value)";
			throw std::runtime_error(s.str());
		}
		
		void AssertEquals(bool actual, bool expected) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(boolToString(actual), boolToString(expected), typeid(bool));
			}
		}
		
		void AssertEquals(bool actual, bool expected, const std::string &description) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(boolToString(actual), boolToString(expected), typeid(bool), description);
			}
		}
		
		void AssertEquals(const std::string &actual, const std::string &expected) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(std::string("'") + actual + "'", std::string("'") + expected + "'", typeid(const std::string &));
			}
		}
		
		void AssertEquals(const std::string &actual, const std::string &expected, const std::string &description) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(std::string("'") + actual + "'", std::string("'") + expected + "'", typeid(const std::string &), description);
			}
		}
		
		template<typename T>
		void AssertEquals(T actual, T expected) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(actual, expected, typeid(T));
			}
		}
		
		template<typename T>
		void AssertEquals(T actual, T expected, const std::string &description) const
		{
			if(!(actual == expected))
			{
				throwComparisonError(actual, expected, typeid(T), description);
			}
		}
		
		void AssertEquals(float actual, double expected) const
		{ AssertEquals<float>(actual, expected); }
		
		void AssertEquals(double actual, float expected) const
		{ AssertEquals<float>(actual, expected); }
		
		void AssertEquals(float actual, long double expected) const
		{ AssertEquals<float>(actual, expected); }
		
		void AssertEquals(long double actual, float expected) const
		{ AssertEquals<float>(actual, expected); }
		
		void AssertEquals(double actual, long double expected) const
		{ AssertEquals<double>(actual, expected); }
		
		void AssertEquals(long double actual, double expected) const
		{ AssertEquals<double>(actual, expected); }
		
		void AssertEquals(float actual, double expected, const std::string description) const
		{ AssertEquals<float>(actual, expected, description); }
		
		void AssertEquals(double actual, float expected, const std::string description) const
		{ AssertEquals<float>(actual, expected, description); }
		
		void AssertEquals(float actual, long double expected, const std::string description) const
		{ AssertEquals<float>(actual, expected, description); }
		
		void AssertEquals(long double actual, float expected, const std::string description) const
		{ AssertEquals<float>(actual, expected, description); }
		
		void AssertEquals(double actual, long double expected, const std::string description) const
		{ AssertEquals<double>(actual, expected, description); }
		
		void AssertEquals(long double actual, double expected, const std::string description) const
		{ AssertEquals<double>(actual, expected, description); }
		
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
