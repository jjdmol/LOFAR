//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//


#include <GSM/SkyModel.h>


#include <aips/Tables/Table.h>


using namespace GSM;

//===============>>>  SkyModel::SkyModel  <<<===============

SkyModel::SkyModel()
  : itsSources(0)
{
}






//===============>>>  SkyModel::SkyModel  <<<===============

SkyModel::SkyModel(const Table& table)
  : itsSources(0)
{
  load(table);
}







//===============>>>  SkyModel::~SkyModel  <<<===============

SkyModel::~SkyModel()
{
  for(unsigned int i = 0; i < itsSources.size(); i++) {
    delete itsSources[i];
  }
}






//===============>>>  SkyModel::load  <<<===============

void SkyModel::load(const Table& table)
{
  for(unsigned int i = 0; i < table.nrow(); i++) {
    PointSource* source = new PointSource;
    
    source->load(table, i);

    itsSources.push_back(source);
  }
}










//===============>>>  SkyModel::store  <<<===============

void SkyModel::store(Table& table) const
{
  for(unsigned int i = 0; i < itsSources.size(); i++) {
    while(i >= table.nrow()) {
      table.addRow();
    }
    itsSources[i]->store(table, i);
  }
}






//===============>>>  SkyModel::add  <<<===============

void SkyModel::add(AbstractSource* source)
{
  itsSources.push_back(source);
}






//===============>>>  SkyModel::getPointSources  <<<===============

unsigned int SkyModel::getPointSources(std::vector<GSM::PointSource*> &result)
{
  unsigned int count(0);

  for(unsigned int i = 0; i < itsSources.size(); i++) {
    if(itsSources[i]->getType() == GSM::POINT) {
      result.push_back(dynamic_cast<GSM::PointSource*>(itsSources[i]) );
      count++;
    }
  }
  return count;
}






//===============>>>  SkyModel::getPointSources  <<<===============

unsigned int SkyModel::getPointSources(std::vector<MeqPointSource> &result)
{
  unsigned int count(0);

  for(unsigned int i = 0; i < itsSources.size(); i++) {
    if(itsSources[i]->getType() == GSM::POINT) {
      std::vector<MeqExpr*>  fluxes(NUMBER_OF_POLARIZATIONS);
      MeqExpr*               ra;
      MeqExpr*               dec;
      
      itsSources[i]->getFluxExpressions(fluxes);
      itsSources[i]->getPositionExpressions(ra, dec);

      MeqPointSource source(fluxes[I],
                            fluxes[Q],
                            fluxes[U],
                            fluxes[V],
                            ra,
                            dec);

      result.push_back(source);

      count++;
    }
  }
  return count;
}
