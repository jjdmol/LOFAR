#ifndef ANSWER_H
#define ANSWER_H

#include <GCF/PAL/GCF_Answer.h>

class GCFEvent;
class Task;

class Answer : public GCFAnswer
{
  public:
    Answer(Task& st) : 
      _st(st)
      {;}
      
    ~Answer() {;}

    void handleAnswer(GCFEvent& answer);
    
  private:    
    Task& _st;
};
#endif
