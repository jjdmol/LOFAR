
#include <tinyCEP/MySecondExample.h>

namespace LOFAR
{

  MySecondExample::MySecondExample(int ninput, int noutput)
    : itsArgv(0),
      itsArgc(0),
      itsProto(dhptr),
      itsNinputs(ninput),
      itsNoutputs(noutput),
      itsWHCount(0) {
  }


  MySecondExample::~MySecondExample() {
    delete itsWHs[0];
    delete itsWHs[1];
    delete itsWHs;

  }

  void MySecondExample::define(const KeyValueMap& map) {
    // create a WorkHolder
    WH_Example* myWHExamples[2];
    
    myWHExamples[0] = new WH_Example("WH_Example0", 1, 1, 10);
    myWHExamples[1] = new WH_Example("WH_Example1", 1, 1, 10);
    
    itsWHs[0] = myWHExamples[0];
    itsWHs[1] = myWHExamples[1];

    myWHExamples[1]->getDataManager().getInHolder(0)->connectTo
      ( *myWHExamples[0]->getDataManager().getOutHolder(0), 
	TH_Mem() );

    myWHExamples[0]->getDataManager().getOutHolder(0)->setBlocking(false);
    myWHExamples[1]->getDataManager().getInHolder(0)->setBlocking(false);

    itsWHCount = 2;
  }

  void MySecondExample::init() {

    itsWHs[0]->basePreprocess();
    itsWHs[1]->basePreprocess();
    
    ((DH_Example*)itsWHs[0]->getDataManager().getInHolder(0))->getBuffer()[0] = complex<float> (4,3);
  }

  void MySecondExample::run(int nsteps) {
    for (int i = 0; i < nsteps; i++) {
      
      itsWHs[0]->baseProcess();
      itsWHs[1]->baseProcess();
      
    }
  }
  
  void MySecondExample::run_once() {
    MySecondExample::run(1);
  }

  void MySecondExample::quit(){
    itsWHs[0]->basePostprocess();
    itsWHs[1]->basePostprocess();
  }

  void MySecondExample::dump() const {
    itsWHs[0]->dump();
    itsWHs[1]->dump();
  }

} // namespace LOFAR
