#include "RTAnswer.h"
#include <GCF/TM/GCF_Task.h>

void Answer::handleAnswer(GCFEvent& answer)
{
  _t.dispatch(answer, _dummyPort);
}
