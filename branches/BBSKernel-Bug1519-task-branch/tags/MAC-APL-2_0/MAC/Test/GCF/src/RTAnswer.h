#ifndef ANSWER_H
#define ANSWER_H

#include <GCF/GCF_RTAnswer.h>
#include <GCF/GCF_Fsm.h>

class GCFEvent;
class GCFTask;

class Answer : public GCFRTAnswer
{
  public:
    Answer(GCFTask& t) : 
      _t(t),
      _dummyPort(&t, "RTAnswerPort", 0)
      {;}
      
    ~Answer() {;}

    void handleAnswer(GCFEvent& answer);
    
  private:    
    GCFTask& _t;
    GCFDummyPort _dummyPort;
};
#endif
