package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import javax.xml.parsers.ParserConfigurationException;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.wsrt.util.XMLBuilder;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class XMLGenerator {
	public static final String MOM2_LOFAR_NAMESPACE = "http://www.astron.nl/MoM2-Lofar";
	
	private XMLBuilder xmlBuilder = null;
	
	public Document getObservationXml(LofarObservation observation) throws ParserConfigurationException{
		xmlBuilder = new XMLBuilder();
		xmlBuilder.addNamespace("lofar", MOM2_LOFAR_NAMESPACE,
		"http://proposal.astron.nl/schemas/LofarProposal.xsd");
		Element element = xmlBuilder.addRootElement(MOM2_LOFAR_NAMESPACE,
				"observation");
		xmlBuilder.addAttributeToElement(element,"mom2Id",observation.getMom2Id().toString());
		return xmlBuilder.getDocument();
		

	}
}
