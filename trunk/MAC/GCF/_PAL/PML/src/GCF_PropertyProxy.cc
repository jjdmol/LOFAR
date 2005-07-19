//#  GCF_PropertyProxy.cc: 
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#include <GCF/PAL/GCF_PropertyProxy.h>
#include <GPM_PropertyProxy.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
  namespace PAL
  {
GCFPropertyProxy::GCFPropertyProxy() :
  _pPMProxy(0)
{
  _pPMProxy = new GPMPropertyProxy(*this);
}

GCFPropertyProxy::~GCFPropertyProxy()
{
  delete _pPMProxy;
  _pPMProxy = 0;
}

TGCFResult GCFPropertyProxy::subscribeProp(const string& propName)
{
  return (_pPMProxy->subscribePM(propName) == SA_NO_ERROR ? GCF_NO_ERROR : GCF_PML_ERROR);
}

TGCFResult GCFPropertyProxy::unsubscribeProp(const string& propName)
{
  return (_pPMProxy->unsubscribePM(propName) == SA_NO_ERROR ? GCF_NO_ERROR : GCF_PML_ERROR);
}

TGCFResult GCFPropertyProxy::requestPropValue(const string& propName)
{
  return (_pPMProxy->getPM(propName) == SA_NO_ERROR ? GCF_NO_ERROR : GCF_PML_ERROR);
}

TGCFResult GCFPropertyProxy::setPropValueTimed(const string& propName, 
                                               const GCFPValue& value, 
                                               double timestamp, 
                                               bool wantAnswer)
{
  return (_pPMProxy->setPM(propName, value, timestamp, wantAnswer) == SA_NO_ERROR ? GCF_NO_ERROR : GCF_PML_ERROR);
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
