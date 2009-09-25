package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Date;

import javax.xml.parsers.ParserConfigurationException;

import nl.astron.lofar.odtb.mom2otdbadapter.config.Mom2Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Beam;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.util.Mom2OtdbConverter;
import nl.astron.lofar.odtb.mom2otdbadapter.util.XMLConstants;
import nl.astron.util.AstronConverter;
import nl.astron.util.XMLBuilder;
import nl.astron.util.XMLConverter;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

/**
 * Generates MoM2-xml
 * 
 * @author Bastiaan Verhoef
 * 
 */
public class XMLGenerator {

	private static final String STATION_NAME = "name";

	private static final String MOM2_LOFAR_PREFIX = "lofar";

	private static final String MOM2_PREFIX = "mom2";

	//private static final String OTDB_DATETIME_FORMAT = "yyyy-MMM-dd HH:mm:ss";

	// private static final String FAILED = "failed";
	//
	// private static final String FINISHED = "finished";

	/**
	 * Generates XML Document from lofarObservation
	 * 
	 * @param observation
	 *            LofarObservation
	 * @return MoM2 XML Document
	 * @throws ParserConfigurationException
	 */
	private static Document getObservationDocument(LofarObservation observation, Mom2Configuration config)
			throws ParserConfigurationException {
		XMLBuilder xmlBuilder = new XMLBuilder();
		xmlBuilder.addNamespace(MOM2_LOFAR_PREFIX, XMLConstants.MOM2_LOFAR_NAMESPACE, config.getMom2SchemasUrl()
				+ "LofarMoM2.xsd");
		xmlBuilder.addNamespace(MOM2_PREFIX, XMLConstants.MOM2_NAMESPACE, config.getMom2SchemasUrl() + "MoM2.xsd");
		Element observationElement = xmlBuilder.addRootElement(XMLConstants.MOM2_LOFAR_NAMESPACE,
				XMLConstants.OBSERVATION);
		addObservation(xmlBuilder, observationElement, observation);
		return xmlBuilder.getDocument();

	}

	/**
	 * Generates XML String from LofarObservaiton
	 * 
	 * @param observation
	 * @return MoM2 XML String
	 * @throws IOException
	 * @throws ParserConfigurationException
	 */
	public static String getObservationXml(LofarObservation observation, Mom2Configuration config) throws IOException,
			ParserConfigurationException {
		Document document = getObservationDocument(observation, config);
		StringWriter writer = new StringWriter();
		XMLConverter.serialize(document, writer);
		return writer.toString();

	}

	private static void addObservation(XMLBuilder xmlBuilder, Element observationElement, LofarObservation observation) {
		if (observation.getMom2Id() != null) {
			xmlBuilder.addAttributeToElement(observationElement, XMLConstants.MOM2_ID, observation.getMom2Id()
					.toString());
		}
		Element currentStatusElement = xmlBuilder.addElement(observationElement, XMLConstants.CURRENT_STATUS);
		addXmlStatusElement(xmlBuilder, currentStatusElement, observation.getStatus());
		addChildren(xmlBuilder, observationElement, observation);

		
		Element observationAttributes = xmlBuilder.addElement(observationElement, XMLConstants.OBSERVATION_ATTRIBUTES);
		xmlBuilder.addTextElement(observationAttributes, XMLConstants.OBSERVATION_ID, observation.getObservationId());
		xmlBuilder.addTextElement(observationAttributes, XMLConstants.ANTENNA, Mom2OtdbConverter
				.getMom2Antenna(observation.getAntennaSet()));
		Element clockElement = xmlBuilder.addElement(observationAttributes, XMLConstants.CLOCK);
		
		xmlBuilder.addTextElement(clockElement, "channelWidth" , observation.getChannelWidth());
		xmlBuilder.addTextElement(clockElement, "samplesPerSecond" , observation.getSamplesPerSecond());
		xmlBuilder.addTextElement(clockElement, "subbandWidth" , observation.getSubbandWidth());
		xmlBuilder.addTextElement(clockElement, "systemClock" , observation.getClockFrequency());
		xmlBuilder.addTextElement(observationAttributes, XMLConstants.INSTRUMENT_FILTER, Mom2OtdbConverter
				.getMom2InstrumentFilter(observation.getBandFilter()));
		Date startTime = observation.getStartTime();
		xmlBuilder.addTextDateTimeElement(observationAttributes, XMLConstants.START_TIME, startTime);
		Date endTime = observation.getEndTime();
		xmlBuilder.addTextDateTimeElement(observationAttributes, XMLConstants.END_TIME, endTime);

		if (observation.getStations() != null){
			Element stationsElement = xmlBuilder.addElement(observationAttributes, XMLConstants.STATIONS);
			for (String station: Mom2OtdbConverter.getStringArray(observation.getStations())){
				Element stationElement = xmlBuilder.addElement(stationsElement, XMLConstants.STATION);
				xmlBuilder.addTextElement(stationElement, STATION_NAME , station);
			}
		}



	}

