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
#ifndef AOFLAGGER_SCALEINVARIANTDILATIONTEST_H
#define AOFLAGGER_SCALEINVARIANTDILATIONTEST_H

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/msio/mask2d.h>

#include <AOFlagger/strategy/algorithms/scaleinvariantdilation.h>

#include <AOFlagger/util/rng.h>

class ScaleInvariantDilationTest : public UnitTest {
	public:
		ScaleInvariantDilationTest() : UnitTest("Scale invariant dilation")
		{
			AddTest(TestSingleDilation(), "Single dilation");
			AddTest(TestDilationSpeed(), "Dilation speed");
		}
		
	private:
		struct TestSingleDilation : public Asserter
		{
			void operator()();
		};
		struct TestDilationSpeed : public Asserter
		{
			void operator()();
		};
		
		static std::string maskToString(const bool *flags, unsigned size)
		{
			std::stringstream s;
			for(unsigned i=0;i<size;++i)
			{
				s << (flags[i] ? 'x' : ' ');
			}
			return s.str();
		}
		
		static void setMask(bool *flags, const std::string &str)
		{
			unsigned index = 0;
			for(std::string::const_iterator i = str.begin(); i!=str.end(); ++i)
			{
				flags[index] = ((*i) == 'x');
				++index;
			}
		}
};

inline void ScaleInvariantDilationTest::TestSingleDilation::operator()()
{
	bool *flags = new bool[40];
	setMask(flags, "     x    ");
	
	ScaleInvariantDilation::Dilate(flags, 10, 0.0);
	AssertEquals(maskToString(flags, 10), "     x    ", "Min=0.0, single center flagged, no enlarge");
	
	ScaleInvariantDilation::Dilate(flags, 10, 0.4);
	AssertEquals(maskToString(flags, 10), "     x    ", "Min=0.5, single center flagged, eta 0.4");
	
	ScaleInvariantDilation::Dilate(flags, 10, 0.5);
	AssertEquals(maskToString(flags, 10), "    xxx   ", "Min=0.6, from one to three samples");

	ScaleInvariantDilation::Dilate(flags, 10, 0.0);
	AssertEquals(maskToString(flags, 10), "    xxx   ");
	
	ScaleInvariantDilation::Dilate(flags, 10, 0.25);
	AssertEquals(maskToString(flags, 10), "   xxxxx  ");

	ScaleInvariantDilation::Dilate(flags, 10, 0.16);
	AssertEquals(maskToString(flags, 10), "   xxxxx  ");

	ScaleInvariantDilation::Dilate(flags, 10, 0.17);
	AssertEquals(maskToString(flags, 10), "  xxxxxxx ");

	ScaleInvariantDilation::Dilate(flags, 10, 1.0);
	AssertEquals(maskToString(flags, 10), "xxxxxxxxxx");
	
	setMask(flags, "xx xx     ");

	ScaleInvariantDilation::Dilate(flags, 10, 0.0);
	AssertEquals(maskToString(flags, 10), "xx xx     ");

	ScaleInvariantDilation::Dilate(flags, 10, 0.19);
	AssertEquals(maskToString(flags, 10), "xx xx     ", "Did not fill hole");

	setMask(flags, "xx xx     ");
	ScaleInvariantDilation::Dilate(flags, 10, 0.2);
	AssertEquals(maskToString(flags, 10), "xxxxx     ", "Fills hole");
	
	//              0    5    0    5    0    5    0    5    
	setMask(flags, "     xxxxxx xx xx x x xxx xxxxx         ");
	ScaleInvariantDilation::Dilate(flags, 40, 0.2);
	AssertEquals(maskToString(flags, 40), "    xxxxxxxxxxxxx x xxxxxxxxxxxx        ", "Input: '     xxxxxx xx xx x x xxx xxxxx         '");

	setMask(flags, "     xxxxxx xx xx x x xxx xxxxx         ");
	ScaleInvariantDilation::Dilate(flags, 40, 0.3);
	AssertEquals(maskToString(flags, 40), "   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ", "Input: '     xxxxxx xx xx x x xxx xxxxx         '");

	setMask(flags, "     xxxxxx xx xx x x xxx xxxxx         ");
	ScaleInvariantDilation::Dilate(flags, 40, 0.4);
	AssertEquals(maskToString(flags, 40), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  ");

	setMask(flags, "xxxxxxxxxxxxxxx       xxxxxxxxxxxxxxxxxx");
	ScaleInvariantDilation::Dilate(flags, 40, 0.3);
	AssertEquals(maskToString(flags, 40), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	
	setMask(flags, "      x   x  x xx xxx    ");
	ScaleInvariantDilation::Dilate(flags, 25, 0.5);
	AssertEquals(maskToString(flags, 25), "     xxxxxxxxxxxxxxxxxxxx");
	
	delete[] flags;
}

inline void ScaleInvariantDilationTest::TestDilationSpeed::operator()()
{
	const unsigned flagsSize = 100000;
	bool flags[flagsSize];
	for(unsigned i=0;i<flagsSize; ++i)
	{
		flags[i] = (RNG::Uniform() >= 0.2);
	}
	ScaleInvariantDilation::Dilate(flags, flagsSize, 0.1);
}

#endif
