// Declaration of our SCADAAPI-class
#ifndef  GCF_SCADAAPI_HXX
#define  GCF_SCADAAPI_HXX

#include "GCF_Task.hxx"

class GCFScadaApi
{
  public:
    // Default constructor
	  GCFScadaApi(GCFTask *fsm) : _pFsm(fsm)
    {
      if (fsm) fsm->attachScadaApi(this); 
    }

    // prepare operation for every Manager
	  virtual void init() = 0;
    virtual void workProc() = 0;
    virtual void stop() = 0;

  protected:

    GCFFsm *_pFsm;
};
#endif
