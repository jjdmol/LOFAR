
#include <tinyCEP/MyExample.h>

namespace LOFAR
{

  MyExample::MyExample(int ninput, int noutput)
    : itsArgv(0),
      itsArgc(0),
      itsProto(dhptr),
      itsNinputs(ninput),
      itsNoutputs(noutput) {
   
//     itsDataManager = new MiniDataManager(ninput, noutput);
    
//     for (int i=0; i < itsNinputs; i++) {
//       itsDataManager->addInDataHolder(i, dhptr);
//     }
//     for (int i=0; i < itsNoutputs; i++){
//       itsDataManager->addOutDataHolder(i, dhptr);
//     }

  }


  MyExample::~MyExample() {
    delete itsWorkHolder;
  }

  void MyExample::define(const KeyValueMap& map) {
    // create a WorkHolder
    itsWorkHolder = new WH_Example("WH_Example", 1, 1, 10);
  }

  void MyExample::init() {
    itsWorkHolder->preprocess();
  }

  void MyExample::run(int nsteps) {
    for (int i = 0; i < nsteps; i++) {
      itsWorkHolder->process();
    }
  }
  
  void MyExample::run_once() {
    MyExample::run(1);
  }

  void MyExample::quit(){
    itsWorkHolder->postprocess();
  }

  void MyExample::dump() const {
    itsWorkHolder->dump();
  }

} // namespace LOFAR
