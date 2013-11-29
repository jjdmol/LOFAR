#include "RTAnswer.h"
#include <GCF/TM/GCF_Task.h>

namespace LOFAR
{
 namespace GCF
 {
using namespace TM;
  namespace Test
  {
void Answer::handleAnswer(GCFEvent& answer)
{
  _t.dispatch(answer, _dummyPort);
}
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
