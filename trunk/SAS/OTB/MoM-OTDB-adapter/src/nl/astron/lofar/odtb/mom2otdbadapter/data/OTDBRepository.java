package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Vector;

import nl.astron.lofar.sas.otb.jotdb2.jConverterInterface;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jTreeMaintenanceInterface;
import nl.astron.lofar.sas.otb.jotdb2.jTreeState;
import nl.astron.util.AstronConverter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Repository that stores and retrieves LofarObservation objects to the jOTDB
 * RMI interface. It converts it to the tree structure of jOTDB and vice versa.
 * 
 * @author Bastiaan Verhoef
 * 
 */
public class OTDBRepository implements Repository {
	private Log log = LogFactory.getLog(this.getClass());

	private jOTDBinterface remoteOTDB = null;

	private jTreeMaintenanceInterface tm = null;

	private jConverterInterface converter = null;

	private static final int TEMPLATE_ID = 5031;

	// private static final int TEMPLATE_ID = 50091980;

	public static final String DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss";

	/**
	 * Constructor that makes a connection to the specified rmi server on the
	 * specified rmi port
	 * 
	 * @param rmiServerName
	 * @param port
	 * @throws RemoteException
	 * @throws NotBoundException
	 */
	public OTDBRepository(String rmiServerName, int port) throws RemoteException, NotBoundException {

		Registry remoteRegistry = LocateRegistry.getRegistry(rmiServerName, port);
		remoteOTDB = (jOTDBinterface) remoteRegistry.lookup(jOTDBinterface.SERVICENAME);
		tm = (jTreeMaintenanceInterface) remoteRegistry.lookup(jTreeMaintenanceInterface.SERVICENAME);
		converter = (jConverterInterface) remoteRegistry.lookup(jConverterInterface.SERVICENAME);

	}

	/**
	 * Stores a lofarObservation to jOTDB
	 * 
	 * @param lofarObservation
	 * @throws RemoteException
	 */
	public void store(LofarObservation lofarObservation) throws RepositoryException {
		try {
			int treeId = tm.copyTemplateTree(TEMPLATE_ID);
			// int treeId = 6;
			jOTDBnode observationNode = getObservationNode(treeId);
			List<jOTDBnode> beams = new ArrayList<jOTDBnode>();
			Vector<jOTDBnode> childs = getChilds(observationNode);
			for (jOTDBnode node : childs) {
				if ("antennaArray".equals(node.name)) {
					fillNode(node, lofarObservation.getAntennaArray());
				} else if ("antennaSet".equals(node.name)) {
					fillNode(node, lofarObservation.getAntennaSet());
				} else if ("bandFilter".equals(node.name)) {
					fillNode(node, lofarObservation.getBandFilter());
				} else if ("clockMode".equals(node.name)) {
					fillNode(node, lofarObservation.getClockMode());
				} else if ("Beam".equals(node.name)) {
					beams.add(node);
				} else if ("VirtualInstrument".equals(node.name)) {
					jOTDBnode stationSetNode = getNode(node, "stationSet");
					fillNode(stationSetNode, lofarObservation.getStationSet());
					jOTDBnode stationsNode = getNode(node, "stationList");
					fillNode(stationsNode, lofarObservation.getStations());
				}
			}

			if (beams.size() > 0) {
				for (int i = 1; i < beams.size(); i++) {
					tm.deleteNode(beams.get(0));
				}

				jOTDBnode aDefaultNode = beams.get(0);
				beams.clear();
				for (int i = 0; i < lofarObservation.getBeams().size(); i++) {
					int nodeID = tm.dupNode(treeId, aDefaultNode.nodeID(), (short) i);
					Beam beam = lofarObservation.getBeams().get(i);
					jOTDBnode beamNode = tm.getNode(treeId, nodeID);

					Vector<jOTDBnode> beamChilds = getChilds(beamNode);
					// get all the params per child
					for (jOTDBnode beamChild : beamChilds) {
						if ("directionTypes".equals(beamChild.name)) {
							fillNode(beamChild, beam.getEquinox());
						} else if ("angle1".equals(beamChild.name)) {
							fillNode(beamChild, AstronConverter.toString(beam.getRa()));
						} else if ("angle2".equals(beamChild.name)) {
							fillNode(beamChild, AstronConverter.toString(beam.getDec()));
						} else if ("angleTimes".equals(beamChild.name)) {
							fillNode(beamChild, AstronConverter.toString(beam.getDuration()));
						} else if ("subbandList".equals(beamChild.name)) {
							fillNode(beamChild, beam.getSubbands());
						}
						saveNode(beamChild);
					}

				}

			}
			short statusId = converter.getTreeState(lofarObservation.getStatus());
			tm.setTreeState(treeId, statusId);
			tm.setMomInfo(treeId, lofarObservation.getMom2Id(), "no campaign");
		} catch (RemoteException re) {
			throw new RepositoryException(re);
		}
	}

