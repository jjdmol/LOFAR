#include "Answer.h"
#include "SupervisedTask.h"

namespace LOFAR
{
 namespace GCF
 {
using namespace TM;
  namespace Test
  {
    
void Answer::handleAnswer(GCFEvent& answer)
{
  _st.handleAnswer(answer);
}
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
