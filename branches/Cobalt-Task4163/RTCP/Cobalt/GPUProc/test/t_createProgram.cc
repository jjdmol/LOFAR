#include <lofar_config.h>

#include <GPUProc/createProgram.h>
#include <CoInterface/Parset.h>
#include <GPUProc/OpenCL_Support.h>
#include <vector>

using namespace LOFAR::Cobalt;
using namespace std;

int main()
{
  Parset ps("tCorrelate_1sec_1st_5sb_noflagging.parset");

  cl::Context context;
  vector<cl::Device> devices;
  createContext(context, devices);

  createProgram(ps, context, devices, "t_createProgram.cl");
}
