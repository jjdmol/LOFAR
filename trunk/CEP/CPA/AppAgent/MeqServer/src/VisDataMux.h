#ifndef MEQSERVER_SRC_VISDATAMUX_H_HEADER_INCLUDED_82375EEB
#define MEQSERVER_SRC_VISDATAMUX_H_HEADER_INCLUDED_82375EEB

#include <MeqServer/VisHandlerNode.h>
#include <vector>
    
#pragma aid Station Index Tile Format
    
namespace MEQ {

//##ModelId=3F98DAE503DA
class VisDataMux
{
  public:
    //##ModelId=3F9FF71B006A
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
    //##ModelId=3F9FF71B00AE
    VisDataMux ();
    //##ModelId=3F9FF71B00C7
    VisDataMux (const VisDataMux &);
    
    //##ModelId=3F992F280174
    static int formDataId (int sta1,int sta2);
  
    //##ModelId=3F99305003E3
    typedef std::list<VisHandlerNode *> VisHandlerList;
    //##ModelId=3F98DAE60247
    std::vector<VisHandlerList> handlers_;
    
    //##ModelId=3F9FF71B004E
    MEQ::Forest & forest_;
    
    VisTile::Format::Ref output_format_;
};

} // namespace MEQ

#endif /* MEQSERVER_SRC_SPIGOTMUX_H_HEADER_INCLUDED_82375EEB */
