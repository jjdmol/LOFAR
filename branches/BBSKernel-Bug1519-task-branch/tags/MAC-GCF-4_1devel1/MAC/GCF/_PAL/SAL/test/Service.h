#include <GSA_Service.h>

class Service : public GSAService
{
  public:
    Service() {};
    virtual ~Service() {};

    virtual TSAResult dpCreate(const string& propName, 
                               const string& macType);
    virtual TSAResult dpDelete(const string& propName);
    virtual TSAResult dpeSubscribe(const string& propName);
    virtual TSAResult dpeUnsubscribe(const string& propName);
    virtual TSAResult dpeGet(const string& propName);
    virtual TSAResult dpeSet(const string& propName, const GCFPValue& value);
    virtual bool dpeExists(const string& propName);
    
  protected:
    virtual void dpCreated(const string& propName);
    virtual void dpDeleted(const string& propName);
    virtual void dpeSubscribed(const string& propName);
    virtual void dpeUnsubscribed(const string& propName);
    virtual void dpeValueGet(const string& propName, const GCFPValue& value);
    virtual void dpeValueChanged(const string& propName, const GCFPValue& value);
  
  private:
};
