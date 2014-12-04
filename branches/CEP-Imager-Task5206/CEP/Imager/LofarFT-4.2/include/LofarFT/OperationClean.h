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

#ifndef LOFAR_LOFARFT_OPERATIONCLEAN_H
#define LOFAR_LOFARFT_OPERATIONCLEAN_H

#include <LofarFT/Operation.h>

// \file

namespace LOFAR {
namespace LofarFT {

  class OperationClean: public Operation
  {
  public:
    
    OperationClean(ParameterSet& parset);
    virtual ~OperationClean() {};

    virtual void init();

    virtual void run();

    virtual void showHelp (ostream& os, const string& name);

  
  private:
    casa::String itsAvgpbName;
    casa::Vector<casa::String> itsModelNames;
    casa::Vector<casa::String> itsResidualNames;
    casa::Vector<casa::String> itsRestoredNames;
    casa::Vector<casa::String> itsModelNames_normalized;
    casa::Vector<casa::String> itsResidualNames_normalized;
    casa::Vector<casa::String> itsRestoredNames_normalized;
    casa::Vector<casa::String> itsPsfNames;
  };


} //# namespace LofarFT
} //# namespace LOFAR

#endif
