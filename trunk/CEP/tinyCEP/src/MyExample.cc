
#include <tinyCEP/MyExample.h>

namespace LOFAR
{

  MyExample::MyExample(int ninput, int noutput, DataHolder* dhptr)
    : itsArgv(0),
      itsArgc(0),
      itsProto(dhptr),
      itsNinputs(ninput),
      itsNoutputs(noutput) {

    itsWorkHolder = new WH_Example("WH_Example", 1, 1, 10);
  }


  MyExample::~MyExample() {
  }

  void MyExample::define(const KeyValueMap& map) {
  }

  void MyExample::init() {
  }

  void MyExample::run(int nsteps) {

    for (int i = 0; i < nsteps; i++) {
      
      itsWorkHolder->preprocess();
      itsWorkHolder->process();
      itsWorkHolder->postprocess();

    }
    
  }
  
  void MyExample::run_once() {
    MyExample::run(1);
  }

  void MyExample::quit(){
  }

} // namespace LOFAR
