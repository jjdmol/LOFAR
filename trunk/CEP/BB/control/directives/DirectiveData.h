#ifndef DIRECTIVEDATA_H_HEADER_INCLUDED_C0B1CAD2
#define DIRECTIVEDATA_H_HEADER_INCLUDED_C0B1CAD2

#include <string>

//##ModelId=3F4DC5270157
class DirectiveData
{
 public:
  //##ModelId=3F4DCB93005D
  std::string id;

    //##ModelId=3F4DCC880271
    //##Documentation
    //## Initially it contains a full script. This is parsed as soon as
    //## getParts is called on the Directive. It is then replaced with only the
    //## outer nestting layer containing references to the directive data
    //## implementing the nested parts. i.e. record with id 0:
    //## do calibration{
    //##  part do peeling{
    //##    sourcelist = {...}
    //##  }
    //##  part do montecarlo{
    //##   ...
    //##  }
    //## }
    //## becomes record with id 0
    //## do calibration{
    //##  part 0.0
    //##  part 0.1
    //## }
    //## and record with id 0.0
    //## do peeling{
    //##  sourcelist = {...}
    //## }
    //## and record with id 0.1
    //## do montecarlo{
    //##  ...
    //## }
  std::string text;

  DirectiveData(const std::string& initId,
		const std::string& initText):
    id(initId),
    text(initText)
  {}

  DirectiveData():id(""),text(""){}
};

#endif /* DIRECTIVEDATA_H_HEADER_INCLUDED_C0B1CAD2 */
