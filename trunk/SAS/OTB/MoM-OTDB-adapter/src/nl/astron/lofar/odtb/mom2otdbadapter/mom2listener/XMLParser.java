package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.wsrt.util.WsrtConverter;
import nl.astron.wsrt.util.WsrtValidator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;

public class XMLParser {
	private Log log = LogFactory.getLog(this.getClass());

	private static final String PREFIX = "lofar";

	private static final String MOM2_ID = "mom2Id";

	private static final String OBSERVATION = "observation";

	private static final String CURRENT_STATUS = "currentStatus";

	private static final String DESCRIBED_STATUS = "describedStatus";

	private static final String SPECIFIED_STATUS = "specifiedStatus";

	private static final String OBSERVATION_ATTRIBUTES = "observationAttributes";

	private static final String ARRAY_CONFIGURATION = "arrayConfiguration";

	private static final String DEFAULT_ARRAY_CONFIGURATION = "default";

	private static final String DETAILED_ARRAY_CONFIGURATION = "detailed";

	private static final String STATION = "station";

	private static final String SRG_CONFIGURATION = "srgConfiguration";

	private static final String BAND_FILTER = "bandFilter";

	private static final String SUBBAND_PLACEMENT = "subbandPlacement";

	private static final String NUMBER_OF_BANDS = "numberOfBands";

	private static final String START_FREQUENCY = "startFrequency";

	private static final String SPACING = "spacing";

	private static final String BACKEND = "backend";

	private static final String CHILDREN = "children";

	private static final String ITEM = "item";

	private static final String MEASUREMENT = "measurement";

	private static final String MEASUREMENT_ATTRIBUTES = "measurementAttributes";

	private static final String RA = "ra";

	private static final String DEC = "dec";

	private static final String DIRECTION_TYPE = "coordinateRef";

	private static final String REQUESTED_DURATION = "requestedDuration";

	public LofarObservation getLofarObservation(Document document) {
		LofarObservation lofarObservation = new LofarObservation();

		/*
		 * search for project
		 */
		Node element = document.getDocumentElement();
		if (equal(element, withPrefix(OBSERVATION))) {
			lofarObservation.setMom2Id(getAttribute(element.getAttributes(),
					MOM2_ID));
			for (int i = 0; i < element.getChildNodes().getLength(); i++) {
				Node nodeChild = element.getChildNodes().item(i);
				/*
				 * if child is an element
				 */
				if (WsrtValidator.implementsInterface(Element.class, nodeChild
						.getClass())) {

					if (equal(nodeChild, CURRENT_STATUS)) {
						parseCurrentStatus(nodeChild, lofarObservation);
					} else if (equal(nodeChild,
							withPrefix(OBSERVATION_ATTRIBUTES))) {
						parseObservationAttributes(nodeChild, lofarObservation);
					} else if (equal(nodeChild, CHILDREN)) {
						parseChildren(nodeChild, lofarObservation);
					}
				}
			}
			if (lofarObservation.getAngle1() != null) {
				lofarObservation.setAngle1(lofarObservation.getAngle1() + "]");
			}
			if (lofarObservation.getAngle2() != null) {
				lofarObservation.setAngle2(lofarObservation.getAngle2() + "]");
			}
			if (lofarObservation.getAngleTimes() != null) {
				lofarObservation.setAngleTimes(lofarObservation.getAngleTimes()
						+ "]");
			}
			if (lofarObservation.getMeasurementMom2Ids() != null) {
				lofarObservation.setMeasurementMom2Ids(lofarObservation.getMeasurementMom2Ids()
						+ "]");
			}
		}
		return lofarObservation;
	}

