#include <lofar_config.h>
#include <Common/StringUtil.h>
#include <PLC/PCCmd.h>

using namespace LOFAR;
using namespace LOFAR::ACC::PLC;

#define NAME_TEST(cmd)	cout <<	"value: " << (cmd) << " = " << PCCmdName(cmd) << endl;

int main (int /*argc*/, char** /*argv*/) {
	NAME_TEST(PCCmdNone);
	NAME_TEST(PCCmdBoot);
	NAME_TEST(PCCmdQuit);
	NAME_TEST(PCCmdDefine);
	NAME_TEST(PCCmdInit);
	NAME_TEST(PCCmdPause);
	NAME_TEST(PCCmdRun);
	NAME_TEST(PCCmdRelease);
	NAME_TEST(PCCmdSnapshot);
	NAME_TEST(PCCmdRecover);
	NAME_TEST(PCCmdReinit);
	NAME_TEST(PCCmdParams);
	NAME_TEST(PCCmdInfo);
	NAME_TEST(PCCmdAnswer);
	NAME_TEST(PCCmdReport);
	NAME_TEST(PCCmdAsync);
	NAME_TEST(PCCmdResult);

	NAME_TEST((PCCmd)(PCCmdNone | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdBoot | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdQuit | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdDefine | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdInit | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdPause | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdRun | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdRelease | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdSnapshot | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdRecover | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdReinit | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdParams | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdInfo | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdAnswer | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdReport | PCCmdResult));
	NAME_TEST((PCCmd)(PCCmdAsync | PCCmdResult));

	return (0);
}

