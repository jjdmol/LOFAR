#include <SAL/GSA_Service.h>

class Service : public GSAService
{
  public:
    Service() {};
    virtual ~Service() {};

    virtual TSAResult createProp(const string& macType, const string& propName);
    virtual TSAResult deleteProp(const string& propName);
    virtual TSAResult subscribe(const string& propName);
    virtual TSAResult unsubscribe(const string& propName);
    virtual TSAResult get(const string& propName);
    virtual TSAResult set(const string& propName, const GCFPValue& value);
    virtual bool exists(const string& propName);
    
  protected:
    virtual void propCreated(string& propName);
    virtual void propDeleted(string& propName);
    virtual void propSubscribed(string& propName);
    virtual void propUnsubscribed(string& propName);
    virtual void propValueGet(string& propName, GCFPValue& value);
    virtual void propValueChanged(string& propName, GCFPValue& value);
  
  private:
};
