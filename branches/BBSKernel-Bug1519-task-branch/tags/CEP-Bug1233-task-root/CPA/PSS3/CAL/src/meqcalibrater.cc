#include <CAL/MeqCalibraterImpl.h>
#include <tasking/Tasking.h>

#include <tasking/Glish/GlishRecord.h>
#include <tasking/Glish/GlishValue.h>

int main(int argc, char** argv)
{
  ObjectController controller(argc, argv);

  String name = "meqcalibrater";
  MeqCalibraterFactory* factory = new MeqCalibraterFactory;
  controller.addMaker(name, factory);
  controller.loop();

  return 0;
}
