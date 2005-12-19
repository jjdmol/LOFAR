package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Date;

import javax.xml.parsers.ParserConfigurationException;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.wsrt.util.WsrtConverter;
import nl.astron.wsrt.util.XMLBuilder;

import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class XMLGenerator {
	protected static final String MOM2_LOFAR_NAMESPACE = "http://www.astron.nl/MoM2-Lofar";

	protected static final String MOM2_NAMESPACE = "http://www.astron.nl/MoM2";

	protected static final String OTDB_DATETIME_FORMAT = "dd-MM-yyyy HH:mm";

	private XMLBuilder xmlBuilder = null;

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
	public String getObservationXml(LofarObservation observation) throws IOException, ParserConfigurationException{
		Document document = getObservationDocument(observation);
        OutputFormat    format  = new OutputFormat( document );   //Serialize DOM
        StringWriter  stringOut = new StringWriter();        //Writer will be a String
        XMLSerializer    serial = new XMLSerializer( stringOut, format );
        serial.asDOMSerializer();                            // As a DOM Serializer

        serial.serialize( document.getDocumentElement() );

        return stringOut.toString(); //Spit out DOM as a String
	
	}

	protected void addObservation(Element observationElement,
			LofarObservation observation) {

		xmlBuilder.addAttributeToElement(observationElement, "mom2Id",
				observation.getMom2Id());
		Element currentStatusElement = xmlBuilder.addElement(
				observationElement, "currentStatus");
		addXmlStatusElement(currentStatusElement, observation.getStatus());
		addChildren(observationElement, observation);

	}

	protected void addChildren(Element parent, LofarObservation observation) {
		Element childrenElement = xmlBuilder.addElement(parent, "children");
		String[] ids = getArray(observation.getMeasurementMom2Ids());
		String[] angleTimes = getArray(observation.getAngleTimes());
		Date startTime = WsrtConverter.toDate(observation.getStartTime(),
				OTDB_DATETIME_FORMAT);
		Date endTime = WsrtConverter.toDate(observation.getEndTime(),
				OTDB_DATETIME_FORMAT);
		long startTimeLong = startTime.getTime();
		for (int i = 0; i < ids.length; i++){
			String mom2Id = ids[i];
			String status = observation.getStatus();
			long startOffset = new Long(angleTimes[i].substring(1)).longValue() * 1000;
			Date startDate  = new Date();
			startDate.setTime(startTimeLong + startOffset);
			Date endDate = new Date();
			if (i < ids.length-1){
				long endOffset = new Long(angleTimes[i+1].substring(1)).longValue();
				/*
				 * add 1 second, because a measurement can not be start and end on the same second
				 */
				endOffset =  (endOffset+1)* 1000;
				endDate.setTime(endOffset);
			}else {
				endDate = endTime;
			}
			addMeasurement(childrenElement,mom2Id,status,startDate,endDate);
		}
		

	}

	protected void addMeasurement(Element childrenElement, String mom2Id,
			String status, Date startTime, Date endTime) {
		Element observationElement = xmlBuilder.addIndexedElement(
				childrenElement, MOM2_LOFAR_NAMESPACE, "measurement");
		xmlBuilder.addAttributeToElement(observationElement, "mom2Id", mom2Id);

		Element currentStatusElement = xmlBuilder.addElement(
				observationElement, "currentStatus");
		addXmlStatusElement(currentStatusElement, status);
		xmlBuilder.addTextElement(observationElement, "startTime", startTime);
		xmlBuilder.addTextElement(observationElement, "endTime",endTime);
	}

	protected void addXmlStatusElement(Element parent, String status) {

		xmlBuilder.addElement(parent, MOM2_NAMESPACE,
				getStatusStringFromCode(status));

	}

	protected String getStatusStringFromCode(String code) {
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

	protected String[] getArray(String string) {
		String temp = string;
		/*
		 * remove '[' ']'
		 */
		temp = temp.replaceAll("\\[", "");
		temp = temp.replaceAll("\\]", "");
		return temp.split(",");
	}
}
