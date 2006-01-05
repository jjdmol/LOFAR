package nl.astron.lofar.odtb.mom2otdbadapter.data;


import jOTDB.jConverterInterface;
import jOTDB.jOTDBinterface;
import jOTDB.jOTDBnode;
import jOTDB.jTreeMaintenanceInterface;
import jOTDB.jTreeState;
import jOTDB.jOTDBtree;

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

/**
 * Repository that stores and retrieves LofarObservation objects to the jOTDB RMI interface.
 * It converts it to the tree structure of jOTDB and vice versa.
 * 
 * @author Bastiaan Verhoef
 *
 */
public class OTDBRepository {
	private Log log = LogFactory.getLog(this.getClass());

	private jOTDBinterface remoteOTDB = null;

	private jTreeMaintenanceInterface tm = null;
	
	private jConverterInterface converter = null;

	private static final int TEMPLATE_ID = 2;
	
	public static final String DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss";


	
	/**
	 * Constructor that makes a connection to the specified rmi server on the specified rmi port
	 * @param rmiServerName
	 * @param port
	 * @throws RemoteException
	 * @throws NotBoundException
	 */
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

	/**
	 * Stores a lofarObservation to jOTDB
	 * 
	 * @param lofarObservation
	 * @throws RemoteException
	 */
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

	/**
	 * Stores parameter in the tree by given parent node
	 * @param parentNode parent node
	 * @param paramName parameter name
	 * @param paramValue parameter value
	 * @throws RemoteException
	 */
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

	/**
	 * Retrieves node by parent node and parameter name
	 * 
	 * @param parentNode parent node
	 * @param paramName name of the parameter
	 * @return node
	 * @throws RemoteException
	 */
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

	/**
	 * Retrieve lates changes between start- en endtime
	 * @param startDate start time
	 * @param endDate end time
	 * @return List of lofarObservation objects
	 * @throws RemoteException
	 */
	public List getLatestChanges(Date startDate, Date endDate) throws RemoteException {
		String startTime = WsrtConverter.toDateString(startDate,DATE_TIME_FORMAT);
		String endTime = WsrtConverter.toDateString(endDate,DATE_TIME_FORMAT);
		log.info("Retrieve latest changes between:" + startTime + " and " + endTime);
		remoteOTDB.connect();
		List result = new ArrayList();
		Vector stateList = remoteOTDB.getStateList(0,false,startTime, endTime);
		for (int i = 0; i < stateList.size();i++){
			jTreeState state = (jTreeState) stateList.get(i);
			
			String status = converter.getTreeState(state.newState);
			log.info("momId: "+ state.momID + " status:" + status + " statusId:" + state.newState);
			if (state.momID >0 && isStatusThatMustBeExported(status)){
				LofarObservation observation = new LofarObservation();
				observation.setMom2Id(state.momID);
				observation.setStatus(status);
				observation.setTimeStamp(state.timestamp);
				jOTDBnode observationNode = tm.getTopNode(state.treeID);
				jOTDBnode measurementsNode = getNode(observationNode, "measurementMom2Ids");
				observation.setMeasurementMom2Ids(measurementsNode.limits);
				if (status.equals("finished")){
					jOTDBnode viNode = getNode(observationNode, "VI1");
					jOTDBnode angle1 = getNode(viNode, "angle1");
					jOTDBnode angle2 = getNode(viNode, "angle2");
					jOTDBnode angleTimes = getNode(viNode, "angleTimes");
					observation.setAngle1(angle1.limits);
					observation.setAngle2(angle2.limits);
					observation.setAngleTimes(angleTimes.limits);
					jOTDBtree treeInfo = remoteOTDB.getTreeInfo(state.treeID,false);
					observation.setStartTime(treeInfo.starttime);
					observation.setEndTime(treeInfo.stoptime);
				}
				result.add(observation);
			}
		}

		return result;
	}
	/**
	 * Checkes if observation with a status must be exported to MoM
	 * @param code
	 * @return true, if it must be exported
	 */
	protected boolean isStatusThatMustBeExported(String code) {
		if (code.equals("specified")) {
			return true;
		}
		if (code.equals("active")) {
			return true;
		}
		if (code.equals("finished")) {
			return true;
		}
		if (code.equals("aborted")) {
			return true;
		}
		if (code.equals("failed")) {
			return true;
		}
		return false;
	}

}
