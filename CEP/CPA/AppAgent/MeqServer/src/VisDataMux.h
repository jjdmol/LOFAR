#ifndef MEQSERVER_SRC_VISDATAMUX_H_HEADER_INCLUDED_82375EEB
#define MEQSERVER_SRC_VISDATAMUX_H_HEADER_INCLUDED_82375EEB

#include <AppAgent/AppControlAgent.h>
#include <VisAgent/InputAgent.h>
#include <VisAgent/OutputAgent.h>
#include <MeqServer/VisHandlerNode.h>
#include <vector>
    
#pragma aid Station Index Tile Format

class AppControlAgent;
        
namespace Meq {
  
//##ModelId=3F98DAE503DA
class VisDataMux
{
  public:
    //##ModelId=3F9FF71B006A
    VisDataMux (Meq::Forest &frst);
      
    //##ModelId=3FA1016000B0
    void init (const DataRecord &rec,
                VisAgent::InputAgent  & inp,
                VisAgent::OutputAgent & outp,
                AppControlAgent       & ctrl);
    
    //##ModelId=3F98DAE6024C
    void addNode (Node &node);
  
    //##ModelId=3F98DAE6024F
    void removeNode (Node &node);
    
    //##ModelId=3F98DAE6024A
    //##Documentation
    //## delivers visdata header to data mux
    //## control agent may be used to post error events
    int deliverHeader (const DataRecord &header);
      
    //##ModelId=3F98DAE60251
    //##Documentation
    //## delivers tile to data mux
    //## control agent may be used to post error events
    int deliverTile (VisTile::Ref::Copy &tileref);

    //##Documentation
    //## delivers visdata footer to data mux
    //## control agent may be used to post error events
    int deliverFooter (const DataRecord &footer);

    AppControlAgent &       control()   { return *control_; }
    VisAgent::InputAgent &  input()     { return *input_;   }
    VisAgent::OutputAgent & output()    { return *output_; }
        
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
    Meq::Forest & forest_;
 
    //  list of columns to be added to output tiles
    //##ModelId=3FAA52A6008E
    std::vector<int>     out_columns_;
    //##ModelId=3FAA52A6014F
    std::vector<string>  out_colnames_;
    //##ModelId=3FAA52A6018C
    VisTile::Format::Ref out_format_;
    
    // header is cached; it is dumped to output only if some tiles are being
    // written
    DataRecord::Ref cached_header_;
    // flag: tiles are being written
    bool writing_data_;
    
    //##ModelId=400E5B6D0151
    double minfreq;
    //##ModelId=400E5B6D0177
    double maxfreq;
    
    AppControlAgent       * control_;
    VisAgent::InputAgent  * input_;
    VisAgent::OutputAgent * output_;
};

} // namespace Meq

#endif /* MEQSERVER_SRC_SPIGOTMUX_H_HEADER_INCLUDED_82375EEB */
