#include <lofar_config.h>
#include <Common/StringUtil.h>
#include <ALC/ACCmd.h>

using namespace LOFAR;
using namespace LOFAR::ACC::ALC;

#define NAME_TEST(cmd)	cout <<	"value: " << (cmd) << " = " << ACCmdName(cmd) << endl;

int main (int /*argc*/, char** /* argv*/) {
	NAME_TEST(ACCmdNone);
	NAME_TEST(ACCmdBoot);
	NAME_TEST(ACCmdQuit);
	NAME_TEST(ACCmdDefine);
	NAME_TEST(ACCmdInit);
	NAME_TEST(ACCmdPause);
	NAME_TEST(ACCmdRun);
	NAME_TEST(ACCmdRelease);
	NAME_TEST(ACCmdSnapshot);
	NAME_TEST(ACCmdRecover);
	NAME_TEST(ACCmdReinit);
	NAME_TEST(ACCmdInfo);
	NAME_TEST(ACCmdAnswer);
	NAME_TEST(ACCmdReport);
	NAME_TEST(ACCmdAsync);
	NAME_TEST(ACCmdCancelQueue);
	NAME_TEST(ACCmdResult);

	NAME_TEST((ACCmd)(ACCmdNone | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdBoot | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdQuit | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdDefine | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdInit | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdPause | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdRun | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdRelease | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdSnapshot | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdRecover | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdReinit | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdInfo | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdAnswer | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdReport | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdAsync | ACCmdResult));
	NAME_TEST((ACCmd)(ACCmdCancelQueue | ACCmdResult));

	return (0);
}