	private static void addChildren(XMLBuilder xmlBuilder, Element observationElement, LofarObservation observation) {
		Element childrenElement = xmlBuilder.addElement(observationElement, XMLConstants.CHILDREN);
		for (int index = 0; index < observation.getBeams().size();index++){
			addMeasurement(xmlBuilder, childrenElement, observation.getBeams().get(index),  index);
		}


	}

	private static void addMeasurement(XMLBuilder xmlBuilder, Element childrenElement, Beam beam, int index) {
		Element measurementElement = xmlBuilder.addIndexedElement(childrenElement, XMLConstants.MOM2_LOFAR_NAMESPACE,
				XMLConstants.MEASUREMENT, index);
		xmlBuilder.addTypeAttributeToElement(measurementElement, XMLConstants.MOM2_LOFAR_NAMESPACE, XMLConstants.UVMEASUREMENT_TYPE);
		xmlBuilder.addAttributeToElement(measurementElement, XMLConstants.MOM2_ID, beam.getMom2Id().toString());
		String[] subbands = Mom2OtdbConverter.getStringArray(beam.getSubbands());
		String fileMask  = beam.getParentObservation().getFileNameMask();
		fileMask = fileMask.substring(fileMask.lastIndexOf("/")+1);
		fileMask = fileMask.replaceAll("\\$\\{OBSERVATION\\}", beam.getParentObservation().getObservationId().toString());
		fileMask = fileMask.replaceAll("\\$\\{BEAM\\}", index + "");
		if (subbands.length > 0){
			Element resultDataProducts = xmlBuilder.addElement(measurementElement, "resultDataProducts");
			for (int i = 0; i < subbands.length;i++){
			Element uvDataProduct = xmlBuilder.addIndexedElement(resultDataProducts, "uvDataProduct");

			xmlBuilder.addTextElement(uvDataProduct, "name", fileMask.replaceAll("\\$\\{SUBBAND\\}", i + ""));
			xmlBuilder.addTextElement(uvDataProduct, "type", Mom2OtdbConverter.getMom2DPFileType(fileMask));
			xmlBuilder.addTextElement(uvDataProduct, "subband", i);
			xmlBuilder.addTextElement(uvDataProduct, "stationSubband", subbands[i]);
			xmlBuilder.addDurationElement(uvDataProduct, "duration", beam.getDuration());
			//xmlBuilder.addTextElement(uvDataProduct, "centralFrequency", beam.getDuration());
			//xmlBuilder.addTextElement(uvDataProduct, "channelWidth", beam.getParentObservation().getChannelWidth());
			//xmlBuilder.addTextElement(uvDataProduct, "numberOfChannels", beam.getParentObservation().getChannelsPerSubband());
			}

			
		}
	
		Element measurementAttributes = xmlBuilder.addElement(measurementElement, XMLConstants.MEASUREMENT_ATTRIBUTES);
		xmlBuilder.addTextElement(measurementAttributes, XMLConstants.RA, beam.getRa());
		xmlBuilder.addTextElement(measurementAttributes, XMLConstants.DEC, beam.getDec());
		xmlBuilder.addTextElement(measurementAttributes, XMLConstants.EQUINOX, beam.getEquinox());
		Integer duration = AstronConverter.toInteger(beam.getDuration());
		xmlBuilder.addDurationElement(measurementAttributes, XMLConstants.DURATION, duration );
		xmlBuilder.addTextDateTimeElement(measurementAttributes, XMLConstants.START_TIME, beam.getParentObservation().getStartTime());
		Date endTime = beam.getParentObservation().getStartTime();
		endTime.setSeconds(endTime.getSeconds() + duration);
		xmlBuilder.addTextDateTimeElement(measurementAttributes, XMLConstants.END_TIME, endTime);
		xmlBuilder.addTextElement(measurementAttributes, XMLConstants.SUBBANDS, Mom2OtdbConverter.getMom2Subbands(beam.getSubbands()));

	}

	private static void addXmlStatusElement(XMLBuilder xmlBuilder, Element parent, String status) {

		xmlBuilder.addElement(parent, XMLConstants.MOM2_NAMESPACE, getStatusStringFromCode(status));

	}



	private static String getStatusStringFromCode(String code) {
		code = Mom2OtdbConverter.getMom2Status(code);
		String[] splitted = code.split(" ");
		String result = splitted[0];
		if (splitted.length > 1) {
			for (int i = 1; i < splitted.length; i++) {
				String split = splitted[i];
				String firstChar = split.substring(0, 1).toUpperCase();
				String otherChars = split.substring(1);
				result += firstChar + otherChars;
			}
		}
		return result + XMLConstants.STATUS_SUFFIX;
	}
}
