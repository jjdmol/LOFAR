//# showinfo.cc: implementation of show contents of vectors and maps 
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: addbeaminfo.cc 18832 2011-11-09 17:22:32Z duscha $
//#
//# @author Sven Duscha


#include <lofar_config.h>         // this is needed for all .cc files!

#include <Beaminfo/showinfo.h>

// LOFAR
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <Common/SystemUtil.h>    // needed for basename

// Boost
#include <boost/date_time.hpp>

using namespace LOFAR;
using namespace boost;
using namespace LOFAR::OTDB;

//
// showTreeList
//
void showTreeList(const vector<OTDBtree>&	trees)
{
	cout << "treeID|Classif|Creator   |Creationdate        |Type|Campaign|Starttime" << endl;
	cout << "------+-------+----------+--------------------+----+--------+------------------" << endl;
	for (uint32	i = 0; i < trees.size(); ++i) {
		string row(formatString("%6d|%7d|%-10.10s|%-20.20s|%4d|%-8.8s|%s",
			trees[i].treeID(),
			trees[i].classification,
			trees[i].creator.c_str(),
			to_simple_string(trees[i].creationDate).c_str(),
			trees[i].type,
			trees[i].campaign.c_str(),
			to_simple_string(trees[i].starttime).c_str()));
		cout << row << endl;
	}

	cout << trees.size() << " records" << endl << endl;
}

//
// showNodeList
//
void showNodeList(const vector<OTDBnode>&	nodes)
{
	cout << "treeID|nodeID|parent|par.ID|index|leaf|name" << endl;
	cout << "------+------+------+------+-----+----+--------------------------------------" << endl;
	for (uint32	i = 0; i < nodes.size(); ++i) {
		string row(formatString("%6d|%6d|%6d|%6d|%5d|%s|%s",
			nodes[i].treeID(),
			nodes[i].nodeID(),
			nodes[i].parentID(),
			nodes[i].paramDefID(),
			nodes[i].index,
			nodes[i].leaf ? " T  " : " F  ",
			nodes[i].name.c_str()));
		cout << row << endl;
	}

	cout << nodes.size() << " records" << endl << endl;
}

//
// show the resulting list of Values
//
void showValueList(const vector<OTDBvalue>&	items) 
{
	cout << "name                                         |value |time" << endl;
	cout << "---------------------------------------------+------+--------------------" << endl;
	for (uint32	i = 0; i < items.size(); ++i) {
		string row(formatString("%-55.55s|%-7.7s|%s",
			items[i].name.c_str(),
			items[i].value.c_str(),
			to_simple_string(items[i].time).c_str()));
		cout << row << endl;
	}

	cout << items.size() << " records" << endl << endl;
}

//
// Show the content of a STL vector
//
void showVector(const vector<string> &v, const string &key)
{
  for(vector<string>::const_iterator it=v.begin(); it!=v.end(); ++it)
  {
    if(key!="")
    {
      if(*it==key)
        cout << *it << endl;
    }
    else
    {
      cout << *it << endl;
    }
  }
}


void showVector(const vector<int> &v)
{
  for(vector<int>::const_iterator it=v.begin(); it!=v.end(); ++it)
  {
    cout << *it << endl;
  }
}


void showVector(const vector<unsigned int> &v)
{
  for(vector<unsigned int>::const_iterator it=v.begin(); it!=v.end(); ++it)
  {
    cout << *it << endl;
  }
}


void showMap(const map<string, string> &m, const string &key)
{
  for(map<string, string>::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    if(key!="")
    {
      if(it->first==key)
        cout << (*it).first << "\t" << (*it).second << endl;
    }
    else
    {
      cout << (*it).first << "\t" << (*it).second << endl;    
    }
  }
}

