// The Resource file for the ApiTest manager
#ifndef  GPI_RESOURCES_HXX
#define  GPI_RESOURCES_HXX

#include  <Resources.hxx>

class  GPIResources : public Resources
{
  public:
    // These functions initializes the manager
    static  void  init(int &argc, char *argv[]);  

    // Read the config section
    static  PVSSboolean  readSection();        

  public:
    // Get the name of our Datapoints
    static const PVSSuint getServerPort()  {return _serverPort;}
    static void setProjectName(const char *projName);
    
  private:
    static  PVSSuint  _serverPort;
};

#endif
