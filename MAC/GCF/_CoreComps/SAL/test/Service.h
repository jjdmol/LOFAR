#include <GSA_Service.h>

class Service : public GSAService
{
  public:
    Service() {};
    virtual ~Service() {};

    virtual TSAResult createProp(const string& propName, 
                                 GCFPValue::TMACValueType macType);
    virtual TSAResult deleteProp(const string& propName);
    virtual TSAResult subscribe(const string& propName);
    virtual TSAResult unsubscribe(const string& propName);
    virtual TSAResult get(const string& propName);
    virtual TSAResult set(const string& propName, const GCFPValue& value);
    virtual bool exists(const string& propName);
    
  protected:
    virtual void propCreated(const string& propName);
    virtual void propDeleted(const string& propName);
    virtual void propSubscribed(const string& propName);
    virtual void propUnsubscribed(const string& propName);
    virtual void propValueGet(const string& propName, const GCFPValue& value);
    virtual void propValueChanged(const string& propName, const GCFPValue& value);
  
  private:
};
