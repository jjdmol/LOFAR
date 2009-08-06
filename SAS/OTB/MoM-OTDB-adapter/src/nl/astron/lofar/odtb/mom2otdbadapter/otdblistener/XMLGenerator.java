package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Date;

import javax.xml.parsers.ParserConfigurationException;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.util.AstronConverter;
import nl.astron.util.XMLBuilder;

import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

/**
 * Generates MoM2-xml
 * @author Bastiaan Verhoef
 *
 */
public class XMLGenerator {
	protected static final String MOM2_LOFAR_NAMESPACE = "http://www.astron.nl/MoM2-Lofar";

	protected static final String MOM2_NAMESPACE = "http://www.astron.nl/MoM2";

	protected static final String OTDB_DATETIME_FORMAT = "yyyy-MMM-dd HH:mm:ss";

	protected static final String FAILED = "failed";

	protected static final String FINISHED = "finished";

	private XMLBuilder xmlBuilder = null;

	/**
	 * Generates XML Document from lofarObservation
	 * @param observation LofarObservation
	 * @return MoM2 XML Document
	 * @throws ParserConfigurationException
	 */
	protected Document getObservationDocument(LofarObservation observation)
			throws ParserConfigurationException {
		xmlBuilder = new XMLBuilder();
		xmlBuilder.addNamespace("lofar", MOM2_LOFAR_NAMESPACE,
				"http://wop17.nfra.nl/twiki/pub/Tools/Schemas/LofarMoM2.xsd");
		xmlBuilder.addNamespace("mom2", MOM2_NAMESPACE,
				"http://wop17.nfra.nl/twiki/pub/Tools/Schemas/MoM2.xsd");
		Element observationElement = xmlBuilder.addRootElement(
				MOM2_LOFAR_NAMESPACE, "observation");
		addObservation(observationElement, observation);
		return xmlBuilder.getDocument();

	}

	/**
	 * Generates XML String from LofarObservaiton
	 * @param observation
	 * @return MoM2 XML String
	 * @throws IOException
	 * @throws ParserConfigurationException
	 */
	public String getObservationXml(LofarObservation observation)
			throws IOException, ParserConfigurationException {
		Document document = getObservationDocument(observation);
		OutputFormat format = new OutputFormat(document); // Serialize DOM
		StringWriter stringOut = new StringWriter(); // Writer will be a
		// String
		XMLSerializer serial = new XMLSerializer(stringOut, format);
		serial.asDOMSerializer(); // As a DOM Serializer

		serial.serialize(document.getDocumentElement());

		return stringOut.toString(); // Spit out DOM as a String

	}

	protected void addObservation(Element observationElement,
			LofarObservation observation) {
		if (observation.getMom2Id() > 0) {
			xmlBuilder.addAttributeToElement(observationElement, "mom2Id",
					observation.getMom2Id() + "");
		}
		Element currentStatusElement = xmlBuilder.addElement(
				observationElement, "currentStatus");
		addXmlStatusElement(currentStatusElement, observation.getStatus());
		xmlBuilder.addElement(observationElement, MOM2_LOFAR_NAMESPACE,
				"observationAttributes");
		addChildren(observationElement, observation);

	}

	protected void addChildren(Element parent, LofarObservation observation) {
		Element childrenElement = xmlBuilder.addElement(parent, "children");
		String[] ids = getArray(observation.getMeasurementMom2Ids());
		String[] angleTimes = getArray(observation.getAngleTimes());
		String status = observation.getStatus();
		Date startTime = AstronConverter.toDate(observation.getStartTime(),
				OTDB_DATETIME_FORMAT);
		Date endTime = AstronConverter.toDate(observation.getEndTime(),
				OTDB_DATETIME_FORMAT);

		for (int i = 0; i < ids.length; i++) {
			String mom2Id = ids[i];
			if (angleTimes == null) {
				addMeasurement(childrenElement, mom2Id, status, null, null);
			} else {
				long startTimeLong = startTime.getTime();
				long startOffset = new Long(angleTimes[i].substring(1))
						.longValue();
				if (i > 0) {
					/*
					 * add 1 second, because a measurement can not be start and
					 * end on the same second
					 */
					startOffset = (startOffset + 1) * 1000;
				}
				Date startDate = new Date();
				startDate.setTime(startTimeLong + startOffset);
				Date endDate = new Date();
				if (i < ids.length - 1) {
					long endOffset = new Long(angleTimes[i + 1].substring(1))
							.longValue() * 1000;

					endDate.setTime(startTimeLong + endOffset);
				} else {
					endDate = endTime;
				}
				/*
				 * start and endtime is before the calculated times, according
				 * angleTimes. Measurements must be failed
				 */
				if (endTime.before(endDate)) {
					addMeasurement(childrenElement, mom2Id, FAILED, startDate,
							endTime);
				} else if (endTime.before(startDate)) {
					addMeasurement(childrenElement, mom2Id, FAILED, null, null);
				} else {
					addMeasurement(childrenElement, mom2Id, FINISHED,
							startDate, endDate);
				}
			}
		}

	}

	protected void addMeasurement(Element childrenElement, String mom2Id,
			String status, Date startTime, Date endTime) {
		Element measurementElement = xmlBuilder.addIndexedElement(
				childrenElement, MOM2_LOFAR_NAMESPACE, "measurement");
		xmlBuilder.addAttributeToElement(measurementElement, "mom2Id", mom2Id);

		Element currentStatusElement = xmlBuilder.addElement(
				measurementElement, "currentStatus");
		addXmlStatusElement(currentStatusElement, status);
		Element measurementAttributes = xmlBuilder.addElement(
				measurementElement, MOM2_LOFAR_NAMESPACE,
				"measurementAttributes");
		if (startTime != null && endTime != null) {
			if (startTime != null) {
				xmlBuilder.addTextElement(measurementAttributes, "startTime",
						AstronConverter.toXmlDateTimeString(startTime));
			}
			if (endTime != null) {
				xmlBuilder.addTextElement(measurementAttributes, "endTime",
						AstronConverter.toXmlDateTimeString(endTime));
			}
		}
	}

	protected void addXmlStatusElement(Element parent, String status) {

		xmlBuilder.addElement(parent, MOM2_NAMESPACE,
				getStatusStringFromCode(status));

	}

	protected String getStatusStringFromCode(String code) {
		code = convertToMomStatus(code);
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
		return result + "Status";
	}

	protected String convertToMomStatus(String code) {
		if (code.equals("specified")) {
			return "prepared";
		}
		if (code.equals("active")) {
			return "running";
		}
		if (code.equals("finished")) {
			return "finished";
		}
		if (code.equals("aborted")) {
			return "aborted";
		}
		if (code.equals("failed")) {
			return "failed";
		}
		return null;
	}

	protected String[] getArray(String string) {
		if (string != null) {
			String temp = string;
			/*
			 * remove '[' ']'
			 */
			temp = temp.replaceAll("\\[", "");
			temp = temp.replaceAll("\\]", "");
			return temp.split(",");
		} else {
			return null;
		}
	}
}
