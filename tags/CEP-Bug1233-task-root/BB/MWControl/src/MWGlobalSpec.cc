//#  MWGlobalSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <MWControl/MWGlobalSpec.h>
#include <MWCommon/MWStepFactory.h>

using namespace LOFAR;
using namespace std;

namespace LOFAR { namespace CEP {

  MWGlobalSpec::MWGlobalSpec()
  {}

  MWGlobalSpec::MWGlobalSpec(const string& name,
			     const ParameterSet& parset,
			     const MWSpec* parent)
    : itsSpec(name, parset, parent)
  {}

  MWGlobalSpec::~MWGlobalSpec()
  {}

  MWStep::ShPtr MWGlobalSpec::create()
  {
    return MWStep::ShPtr (new MWGlobalSpec());
  }

  void MWGlobalSpec::registerCreate()
  {
    MWStepFactory::push_back ("MWGlobalSpec", &create);
  }

  MWGlobalSpec* MWGlobalSpec::clone() const
  {
    return new MWGlobalSpec(*this);
  }

  std::string MWGlobalSpec::className() const
  {
    return "MWGlobalSpec";
  }

  ParameterSet MWGlobalSpec::getParms() const
  {
    return itsSpec.getFullParSet();
  }

  void MWGlobalSpec::toBlob (LOFAR::BlobOStream& bos) const
  {
    bos.putStart (className(), 0);
    itsSpec.toBlob (bos);
    bos.putEnd();
  }
  void MWGlobalSpec::fromBlob (LOFAR::BlobIStream& bis)
  {
    bis.getStart (className());
    itsSpec.fromBlob (bis);
    bis.getEnd();
  }

  void MWGlobalSpec::print (ostream& os, const string& indent) const
  {
    itsSpec.printSpec (os, indent, "MWGlobalSpec");
  }

  void MWGlobalSpec::visit (MWStepVisitor& visitor) const
  {
    visitor.visitGlobal (*this);
  }

}} // end namespaces
