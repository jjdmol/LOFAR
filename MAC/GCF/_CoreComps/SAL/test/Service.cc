#include "Service.h"

TSAResult Service::createProp(const string& macType, const string& propName)
{
  return GSAService::createProp(macType, propName);
}

TSAResult Service::deleteProp(const string& propName)
{
  return GSAService::deleteProp(propName);
}

TSAResult Service::subscribe(const string& propName)
{
  return GSAService::subscribe(propName);
}

TSAResult Service::unsubscribe(const string& propName)
{
  return GSAService::unsubscribe(propName);
}

TSAResult Service::get(const string& propName)
{
  return GSAService::get(propName);
}

TSAResult Service::set(const string& propName, const GCFPValue& value)
{
  return GSAService::set(propName, value);
}

bool Service::exists(const string& propName)
{
  return GSAService::exists(propName);
}

    
void Service::propCreated(string& propName)
{
  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Property '%s' created", propName.c_str()));
}

void Service::propDeleted(string& propName)
{
  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Property '%s' deleted", propName.c_str()));
}

void Service::propSubscribed(string& propName)
{
  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Property '%s' subscribed", propName.c_str()));
}

void Service::propUnsubscribed(string& propName)
{
  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Property '%s' unsubscribed", propName.c_str()));
}

void Service::propValueGet(string& propName, GCFPValue& value)
{
  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Value of property '%s' get", propName.c_str()));
}

void Service::propValueChanged(string& propName, GCFPValue& value)
{
  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Value of property '%s' changed", propName.c_str()));
}

