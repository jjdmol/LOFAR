#ifndef MEQSERVER_SRC_VISDATAMUX_H_HEADER_INCLUDED_82375EEB
#define MEQSERVER_SRC_VISDATAMUX_H_HEADER_INCLUDED_82375EEB

#include <MeqServer/VisHandlerNode.h>
#include <vector>
    
#pragma aid Station Index 
    
namespace MEQ {

//##ModelId=3F98DAE503DA
class VisDataMux
{
  public:
    VisDataMux (MEQ::Forest &frst);
      
    //##ModelId=3F98DAE6024A
    void init (const DataRecord &header);
      
    //##ModelId=3F98DAE6024C
    void addNode (Node &node);
  
    //##ModelId=3F98DAE6024F
    void removeNode (Node &node);
    
    //##ModelId=3F98DAE60251
    //##Documentation
    int deliver (VisTile::Ref::Copy &tileref);

    //##ModelId=3F98DAE60246
    ImportDebugContext(VisHandlerNode);
    
  private:
    VisDataMux ();
    VisDataMux (const VisDataMux &);
    
    //##ModelId=3F992F280174
    static int formDataId (int sta1,int sta2);
  
    //##ModelId=3F99305003E3
    typedef std::list<VisHandlerNode *> VisHandlerList;
    //##ModelId=3F98DAE60247
    std::vector<VisHandlerList> handlers_;
    
    MEQ::Forest & forest_;
};

} // namespace MEQ

#endif /* MEQSERVER_SRC_SPIGOTMUX_H_HEADER_INCLUDED_82375EEB */
