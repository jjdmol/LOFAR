#ifndef ANSWER_H
#define ANSWER_H

#include <GCF/GCF_Answer.h>

class GCFEvent;
class SupervisedTask;

class Answer : public GCFAnswer
{
  public:
    Answer(SupervisedTask& st) : 
      _st(st)
      {;}
      
    ~Answer() {;}

    void handleAnswer(GCFEvent& answer);
    
  private:    
    SupervisedTask& _st;
};
#endif
