package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.util.HashMap;
import java.util.Map;

import nl.astron.lofar.odtb.mom2otdbadapter.data.Beam;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.util.Mom2OtdbConverter;
import nl.astron.lofar.odtb.mom2otdbadapter.util.XMLConstants;
import nl.astron.util.AstronConverter;
import nl.astron.util.AstronValidator;
import nl.astron.util.Frequency;
import nl.astron.util.XMLBuilder;
import nl.astron.util.Frequency.Unit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * parse the XML input from MoM and returns a LofarObservations. If needed it
 * used the Mom2OtdbConverter to convert values to otdb values
 * 
 * @author Bastiaan Verhoef
 * 
 */
public class XMLParser {

	private static Log log = LogFactory.getLog(XMLParser.class);

	/**
	 * Parse a xml document and returns a lofar obseravation
	 * 
	 * @param document
	 *            xml document
	 * @return LofarObservation
	 */
	public static LofarObservation getLofarObservation(Document document) {
		LofarObservation lofarObservation = new LofarObservation();

		/*
		 * search for project
		 */
		Node element = document.getDocumentElement();
		if (equalIgnorePrefix(element, XMLConstants.OBSERVATION)) {
			String mom2Id = getAttribute(element.getAttributes(), XMLConstants.MOM2_ID);
			lofarObservation.setMom2Id(AstronConverter.toInt(AstronConverter.toInteger(mom2Id)));
			Map<String, Element> elementsMap = getElementMap(element.getChildNodes());

			if (elementsMap.containsKey(XMLConstants.CURRENT_STATUS)) {
				parseCurrentStatus(elementsMap.get(XMLConstants.CURRENT_STATUS), lofarObservation);
			}
			if (lofarObservation.getStatus().equals(Mom2OtdbConverter.OTDB_BEING_SPECIFIED_STATUS)) {
				if (elementsMap.containsKey(XMLConstants.OBSERVATION_ATTRIBUTES)) {
					parseObservationAttributes(elementsMap.get(XMLConstants.OBSERVATION_ATTRIBUTES), lofarObservation);
				}
				if (elementsMap.containsKey(XMLConstants.CHILDREN)) {
					parseChildren(elementsMap.get(XMLConstants.CHILDREN), lofarObservation);
				}
			}

		}
		int offSet = 0;
		for (Beam beam: lofarObservation.getBeams()){
			beam.getAngleTimes().add(offSet);
			offSet += beam.getDurations().get(0);
		}
		return lofarObservation;
	}

	/**
	 * Parse the observationAttributes xml element.
	 * 
	 * @param node
	 *            xml node that must be parsed
	 * @param lofarObservation
	 *            LofarObservation that must be filled
	 */
	private static void parseObservationAttributes(Element element, LofarObservation lofarObservation) {
		NodeList specificationList = element.getElementsByTagName(XMLConstants.SPECIFICATION);
		if (specificationList.getLength() > 0) {
			Element specification = (Element) specificationList.item(0);
			Map<String, Element> elements = getElementMap(specification.getChildNodes());
			if (elements.containsKey(XMLConstants.ANTENNA)) {
				String antenna = getValue(elements.get(XMLConstants.ANTENNA));
				lofarObservation.setAntennaArray(Mom2OtdbConverter.getOTDBAntennaArray(antenna));
				lofarObservation.setAntennaSet(Mom2OtdbConverter.getOTDBAntennaSet(antenna));
			}
			if (elements.containsKey(XMLConstants.CLOCK)) {
				lofarObservation.setClockMode(Mom2OtdbConverter.getOTDBClockMode(getMHzFrequency(elements.get(XMLConstants.CLOCK))));
			}
			if (elements.containsKey(XMLConstants.INTEGRATION_INTERVAL)) {
				lofarObservation.setIntegrationInterval(AstronConverter.toDouble(getValue(elements
						.get(XMLConstants.INTEGRATION_INTERVAL))));
			}
			if (elements.containsKey(XMLConstants.INSTRUMENT_FILTER)) {
				lofarObservation.setBandFilter(Mom2OtdbConverter.getOTDBBandFilter(getValue(elements
						.get(XMLConstants.INSTRUMENT_FILTER)), lofarObservation.getAntennaArray()));
			}
			if (elements.containsKey(XMLConstants.STATION_SET)) {
				lofarObservation.setStationSet(getValue(elements.get(XMLConstants.STATION_SET)));
			}
			if (elements.containsKey(XMLConstants.STATIONS)) {
				String[] stations = getValue(elements.get(XMLConstants.STATIONS)).split(",");
				for(String station: stations){
					lofarObservation.getStations().add(station);
				}
			}

		}

	}
	
	private static Double getMHzFrequency(Element frequencyElement){
		Frequency frequency = new Frequency();
		Double result = null;
		if (frequencyElement != null){
			frequency.setFrequency(AstronConverter.toDouble(getValue(frequencyElement)));
			frequency.setUnit(getAttribute(frequencyElement.getAttributes() , XMLBuilder.UNITS));
			if (!frequency.getUnit().equals(Unit.MHZ)){
				result = AstronConverter.getFrequencyFromFrequencyWithUnit(frequency);
			}else {
				result = frequency.getFrequency();
			}
		}
		return result;
	}