	protected void parseObservationAttributes(Node node,
			LofarObservation lofarObservation) {
		String filter = null;
		String subbandPlacement = null;
		Integer numberOfBands = null;
		Integer startFrequency = null;
		Integer spacing = null;

		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equal(nodeChild, ARRAY_CONFIGURATION)) {
					parseArrayConfiguration(nodeChild,lofarObservation);
				} else if (equal(nodeChild, SRG_CONFIGURATION)) {
					lofarObservation.setSrgConfiguration(getValue(nodeChild));
				} else if (equal(nodeChild, BAND_FILTER)) {
					filter = getValue(nodeChild);
				} else if (equal(nodeChild, SUBBAND_PLACEMENT)) {
					subbandPlacement = getValue(nodeChild);
				} else if (equal(nodeChild, NUMBER_OF_BANDS)) {
					numberOfBands = WsrtConverter
							.toInteger(getValue(nodeChild));
				} else if (equal(nodeChild, START_FREQUENCY)) {
					startFrequency = Mom2OtdbConverter
							.getOTDBFrequency(getValue(nodeChild));
				} else if (equal(nodeChild, SPACING)) {
					spacing = WsrtConverter.toInteger(getValue(nodeChild));
				} else if (equal(nodeChild, BACKEND)) {
					lofarObservation.setBackend(getValue(nodeChild));
				}
			}
		}
		lofarObservation.setBandSelection(Mom2OtdbConverter
				.getOTDBBandSelection(filter));
		lofarObservation.setSamplingFrequency(Mom2OtdbConverter
				.getOTDBSamplingFrequency(filter));
		lofarObservation.setSubbands(Mom2OtdbConverter.getOTDBSubbands(
				lofarObservation.getSamplingFrequency(), numberOfBands,
				subbandPlacement, startFrequency, spacing));
	}

	protected void parseArrayConfiguration(Node node,
			LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equal(nodeChild, DEFAULT_ARRAY_CONFIGURATION)) {
					lofarObservation
							.setArrayConfiguration(DEFAULT_ARRAY_CONFIGURATION);
					lofarObservation.setStations(getValue(nodeChild));
				} else if (equal(nodeChild, DETAILED_ARRAY_CONFIGURATION)) {
					lofarObservation
							.setArrayConfiguration(DETAILED_ARRAY_CONFIGURATION);
					parseStations(nodeChild, lofarObservation);
				}
			}
		}
	}

	protected void parseStations(Node node, LofarObservation lofarObservation) {
		String stations = null;
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equal(nodeChild, STATION)) {
					if (stations == null) {
						stations = "[" + getValue(nodeChild);
					} else {
						stations = stations + "," + getValue(nodeChild);
					}
				}
			}
		}
		if (stations != null) {
			stations += "]";
			lofarObservation.setStations(stations);
		}

	}

	protected void parseCurrentStatus(Node node,
			LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equalIgnorePrefix(nodeChild, DESCRIBED_STATUS)) {
					lofarObservation.setStatus(Mom2OtdbConverter.getOTDBStatus(formatStatus(DESCRIBED_STATUS)));
				} else if (equalIgnorePrefix(nodeChild, SPECIFIED_STATUS)) {
					lofarObservation.setStatus(Mom2OtdbConverter.getOTDBStatus(formatStatus(SPECIFIED_STATUS)));
				}
			}
		}
	}

	protected void parseChildren(Node node, LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equalIgnorePrefix(nodeChild, ITEM)) {
					parseItem(nodeChild, lofarObservation);
				}
			}
		}
	}

	protected void parseItem(Node node, LofarObservation lofarObservation) {

		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equalIgnorePrefix(nodeChild, MEASUREMENT)) {

					parseMeasurement(nodeChild, lofarObservation);
				}
			}
		}
	}

	protected void parseMeasurement(Node node, LofarObservation lofarObservation) {
		String measurementMom2Ids = lofarObservation.getMeasurementMom2Ids();
		String mom2Id = getAttribute(node.getAttributes(),
				MOM2_ID);
		if (measurementMom2Ids == null) {
			measurementMom2Ids = "[" + mom2Id;
		} else {
			measurementMom2Ids = measurementMom2Ids + "," + mom2Id;
		}
		lofarObservation.setMeasurementMom2Ids(measurementMom2Ids);
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equalIgnorePrefix(nodeChild,
						MEASUREMENT_ATTRIBUTES)) {
					parseMeasurementAttributes(nodeChild, lofarObservation);
				}
			}
		}
	}

	protected void parseMeasurementAttributes(Node node,
			LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (WsrtValidator.implementsInterface(Element.class, nodeChild
					.getClass())) {

				if (equalIgnorePrefix(nodeChild, RA)) {
					if (lofarObservation.getAngle1() == null) {
						lofarObservation.setAngle1("[" + getValue(nodeChild));
					} else {
						lofarObservation.setAngle1(lofarObservation.getAngle1() + "," + getValue(nodeChild));
					}
				} else if (equalIgnorePrefix(nodeChild, DEC)) {
					if (lofarObservation.getAngle2() == null) {
						lofarObservation.setAngle2("[" + getValue(nodeChild));
					} else {
						lofarObservation.setAngle2(lofarObservation.getAngle2()+ "," + getValue(nodeChild));
					}
				} else if (equalIgnorePrefix(nodeChild, DIRECTION_TYPE)) {
					if (lofarObservation.getDirectionType() == null) {
						lofarObservation.setDirectionType(getValue(nodeChild));
					}
				} else if (equalIgnorePrefix(nodeChild, REQUESTED_DURATION)) {

					if (lofarObservation.getAngleTimes() == null) {
						lofarObservation.setAngleTimes("[+"
								+ lofarObservation.getRequestedDuration());
					} else {
						lofarObservation.setAngleTimes(lofarObservation.getAngleTimes() + ",+"
								+ lofarObservation.getRequestedDuration());
					}
					Integer seconds = getSeconds(getValue(nodeChild));
					lofarObservation.setRequestedDuration(lofarObservation
							.getRequestedDuration()
							+ seconds.intValue());
				}
			}
		}
	}

	protected String getAttribute(NamedNodeMap map, String name) {
		Node node = map.getNamedItem(name);
		return node.getNodeValue();
	}

	protected String withPrefix(String string) {

		return PREFIX + ":" + string;
	}

	/**
	 * The getValue method returns the value of an node
	 * 
	 * @param node
	 * @return
	 */
	protected String getValue(Node node) {
		String value = null;
		if (node.getFirstChild() != null) {
			value = node.getFirstChild().getNodeValue();
			if (log.isDebugEnabled()) {
				log.debug("Node: " + node.getNodeName() + " value: " + value);
			}
		}
		return value;
	}

	protected Integer getSeconds(String string) {
		String[] splitted = string.split("T");
		splitted = splitted[splitted.length - 1].split("H");
		splitted = splitted[splitted.length - 1].split("M");
		String seconds = splitted[splitted.length - 1];
		if (seconds.endsWith("S")) {
			seconds = seconds.substring(0, seconds.length() - 1);
			int sec = WsrtConverter.toDouble(seconds).intValue();
			return new Integer(sec);
		}
		return null;
	}

	/**
	 * The equal method compares if an node has the given name
	 * 
	 * @param node
	 * @param nodeName
	 * @return
	 */
	protected boolean equal(Node node, String nodeName) {
		return node.getNodeName().equals(nodeName);
	}

	protected boolean equalIgnorePrefix(Node node, String nodeName) {
		String[] nodeSplit = node.getNodeName().split(":");
		String withoutPrefix = nodeSplit[nodeSplit.length - 1];
		return withoutPrefix.equals(nodeName);
	}

	protected String formatStatus(String status) {
		return status.replaceAll("Status", "");
	}
}
