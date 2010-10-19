#include "Answer.h"
#include "SupervisedTask.h"

void Answer::handleAnswer(GCFEvent& answer)
{
  _st.handleAnswer(answer);
}
