#ifndef ANSWER_H
#define ANSWER_H

#include <GCF/PAL/GCF_Answer.h>

namespace LOFAR
{
 namespace GCF
 {
  namespace TM
  {
class GCFEvent;
  }
  namespace Test
  {
class Task;

class Answer : public PAL::GCFAnswer
{
  public:
    Answer(Task& st) : _st(st) {}
      
    ~Answer() {}

    void handleAnswer(TM::GCFEvent& answer);
    
  private:    
    Task& _st;
};
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
#endif
