#ifndef GPA_EXTERNHDL_H
#define GPA_EXTERNHDL_H

#include <BaseExternHdl.hxx>
#include <TextVar.hxx>
#include <BlobVar.hxx>

# ifdef _WIN32
#   define  PVSS_EXPORT  __declspec(dllexport)
# else
#   define  PVSS_EXPORT
# endif

class GCFEvent;

class GPAExternHdl : public BaseExternHdl
{
  public:
    // List of user defined functions
    // Internal use only
    enum 
    {
      F_gpaConvertMsgToGCFEvent = 0,
      F_gpaConvertGCFEventToMsg,
    };

    // Descritption of user defined functions. 
    // Used by libCtrl to identify function calls
    static FunctionListRec  fnList[];

    GPAExternHdl(BaseExternHdl* nextHdl, PVSSulong funcCount, FunctionListRec fnList[])
      : BaseExternHdl(nextHdl, funcCount, fnList) {}
    
    // Execute user defined function
    virtual const Variable* execute(ExecuteParamRec& param);

  private:
    bool gpaConvertMsgToGCFEvent(const DynVar& msg, BlobVar& gcfEvent);
    bool gpaConvertGCFEventToMsg(const BlobVar& gcfEvent, DynVar& msg);
    
    ssize_t recv (void* buf, size_t count);
    void encodeEvent(GCFEvent& e, DynVar& msg);
  
  private:  
    const unsigned char* _msgBuffer;
    unsigned int _bytesLeft;
};


// Create new ExternHdl. This must be global function named newExternHdl
PVSS_EXPORT   
BaseExternHdl *newExternHdl(BaseExternHdl *nextHdl);

#endif 