	/**
	 * Parse the currentStatus xml element.
	 * 
	 * @param node
	 *            xml node that must be parsed
	 * @param lofarObservation
	 *            LofarObservation that must be filled
	 */
	private static void parseCurrentStatus(Node node, LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {
				String nodeName = removePrefix(nodeChild);
				if (nodeName.endsWith(XMLConstants.STATUS_SUFFIX)) {
					lofarObservation.setStatus(Mom2OtdbConverter.getOTDBStatus(formatStatus(nodeName)));
				}
			}
		}
	}

	/**
	 * Parse the children xml element.
	 * 
	 * @param node
	 *            xml node that must be parsed
	 * @param lofarObservation
	 *            LofarObservation that must be filled
	 */
	private static void parseChildren(Node node, LofarObservation lofarObservation) {
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {

				if (equalIgnorePrefix(nodeChild, XMLConstants.ITEM)) {
					parseItem(nodeChild, lofarObservation);
				}
			}
		}
	}

	/**
	 * Parse the item xml element.
	 * 
	 * @param node
	 *            xml node that must be parsed
	 * @param lofarObservation
	 *            LofarObservation that must be filled
	 */
	private static void parseItem(Node node, LofarObservation lofarObservation) {

		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {
				if (equalIgnorePrefix(nodeChild, XMLConstants.MEASUREMENT)) {
					String measurementType = getAttribute(nodeChild.getAttributes(), XMLConstants.SCHEMA_NAMESPACE,
							XMLConstants.XSI_TYPE);
					if (XMLConstants.UVMEASUREMENT_TYPE.equals(removePrefix(measurementType))) {
						lofarObservation.getBeams().add(parseUVMeasurement(nodeChild, lofarObservation));
					}
				}
			}
		}
	}

	/**
	 * Parse the measurement xml element.
	 * 
	 * @param node
	 *            xml node that must be parsed
	 * @param lofarObservation
	 *            LofarObservation that must be filled
	 */
	private static Beam parseUVMeasurement(Node node, LofarObservation observation) {
		Beam beam = new Beam(observation);
		String mom2Id = getAttribute(node.getAttributes(), XMLConstants.MOM2_ID);
		beam.setMom2Id(AstronConverter.toInteger(mom2Id));
		for (int i = 0; i < node.getChildNodes().getLength(); i++) {
			Node nodeChild = node.getChildNodes().item(i);
			/*
			 * if child is an element
			 */
			if (isElement(nodeChild)) {

				if (equalIgnorePrefix(nodeChild, XMLConstants.MEASUREMENT_ATTRIBUTES)) {
					parseMeasurementAttributes((Element) nodeChild, beam);
				}
			}
		}
		return beam;

	}

	/**
	 * Parse the measurementAttributes xml element.
	 * 
	 * @param node
	 *            xml node that must be parsed
	 * @param lofarObservation
	 *            LofarObservation that must be filled
	 */
	private static void parseMeasurementAttributes(Element element, Beam beam) {
		NodeList specificationList = element.getElementsByTagName(XMLConstants.SPECIFICATION);
		if (specificationList.getLength() > 0) {
			Element specification = (Element) specificationList.item(0);
			Map<String, Element> elements = getElementMap(specification.getChildNodes());
			if (elements.containsKey(XMLConstants.RA)) {
				beam.getRaList().add(AstronConverter.toDouble(getValue(elements.get(XMLConstants.RA))));
			}
			if (elements.containsKey(XMLConstants.DEC)) {
				beam.getDecList().add(AstronConverter.toDouble(getValue(elements.get(XMLConstants.DEC))));
			}
			if (elements.containsKey(XMLConstants.EQUINOX)) {
				beam.setEquinox(getValue(elements.get(XMLConstants.EQUINOX)));
			}
			if (elements.containsKey(XMLConstants.DURATION)) {
				beam.getDurations().add(AstronConverter.convertXMLDurationToSeconds(getValue(elements
						.get(XMLConstants.DURATION))));
			}
			if (elements.containsKey(XMLConstants.SUBBANDS_SPECIFICATION)) {
				Element subbandsSpecificationsElement = elements.get(XMLConstants.SUBBANDS_SPECIFICATION);
				Map<String, Element> subbandsSpecificationsElements = getElementMap(subbandsSpecificationsElement
						.getChildNodes());
				if (subbandsSpecificationsElements.containsKey(XMLConstants.SUBBANDS)) {
					String[] subbands = getValue(subbandsSpecificationsElements.get(XMLConstants.SUBBANDS)).split(",");
					for(String subband: subbands){
						beam.getSubbands().add(AstronConverter.toInteger(subband));
						beam.getBeamlets().add(beam.getParentObservation().getBeamletNumber());
						beam.getParentObservation().setBeamletNumber(beam.getParentObservation().getBeamletNumber()+1);
					}

				}
			}

		}

	}

	/**
	 * Retrieve an attribute value from a attribute map
	 * 
	 * @param map
	 *            attribute map
	 * @param name
	 *            name of the attribute
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
	 * Compares if a node has the given name, ignoring the prefix of the node
	 * 
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
	 * 
	 * @param node
	 * @return node name withoud prefix
	 */
	private static String removePrefix(Node node) {
		return removePrefix(node.getNodeName());
	}

	private static String removePrefix(String nodeName) {
		String[] nodeSplit = nodeName.split(":");
		String withoutPrefix = nodeSplit[nodeSplit.length - 1];
		return withoutPrefix;
	}

	/**
	 * Retrieve status from status element
	 * 
	 * @param status
	 *            status element string
	 * @return status
	 */
	private static String formatStatus(String status) {
		return status.replaceAll(XMLConstants.STATUS_SUFFIX, "");
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
