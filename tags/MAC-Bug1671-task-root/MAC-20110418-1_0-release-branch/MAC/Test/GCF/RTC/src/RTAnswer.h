#ifndef ANSWER_H
#define ANSWER_H

#include <GCF/PALlight/GCF_RTAnswer.h>
#include <GCF/TM/GCF_Fsm.h>

namespace LOFAR
{
 namespace GCF
 {
  namespace TM
  {
class GCFEvent;
class GCFTask;
  }
  namespace Test
  {

class Answer : public RTCPMLlight::GCFRTAnswer
{
  public:
    Answer(TM::GCFTask& t) : 
      _t(t),
      _dummyPort(&t, "RTAnswerPort", 0)
      {;}
      
    ~Answer() {;}

    void handleAnswer(TM::GCFEvent& answer);
    
  private:    
    TM::GCFTask& _t;
    TM::GCFDummyPort _dummyPort;
};
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
#endif
