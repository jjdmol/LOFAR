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

#ifndef LOFAR_LOFARFT_OPERATIONIMAGE_H
#define LOFAR_LOFARFT_OPERATIONIMAGE_H

#include <LofarFT/Operation.h>

// \file

namespace LOFAR {
namespace LofarFT {

  class OperationImage : public Operation
  {
  public:
    
    OperationImage(ParameterSet& parset);

    virtual ~OperationImage() {};

    virtual void init();

    virtual void run();

    virtual void showHelp (ostream& os, const std::string& name);

  protected:
    casa::String itsImageName;
  };

} //# namespace LofarFT
} //# namespace LOFAR

#endif
