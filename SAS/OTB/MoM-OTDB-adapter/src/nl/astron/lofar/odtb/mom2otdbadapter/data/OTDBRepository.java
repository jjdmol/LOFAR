package nl.astron.lofar.odtb.mom2otdbadapter.data;

import jOTDB.jOTDBinterface;
import jOTDB.jTreeMaintenanceInterface;

import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;



public class OTDBRepository {
	private Log log = LogFactory.getLog(this.getClass());
	private jOTDBinterface remoteOTDB = null;
	private jTreeMaintenanceInterface tm = null;
	private static final int TEMPLATE_ID = 3;
	
	public OTDBRepository(String rmiServerName, int port) throws RemoteException, NotBoundException{

/*	     Registry remoteRegistry = LocateRegistry.getRegistry (rmiServerName,port);
	     remoteOTDB = (jOTDBinterface) remoteRegistry.lookup (jOTDBinterface.SERVICENAME);
	     tm = (jTreeMaintenanceInterface) remoteRegistry.lookup (jTreeMaintenanceInterface.SERVICENAME);	

*/
	
   
	}

	public void story(LofarObservation lofarObservation) throws RemoteException{
/*		int treeId = tm.copyTemplateTree(TEMPLATE_ID);
		jOTDBtree tInfo = remoteOTDB.getTreeInfo(treeId, false);
	*/	
	}	

	public List getLatestChanges(String date) throws RemoteException{
	    List result = new ArrayList();
	     // do the test	
	     //log.info("Trying to connect to the database");
/*	     remoteOTDB.connect();

	     
	     log.info("Connection succesful!");
	     
	     log.info("getTreeList(0,0)");
	     Vector treeList;
	     treeList = remoteOTDB.getTreeList((short)0,(short)0);
	     //treeList = remoteOTDB.getStateList(1,false,"2005-12-16 12:00:00");
	     if (treeList.size() == 0) 
	       {
	    	 log.info("Error:" + remoteOTDB.errorMsg());

	       }
	     else 
	       {
	    	 log.info("Collected tree list");
	       }
	     
	     log.info("getTreeInfo(treeList.elementAt(1))");
	     Integer i = (Integer)treeList.elementAt(1);
	     jOTDBtree tInfo = remoteOTDB.getTreeInfo(i.intValue(), false);
	     if (tInfo.treeID()==0) 
	       {
		  System.out.println("No such tree found!");
	       }
	     else 
	       {
	    	 log.info(tInfo.classification +"");
	    	 log.info(tInfo.creator);
	    	 log.info(tInfo.creationDate);	
	    	 log.info(tInfo.type +"");
	    	 log.info(tInfo.state+"");
	    	 log.info(tInfo.originalTree+"");
	    	 log.info(tInfo.campaign);	
	    	 log.info(tInfo.starttime);
	    	 log.info(tInfo.stoptime);
	    	 log.info(tInfo.treeID()+"");	   
	    	 log.info("MomID: " + tInfo.treeID()+"");	 
	       }
	  */
		LofarObservation observation = new LofarObservation();
		observation.setMom2Id("15");
		observation.setAngleTimes("[+0,+30,+60]");
		observation.setStatus("aborted");
		observation.setMeasurementMom2Ids("[16,17,18]");
		observation.setStartTime("16-12-2005 12:00:15");
		observation.setEndTime("16-12-2005 12:01:14");
		result.add(observation);
		observation = new LofarObservation();
		observation.setMom2Id("20");
		observation.setStatus("specified");
		observation.setMeasurementMom2Ids("[21,22,23]");
		result.add(observation);
		observation = new LofarObservation();
		observation.setMom2Id("30");
		observation.setStatus("active");
		observation.setMeasurementMom2Ids("[31,32,33]");
		result.add(observation);
		observation = new LofarObservation();
		observation.setMom2Id("40");
		observation.setAngleTimes("[+0,+30,+60]");
		observation.setStatus("finished");
		observation.setMeasurementMom2Ids("[41,42,43]");
		observation.setStartTime("16-12-2005 12:00:15");
		observation.setEndTime("16-12-2005 12:02:14");
		result.add(observation);
		return result;
	}

}
