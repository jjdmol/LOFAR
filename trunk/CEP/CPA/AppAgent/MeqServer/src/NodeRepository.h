#ifndef MEQSERVER_SRC_NODEREPOSITORY_H_HEADER_INCLUDED_C53FA569
#define MEQSERVER_SRC_NODEREPOSITORY_H_HEADER_INCLUDED_C53FA569

#include <MeqServer/Node.h>
#include <vector>
#include <map>
    
namespace MEQ {

//##ModelId=3F5F21740281
class NodeRepository
{
  public:
    //##ModelId=3F60697A00ED
    NodeRepository();
      
    //##ModelId=3F5F572601B2
    //##Documentation
    //## Creates a node using the given init record. The class of the node is
    //## determined by initrec["Class"].
    //## Returns node index.
    int create (DataRecord::Ref::Xfer &initrec);
  
    //##ModelId=3F5F5CA300E0
    //##Documentation
    //## Removes the node with the given index.
    int remove (int node_index);
  
    //##ModelId=3F5F5B4F01BD
    //##Documentation
    //## Returns node specified by index
    Node & get (int node_index);
  
    //##ModelId=3F5F5B9D016A
    bool valid (int node_index) const;
 
    //##ModelId=3F5F5BB2032C
    //##Documentation
    //## Finds node by name, returns index (<0 if not found)
    int findIndex (const string& name) const;
    
    //##ModelId=3F60549B02FE
    //##Documentation
    //## Finds node by name, returns reference to node. Throws exception if not
    //## found.
    Node &findNode(const string &name);

    //##ModelId=3F60697A0078
    LocalDebugContext;

  private:
    //##ModelId=3F60697903A7
    typedef std::vector<Node::Ref> Repository;
    Repository nodes;
    
    //##ModelId=3F60697A00A3
    static const int RepositoryChunkSize = 8192;
  
    //##ModelId=3F60697903C1
    typedef std::map<string,int> NameMap;
    //##ModelId=3F60697A00D5
    NameMap name_map;
};

} // namespace MEQ



#endif /* MEQSERVER_SRC_NODEREPOSITORY_H_HEADER_INCLUDED_C53FA569 */
