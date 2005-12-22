package nl.astron.lofar.odtb.mom2otdbadapter.data;


import jOTDB.jConverterInterface;
import jOTDB.jOTDBinterface;
import jOTDB.jOTDBnode;
import jOTDB.jTreeMaintenanceInterface;

import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Vector;

import nl.astron.wsrt.util.WsrtConverter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class OTDBRepository {
	private Log log = LogFactory.getLog(this.getClass());

	private jOTDBinterface remoteOTDB = null;

	private jTreeMaintenanceInterface tm = null;
	
	private jConverterInterface converter = null;

	private static final int TEMPLATE_ID = 2;
	
	public static final String DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss";

	public OTDBRepository(String rmiServerName, int port)
			throws RemoteException, NotBoundException {

		Registry remoteRegistry = LocateRegistry.getRegistry(rmiServerName,
				port);
		remoteOTDB = (jOTDBinterface) remoteRegistry
				.lookup(jOTDBinterface.SERVICENAME);
		tm = (jTreeMaintenanceInterface) remoteRegistry
				.lookup(jTreeMaintenanceInterface.SERVICENAME);
	     converter = (jConverterInterface) remoteRegistry.lookup (jConverterInterface.SERVICENAME);	

	}

	public void store(LofarObservation lofarObservation) throws RemoteException {
		int treeId = tm.copyTemplateTree(TEMPLATE_ID);
		//int treeId = 6;
		jOTDBnode observationNode = tm.getTopNode(treeId);
		/*
		 * add observation1 parameters
		 */
		storeParam(observationNode, "arrayConfiguration", lofarObservation
				.getArrayConfiguration());
		storeParam(observationNode, "measurementMom2Ids", lofarObservation
				.getMeasurementMom2Ids());
		storeParam(observationNode, "requestedDuration", lofarObservation
				.getRequestedDuration()
				+ "");
		storeParam(observationNode, "stations", lofarObservation.getStations());
		storeParam(observationNode, "subbands", lofarObservation.getSubbands());
		/*
		 * add a01 parameters
		 */
		jOTDBnode a01Node = getNode(observationNode, "AO1");
		storeParam(a01Node, "samplingFrequency", lofarObservation
				.getSamplingFrequency().toString());
		/*
		 * add arg1 parameters
		 */
		jOTDBnode arg1Node = getNode(observationNode, "ARG1");
		storeParam(arg1Node, "bandSelection", lofarObservation
				.getBandSelection());
		storeParam(arg1Node, "srgConfiguration", lofarObservation
				.getSrgConfiguration());
		/*
		 * add vb parameters
		 */
		jOTDBnode vbNode = getNode(observationNode, "VB");
		storeParam(vbNode, "measurementType", lofarObservation.getBackend());
		/*
		 * add vi parameters
		 */
		jOTDBnode viNode = getNode(observationNode, "VI1");
		storeParam(viNode, "angle1", lofarObservation.getAngle1());
		storeParam(viNode, "angle2", lofarObservation.getAngle2());
		storeParam(viNode, "angleTimes", lofarObservation.getAngleTimes());
		storeParam(viNode, "directionType", lofarObservation.getDirectionType());
		short statusId = converter.getTreeState(lofarObservation.getStatus());
		tm.setTreeState(treeId, statusId);
		tm.setMomInfo(treeId,lofarObservation.getMom2Id(),"no campaign");
	}

	protected void storeParam(jOTDBnode parentNode, String paramName,
			String paramValue) throws RemoteException {
		jOTDBnode node = getNode(parentNode, paramName);
		if (node != null) {
			if (paramValue != null) {
				node.limits = paramValue;
			} else {
				node.limits = "";
			}
			tm.saveNode(node);
		}

	}

	protected jOTDBnode getNode(jOTDBnode parentNode, String paramName)
			throws RemoteException {
		jOTDBnode node = null;
		/*
		 * if no parent node, do not retrieve something
		 */
		if (parentNode != null) {
			Vector vector = tm.getItemList(parentNode.treeID(), paramName);
			for (int i = 0; i < vector.size(); i++) {
				node = (jOTDBnode) vector.get(i);
				/*
				 * 
				 */
				if (node.parentID() == parentNode.nodeID()) {
					return node;
				}

			}
		}
		return node;
	}

	public List getLatestChanges(Date startDate, Date endDate) throws RemoteException {
		String startTime = WsrtConverter.toDateString(startDate,DATE_TIME_FORMAT);
		String endTime = WsrtConverter.toDateString(endDate,DATE_TIME_FORMAT);
		log.info("Query latest changes between:" + startTime + " and " + endTime);
/*	     System.out.println("Trying to connect to the database");
	     remoteOTDB.connect();	
	     remoteOTDB.isConnected();
	     
	     System.out.println("Connection succesful!");
	     
	     System.out.println("getTreeList(0,0)");
	     Vector treeList;
	     treeList = remoteOTDB.getTreeList((short)0, (short)0);
	     if (treeList.size() == 0) 
	       {
		  System.out.println("Error:" + remoteOTDB.errorMsg());
		  System.exit (0);
	       }
	     else 
	       {
		  System.out.println("Collected tree list");
		  //showTreeList(treeList);
	       }
	     
	     System.out.println("getTreeInfo(treeList.elementAt(1))");
	     Integer i = new Integer((Integer)treeList.elementAt(1));
	     jOTDBtree tInfo = remoteOTDB.getTreeInfo(i.intValue());
		Vector stateList =  remoteOTDB.getStateList(0,false,""); 
		for (int i = 0; i < stateList.size();i++){
			Object object = (Object) stateList.get(i);
			//tInfo.starttime;
		}

		// do the test
		// log.info("Trying to connect to the database");
		/*
		 * remoteOTDB.connect();
		 * 
		 * 
		 * log.info("Connection succesful!");
		 * 
		 * log.info("getTreeList(0,0)"); Vector treeList; treeList =
		 * remoteOTDB.getTreeList((short)0,(short)0); //treeList =
		 * remoteOTDB.getStateList(1,false,"2005-12-16 12:00:00"); if
		 * (treeList.size() == 0) { log.info("Error:" + remoteOTDB.errorMsg()); }
		 * else { log.info("Collected tree list"); }
		 * 
		 * log.info("getTreeInfo(treeList.elementAt(1))"); Integer i =
		 * (Integer)treeList.elementAt(1); jOTDBtree tInfo =
		 * remoteOTDB.getTreeInfo(i.intValue(), false); if (tInfo.treeID()==0) {
		 * System.out.println("No such tree found!"); } else {
		 * log.info(tInfo.classification +""); log.info(tInfo.creator);
		 * log.info(tInfo.creationDate); log.info(tInfo.type +"");
		 * log.info(tInfo.state+""); log.info(tInfo.originalTree+"");
		 * log.info(tInfo.campaign); log.info(tInfo.starttime);
		 * log.info(tInfo.stoptime); log.info(tInfo.treeID()+"");
		 * log.info("MomID: " + tInfo.treeID()+""); }
		 */
		List result = new ArrayList();
/*		LofarObservation observation = new LofarObservation();
		observation.setMom2Id(15);
		observation.setAngleTimes("[+0,+30,+60]");
		observation.setStatus("aborted");
		observation.setMeasurementMom2Ids("[16,17,18]");
		observation.setStartTime("16-12-2005 12:00:15");
		observation.setEndTime("16-12-2005 12:01:14");
		result.add(observation);
		observation = new LofarObservation();
		observation.setMom2Id(20);
		observation.setStatus("specified");
		observation.setMeasurementMom2Ids("[21,22,23]");
		result.add(observation);
		observation = new LofarObservation();
		observation.setMom2Id(30);
		observation.setStatus("active");
		observation.setMeasurementMom2Ids("[31,32,33]");
		result.add(observation);
		observation = new LofarObservation();
		observation.setMom2Id(40);
		observation.setAngleTimes("[+0,+30,+60]");
		observation.setStatus("finished");
		observation.setMeasurementMom2Ids("[41,42,43]");
		observation.setStartTime("16-12-2005 12:00:15");
		observation.setEndTime("16-12-2005 12:02:14");
		result.add(observation);*/
/*		LofarObservation observation = new LofarObservation();
		observation = new LofarObservation();
		observation.setMom2Id(65);
		observation.setAngleTimes("[+00]");
		observation.setStatus("finished");
		observation.setMeasurementMom2Ids("[66]");
		observation.setStartTime("16-12-2005 12:00:15");
		observation.setEndTime("16-12-2005 12:02:14");
		result.add(observation);*/
		LofarObservation observation = new LofarObservation();
		observation = new LofarObservation();
		observation.setMom2Id(65);
		observation.setStatus("specified");
		observation.setMeasurementMom2Ids("[66]");
		result.add(observation);
		return result;
	}

}
