package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.util.HashMap;
import java.util.Map;

import nl.astron.lofar.odtb.mom2otdbadapter.data.Beam;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.util.AstronConverter;
import nl.astron.util.AstronValidator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * parse the XML input from MoM and returns a LofarObservations.
 * If needed it used the Mom2OtdbConverter to convert values to otdb values
 * 
 * @author Bastiaan Verhoef
 *
 */
public class XMLParser {
	private static final String STATIONS = "stations";

	private static final String STATION_SET = "stationSet";

	private static final String INSTRUMENT_FILTER = "instrumentFilter";

	private static final String CLOCK = "clock";

	private static final String ANTENNA = "antenna";

	private static final String SPECIFICATION = "specification";

	private static Log log = LogFactory.getLog(XMLParser.class);

	private static final String SCHEMA_NAMESPACE = "http://www.w3.org/2001/XMLSchema-instance";
	
	private static final String MOM2_ID = "mom2Id";

	private static final String OBSERVATION = "observation";

	private static final String CURRENT_STATUS = "currentStatus";

	private static final String OBSERVATION_ATTRIBUTES = "observationAttributes";

	private static final String CHILDREN = "children";

	private static final String ITEM = "item";

	private static final String MEASUREMENT = "measurement";

	private static final String MEASUREMENT_ATTRIBUTES = "measurementAttributes";

	private static final String RA = "ra";

	private static final String DEC = "dec";

	private static final String EQUINOX = "equinox";
	
	private static final String DURATION = "duration";
	
	private static final String SUBBANDS_SPECIFICATION = "subbandsSpecification";
	
	private static final String SUBBANDS = "subbands";


	/**
	 * Parse a xml document and returns a lofar obseravation
	 * @param document xml document
	 * @return LofarObservation
	 */
	public static LofarObservation getLofarObservation(Document document) {
		LofarObservation lofarObservation = new LofarObservation();

		/*
		 * search for project
		 */
		Node element = document.getDocumentElement();
		if (equalIgnorePrefix(element, OBSERVATION)) {
			String mom2Id = getAttribute(element.getAttributes(),
					MOM2_ID);
			lofarObservation.setMom2Id(AstronConverter.toInt(AstronConverter.toInteger(mom2Id)));
			for (int i = 0; i < element.getChildNodes().getLength(); i++) {
				Node nodeChild = element.getChildNodes().item(i);
				/*
				 * if child is an element
				 */
				if (isElement(nodeChild)) {

					if (equal(nodeChild, CURRENT_STATUS)) {
						parseCurrentStatus(nodeChild, lofarObservation);
					} else if (equalIgnorePrefix(nodeChild,
							OBSERVATION_ATTRIBUTES)) {
						parseObservationAttributes((Element) nodeChild, lofarObservation);
					} else if (equal(nodeChild, CHILDREN)) {
						parseChildren(nodeChild, lofarObservation);
					}
				}
			}

		}
		return lofarObservation;
	}

