import java.rmi.RemoteException;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jClassifConv;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBconnection;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.jotdb2.jTreeMaintenance;
import nl.astron.lofar.sas.otb.jotdb2.jTreeStateConv;
import nl.astron.lofar.sas.otb.jotdb2.jTreeTypeConv;
import nl.astron.lofar.sas.otb.jotdb2.jInitCPPLogger;

class tTreeMaintenance
{
   static
     {
	System.loadLibrary ("jotdb2");
     }
   
   
   public static void main (String[] args)
     {
	String logConfig="tTreeMaintenance.log_prop";
        try {
            jInitCPPLogger aCPPLogger= new jInitCPPLogger(logConfig);
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }
	tTreeMaintenance ttreemain = new tTreeMaintenance ();
	ttreemain.test ();
     }

    public void showTreeList (Vector trees, jOTDBconnection conn)
    {

	jOTDBtree aTree;
	Integer treeID;
        boolean firstLine=true;
        
	for (int i = 0; i < trees.size (); ++i) 
	    {
		aTree = (jOTDBtree)trees.elementAt(1);
                showTree(aTree,firstLine);
                firstLine=false;
	    }	
	System.out.println (trees.size () + " records\n");
    }
    
    public void showTree(jOTDBtree aTree, boolean firstLine) {
        if (firstLine) {
            System.out.println ("treeID|Classif|Creator   |Creationdate        |Type|Campaign|Starttime         |State|Description");
            System.out.println ("------+-------+----------+--------------------+----+--------+------------------+-----+--------------------");
        }

        System.out.printf ("%6d|%7d|%-10.10s|%-20.20s|%4d|%-8.8s|%s-18.18s|%5d|%s", aTree.treeID(), aTree.classification, 
                aTree.creator, aTree.creationDate, aTree.type, aTree.campaign, aTree.starttime, aTree.state, aTree.description); 
	System.out.println ();
    }
    
    public void showNodeList (Vector nodes) 
    {
        boolean firstLine=true;
	jOTDBnode aNode;

	for (int i = 0; i < nodes.size (); ++i) 
	    {
		aNode = (jOTDBnode)nodes.elementAt (i);
                showNode(aNode,firstLine);
                firstLine=false;
	    }	
	System.out.println (nodes.size () + " records\n");
    }
    
