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
#ifndef AOFLAGGER_UNITTEST_H
#define AOFLAGGER_UNITTEST_H

#include <string>
#include <vector>
#include <iostream>

#include <AOFlagger/test/testingtools/testitem.h>

class UnitTest : public TestItem {
	public:
		UnitTest(const std::string &name) : _name(name)
		{
		}
		
		virtual ~UnitTest()
		{
			for(std::vector<RunnableTest*>::iterator i=_tests.begin();i!=_tests.end();++i)
			{
				delete *i;
			}
		}
		
		template<typename Functor>
		void AddTest(Functor testFunctor, const std::string &name)
		{
			_tests.push_back(new SpecificTest<Functor>(testFunctor, name));
		}
		
		void Run()
		{
			for(std::vector<RunnableTest*>::iterator i=_tests.begin();i!=_tests.end();++i)
			{
				std::cout << "* Running subtest '" << (*i)->_name << "'... ";
				try {
					(*i)->Run();
					std::cout << "SUCCESS\n";
				} catch(std::exception &exception)
				{
					std::cout << "FAIL\nDetails of failure:\n" << exception.what() << '\n';
				}
			}
		}
		
		const std::string &Name() const { return _name; }
	private:
		struct RunnableTest {
			public:
				RunnableTest(const std::string &name) : _name(name) { }
				virtual ~RunnableTest() { }
				virtual void Run() = 0;
				std::string _name;
			private:
				RunnableTest(const RunnableTest &) { }
				void operator=(const RunnableTest &) { }
		};
		
		template<typename Functor>
		struct SpecificTest : public RunnableTest {
			SpecificTest(Functor functor, const std::string &name) : RunnableTest(name), _functor(functor)
			{
			}
			virtual void Run()
			{
				_functor();
			}
			Functor _functor;
		};

		std::vector<RunnableTest*> _tests;
		std::string _name;
};

#endif