	/**
	 * Parse the observationAttributes xml element.
	 * @param node xml node that must be parsed
	 * @param lofarObservation LofarObservation that must be filled
	 */
	private static void parseObservationAttributes(Element element,
			LofarObservation lofarObservation) {
		NodeList specificationList = element.getElementsByTagName(SPECIFICATION);
		if (specificationList.getLength() > 0){
			Element specification = (Element) specificationList.item(0);
			Map<String, Element> elements = getElementMap(specification.getChildNodes());
			if (elements.containsKey(ANTENNA)){
				String antenna = getValue(elements.get(ANTENNA));
				lofarObservation.setAntennaArray(Mom2OtdbConverter.getOTDBAntennaArray(antenna));
				lofarObservation.setAntennaSet(Mom2OtdbConverter.getOTDBAntennaSet(antenna));
			}
			if (elements.containsKey(CLOCK)){
				lofarObservation.setClockMode(Mom2OtdbConverter.getOTDBClockMode(getValue(elements.get(CLOCK))));
			}
			if (elements.containsKey(INSTRUMENT_FILTER)){
				lofarObservation.setBandFilter(Mom2OtdbConverter.getOTDBBandFilter(getValue(elements.get(INSTRUMENT_FILTER)), lofarObservation.getAntennaArray()));
			}
			if (elements.containsKey(STATION_SET)){
				lofarObservation.setStationSet(getValue(elements.get(STATION_SET)));
			}
			if (elements.containsKey(STATIONS)){
				lofarObservation.setStations(getValue(elements.get(STATIONS)));
				if (lofarObservation.getStations() != null){
					lofarObservation.setStations("[" + lofarObservation.getStations() + "]");
				}
			}
			
		}

//		lofarObservation.setBandFilter("HBA_100_190");
//		lofarObservation.setClockMode("<<Clock200");
//		lofarObservation.setStationSet("Custom");
//		lofarObservation.setStations("[CS302, CS010]");
//        <antenna>HBA Both</antenna>
//        <clock>160 MHz</clock>
//        <instrument>Interferometer</instrument>
//        <instrumentFilter>10-80 MHz</instrumentFilter>
//        <integrationInterval>4</integrationInterval>
//        <stationSet>Custom</stationSet>
//        <stations>CS302,CS303</stations>
	}


