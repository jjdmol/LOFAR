//#  MWLocalSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <MWControl/MWLocalSpec.h>
#include <MWCommon/MWStepFactory.h>

using namespace LOFAR;
using namespace std;

namespace LOFAR { namespace CEP {

  MWLocalSpec::MWLocalSpec()
  {}

  MWLocalSpec::MWLocalSpec(const string& name,
			   const ParameterSet& parset,
			   const MWSpec* parent)
    : itsSpec(name, parset, parent)
  {}

  MWLocalSpec::~MWLocalSpec()
  {}

  MWStep::ShPtr MWLocalSpec::create()
  {
    return MWStep::ShPtr (new MWLocalSpec());
  }

  void MWLocalSpec::registerCreate()
  {
    MWStepFactory::push_back ("MWLocalSpec", &create);
  }

  MWLocalSpec* MWLocalSpec::clone() const
  {
    return new MWLocalSpec(*this);
  }

  std::string MWLocalSpec::className() const
  {
    return "MWLocalSpec";
  }

  ParameterSet MWLocalSpec::getParms() const
  {
    return itsSpec.getFullParSet();
  }

  void MWLocalSpec::toBlob (LOFAR::BlobOStream& bos) const
  {
    bos.putStart (className(), 0);
    itsSpec.toBlob (bos);
    bos.putEnd();
  }
  void MWLocalSpec::fromBlob (LOFAR::BlobIStream& bis)
  {
    bis.getStart (className());
    itsSpec.fromBlob (bis);
    bis.getEnd();
  }

  void MWLocalSpec::print (ostream& os, const string& indent) const
  {
    itsSpec.printSpec (os, indent, "MWLocalSpec");
  }

  void MWLocalSpec::visit (MWStepVisitor& visitor) const
  {
    visitor.visitLocal (*this);
  }

}} // end namespaces
