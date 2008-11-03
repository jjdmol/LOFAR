//#  ParameterHandler.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <lofar_config.h>

#include <MWCommon/ParameterHandler.h>

using namespace std;

namespace LOFAR { namespace CEP {

  ParameterHandler::ParameterHandler (const ParameterSet& parSet)
    : itsParms (parSet)
  {}

  string ParameterHandler::getString (const string& parm,
				      const string& defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getString (parm);
    }
    return defVal;
  }

  double ParameterHandler::getDouble (const string& parm,
				      double defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getDouble (parm);
    }
    return defVal;
  }

  unsigned ParameterHandler::getUint (const string& parm,
				      unsigned defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getUint32 (parm);
    }
    return defVal;
  }

  bool ParameterHandler::getBool (const string& parm,
				  bool defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getBool (parm);
    }
    return defVal;
  }

  vector<string> ParameterHandler::getStringVector
  (const string& parm, const vector<string>& defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getStringVector (parm);
    }
    return defVal;
  }

  void ParameterHandler::fillString (const string& parm,
				     string& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getString (parm);
    }
  }

  void ParameterHandler::fillDouble (const string& parm,
				     double& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getDouble (parm);
    }
  }

  void ParameterHandler::fillUint (const string& parm,
				   unsigned& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getUint32 (parm);
    }
  }

  void ParameterHandler::fillBool (const string& parm,
				   bool& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getBool (parm);
    }
  }

  void ParameterHandler::fillStringVector (const string& parm,
					   vector<string>& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getStringVector (parm);
    }
  }


  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs, const ParameterSet& m)
  {
    bs.putStart ("ParameterSet", 1);
    bs << static_cast<LOFAR::uint32>(m.size());
    for (ParameterSet::const_iterator it=m.begin();
         it!=m.end();
         ++it) {
      bs << it->first << it->second.get();
    }
    bs.putEnd();
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs, ParameterSet& m)
  {
    bs.getStart ("ParameterSet");
    m.clear();
    LOFAR::uint32 size;
    bs >> size;
    std::string k,v;
    for (LOFAR::uint32 i=0; i<size; ++i) {
      bs >> k >> v;
      m.add (k, v);
    }
    bs.getEnd();
    return bs;
  }

}} // end namespaces
