package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Vector;

import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;
import nl.astron.lofar.sas.otb.jotdb2.jConverterInterface;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
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

	private static final String CHANNELS_PER_SUBBAND = "channelsPerSubband";

	private static final String FILE_NAME_MASK = "MSNameMask";

	private static final String INTEGRATION_STEPS = "integrationSteps";

	private static final String IONPROC = "IONProc";

	private static final String OLAP = "OLAP";

	private static final String ONLINE_CONTROL = "OnlineControl";

	private static final String OBSERVATION_CONTROL = "ObservationControl";

	private static final String STATION_LIST = "stationList";

	private static final String VIRTUAL_INSTRUMENT = "VirtualInstrument";

	private static final String BEAM_MOM_ID = "momID";

	private static final String SUBBAND_LIST = "subbandList";
	
	private static final String BEAMLET_LIST = "beamletList";
	
	private static final String DURATIONS = "durations";

	private static final String ANGLE_TIMES = "angleTimes";

	private static final String ANGLE2 = "angle2";

	private static final String ANGLE1 = "angle1";

	private static final String DIRECTION_TYPES = "directionTypes";

	private static final String SAMPLE_CLOCK = "systemClock";

	private static final String SUBBAND_WIDTH = "subbandWidth";

	private static final String SAMPLES_PER_SECOND = "samplesPerSecond";

	private static final String CHANNEL_WIDTH = "channelWidth";

	private static final String BEAM = "Beam";

	private static final String CLOCK_MODE = "clockMode";

	private static final String BAND_FILTER = "bandFilter";

	private static final String ANTENNA_SET = "antennaSet";

	private static final String ANTENNA_ARRAY = "antennaArray";
	
	private static final String TEMPLATE_TYPE = "VItemplate";
	
	private static final String VIC_TYPE = "VHtree";

	private Log log = LogFactory.getLog(this.getClass());

	private jOTDBinterface remoteOTDB = null;

	private jTreeMaintenanceInterface tm = null;

	private jConverterInterface converter = null;

	// private static final int TEMPLATE_ID = 50091980;

	public static final String DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss.SSS";

	public static final String OTDB_DATE_TIME_FORMAT = "yyyy-MMM-dd HH:mm:ss";

	private boolean connected = false;

	private OTDBConfiguration config;

	/**
	 * Constructor that makes a connection to the specified rmi server on the
	 * specified rmi port
	 * 
	 * @param rmiServerName
	 * @param port
	 * @throws NotBoundException
	 * @throws RemoteException
	 * @throws NotBoundException
	 */
	public OTDBRepository(OTDBConfiguration config) {
		this.config = config;

	}

	private void init() throws RemoteException, NotBoundException {
		if (!connected) {
			Registry remoteRegistry = LocateRegistry.getRegistry(config.getRmiHost(), config.getRmiPort());
			remoteOTDB = (jOTDBinterface) remoteRegistry.lookup(jOTDBinterface.SERVICENAME);
			tm = (jTreeMaintenanceInterface) remoteRegistry.lookup(jTreeMaintenanceInterface.SERVICENAME);
			converter = (jConverterInterface) remoteRegistry.lookup(jConverterInterface.SERVICENAME);
			connected = true;
		}
	}

	/**
	 * Stores a lofarObservation to jOTDB
	 * 
	 * @param lofarObservation
	 * @throws RemoteException
	 */
	public void store(LofarObservation lofarObservation) throws RepositoryException {
		try {
			init();
			Integer treeId = lofarObservation.getObservationId();
			if (treeId == null) {
				treeId = tm.copyTemplateTree(config.getTemplateId());
			}
			// int treeId = 6;
			jOTDBnode observationNode = getObservationNode(treeId);
			List<jOTDBnode> beams = new ArrayList<jOTDBnode>();
			Vector<jOTDBnode> childs = getChilds(observationNode);
			for (jOTDBnode node : childs) {
				if (ANTENNA_ARRAY.equals(node.name)) {
					fillNode(node, lofarObservation.getAntennaArray());
				} else if (ANTENNA_SET.equals(node.name)) {
					fillNode(node, lofarObservation.getAntennaSet());
				} else if (BAND_FILTER.equals(node.name)) {
					fillNode(node, lofarObservation.getBandFilter());
				} else if (CLOCK_MODE.equals(node.name)) {
					fillNode(node, lofarObservation.getClockMode());
				} else if (BEAM.equals(node.name)) {
					beams.add(node);
				} else if (VIRTUAL_INSTRUMENT.equals(node.name)) {
					jOTDBnode stationSetNode = getNode(node, "stationSet");
					fillNode(stationSetNode, lofarObservation.getStationSet());
					jOTDBnode stationsNode = getNode(node, STATION_LIST);
					fillNode(stationsNode, lofarObservation.getStations());
				} else if (OBSERVATION_CONTROL.equals(node.name)) {
					jOTDBnode onlineControlNode = getNode(node, ONLINE_CONTROL);
					if (onlineControlNode != null) {
						jOTDBnode olapNode = getNode(onlineControlNode, OLAP);
						if (olapNode != null) {
							jOTDBnode ionProcNode = getNode(olapNode, IONPROC);
							if (ionProcNode != null) {
								jOTDBnode integrationStepsNode = getNode(olapNode, INTEGRATION_STEPS);
								fillNode(integrationStepsNode, lofarObservation.getIntegrationInterval().toString());
							}
						}
					}

				}
			}

			if (beams.size() > 0) {
				for (int i = 1; i < beams.size(); i++) {
					tm.deleteNode(beams.get(i));
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
						if (DIRECTION_TYPES.equals(beamChild.name)) {
							fillNode(beamChild, beam.getEquinox());
						} else if (ANGLE1.equals(beamChild.name)) {
							fillNode(beamChild, beam.getRaList());
						} else if (ANGLE2.equals(beamChild.name)) {
							fillNode(beamChild, beam.getDecList());
						} else if (ANGLE_TIMES.equals(beamChild.name)) {
							fillNode(beamChild, beam.getAngleTimes());
						} else if (DURATIONS.equals(beamChild.name)) {
							fillNode(beamChild, beam.getDurations());
						} else if (SUBBAND_LIST.equals(beamChild.name)) {
							fillNode(beamChild, beam.getSubbands());
						}  else if (BEAMLET_LIST.equals(beamChild.name)) {
							fillNode(beamChild, beam.getBeamlets());
						} else if (BEAM_MOM_ID.equals(beamChild.name)) {
							fillNode(beamChild, beam.getMom2Id().toString());
						}
						saveNode(beamChild);
					}

				}
				 // store new number of instances in baseSetting
	            aDefaultNode.instances=(short)(lofarObservation.getBeams().size()); // - default at -1 
	            saveNode(aDefaultNode);
			}
           

			short statusId = converter.getTreeState(lofarObservation.getStatus());
			tm.setTreeState(treeId, statusId);
			tm.setMomInfo(treeId, lofarObservation.getMom2Id(), "no campaign");
		} catch (RemoteException re) {
			connected = false;
			throw new RepositoryException(re);
		} catch (NotBoundException e) {
			connected = false;
			throw new RepositoryException(e);
		}
	}

	@SuppressWarnings("unchecked")
	private jOTDBnode getClockModeNode(int treeId, String clockMode) throws RemoteException {
		jOTDBnode node = null;
		Vector<jOTDBnode> childs = tm.getItemList(treeId, "%" + clockMode.substring(2));
		if (childs.size() > 0) {
			node = childs.get(0);
		}
		return node;
	}

	@SuppressWarnings("unchecked")
	private Vector<jOTDBnode> getChilds(jOTDBnode node) throws RemoteException {
		return tm.getItemList(node.treeID(), node.nodeID(), 1);

	}

	@SuppressWarnings("unchecked")
	private jOTDBnode getObservationNode(int treeId) throws RemoteException {
		jOTDBnode node = null;
		Vector<jOTDBnode> childs = tm.getItemList(treeId, "%Observation");
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

	private void fillNode(jOTDBnode node, List<?> values) throws RemoteException {

		if (values != null && node != null) {
			String value = "[";
			for (int i = 0; i < values.size(); i++) {
				if (i > 0) {
					value += ",";
				}
				value += values.get(i);
			}
			value += "]";
			node.limits = value;
			saveNode(node);
		}
	}

	private void fillDoubleList(jOTDBnode node, List<Double> values) throws RemoteException {

		if (values != null && node != null) {
			String value = node.limits.replaceAll("[\\]\\[ ]", "");
			String[] array = value.split(",");
			for (String arrayItem : array) {
				Double doubleValue = AstronConverter.toDouble(arrayItem);
				if (doubleValue != null) {
					values.add(doubleValue);
				}
			}
		}
	}

	private void fillStringList(jOTDBnode node, List<String> values) throws RemoteException {

		if (values != null && node != null) {
			String value = node.limits.replaceAll("[\\]\\[ ]", "");
			String[] array = value.split(",");
			for (String arrayItem : array) {
				values.add(arrayItem);
			}
		}
	}

	private void fillIntegerList(jOTDBnode node, List<Integer> values) throws RemoteException {

		if (values != null && node != null) {
			String value = node.limits.replaceAll("[\\]\\[ ]", "");
			String[] array = value.split(",");
			for (String arrayItem : array) {
				Integer intValue = AstronConverter.toInteger(arrayItem);
				if (intValue != null) {
					values.add(intValue);
				}
			}
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
	@SuppressWarnings("unchecked")
	private jOTDBnode getNode(jOTDBnode parentNode, String paramName) throws RemoteException {
		jOTDBnode node = null;
		/*
		 * if no parent node, do not retrieve something
		 */
		if (parentNode != null) {
			Vector<jOTDBnode> vector = tm.getItemList(parentNode.treeID(), paramName);
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

	private String getValue(jOTDBnode parentNode, String paramName) throws RemoteException {
		jOTDBnode node = getNode(parentNode, paramName);
		if (node != null) {
			return node.limits;
		} else {
			return null;
		}
	}

	private String getValue(jOTDBnode node) throws RemoteException {
		if (node != null) {
			return node.limits;
		} else {
			return null;
		}
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
	@SuppressWarnings("unchecked")
	public List<LofarObservation> getLatestChanges(Date startDate, Date endDate) throws RepositoryException {
		try {
			init();
			String startTime = AstronConverter.toDateString(startDate, DATE_TIME_FORMAT);
			String endTime = AstronConverter.toDateString(endDate, DATE_TIME_FORMAT);
			log.info("Retrieve latest changes between:" + startTime + " and " + endTime);
			remoteOTDB.connect();
			List<LofarObservation> result = new ArrayList<LofarObservation>();
			Vector<jTreeState> stateList = remoteOTDB.getStateList(0, false, startTime, endTime);
			Collections.sort(stateList, new TreeStateComparator());
			short vicTree = converter.getTreeType(VIC_TYPE);
			short templateTree = converter.getTreeType(TEMPLATE_TYPE);
			for (jTreeState state: stateList) {
				jOTDBtree treeInfo = remoteOTDB.getTreeInfo(state.treeID, false);
				String status = converter.getTreeState(state.newState);
				if (state.momID > 0 && isStatusThatMustBeExported(status) && (treeInfo.type == vicTree || treeInfo.type == templateTree) ) {
					log.info("treeId: " + state.treeID + " momId: " + state.momID + " status:" + status +  " date: " + state.timestamp);
					LofarObservation lofarObservation = new LofarObservation();
					lofarObservation.setMom2Id(state.momID);
					lofarObservation.setObservationId(state.treeID);
					lofarObservation.setStatus(status);
					// it must be a template
					if (treeInfo.type == templateTree) {
						
						jOTDBnode observationNode = getObservationNode(state.treeID);
						fillLofarObservation(observationNode, lofarObservation);

					} else if (treeInfo.type == vicTree) {
						
						jOTDBnode observationNode = getObservationNode(state.treeID);				
                        fillLofarObservation( observationNode, lofarObservation);
						lofarObservation
								.setStartTime(AstronConverter.toDate(treeInfo.starttime, OTDB_DATE_TIME_FORMAT));
						lofarObservation.setEndTime(AstronConverter.toDate(treeInfo.stoptime, OTDB_DATE_TIME_FORMAT));
					}
					result.add(lofarObservation);
				}
			}

			return result;
		} catch (RemoteException re) {
			connected = false;
			throw new RepositoryException(re);
		} catch (NotBoundException e) {
			connected = false;
			throw new RepositoryException(e);
		}
	}
	private void fillLofarObservation(jOTDBnode observationNode, LofarObservation lofarObservation) throws RemoteException{
		List<jOTDBnode> beams = new ArrayList<jOTDBnode>();
		Vector<jOTDBnode> childs = getChilds(observationNode);
		for (jOTDBnode node : childs) {
			String nodeName = getNodeNameWithoutPrefix(node.name);
			if (ANTENNA_ARRAY.equals(nodeName)) {
				lofarObservation.setAntennaArray(getValue(node));
			} else if (ANTENNA_SET.equals(nodeName)) {
				lofarObservation.setAntennaSet(getValue(node));
			} else if (BAND_FILTER.equals(nodeName)) {
				lofarObservation.setBandFilter(getValue(node));
			} else if (CLOCK_MODE.equals(nodeName)) {
				String clockMode = getValue(node);
				jOTDBnode clockNode = getClockModeNode(lofarObservation.getObservationId(), clockMode);
				Vector<jOTDBnode> clockNodeChilds = getChilds(clockNode);
				for (jOTDBnode clockNodeChild : clockNodeChilds) {
					String clockNodeName = getNodeNameWithoutPrefix(clockNodeChild.name);
					if (CHANNEL_WIDTH.equals(clockNodeName)) {
						lofarObservation.setChannelWidth(AstronConverter
								.toDouble(getValue(clockNodeChild)));
					} else if (SAMPLES_PER_SECOND.equals(clockNodeName)) {
						lofarObservation.setSamplesPerSecond(AstronConverter
								.toInteger(getValue(clockNodeChild)));
					} else if (SUBBAND_WIDTH.equals(clockNodeName)) {
						lofarObservation.setSubbandWidth(AstronConverter
								.toDouble(getValue(clockNodeChild)));
					} else if (SAMPLE_CLOCK.equals(clockNodeName)) {
						lofarObservation.setClockFrequency(AstronConverter
								.toDouble(getValue(clockNodeChild)));
					}
				}
			} else if (CHANNELS_PER_SUBBAND.equals(nodeName)) {
				lofarObservation.setChannelsPerSubband(AstronConverter.toInteger(getValue(node)));
			} else if (FILE_NAME_MASK.equals(nodeName)) {
				lofarObservation.setFileNameMask((getValue(node)));
			} else if (nodeName.startsWith(BEAM + "[")) {
				beams.add(node);
			} else if (VIRTUAL_INSTRUMENT.equals(nodeName)) {
				jOTDBnode stationsNode = getNode(node, STATION_LIST);
				fillStringList(stationsNode, lofarObservation.getStations());
			} else if (OBSERVATION_CONTROL.equals(nodeName)) {
				jOTDBnode onlineControlNode = getNode(node, ONLINE_CONTROL);
				if (onlineControlNode != null) {
					jOTDBnode olapNode = getNode(onlineControlNode, OLAP);
					if (olapNode != null) {
						jOTDBnode ionProcNode = getNode(olapNode, IONPROC);
						if (ionProcNode != null) {
							lofarObservation.setIntegrationInterval(AstronConverter.toDouble(getValue(
									olapNode, INTEGRATION_STEPS)));
						}
					}
				}

			}
		}

		if (beams.size() > 0) {
			for (jOTDBnode beamNode : beams) {
				Beam beam = new Beam(lofarObservation);
				Vector<jOTDBnode> beamChilds = getChilds(beamNode);

				// get all the params per child
				for (jOTDBnode beamChild : beamChilds) {
					String nodeName = getNodeNameWithoutPrefix(beamChild.name);
					if (DIRECTION_TYPES.equals(nodeName)) {
						beam.setEquinox(getValue(beamChild));
					} else if (ANGLE1.equals(nodeName)) {
						fillDoubleList(beamChild, beam.getRaList());
					} else if (ANGLE2.equals(nodeName)) {
						fillDoubleList(beamChild, beam.getDecList());
					} else if (ANGLE_TIMES.equals(nodeName)) {
						fillIntegerList(beamChild, beam.getAngleTimes());
					} else if (DURATIONS.equals(nodeName)) {
						fillIntegerList(beamChild, beam.getDurations());
					} else if (SUBBAND_LIST.equals(nodeName)) {
						fillIntegerList(beamChild, beam.getSubbands());
					}else if (BEAMLET_LIST.equals(nodeName)) {
						fillIntegerList(beamChild, beam.getBeamlets());
					}else if (BEAM_MOM_ID.equals(nodeName)) {
						beam.setMom2Id(AstronConverter.toInteger(getValue(beamChild)));
					}

				}
				lofarObservation.getBeams().add(beam);
			}

		}
	}

	private String getNodeNameWithoutPrefix(String nodeName){
		return nodeName.substring(nodeName.lastIndexOf('.')+1);	

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
	public static class TreeStateComparator implements Comparator<jTreeState> {

		@Override
		public int compare(jTreeState treeStateOne, jTreeState treeStateTwo) {
			Date dateOne = AstronConverter.toDate(treeStateOne.timestamp, OTDB_DATE_TIME_FORMAT);
			Date dateTwo= AstronConverter.toDate(treeStateTwo.timestamp, OTDB_DATE_TIME_FORMAT);
			return dateOne.compareTo(dateTwo);
		}

	}
}
