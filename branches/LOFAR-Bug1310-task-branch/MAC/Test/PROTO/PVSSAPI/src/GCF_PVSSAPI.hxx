// Declaration of our PVSSAPI-class
#ifndef  GCF_PVSSAPI_HXX
#define  GCF_PVSSAPI_HXX

#include <Manager.hxx>        // include/Manager
#include <DpIdentifier.hxx>   // include/Basics
#include <Timer.hxx>
#include <TASK/GCF_SCADAAPI.hxx>

class GCFPvssApi;

// ---------------------------------------------------------------
// Our callback class

class GCFHotlinkWaitForAnswer : public HotLinkWaitForAnswer
{
  public:
    GCFHotlinkWaitForAnswer(GCFPvssApi *manager);

    void hotLinkCallBack(DpMsgAnswer &answer);

protected:
    // Answer on conenct
	  void hotLinkCallBack(DpHLGroup &group);

  private:
    GCFPvssApi *_manager;
};                                 


class GCFPvssApi : public Manager, public GCFScadaApi
{
  public:
    // Default constructor
	  GCFPvssApi(GCFTask *task);

    // prepare operation for every Manager
	  void init();
    void workProc();
    void stop() {};

    // handle incoming hotlinks by group
    void handleHotLink(const DpHLGroup &group);
    // handle incoming hotlinks by answer
    void handleHotLink(const DpMsgAnswer &answer);

    PVSSboolean connectToDp(const CharString &dpName, DpIdentifier &dpId);
    PVSSboolean getDpId(const CharString &dpName, DpIdentifier &dpId);
    PVSSboolean createDp(const CharString &typeName, const CharString &dpName,
                         DpIdentifier *pDpId = NULL);

  private:

    // private Variables
    DpIdentifier  _dpId;       // DP the values are copied to

    Timer _timer;
    GCFHotlinkWaitForAnswer _wait;
    
  private:

    static GCFDummyPort _pvssPort;
};

struct GCFVChangeMsgEvent : public GCFEvent
{
  GCFVChangeMsgEvent() : GCFEvent(F_VCHANGEMSG_SIG)
  {
      length = sizeof(GCFVChangeMsgEvent);
  }
  DpVCItem *_pMsg;
};

struct GCFAnswerEvent : public GCFEvent
{
  GCFAnswerEvent(unsigned short sig) : GCFEvent(sig)
  {
      length = sizeof(GCFAnswerEvent);
  }
  AnswerItem *_pMsg;
};

#endif
