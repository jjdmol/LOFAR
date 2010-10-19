//# MeqParm.cc: The base class for a parameter
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/MNS/MeqParm.h>
#include <Common/LofarLogger.h>


namespace LOFAR
{
namespace BBS
{


MeqParm::MeqParm (const string& name)
: itsName       (name),
  itsIsSolvable (false)
{}

MeqParm::~MeqParm()
{}

int MeqParm::getParmDBSeqNr() const
{
  return -1;
}

void MeqParm::removeFunklets()
{}

void MeqParm::fillFunklets (const std::map<std::string,LOFAR::ParmDB::ParmValueSet>&,
                const MeqDomain&)
{}

const vector<MeqFunklet*>& MeqParm::getFunklets() const
{
  static vector<MeqFunklet*> vec;
  return vec;
}

int MeqParm::initDomain (const vector<MeqDomain>&, int&, vector<int>&)
{
  ASSERT (! isSolvable());
  return 0;
}

void MeqParm::save()
{}

void MeqParm::save(size_t domainIndex)
{}

void MeqParm::update (const ParmData&)
{
  throw Exception("MeqParm::update(ParmData) should not be called");
}
void MeqParm::update (const vector<double>&)
{
  throw Exception("MeqParm::update(vector<double>) should not be called");
}

void MeqParm::update(size_t domain, const vector<double> &unknowns)
{
  throw Exception("MeqParm::update(size_t, vector<double>) should not be called");
}

void MeqParm::updateFromTable()
{
  throw Exception("MeqParm::updateFromTable should not be called");
}

#ifdef EXPR_GRAPH
std::string MeqParm::getLabel()
{
    return std::string("MeqParm\\n" + (itsIsSolvable ? std::string("[*]") : std::string("[ ]")) + " " + itsName);
}
#endif

MeqPExpr::MeqPExpr (const MeqExpr& expr)
: MeqExpr    (expr),
  itsParmPtr (0)
{
  itsParmPtr = dynamic_cast<MeqParm*>(itsRep);
  ASSERT (itsParmPtr != 0);
}


MeqParmGroup::MeqParmGroup()
{}

void MeqParmGroup::add (const MeqPExpr& parm)
{
  itsParms.insert (make_pair (parm.getName(), parm));
}

void MeqParmGroup::clear()
{
  itsParms.clear();
}

} // namespace BBS
} // namespace LOFAR