void showMap(const map<string, vector<string> > &m, const string &key)
{
  //map<string, vector<string> >::const_iterator it;
  for(map<string, vector<string> >::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    vector<string> v=it->second;
    
    if(key!="")
    {
      if(it->first==key)
      {
        cout << it->first << endl;
        for(vector<string>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
        {
          cout << (*vit) << "\t";
        }
        cout << endl;
      }
    }
    else
    {
      cout << it->first << endl;
      for(vector<string>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
      {
        cout << (*vit) << "\t";
      }
      cout << endl;
    }
  }
}


void showMap(const map<string, int> &m, const string &key)
{
  for(map<string, int>::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    if(key!="")
    {
      if(it->first==key)
        cout << (*it).first << "\t" << (*it).second << endl;
    }
    else
    {
      cout << (*it).first << "\t" << (*it).second << endl;    
    }
  }
}

void showMap(const map<string, vector<int> > &m, const string &key)
{
  for(map<string, vector<int> >::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    vector<int> v=it->second;
    
    if(key!="")
    {
      if(it->first==key)
      {
        cout << it->first << endl;
        for(vector<int>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
        {
          cout << (*vit) << "\t";
        }
        cout << endl;
      }
    }
    else
    {
      cout << it->first << endl;
      for(vector<int>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
      {
        cout << (*vit) << "\t";
      }
      cout << endl;
    }
  }
}

void showMap(const map<string, vector<unsigned int> > &m, const string &key)
{
  for(map<string, vector<unsigned int> >::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    vector<unsigned int> v=it->second;
    
    if(key!="")
    {
      if(it->first==key)
      {
        cout << it->first << endl;
        for(vector<unsigned int>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
        {
          cout << (*vit) << "\t";
        }
        cout << endl;
      }
    }
    else
    {
      cout << it->first << endl;
      for(vector<unsigned int>::const_iterator vit=v.begin(); vit!=v.end(); ++vit)
      {
        cout << (*vit) << "\t";
      }
      cout << endl;
    }
  }
}

void showMap(const map<string, ptime> &m, const string &key)
{
  for(map<string, ptime>::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
    if(key!="")
    {
      if(it->first==key)
      {
        cout << it->first << "\t" << it->second << endl;
      }
    }
    else
    {
        cout << it->first << "\t" << it->second << endl;
    }
  }
}


void showMap(const map<vector<unsigned int>, vector<unsigned int> > &m)
{
  for(map<vector<unsigned int>, vector<unsigned int> >::const_iterator it=m.begin(); 
      it!=m.end(); ++it)
  {
    for(unsigned int i=0; i< it->first.size(); i++)
    {
      cout << "i = " << i << endl;
      for(unsigned int j=0; j<it->second.size(); j++)
      {
        cout << j << "\t" << endl;
      }    
    }
  }
}


void showMap(const std::map<unsigned int, std::vector<unsigned int> > &m)
{
  for(map<unsigned int, vector<unsigned int> >::const_iterator it=m.begin(); it!=m.end(); ++it)
  {
      cout << it->first << endl;
      for(unsigned int i=0; i<it->second.size(); i++)
      {
        cout << i << "\t";
      }    
  }
}

void showVector(const vector<ptime> &v)
{
  for(unsigned int i=0; i<v.size(); i++)
  {
    std::cout << v[i] << "\t" << std::endl;
  }
} 

void showFailedTiles(const std::vector<failedTile> &failedTiles)
{
  unsigned int length=failedTiles.size();
  for(unsigned int i=0; i<length;i++)
  {
    cout << "antennaFieldId: " << failedTiles[i].antennaFieldId << endl;
    cout << "rcus:"; 
    for(unsigned int j=0; j<failedTiles[i].rcus.size() ; j++)
    {
      cout << "\t" << failedTiles[i].rcus[j];
    }
    cout << endl << "timeStamps:"; 
    for(unsigned int j=0; j<failedTiles[i].timeStamps.size(); j++)
    {
      cout << "\t" << failedTiles[i].timeStamps[j];
    }
    cout << endl;
    cout << "------------------------------------------" << endl;
  }
}
