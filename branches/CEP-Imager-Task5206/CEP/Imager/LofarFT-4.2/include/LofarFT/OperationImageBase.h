//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: $

#ifndef LOFAR_LOFARFT_OPERATIONIMAGEBASE_H
#define LOFAR_LOFARFT_OPERATIONIMAGEBASE_H

// \file

#include <LofarFT/Operation.h>
#include <LofarFT/OperationParamFTMachine.h>

namespace LOFAR {
namespace LofarFT {

  class OperationImageBase : public virtual Operation, OperationParamFTMachine
  {
  public:
    
    OperationImageBase();
    
    virtual void run();
    
    void makeEmpty (const casa::String& imgName, casa::Int fieldid);
    
  };

} //# namespace LofarFT
} //# namespace LOFAR

#endif
