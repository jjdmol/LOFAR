#include "RTAnswer.h"
#include <GCF/GCF_Task.h>

void Answer::handleAnswer(GCFEvent& answer)
{
  _t.dispatch(answer, _dummyPort);
}
