package nl.astron.lofar.odtb.mom2otdbadapter.config;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import nl.astron.util.AstronConverter;
import nl.astron.util.AstronValidator;
import nl.astron.util.XMLConverter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

/**
 * parse the XML input from MoM and returns a LofarObservations. If needed it
 * used the Mom2OtdbConverter to convert values to otdb values
 * 
 * @author Bastiaan Verhoef
 * 
 */
public class ConfigXMLParser {

	private static final String ADAPTER = "adapter";
	private static final String OTDB = "otdb";
	private static final String STUB = "stub";
	private static final String MOM2 = "mom2";
	private static Log log = LogFactory.getLog(ConfigXMLParser.class);

	/**
	 * Parse a xml document and returns a lofar obseravation
	 * 
	 * @param document
	 *            xml document
	 * @return LofarObservation
	 * @throws Exception
	 * @throws URISyntaxException
	 * @throws FileNotFoundException
	 */
	public static Configuration parse(File xml) throws Exception {
		log.info("Mom-otdb-adapter config file: " + xml.getPath());
		URL url = ConfigXMLParser.class.getClassLoader().getResource("config.xsd");
		Document document = XMLConverter.convertXMLToDocument(new InputSource(new FileInputStream(xml)), url);
		Configuration configuration = new Configuration();

		/*
		 * search for project
		 */
		Node element = document.getDocumentElement();
		Map<String, Element> elementsMap = getElementMap(element.getChildNodes());

		configuration.setMom2(parseMom2Configuration(elementsMap.get(MOM2)));

		if (elementsMap.containsKey(STUB)) {
			configuration.setRepository(parseStubConfiguration(elementsMap.get(STUB)));
		} else if (elementsMap.containsKey(OTDB)) {
			configuration.setRepository(parseOTDBConfiguration(elementsMap.get(OTDB)));
		}
		configuration.setAdapter(parseAdapterConfiguration(elementsMap.get(ADAPTER)));
		return configuration;
	}

	private static Mom2Configuration parseMom2Configuration(Element mom2Element) {
		Map<String, Element> elements = getElementMap(mom2Element.getChildNodes());
		Mom2Configuration mom2Configuration = new Mom2Configuration();
		mom2Configuration.setUsername(getValue(elements.get("username")));
		mom2Configuration.setPassword(getValue(elements.get("password")));
		mom2Configuration.setAuthUrl(getValue(elements.get("authurl")));
		mom2Configuration.setMom2ImportUrl(getValue(elements.get("mom2-import-url")));
		mom2Configuration.setMom2SchemasUrl(getValue(elements.get("mom2-schemas-url")));
		return mom2Configuration;
	}

	private static StubConfiguration parseStubConfiguration(Element element) {
		Map<String, Element> elements = getElementMap(element.getChildNodes());
		StubConfiguration configuration = new StubConfiguration();
		configuration.setInterval(AstronConverter.toInteger(getValue(elements.get("interval"))));
		return configuration;
	}

	private static OTDBConfiguration parseOTDBConfiguration(Element element) {
		Map<String, Element> elements = getElementMap(element.getChildNodes());
		OTDBConfiguration configuration = new OTDBConfiguration();
		configuration.setInterval(AstronConverter.toInteger(getValue(elements.get("interval"))));
		configuration.setTemplateId(AstronConverter.toInteger(getValue(elements.get("uvObservationTemplateId"))));
		configuration.setRmiPort(AstronConverter.toInteger(getValue(elements.get("rmiport"))));
		configuration.setRmiHost(getValue(elements.get("rmihost")));
		return configuration;
	}

	private static AdapterConfiguration parseAdapterConfiguration(Element element) {
		Map<String, Element> elements = getElementMap(element.getChildNodes());
		AdapterConfiguration configuration = new AdapterConfiguration();
		configuration.setHttpPort(AstronConverter.toInteger(getValue(elements.get("httpport"))));
		configuration.setKeystoreLocation(getValue(elements.get("keystore-location")));
		configuration.setKeystorePassword(getValue(elements.get("keystore-password")));
		configuration.setTrustedKeystoreLocation(getValue(elements.get("trusted-keystore-location")));
		configuration.setTrustedKeystorePassword(getValue(elements.get("trusted-keystore-password")));
		return configuration;

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
		}
		return value;
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
			return true;
		} else {
			return false;
		}
	}

}