    public void showNode(jOTDBnode aNode,boolean firstLine) {
        if (firstLine) {
            System.out.println ("treeID|nodeID|parent|name           |index|leaf|inst|description");
            System.out.println ("------+------+------+---------------+-----+----+----+------------------");
        }
        System.out.printf ("%6d|%6d|%6d|%-15.15s|%5d|%s|%4d|%s",
                aNode.treeID (), aNode.nodeID (), aNode.parentID (), aNode.name, aNode.index,
		aNode.leaf, aNode.instances, aNode.description);
        System.out.println ();
    }

   
    public void test () {
        try {
            // do the test
            System.out.println ("Starting... ");
	
            // create a jOTDBconnection
            jOTDBconnection conn = new jOTDBconnection ("paulus","boskabouter","otdbtest","dop50.astron.nl");
	
            System.out.println ("Trying to connect to the database");
            assert conn.connect () : "Connection failed";	
            assert conn.isConnected () : "Connnection flag failed";

            System.out.println ("Connection succesful!");

            System.out.println ("Trying to construct a TreeMaintenance object");
            jTreeMaintenance tm = new jTreeMaintenance ();
            assert tm!=null : "Creation of treeMaintenance Failed!";
        
            // used Converters:
            jTreeTypeConv  TTconv = new jTreeTypeConv();
            jTreeStateConv TSconv = new jTreeStateConv(); 
            jClassifConv   CTconv = new jClassifConv();

            System.out.println ("Trying to load the componentfile: tVTtree.in");
            int topNodeID = tm.loadComponentFile ("tTreeMaintenance.in","","");
            assert topNodeID!=0 : "Loading of componentfile failed";

        
            System.out.println("Building a template tree");
            int treeID = tm.buildTemplateTree(topNodeID,CTconv.get("test"));
            assert treeID!=0 : "Creation of template tree failed";
            System.out.println("TreeID = " + treeID);
        
            System.out.println ("Searching for a Template tree");
            Vector treeList = conn.getTreeList (TTconv.get("VItemplate"), CTconv.get("test"));
            showTreeList (treeList, conn);
            assert treeList.size()!=0 : "No template tree found!";

            int VTtreeID=((jOTDBtree)treeList.elementAt(treeList.size()-1)).treeID();
            System.out.println("Using tree " + VTtreeID + " for the tests");
            jOTDBtree treeInfo = conn.getTreeInfo(VTtreeID,false);
            showTree(treeInfo,true);
                
            System.out.println("Changing the description to 'test_tree'");
            tm.setDescription(VTtreeID, "test_tree");
            treeInfo = conn.getTreeInfo(VTtreeID, false);
            showTree(treeInfo,true);

            System.out.println("Trying to get the topnode of the tree");
            jOTDBnode topNode = tm.getTopNode (treeInfo.treeID());
            showNode(topNode,true);

            System.out.println("Trying to get a collection of items on depth=1");
            Vector nodeList = tm.getItemList (VTtreeID, topNode.nodeID(), 1);
            showNodeList (nodeList);

            System.out.println("Trying to get a collection of items on depth=2");
            nodeList = tm.getItemList (VTtreeID, topNode.nodeID(), 2);
            showNodeList (nodeList);
		
            int elemNr = nodeList.size () - 1;
            System.out.println ("Zooming in on last element : "+ elemNr);
            jOTDBnode aNode = tm.getNode (VTtreeID,  ((jOTDBnode)nodeList.elementAt(elemNr)).nodeID());
            showNode(aNode, true);

            System.out.println ("Trying to classify the tree to: "+CTconv.get("operational"));
            boolean actionOK =tm.setClassification(VTtreeID,CTconv.get("operational"));
            if (actionOK) {
                System.out.println("Setting classification was succesful");
            } else {
                System.out.println("Setting classification was NOT succesful");
            }
            treeInfo = conn.getTreeInfo(VTtreeID,false);
            showTree(treeInfo,true);
        
            short aTreeState = TSconv.get("active");
            System.out.println ("Trying to change the state of the tree to "+ aTreeState);
            actionOK = tm.setTreeState (VTtreeID, aTreeState); 
            assert actionOK : "Changing the state to active should have NOT have failed!";
            treeInfo = conn.getTreeInfo (VTtreeID, false);
            showTree(treeInfo,true);
        
            System.out.println ("========== Testing manipulation of nodes ==========");
	
            System.out.println ("Searching for node 'Virt Telescope'");
            Vector VtelCol = tm.getItemList (VTtreeID, "%Telescope");
            System.out.println ("Found " + VtelCol.size () + " nodes");
            jOTDBnode VtelDef = (jOTDBnode)VtelCol.firstElement ();
            assert VtelDef.nodeID () != 0 : "Node 'Virt Telescope' not found";
            System.out.println ("Found definition: " );
            showNode(VtelDef,true);
	
            // Test the manipulations on the VT
            System.out.println ("Trying to duplicate the subtree");
            int nodeID = tm.dupNode (VTtreeID, VtelDef.nodeID (), (short)1);
            System.out.println ("New subtree starts at node: " + nodeID);
	
            System.out.println ("Trying to retrieve one node");
            aNode = tm.getNode (VTtreeID, nodeID);
            showNode(aNode,true);
            System.out.println ("Modifying the instances and limits");
            aNode.instances = 5;
            aNode.limits = "no more limits";
            tm.saveNode (aNode);
            showNode(aNode,true);
	
            System.out.println ("Trying to retrieve one node");
            aNode = tm.getNode (VTtreeID, nodeID);
            showNode(aNode, true);
            System.out.println ("Removing the just created subtree");
            System.out.println ("nodeID before removal: " + aNode.nodeID ());
            int orgNodeID = aNode.nodeID();
            tm.deleteNode (aNode);
            System.out.println ("nodeID after removal : " + aNode.nodeID ());
	
            System.out.println ("Trying to retrieve the deleted node");
            aNode = tm.getNode (VTtreeID, orgNodeID);
            showNode(aNode,true);

            // Test the manipulations off the parameters
            System.out.println ("Duplicating node Beamformer for index=3");
            Vector BformCol = tm.getItemList (VTtreeID, "Beamformer");
            jOTDBnode BformDef = (jOTDBnode)BformCol.firstElement ();
            System.out.println ("Beamformer has ID " + BformDef.nodeID());
            System.out.println (VTtreeID +" " + BformDef.nodeID ());
            int dupNodeID = tm.dupNode (VTtreeID, BformDef.nodeID (), (short)3);
            System.out.println ("New subtree starts at node: " + dupNodeID);
	
            System.out.println ("Getting param info for " + dupNodeID+2 + " and " + dupNodeID+3);
            jOTDBnode param1 = tm.getNode (VTtreeID, dupNodeID+2);
            jOTDBnode param2 = tm.getNode (VTtreeID, dupNodeID+3);
            showNode(param1,true);
            showNode(param2,true);
            param1.limits = "1.33333";
            param2.limits = "---1---";
            System.out.println ("Changing param " + param1.name + " to " + param1.limits);
            System.out.println ("Changing param " + param2.name + " to " + param2.limits);
            tm.saveNode (param1);
            tm.saveNode (param2);

            // Setting nr instances to some nice values
            System.out.println ("Setting up tree counts");
            Vector aNodeCol = tm.getItemList (VTtreeID, "RFI dete%");
            aNode = (jOTDBnode)aNodeCol.firstElement ();
            aNode.instances = 40;
            tm.saveNode (aNode);
            System.out.println ("RFI detectors  : 40");
            aNodeCol = tm.getItemList (VTtreeID, "Correlator%");
            aNode = (jOTDBnode)aNodeCol.firstElement ();
            aNode.instances = 130;
            tm.saveNode (aNode);
            System.out.println ("Correlators    : 130");
            aNodeCol = tm.getItemList (VTtreeID, "Storage");
            aNode = (jOTDBnode)aNodeCol.firstElement ();
            aNode.instances = 86;
            tm.saveNode (aNode);
            System.out.println ("Storage        : 86");
            aNodeCol = tm.getItemList (VTtreeID, "Visua%");
            aNode = (jOTDBnode)aNodeCol.firstElement ();
            aNode.instances = 24;
            tm.saveNode (aNode);
            System.out.println ("Visualisation  : 24");
            aNodeCol = tm.getItemList (VTtreeID, "Virt Tel%");
            aNode = (jOTDBnode)aNodeCol.firstElement ();
            aNode.instances = 8;
            tm.saveNode (aNode);
            System.out.println ("Virt.Telescopes: 8");
            aNodeCol = tm.getItemList (VTtreeID, "Beamfor%");
            aNode = (jOTDBnode)aNodeCol.firstElement ();
            aNode.instances = 4;
            tm.saveNode (aNode);
            System.out.println ("Beamformers    : 4");

            // Test copying a template
            System.out.println ("Trying to copy the template tree");
            int secondVTtreeID = tm.copyTemplateTree (VTtreeID);
            System.out.println ("ID of new tree is " + secondVTtreeID);
            if (secondVTtreeID == 0) {
                System.out.println (tm.errorMsg());
            }
            jOTDBtree VTtree = conn.getTreeInfo (secondVTtreeID,false);
            showTree(VTtree,true);
	
            // Test creating a full tree of the template tree
            System.out.println ("Trying to instanciate the copied tree");
            int VHtreeID = tm.instanciateTree (secondVTtreeID);
            System.out.println ("ID of new tree is " + VHtreeID);
            if (VHtreeID == 0) {
                System.out.println (tm.errorMsg());
            }
            jOTDBtree VHtree = conn.getTreeInfo (VHtreeID,false);
            showTree(VHtree,true);
        
        
            // Test deleting an active tree
            System.out.println("Trying to delete original template tree " + VTtreeID);
            try {
                tm.deleteTree(VTtreeID);
                assert false : "DELETING AN ACTIVE TREE IS NOT ALLOWED!";
            } catch (Exception  e)  {
                System.out.println("EXPECTED  exception: " + e);
            }

            aTreeState = TSconv.get("obsolete");		
            System.out.println("Trying to change the state of the tree to " +aTreeState);
            actionOK = tm.setTreeState(VTtreeID, aTreeState);
            assert actionOK : "Changing the state to " + aTreeState + " should have NOT have failed!";
        
        
            // Test deleting a tree
            System.out.println("Retrying to delete original template tree " + VTtreeID);
            assert tm.deleteTree(VTtreeID) : "Deletion of original tree went wrong:" + tm.errorMsg();
            System.out.println("Deletion of original tree was succesful");

            System.out.println ("Terminated succesfully: ");
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }
     }
}




