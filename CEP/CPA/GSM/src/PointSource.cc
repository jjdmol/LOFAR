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


#include <GSM/PointSource.h>

#include <MNS/MesParmPolc.h>
#include <MNS/MnsMatrix.h>

#include <aips/aips.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Arrays/Matrix.h>

using namespace GSM;

static const char* STOKES_NAMES[4]={"I","Q","U","V"};

//====================>>>  PointSource::PointSource  <<<====================

PointSource::PointSource(double                     ra,
                         double                     dec,
                         unsigned int               catNumber,
                         const std::string&         name,
                         const std::vector<double>& flux)
  : AbstractSource(POINT, ra, dec, catNumber, name)
{
  for(unsigned int i = 0; i < NUMBER_OF_POLARIZATIONS;i++) {
    MnsMatrix   matrix(flux[i]);
    itsFlux[i] =  new MesParmPolc(STOKES_NAMES[i],matrix);
  }
}





//====================>>>  PointSource::~PointSource  <<<====================

PointSource::~PointSource()
{
  for(unsigned int i = 0; i < NUMBER_OF_POLARIZATIONS;i++) {
    delete itsFlux[i];
  }
}





//====================>>>  PointSource:load:  <<<====================

void PointSource::load(Table&       table,
                       unsigned int row)
{
  AbstractSource::load(table, row);
  
  ROArrayColumn<double> IParms(table, "IPARMS");
  ROArrayColumn<double> QParms(table, "QPARMS");
  ROArrayColumn<double> UParms(table, "UPARMS");
  ROArrayColumn<double> VParms(table, "VPARMS");
  
  Matrix<double> coef(1,1);
  IParms.get(row, coef);
  itsFlux[I]->setCoeff(coef);

  QParms.get(row, coef);
  itsFlux[Q]->setCoeff(coef);

  UParms.get(row, coef);
  itsFlux[U]->setCoeff(coef);

  VParms.get(row, coef);
  itsFlux[V]->setCoeff(coef);
}




//====================>>>  PointSource::store  <<<====================

void PointSource::store(Table&       table,
                        unsigned int row)
{
  AbstractSource::store(table, row);
  
  ArrayColumn<double> IParms(table, "IPARMS");
  ArrayColumn<double> QParms(table, "QPARMS");
  ArrayColumn<double> UParms(table, "UPARMS");
  ArrayColumn<double> VParms(table, "VPARMS");
  
  IParms.put(row, itsFlux[I]->getCoeff().getDoubleMatrix());
  QParms.put(row, itsFlux[Q]->getCoeff().getDoubleMatrix());
  UParms.put(row, itsFlux[U]->getCoeff().getDoubleMatrix());
  VParms.put(row, itsFlux[V]->getCoeff().getDoubleMatrix());
}





//====================>>>  PointSource::getParameters  <<<====================

unsigned int PointSource::getParameters(std::vector<MesParm *> &parameters)
{

  unsigned int i = AbstractSource::getParameters(parameters);
  
  for(unsigned int j = 0; j < NUMBER_OF_POLARIZATIONS; j++) {
    parameters[i+j] = itsFlux[j];
    i++;
  }
  
  return i;
}





//====================>>>  PointSource::setParameters  <<<====================

unsigned int PointSource::setParameters(const std::vector<MesParm*> &parameters)
{
  unsigned int i = AbstractSource::setParameters(parameters);
  
  for(unsigned int j = 0; j < NUMBER_OF_POLARIZATIONS; j++) {
    *itsFlux[j] = *parameters[i+j];
    i++;
  }
  return i;
}





//===============>>>  PointSource::getNumberOfParameters  <<<===============

unsigned int PointSource::getNumberOfParameters() const
{
  return AbstractSource::getNumberOfParameters()+ NUMBER_OF_POLARIZATIONS;
}





//===============>>>  PointSource::getFluxExpressions  <<<===============

void PointSource::getFluxExpressions(std::vector<MesExpr*> &flux)
{
  for(unsigned int i = 0; i < NUMBER_OF_POLARIZATIONS; i++) {
    flux[i] = itsFlux[i];
  }
}
