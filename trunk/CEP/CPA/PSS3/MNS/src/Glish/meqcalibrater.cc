#include <MeqCalibraterImpl.h>
#include <trial/Tasking.h>

#include <aips/Glish/GlishRecord.h>
#include <aips/Glish/GlishValue.h>

int main(int argc, char** argv)
{
  ObjectController controller(argc, argv);

  String name = "meqcalibrater";
  MeqCalibraterFactory* factory = new MeqCalibraterFactory;
  controller.addMaker(name, factory);
  controller.loop();

  return 0;
}