	/**
	 * Parse the currentStatus xml element.
	 * @param node xml node that must be parsed
	 * @param lofarObservation LofarObservation that must be filled
	 */
	private static void parseCurrentStatus(Node node,
			LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {
				String nodeName = removePrefix(nodeChild);
				if (nodeName.endsWith("Status")){
					lofarObservation.setStatus(Mom2OtdbConverter.getOTDBStatus(formatStatus(nodeName)));
				}
			}
		}
	}
	/**
	 * Parse the children xml element.
	 * @param node xml node that must be parsed
	 * @param lofarObservation LofarObservation that must be filled
	 */
	private static void parseChildren(Node node, LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {

				if (equalIgnorePrefix(nodeChild, ITEM)) {
					parseItem(nodeChild, lofarObservation);
				}
			}
		}
	}
	/**
	 * Parse the item xml element.
	 * @param node xml node that must be parsed
	 * @param lofarObservation LofarObservation that must be filled
	 */
	private static void parseItem(Node node, LofarObservation lofarObservation) {

		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {
				if (equalIgnorePrefix(nodeChild, MEASUREMENT)) {
					String measurementType = getAttribute(nodeChild.getAttributes(),SCHEMA_NAMESPACE,
							"type");
					if ("UVMeasurementType".equals(removePrefix(measurementType))){
						lofarObservation.getBeams().add(parseUVMeasurement(nodeChild));
					}
				}
			}
		}
	}
	/**
	 * Parse the measurement xml element.
	 * @param node xml node that must be parsed
	 * @param lofarObservation LofarObservation that must be filled
	 */
	private static Beam parseUVMeasurement(Node node) {
		Beam beam = new Beam();
		String mom2Id = getAttribute(node.getAttributes(),
				MOM2_ID);
		beam.setMom2Id(AstronConverter.toInteger(mom2Id));
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {

				if (equalIgnorePrefix(nodeChild,
						MEASUREMENT_ATTRIBUTES)) {
					parseMeasurementAttributes((Element) nodeChild, beam);
				}
			}
		}
		return beam;
		
	}
	/**
	 * Parse the measurementAttributes xml element.
	 * @param node xml node that must be parsed
	 * @param lofarObservation LofarObservation that must be filled
	 */
	private static void parseMeasurementAttributes(Element element,
			Beam beam) {
		NodeList specificationList = element.getElementsByTagName(SPECIFICATION);
		if (specificationList.getLength() > 0){
			Element specification = (Element) specificationList.item(0);
			Map<String, Element> elements = getElementMap(specification.getChildNodes());
			if (elements.containsKey(RA)){
				beam.setRa(getValue(elements.get(RA)));
			}
			if (elements.containsKey(DEC)){
				beam.setDec(getValue(elements.get(DEC)));
			}
			if (elements.containsKey(EQUINOX)){
				beam.setEquinox(getValue(elements.get(EQUINOX)));
			}
			if (elements.containsKey(DURATION)){
				beam.setRequestedDuration(AstronConverter.toString(AstronConverter.convertXMLDurationToSeconds(getValue(elements.get(DURATION)))));
			}
			if (elements.containsKey(SUBBANDS_SPECIFICATION)){
				Element subbandsSpecificationsElement = elements.get(SUBBANDS_SPECIFICATION);
				Map<String, Element> subbandsSpecificationsElements = getElementMap(subbandsSpecificationsElement.getChildNodes());
				if (subbandsSpecificationsElements.containsKey(SUBBANDS)){
					beam.setSubbands("[" + getValue(subbandsSpecificationsElements.get(SUBBANDS)) + "]");
				}
			}
			
		}

	}

	/**
	 * Retrieve an attribute value from a attribute map
	 * @param map attribute map
	 * @param name name of the attribute
	 * @return attribute value
	 */
	private static String getAttribute(NamedNodeMap map, String name) {
		Node node = map.getNamedItem(name);
		return node.getNodeValue();
	}
	private static String getAttribute(NamedNodeMap map, String namespace, String name) {
		Node node = map.getNamedItemNS(namespace, name);
		return node.getNodeValue();
	}


	/**
	 * The getValue method returns the value of an node
	 * 
	 * @param node
	 * @return value of the node
	 */
	private static String getValue(Node node) {
		String value = null;
		if (node.getFirstChild() != null) {
			value = node.getFirstChild().getNodeValue();
			if (log.isDebugEnabled()) {
				log.debug("Node: " + node.getNodeName() + " value: " + value);
			}
		}
		return value;
	}



	/**
	 * The equal method compares if an node has the given name
	 * 
	 * @param node
	 * @param nodeName
	 * @return true if equals
	 */
	private static boolean equal(Node node, String nodeName) {
		return node.getNodeName().equals(nodeName);
	}

	/**
	 * Compares if a node has the given name, ignoring the prefix of the node
	 * @param node
	 * @param nodeName
	 * @return true, if equals
	 */
	private static boolean equalIgnorePrefix(Node node, String nodeName) {
		String withoutPrefix = removePrefix(node);
		return withoutPrefix.equals(nodeName);
	}
	/**
	 * Returns the node name without prefix
	 * @param node
	 * @return node name withoud prefix
	 */
	private static String removePrefix(Node node){
		return removePrefix(node.getNodeName());
	}
	private static String removePrefix(String nodeName){
		String[] nodeSplit = nodeName.split(":");
		String withoutPrefix = nodeSplit[nodeSplit.length - 1];
		return withoutPrefix;
	}
	/**
	 * Retrieve status from status element
	 * @param status status element string
	 * @return status
	 */
	private static String formatStatus(String status) {
		return status.replaceAll("Status", "");
	}
	
	private static Map<String, Element> getElementMap(NodeList nodeList) {
		Map<String, Element> elementMap = new HashMap<String, Element>();
		for (int i = 0; i < nodeList.getLength(); i++) {
			Node childNode = nodeList.item(i);
			if (isElement(childNode)) {
				String key = removePrefix(childNode);
				elementMap.put(key, (Element) childNode);
			}
		}
		return elementMap;

	}
	private static boolean isElement(Node node) {
		if (AstronValidator.implementsInterface(Element.class, node.getClass())) {
			if (log.isDebugEnabled()) {
				log.debug("<" + node.getNodeName() + ">");
			}
			return true;
		} else {
			return false;
		}
	}
}
