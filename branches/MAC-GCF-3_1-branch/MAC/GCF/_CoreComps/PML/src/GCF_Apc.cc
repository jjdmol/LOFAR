//#  GCF_Apc.cc:
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

#include <GCF/GCF_Apc.h>
#include <GPM_Controller.h>
#include <GCF/GCF_Answer.h>
#include <Utils.h>

GCFApc::~GCFApc ()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  pController->unregisterAPC(*this);  
}

TGCFResult GCFApc::load (bool loadDefaults)
{
  TGCFResult result(GCF_NO_ERROR);
  
  if (_isBusy)
  {
    result = GCF_BUSY;
  }
  else if (_isLoaded)
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This Apc %s with scope %s is already loaded in this context.",
        _name.c_str(), _scope.c_str()));
    result = GCF_ALREADY_LOADED;
  }
  else if (_name.length() == 0 || 
           _scope.length() == 0 || 
           !Utils::isValidPropName(_scope.c_str()))
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "APC name or scope not set or scope \"%s\"meets not the naming convention.",
        _scope.c_str()));
    result = GCF_NO_PROPER_DATA;
  }
  else
  {
    GPMController* pController = GPMController::instance();
    assert(pController);
    _loadDefaults = loadDefaults;
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "REQ: Load Apc %s with scope %s",
        _name.c_str(), _scope.c_str()));
    TPMResult pmResult = pController->loadAPC(*this, loadDefaults);
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
    else
    {
      result = GCF_APCLOAD_ERROR;
    }
  }
  return result;
}

TGCFResult GCFApc::unload ()
{
  TGCFResult result(GCF_NO_ERROR);

  if (_isBusy)
  {
    result = GCF_BUSY;
  }
  else if (!_isLoaded)
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, (
        "This Apc %s with scope %s is not loaded in this context.",
        _name.c_str(), _scope.c_str()));
    result = GCF_NOT_LOADED;
  }
  else if (_name.length() == 0 ||
           _scope.length() == 0 ||
           !Utils::isValidPropName(_scope.c_str()))
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, (
        "APC name or scope not set or scope (%s) meets not the naming convention.",
        _scope.c_str()));
    result = GCF_NO_PROPER_DATA;
  }
  else
  {
    GPMController* pController = GPMController::instance();
    assert(pController);
    _loadDefaults = false;
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, (
        "REQ: Unload Apc %s with scope %s",
        _name.c_str(), _scope.c_str()));
    TPMResult pmResult = pController->unloadAPC(*this);
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
    else
    {
      result = GCF_APCUNLOAD_ERROR;
    }
  }
  return result;
}

TGCFResult GCFApc::reload ()
{
  TGCFResult result(GCF_NO_ERROR);

  if (_isBusy)
  {
    result = GCF_BUSY;
  }
  else if (!_isLoaded)
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, (
        "This Apc %s with scope %s is not loaded in this context.",
        _name.c_str(), _scope.c_str()));
    result = GCF_NOT_LOADED;
  }
  else if (_name.length() == 0 ||
           _scope.length() == 0 ||
           !Utils::isValidPropName(_scope.c_str()))
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, (
        "APC name or scope not set or scope (%s) meets not the naming convention.",
        _scope.c_str()));
    result = GCF_NO_PROPER_DATA;
  }
  else
  {
    GPMController* pController = GPMController::instance();
    assert(pController);
    _loadDefaults = true;
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, (
        "REQ: Reload Apc %s with scope %s",
        _name.c_str(), _scope.c_str()));
    TPMResult pmResult = pController->reloadAPC(*this);
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
    else
    {
      result = GCF_APCRELOAD_ERROR;
    }
  }
  return result;
}

void GCFApc::loaded (TGCFResult result)
{
  assert(!_isLoaded);
  assert(_isBusy);
  
  _isBusy = false;
  if (result == GCF_NO_ERROR)
  {
    _isLoaded = true;
  }
  LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
      "PA-RESP: Apc %s loaded with scope %s",
      _name.c_str(), _scope.c_str()));
  
  dispatchAnswer(F_APCLOADED, result);
}

void GCFApc::unloaded(TGCFResult result)
{
  assert(_isLoaded);
  assert(_isBusy);
  
  _isBusy = false;
  if (result == GCF_NO_ERROR)
  {
    _isLoaded = false;
  }
  
  LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
      "PA-RESP: Apc %s unloaded with scope %s",
      _name.c_str(), _scope.c_str()));

  dispatchAnswer(F_APCUNLOADED, result);
}

void GCFApc::reloaded(TGCFResult result)
{
  assert(_isLoaded);
  assert(_isBusy);
  
  _isBusy = false;

  LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
      "PA-RESP: Apc %s reloaded with scope %s",
      _name.c_str(), _scope.c_str()));
  
  dispatchAnswer(F_APCRELOADED, result);
}

TGCFResult GCFApc::setName (const string name)
{
  TGCFResult result(GCF_PROTECTED_STATE);
  if (!_isBusy && !_isLoaded)
  {
    _name = name;
    result = GCF_NO_ERROR;
  }
  return result;
}

TGCFResult GCFApc::setScope (const string scope)
{
  TGCFResult result(GCF_PROTECTED_STATE);
  if (!_isBusy && !_isLoaded)
  {
    _scope = scope;
    result = GCF_NO_ERROR;
  }
  return result;
}

TGCFResult GCFApc::setApcData (const string name, const string scope)
{
  TGCFResult result(GCF_PROTECTED_STATE);
  if (!_isBusy && !_isLoaded)
  {
    _name = name;
    _scope = scope;
    result = GCF_NO_ERROR;
  }
  return result;
}

void GCFApc::dispatchAnswer(unsigned short sig, TGCFResult result)
{
  if (_pAnswerObj != 0)
  {
    GCFAPCAnswerEvent e(sig);
    e.pScope = _scope.c_str();
    e.pApcName = _name.c_str();
    e.result = result;
    _pAnswerObj->handleAnswer(e);
  }
}