	@SuppressWarnings("unchecked")
	private Vector<jOTDBnode> getChilds(jOTDBnode node) throws RemoteException {
		return tm.getItemList(node.treeID(), node.nodeID(), 1);

	}

	@SuppressWarnings("unchecked")
	private jOTDBnode getObservationNode(int treeId) throws RemoteException {
		jOTDBnode node = null;
		Vector<jOTDBnode> childs = tm.getItemList(treeId, "Observation");
		if (childs.size() > 0) {
			node = childs.get(0);
		}

		return node;
	}

	private void saveNode(jOTDBnode aNode) throws RemoteException {
		if (aNode == null) {
			return;
		}
		tm.saveNode(aNode);

	}

	private void fillNode(jOTDBnode node, String value) throws RemoteException {
		if (value != null && node != null) {
			node.limits = value;
			saveNode(node);
		}
	}

	/**
	 * Stores parameter in the tree by given parent node
	 * 
	 * @param parentNode
	 *            parent node
	 * @param paramName
	 *            parameter name
	 * @param paramValue
	 *            parameter value
	 * @throws RemoteException
	 */
	protected void storeParam(jOTDBnode parentNode, String paramName, String paramValue) throws RemoteException {
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
	 * @param parentNode
	 *            parent node
	 * @param paramName
	 *            name of the parameter
	 * @return node
	 * @throws RemoteException
	 */
	protected jOTDBnode getNode(jOTDBnode parentNode, String paramName) throws RemoteException {
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
	 * 
	 * @param startDate
	 *            start time
	 * @param endDate
	 *            end time
	 * @return List of lofarObservation objects
	 * @throws RemoteException
	 */
	public List getLatestChanges(Date startDate, Date endDate) throws RepositoryException {
		try {
			String startTime = AstronConverter.toDateString(startDate, DATE_TIME_FORMAT);
			String endTime = AstronConverter.toDateString(endDate, DATE_TIME_FORMAT);
			log.info("Retrieve latest changes between:" + startTime + " and " + endTime);
			remoteOTDB.connect();
			List result = new ArrayList();
			Vector stateList = remoteOTDB.getStateList(0, false, startTime, endTime);
			for (int i = 0; i < stateList.size(); i++) {
				jTreeState state = (jTreeState) stateList.get(i);

				String status = converter.getTreeState(state.newState);
				log.info("momId: " + state.momID + " status:" + status + " statusId:" + state.newState);
				if (state.momID > 0 && isStatusThatMustBeExported(status)) {
					LofarObservation observation = new LofarObservation();
					observation.setMom2Id(state.momID);
					observation.setStatus(status);
					//observation.setTimeStamp(state.timestamp);
					jOTDBnode observationNode = tm.getTopNode(state.treeID);
					jOTDBnode measurementsNode = getNode(observationNode, "measurementMom2Ids");
					// observation.setMeasurementMom2Ids(measurementsNode.limits);
					// if (status.equals("finished")){
					// jOTDBnode viNode = getNode(observationNode, "VI1");
					// jOTDBnode angle1 = getNode(viNode, "angle1");
					// jOTDBnode angle2 = getNode(viNode, "angle2");
					// jOTDBnode angleTimes = getNode(viNode, "angleTimes");
					// observation.setAngle1(angle1.limits);
					// observation.setAngle2(angle2.limits);
					// observation.setAngleTimes(angleTimes.limits);
					// jOTDBtree treeInfo =
					// remoteOTDB.getTreeInfo(state.treeID,false);
					// observation.setStartTime(treeInfo.starttime);
					// observation.setEndTime(treeInfo.stoptime);
					// }
					result.add(observation);
				}
			}

			return result;
		} catch (RemoteException re) {
			throw new RepositoryException(re);
		}
	}

	/**
	 * Checkes if observation with a status must be exported to MoM
	 * 
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
